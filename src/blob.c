/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 *
 */

/**
* @cond ham_internals
*/


#include "internal_preparation.h"




#if defined(_MSC_VER)
#pragma message(__FILE__ "(" STRING(__LINE__) ") : TODO: partial blob overwrite/copy \n\
    is not yet complete: check that existing blob is both copied before and \n\
    after the new partial chunk, depending on previous blob size; current code \n\
    seems to have the before part, but definitely NOT the after part.")
#endif




#define SMALLEST_CHUNK_SIZE  (sizeof(ham_pers_rid_t)+sizeof(blob_t)+1)

/**
 * if the blob is small enough (or if logging is enabled) then go through
 * the cache. otherwise use direct I/O
 */
static ham_bool_t
__must_alloc_blob_through_cache(ham_env_t *env, ham_size_t size)
{
    if (env_get_log(env))
        return (size <= env_get_usable_pagesize(env));
    return (size <= (env_get_pagesize(env)>>3));
}

/**
 * write a series of data chunks to storage at file offset 'addr'.
 *
 * The chunks are assumed to be stored in sequential order, adjacent
 * to each other, i.e. as one long data strip.
 *
 * Writing is performed on a per-page basis, where special conditions
 * will decide whether or not the write operation is performed
 * through the page cache or directly to device; such is determined
 * on a per-page basis.
 */
static ham_status_t
__write_chunks(ham_env_t *env, ham_page_t *page, ham_offset_t addr,
        ham_bool_t allocated, ham_bool_t freshly_created,
        ham_u8_t **chunk_data, ham_size_t *chunk_size,
        ham_size_t chunks)
{
    ham_size_t i;
    ham_status_t st;
    ham_offset_t pageid;
    ham_device_t *device=env_get_device(env);
    const ham_size_t pagesize = env_get_pagesize(env);

    ham_assert(freshly_created ? allocated : 1, (0));

    /*
     * for each chunk...
     */
    for (i=0; i<chunks; i++) {
        while (chunk_size[i]) {
            /*
             * get the page-ID from this chunk
             */
            pageid = addr - (addr % pagesize);

            /*
             * is this the current page?
             */
            if (page && page_get_self(page)!=pageid)
                page=0;

            /*
             * fetch the page from the cache, if it's in the cache
             * (unless we're logging - in this case always go through
             * the buffered routines)
             */
            if (!page) {
                /*
                 * keep pages in cache when they are located at the 'edges' of
                 * the blob, as they MAY be accessed for different data.
                 * Of course, when a blob is small, there's only one (partial)
                 * page accessed anyhow, so that one should end up in cache
                 * then.
                 *
                 * When transaction logging is turned on, it's the same story,
                 * really. We _could_ keep all those pages in cache now,
                 * but this would be thrashing the cache with blob data that's
                 * accessed once only and for transaction abort (or commit)
                 * the amount of effort does not change.
                 *
                 * THOUGHT:
                 *
                 * Do we actually care what was in that page, which is going
                 * to be overwritten in its entirety, BEFORE we do this, i.e.
                 * before the transaction?
                 *
                 * Answer: NO (and YES in special circumstances).
                 *
                 * Elaboration: As this would have been free space before, the
                 * actual content does not matter, so it's not required to add
                 * the FULL pages written by the blob write action here to the
                 * transaction log: even on transaction abort, that lingering
                 * data is marked as 'bogus'/free as it was before anyhow.
                 *
                 * And then, assuming a longer running transaction, where this
                 * page was freed during a previous action WITHIN
                 * the transaction, well, than the transaction log should
                 * already carry this page's previous content as instructed
                 * by the erase operation. HOWEVER, the erase operation would
                 * not have a particular NEED to edit this page, as an erase op
                 * is complete by just marking this space as free in the
                 * freelist, resulting in the freelist pages (and the btree
                 * pages) being the only ones being edited and ending up in
                 * the transaction log then.
                 *
                 * Which means we'll have to log the previous content of these
                 * pages to the transaction log anyhow. UNLESS, that is, when
                 * WE allocated these pages in the first place: then there
                 * cannot be any 'pre-transaction' state of these pages
                 * except that of 'not existing', i.e. 'free'. In which case,
                 * their actual content doesn't matter! (freshly_created)
                 *
                 * And what if we have recovery logging turned on, but it's
                 * not about an active transaction here?
                 * In that case, the recovery log would only log the OLD page
                 * content, which we've concluded is insignificant, ever. Of
                 * course, that's assuming (again!) that we're writing to
                 * freshly created pages, which no-one has seen before.
                 *
                 * Just as long as we can prevent this section from thrashing
                 * the page cache, thank you very much...
                 */
                ham_bool_t at_blob_edge =
                        (__must_alloc_blob_through_cache(env, chunk_size[i])
                        || (addr % pagesize) != 0
                        || chunk_size[i] < pagesize);
                ham_bool_t cacheonly = (!at_blob_edge
                                    && (!env_get_log(env)
                                        || freshly_created));
                //ham_assert(db_get_txn(db) ? !!env_get_log(env) : 1, (0));

                st=db_fetch_page(&page, env, pageid,
                        cacheonly ? DB_ONLY_FROM_CACHE :
                        at_blob_edge ? 0 : DB_NEW_PAGE_DOES_THRASH_CACHE);
                ham_assert(st ? page == NULL : 1, (0));
                /* blob pages don't have a page header */
                if (page)
                {
                    ham_assert(!st, (0));
                    page_add_npers_flags(page, PAGE_NPERS_NO_HEADER);
                    /* if this page was recently allocated by the parent
                     * function: set a flag */
                    if (cacheonly
                            && allocated
                            && addr==page_get_self(page)
                            && env_get_txn(env))
                    {
                        page_set_alloc_txn_id(page, txn_get_id(env_get_txn(env)));
                    }
                }
                else if (st) {
                    return st;
                }
            }

            /*
             * if we have a page pointer: use it; otherwise write directly
             * to the device
             */
            if (page) {
                ham_size_t writestart=
                        (ham_size_t)(addr-page_get_self(page));
                ham_size_t writesize =
                        (ham_size_t)(pagesize - writestart);
                if (writesize>chunk_size[i])
                    writesize=chunk_size[i];
                st=ham_log_add_page_before(page);
                if (st)
                    return (st);
                memcpy(&page_get_raw_payload(page)[writestart], chunk_data[i],
                            writesize);
                page_set_dirty(page, env);
                addr+=writesize;
                chunk_data[i]+=writesize;
                chunk_size[i]-=writesize;
            }
            else {
                ham_size_t s = chunk_size[i];
                /* limit to the next page boundary */
                if (s > pageid+pagesize-addr)
                    s = (ham_size_t)(pageid+pagesize-addr);

                ham_assert(env_get_log(env) ? freshly_created : 1, (0));

                st=device->write(device, addr, chunk_data[i], s);
                if (st)
                    return st;
                addr+=s;
                chunk_data[i]+=s;
                chunk_size[i]-=s;
            }
        }
    }

    return HAM_SUCCESS;
}

static ham_status_t
__read_chunk(ham_env_t *env, ham_page_t *page, ham_page_t **fpage,
        ham_offset_t addr, ham_u8_t *data, ham_size_t size)
{
    ham_status_t st;
    ham_device_t *device=env_get_device(env);
    const ham_u32_t flags = (__must_alloc_blob_through_cache(env, size) ? 0 : DB_ONLY_FROM_CACHE);

    while (size)
    {
        /*
         * get the page-ID from this chunk
         */
        ham_offset_t pageid;
        pageid = addr - (addr % env_get_pagesize(env));

        if (page) {
            if (page_get_self(page)!=pageid)
                page=0;
        }

        /*
         * is it the current page? if not, try to fetch the page from
         * the cache - but only read the page from disk, if the
         * chunk is small
         */
        if (!page)
        {
            /*
            the
                __must_alloc_blob_through_cache(env, size) ? 0 : DB_ONLY_FROM_CACHE
            code allowed cache thrashing as for each blob, even huge blobs, the initial
            and last page would be fetched through the page cache as the blob header
            and the trailing piece ('size' is decreasing while in this loop!) are
            within the __must_alloc_blob_through_cache() limits with average probability
            for the tail and certainly for the blob header.

            Hence the correct way to tackle this is have the caller take care of the
            proper flags. This also benefits the HAM_PARTIAL code which may be
            copying old record to new blob space when a record is overwritten while
            simultaneously being resized (size increase).

            We cannot prevent loading the blob headers through the cache as we won't know,
            up front, whether a blob will be a regular record or a huge blob, but we /can/
            prevent loading the tail page through cache by having the 'flags' precalculated
            instead of recalculated during every round in the loop.
            */
            st=db_fetch_page(&page, env, pageid, flags);
            ham_assert(st ? page == NULL : 1, (0));
            /* blob pages don't have a page header */
            if (page)
            {
                ham_assert(!st, (0));
                page_add_npers_flags(page, PAGE_NPERS_NO_HEADER);
            }
            else if (st)
            {
                return st;
            }
        }

        /*
         * if we have a page pointer: use it; otherwise read directly
         * from the device
         */
        if (page) {
            ham_size_t readstart=
                    (ham_size_t)(addr-page_get_self(page));
            ham_size_t readsize = (env_get_pagesize(env)-readstart);
            if (readsize>size)
                readsize=size;
            memcpy(data, &page_get_raw_payload(page)[readstart], readsize);
            addr+=readsize;
            data+=readsize;
            size-=readsize;
        }
        else {
            ham_size_t s=(size<env_get_pagesize(env)
                    ? size : env_get_pagesize(env));
            /* limit to the next page boundary */
            if (s>pageid+env_get_pagesize(env)-addr)
                s=(ham_size_t)(pageid+env_get_pagesize(env)-addr);

            st=device->read(device, addr, data, s);
            if (st)
                return st;
            addr+=s;
            data+=s;
            size-=s;
        }
    }

    if (fpage)
        *fpage=page;

    return HAM_SUCCESS;
}

static ham_status_t
__get_duplicate_table(dupe_table_t **table_ref, ham_page_t **page, ham_env_t *env, ham_offset_t table_id)
{
    ham_status_t st;
    blob_t hdr;
    ham_page_t *hdrpage=0;
    dupe_table_t *table;
    ham_size_t table_size;

    *page = 0;

    if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
    {
        blob_t *ptr = (blob_t *)U64_TO_PTR(table_id);
        ham_assert(ptr, (0));
        *table_ref = (dupe_table_t *)blob_get_data(ptr);
        return HAM_SUCCESS;
    }

    *table_ref = 0;

    /*
     * load the blob header
     */
    st=__read_chunk(env, 0, &hdrpage, table_id, (ham_u8_t *)&hdr, sizeof(hdr));
    if (st)
        return st;

    ham_assert(blob_get_size(&hdr) < HAM_MAX_U32, (0));
    table_size = (ham_size_t)blob_get_size(&hdr);

    /*
     * if the whole table is in a page (and not split between several
     * pages), just return a pointer directly in the page.
     */
    if (page_get_self(hdrpage) + env_get_usable_pagesize(env) >= table_id + table_size)
    {
    /*
     WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

     This code assumes the page data is loaded all at once when only the HEADER of the dup table
     is loaded: this works for mmapped I/O but not for regular read/write I/O!

     Check the above statement; either __chunk_read must be fixed or its invocation adjusted
     so that the need to load the 'entire table space' should be clearly visible in this
     routine here!

     (EDIT: this code assumes (correctly) that __read_chunk() will revert to a memory-mapped
            db_fetch_page() for these smallish table sizes. Nevertheless, from the point of code
            readability, still not a beaut' ;-) )

TODO: test this case by creating a database and writing and verifying several thousands of duplicate keys,
      all with the same key value, so that the dupe table will overflow into multiple pages (>= 3 pages)

     */
        ham_u8_t *p = page_get_raw_payload(hdrpage);
        /* yes, table is in the page */
        *page = hdrpage;
        *table_ref = (dupe_table_t *)
                &p[table_id - page_get_self(hdrpage) + sizeof(hdr)];
        ham_assert(dupe_table_get_capacity(*table_ref) * sizeof((*table_ref)->_entries[0]) + OFFSETOF(dupe_table_t, _entries) <= table_size, (0));
        return HAM_SUCCESS;
    }

    /*
     * otherwise allocate memory for the table
     */
    table = (dupe_table_t *)allocator_alloc(env_get_allocator(env), table_size);
    if (!table)
    {
        return HAM_OUT_OF_MEMORY;
    }

    /*
     * then read the rest of the blob
     */
    st=__read_chunk(env, hdrpage, 0, table_id+sizeof(hdr),
            (ham_u8_t *)table, table_size);
    if (st)
        return st;

    *table_ref = table;
    ham_assert(dupe_table_get_capacity(*table_ref) * sizeof((*table_ref)->_entries[0]) + OFFSETOF(dupe_table_t, _entries) <= table_size, (0));

    return HAM_SUCCESS;
}

/**
 * Allocate space in storage for and write the content references by 'record'
 * to storage.
 *
 * Conditions will apply whether the data is written through cache or direct
 * to device.
 *
 * The content is, of course, prefixed by a BLOB header.
 *
 * Partial writes are handled in this function.
 */
ham_status_t
blob_allocate(ham_env_t *env, const ham_record_t *record,
        ham_u32_t flags, dev_alloc_request_info_ex_t *extra_dev_alloc_info,
        blob_t *old_hdr, ham_offset_t *blobid)
{
    ham_status_t st;
    ham_page_t *page = 0;
    ham_offset_t addr;
    blob_t hdr;
    ham_u8_t *chunk_data[2];
    ham_size_t alloc_size;
    ham_size_t chunk_size[2];
    ham_device_t *device = env_get_device(env);
    ham_bool_t freshly_created = HAM_FALSE;
    const ham_size_t size = record->size;
    const ham_size_t page_size = env_get_pagesize(env);

    *blobid = 0;

    /*
     * PARTIAL WRITE
     *
     * if offset+partial_size equals the full record size, then we won't
     * have any gaps. In this case we just write the full record and ignore
     * the partial parameters.
     */
    if (flags & HAM_PARTIAL)
    {
        if (record->partial_offset == 0
                && record->partial_offset + record->partial_size == record->size)
        {
            flags &= ~HAM_PARTIAL;
        }
    }

    /*
     * in-memory-database: the blobid is actually a pointer to the memory
     * buffer, in which the blob (with the blob-header) is stored
     */
    if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
    {
        blob_t *hdr = (blob_t *)allocator_alloc(env_get_allocator(env), size+sizeof(blob_t));
        if (!hdr) {
            return HAM_OUT_OF_MEMORY;
        }

        /* initialize the header */
        memset(hdr, 0, sizeof(*hdr));
        blob_set_self(hdr, (ham_offset_t)PTR_TO_U64(hdr));
        blob_set_alloc_size(hdr, size+sizeof(blob_t));
        blob_set_size(hdr, size);

        /* do we have gaps? if yes, fill them with zeroes */
        if (flags & HAM_PARTIAL)
        {
            ham_u8_t *s = blob_get_data(hdr);
            ham_size_t offset = record->partial_offset;
            ham_size_t oldsize = (old_hdr ? (ham_size_t)blob_get_size(old_hdr) : 0);

            if (offset)
            {
                /*
                check whether we may copy/keep old record data:
                */
                if (oldsize)
                {
                    ham_u8_t *odp = blob_get_data(old_hdr);

                    if (oldsize >= offset)
                    {
                        memcpy(s, odp, offset);
                    }
                    else
                    {
                        memcpy(s, odp, oldsize);
                        memset(s + oldsize, 0, offset - oldsize);
                    }
                }
                else
                {
                    memset(s, 0, offset);
                }
            }
            memcpy(s + offset, record->data, record->partial_size);
            offset += record->partial_size;
            if (offset < size)
            {
                /*
                check whether we may copy/keep old record data:
                */
                if (oldsize > offset)
                {
                    ham_u8_t *odp = blob_get_data(old_hdr);

                    ham_assert(oldsize < size, ("expected usage of blob_allocate is when no old record exists or old record is SMALLER than new record size"));

                    if (oldsize >= size)
                    {
                        ham_assert(0, ("should never get here: expected usage"));
                        memcpy(s + offset, odp, size - offset);
                    }
                    else
                    {
                        memcpy(s + offset, odp, oldsize - offset);
                        memset(s + oldsize, 0, size - oldsize);
                    }
                }
                else
                {
                    memset(s + offset, 0, size - offset);
                }
            }
        }
        else
        {
            memcpy(blob_get_data(hdr), record->data, size);
        }


        *blobid = (ham_offset_t)PTR_TO_U64(hdr);
        return HAM_SUCCESS;
    }

    memset(&hdr, 0, sizeof(hdr));

    /*
     * blobs are CHUNKSIZE-allocated
     */
    alloc_size = sizeof(blob_t) + size;
    alloc_size += DB_CHUNKSIZE - 1;
    alloc_size -= alloc_size % DB_CHUNKSIZE;

    /*
     * check if we have space in the freelist
     */
    st = freel_alloc_area(&addr, alloc_size, extra_dev_alloc_info);
    if (!addr)
    {
        if (st)
            return st;

        /*
         * if the blob is small OR if logging is enabled (and the record fits a single page): load the page
         * through the cache
         */
        if (__must_alloc_blob_through_cache(env, alloc_size))
        {
            ham_assert(extra_dev_alloc_info, (0));
            ham_assert(extra_dev_alloc_info->space_type != PAGE_TYPE_UNKNOWN, (0)); // not necessarily a TYPE_BLOB though...
            ham_assert(alloc_size <= env_get_usable_pagesize(env), (0));
            st = db_alloc_page(&page, PAGE_IGNORE_FREELIST, extra_dev_alloc_info);
            ham_assert(st ? page == NULL : 1, (0));
            ham_assert(!st ? page != NULL : 1, (0));
            if (st)
                return st;
            /* blob pages don't have a page header */
            page_add_npers_flags(page, PAGE_NPERS_NO_HEADER);
            addr=page_get_self(page);
            /*
            move the remaining space to the freelist

            if pagesize != size, and the remaining chunk is large enough:
            move it to the freelist. This extra check is needed as the decision to
            go through the cache is not exclusively taken for 'tiny' blobs
            (i.e. blobs which are significantly smaller than a page); not checking
            implies we'd accept littering the freelist with tiny, costly to use, free
            slots at the end of pages. 'Costly to use' as such free chunks can only
            be used by blobs which will then span two pages at least, even if the blob
            itself would be tiny, as even the tiniest blob (SMALLEST_CHUNK_SIZE)
            would be larger than the freed slot at the end of the page.

            To reduce the number of page I/Os it is then better to keep such superfluous
            space with the allocating blob itself.
            */
            {
                ham_size_t diff = env_get_pagesize(env) - alloc_size;
                if (diff > SMALLEST_CHUNK_SIZE)
                {
                    st = freel_mark_free(env, addr+alloc_size, diff, HAM_FALSE);
                    if (st)
                        return st;
                    blob_set_alloc_size(&hdr, alloc_size);
                }
                else
                {
                    blob_set_alloc_size(&hdr, env_get_pagesize(env));
                }
            }
        }
        else
        {
            /*
             * otherwise use direct IO to allocate the space
             */
            ham_size_t aligned = alloc_size;
            aligned += env_get_pagesize(env) - 1;
            aligned -= aligned % env_get_pagesize(env);

            st=device->alloc(device, aligned, &addr, extra_dev_alloc_info);
            if (st)
                return (st);

            /* if aligned != size, and the remaining chunk is large enough:
             * move it to the freelist */
            {
                ham_size_t diff = aligned - alloc_size;
                if (diff > SMALLEST_CHUNK_SIZE)
                {
                    st = freel_mark_free(env, addr+alloc_size, diff, HAM_FALSE);
                    if (st)
                        return st;
                    blob_set_alloc_size(&hdr, alloc_size /* == aligned-diff */ );
                }
                else
                {
                    blob_set_alloc_size(&hdr, aligned);
                }
            }
            freshly_created = HAM_TRUE;
        }

        //ham_assert(HAM_SUCCESS == freel_check_area_is_allocated(env, addr, alloc_size), (0)); -- this call can modify the freelist!
    }
    else
    {
        ham_assert(!st, (0));
        blob_set_alloc_size(&hdr, alloc_size);
    }

    blob_set_size(&hdr, size);
    blob_set_self(&hdr, addr);

    /*
     * PARTIAL WRITE
     *
     * are there gaps at the beginning? If yes, then we'll fill with zeros,
     * unless, of course, when we are 'updating' a record by resizing it, which is the
     * case when the 'old_hdr' pointer is NOT NULL. In that case, we keep the original
     * data, as far as such data is available, and fill the remainder with zeroes.
     */
    if (flags & HAM_PARTIAL)
    {
        ham_u8_t *zero_blockptr;
        /* the size of the gap leading the partial data block */
        ham_size_t gapsize = record->partial_offset;
        /* the size of the gap trailing the partial data block */
        ham_size_t trail_gapsize = size - (gapsize + record->partial_size);
        const ham_size_t zeroes_count = (gapsize > page_size
                                        ? page_size
                                        : trail_gapsize > page_size
                                          ? page_size
                                          : trail_gapsize > gapsize
                                            ? trail_gapsize
                                            : gapsize);
        ham_size_t oldsize = (old_hdr ? (ham_size_t)blob_get_size(old_hdr) : 0);

        zero_blockptr = (ham_u8_t *)allocator_calloc(env_get_allocator(env), zeroes_count);
        if (!zero_blockptr)
            return HAM_OUT_OF_MEMORY;

        /*
        * first: write the header
        */
        chunk_data[0] = (ham_u8_t *)&hdr;
        chunk_size[0] = sizeof(hdr);
        st=__write_chunks(env, page, addr, HAM_TRUE, freshly_created,
            chunk_data, chunk_size, 1);
        if (st)
            return st;

        addr += sizeof(hdr);

        /*
         * now fill the gap; if the gap is bigger than a pagesize we'll
         * split the gap into smaller chunks
         *
         * But first: check whether we may copy/keep old record data:
         */
        if (oldsize)
        {
            const ham_size_t pagesize = env_get_pagesize(env);
            ham_offset_t old_addr = blob_get_self(old_hdr);
            ham_size_t copy_size;

            if (oldsize > gapsize)
            {
                copy_size = gapsize;
                gapsize = 0;
            }
            else
            {
                copy_size = oldsize;
                gapsize -= oldsize;
            }

            while (copy_size > 0)
            {
                ham_size_t part_size;

                part_size = copy_size - (old_addr % pagesize);
                if (part_size > zeroes_count)
                {
                    part_size = zeroes_count;
                }
                st=__read_chunk(env, NULL, NULL, old_addr, zero_blockptr, part_size);
                if (st)
                {
                    allocator_free(env_get_allocator(env), zero_blockptr);
                    return st;
                }

                chunk_data[0] = zero_blockptr;
                chunk_size[0] = part_size;
                st=__write_chunks(env, page, addr, HAM_TRUE,
                    freshly_created, chunk_data, chunk_size, 1);
                if (st)
                {
                    allocator_free(env_get_allocator(env), zero_blockptr);
                    return st;
                }
                copy_size -= part_size;
                addr += part_size;
                old_addr += part_size;
            }

            while (gapsize > 0)
            {
                chunk_data[0] = zero_blockptr;
                chunk_size[0] = zeroes_count;
                st=__write_chunks(env, page, addr, HAM_TRUE,
                    freshly_created, chunk_data, chunk_size, 1);
                if (st)
                {
                    allocator_free(env_get_allocator(env), zero_blockptr);
                    return st;
                }
                if (gapsize >= zeroes_count)
                {
                    gapsize -= zeroes_count;
                    addr += zeroes_count;
                }
                else
                {
                    addr += gapsize;
                    gapsize = 0;
                }
            }
        }
        else
        {
            while (gapsize > 0)
            {
                chunk_data[0] = zero_blockptr;
                chunk_size[0] = zeroes_count;
                st=__write_chunks(env, page, addr, HAM_TRUE,
                    freshly_created, chunk_data, chunk_size, 1);
                if (st)
                {
                    allocator_free(env_get_allocator(env), zero_blockptr);
                    return st;
                }
                if (gapsize >= zeroes_count)
                {
                    gapsize -= zeroes_count;
                    addr += zeroes_count;
                }
                else
                {
                    addr += gapsize;
                    gapsize = 0;
                }
            }
        }

        /* now write the "real" data */
        chunk_data[0] = (ham_u8_t *)record->data;
        chunk_size[0] = record->partial_size;

        st=__write_chunks(env, page, addr, HAM_TRUE, freshly_created,
                        chunk_data, chunk_size, 1);
        if (st)
        {
            allocator_free(env_get_allocator(env), zero_blockptr);
            return st;
        }
        addr += record->partial_size;

        /*
        * PARTIAL WRITES:
        *
        * if we have gaps at the end of the blob: just append more chunks to
        * fill these gaps. Since they can be pretty large we split them into
        * smaller chunks if necessary.
        *
        * now fill the gap; if the gap is bigger than a pagesize we'll
        * split the gap into smaller chunks
        */
        while (trail_gapsize > 0)
        {
            chunk_data[0] = zero_blockptr;
            chunk_size[0] = zeroes_count;
            st=__write_chunks(env, page, addr, HAM_TRUE,
                freshly_created, chunk_data, chunk_size, 1);
            if (st)
            {
                allocator_free(env_get_allocator(env), zero_blockptr);
                return st;
            }
            if (trail_gapsize >= zeroes_count)
            {
                trail_gapsize -= zeroes_count;
                addr += zeroes_count;
            }
            else
            {
                addr += trail_gapsize;
                trail_gapsize = 0;
            }
        }

        allocator_free(env_get_allocator(env), zero_blockptr);
    }
    else
    {
        /*
         * not writing partially: write header and data, then we're done
         */
        chunk_data[0] = (ham_u8_t *)&hdr;
        chunk_size[0] = sizeof(hdr);
        chunk_data[1] = (ham_u8_t *)record->data;
        chunk_size[1] = size;

        st=__write_chunks(env, page, addr, HAM_TRUE, freshly_created,
                        chunk_data, chunk_size, 2);
        if (st)
            return st;
    }

    /*
     * store the blobid; it will be returned to the caller
     */
    *blobid = blob_get_self(&hdr);

    return HAM_SUCCESS;
}


ham_status_t
blob_read(ham_db_t *db, ham_offset_t blobid,
        ham_record_t *record, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *page;
    blob_t hdr;
    ham_size_t blobsize;
    ham_size_t size_to_copy;
    ham_env_t *env = db_get_env(db);

    /*
    Working with HAM_PARTIAL blobs is a bit more complex than one would imagine
    initially.

    See the documentation for HAM_PARTIAL:


    <h4>wicked scenario #1:</h4>

    Say the user passed in a record with the 'partial' elements set up,
    only to discover that the record stored in the database is smaller
    than expected so that the partial_offset points past the end?

    Do we consider this an error? No we don't.


    <h4>Wicked scenario #2:</h4>

    Say the user passes in a 'partial' record setup such that it receives
    exactly /zero/ bytes of content for the addressed record.<br>
    An error? No.<br>
    Side effects? If we are not careful, YES: the calculated 'blobsize-to-copy'
    <em>MUST NOT</em> blast the record->data and/or record->size values to zero!
    Those are specifically reserved to signal an empty record, which is
    a different thing altogether.

    Otherwise processing is the same as for scenario #1 above.


    <h4>Wicked scenario #3:</h4>

    Suppose the record has fewer bytes on offer for the current part than
    the user requested (@ref ham_record_t::partial_size). Do we adjust the 'partial_size'
    to signal the user that less bytes were written then one might hope for?

    Yes, we do.
    */

    //blobsize = 0;

    /*
     * in-memory-database: the blobid is actually a pointer to the memory
     * buffer, in which the blob is stored
     */
    if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
    {
        blob_t *hdr=(blob_t *)U64_TO_PTR(blobid);
        ham_u8_t *data = blob_get_data(hdr);

        /* when the database is closing, the header is already deleted */
        if (!hdr) {
            record->size = 0;
            record->partial_offset = 0;
            record->partial_size = 0;
            ham_record_set_rid(record, 0);
            return HAM_SUCCESS;
        }

        blobsize = (ham_size_t)blob_get_size(hdr);
        size_to_copy = blobsize;
        if (flags & HAM_PARTIAL)
        {
            if (record->partial_offset > blobsize)
            {
#if 0
                ham_logerr(("partial offset is greater than the total "
                            "record size"));
                return HAM_INV_PARAMETER;
#endif
                record->partial_offset = blobsize;
                size_to_copy = 0;
            }
            else if (record->partial_offset + record->partial_size > blobsize)
            {
                size_to_copy = blobsize - record->partial_offset;
            }
            else
            {
                size_to_copy = record->partial_size;
            }
            record->partial_size = size_to_copy;
        }

        if (!blobsize)
        {
            /* empty blob? */
            record->data = 0;
            record->size = 0;
        }
        else
        {
            ham_u8_t *d = data;

            if (flags & HAM_PARTIAL)
            {
                d += record->partial_offset;
            }

            if ((flags & HAM_DIRECT_ACCESS)
                    && !(record->flags & HAM_RECORD_USER_ALLOC))
            {
                record->size = blobsize;
                record->data = d;
            }
            else
            {
                /* resize buffer, if necessary */
                if (!(record->flags & HAM_RECORD_USER_ALLOC))
                {
                    st = db_resize_record_allocdata(db, size_to_copy);
                    if (st)
                        return (st);
                    record->data = db_get_record_allocdata(db);
                }
                else
                {
                    if (!(flags & HAM_PARTIAL)
                        && record->size < blobsize)
                    {
                        record->size = blobsize;
                        return HAM_RECORDSIZE_TOO_SMALL;
                    }
                }
                /* and copy the data */
                memcpy(record->data, d, size_to_copy);
                record->size = blobsize;
            }
        }
        ham_record_set_rid(record, blobid);

        return HAM_SUCCESS;
    }

    ham_assert(blobid%DB_CHUNKSIZE==0, ("blobid is %llu", blobid));

    /*
     * first step: read the blob header
     */
    st=__read_chunk(env, 0, &page, blobid, (ham_u8_t *)&hdr, sizeof(hdr));
    if (st)
        return (st);

    ham_assert(blob_get_alloc_size(&hdr)%DB_CHUNKSIZE==0, (0));

    /*
     * sanity check
     */
    if (blob_get_self(&hdr) != blobid)
        return (HAM_BLOB_NOT_FOUND);

    blobsize = (ham_size_t)blob_get_size(&hdr);
    size_to_copy = blobsize;
    if (flags & HAM_PARTIAL)
    {
        if (record->partial_offset > blobsize)
        {
#if 0
            ham_trace(("partial offset+size is greater than the total "
                "record size"));
            return HAM_INV_PARAMETER;
#endif
            record->partial_offset = blobsize;
            size_to_copy = 0;
        }
        else if (record->partial_offset + record->partial_size > blobsize)
        {
            size_to_copy = blobsize - record->partial_offset;
        }
        else
        {
            size_to_copy = record->partial_size;
        }
        record->partial_size = size_to_copy;
    }

    /*
     * empty blob?
     */
    if (!blobsize) {
        record->data = 0;
        record->size = 0;
        ham_record_set_rid(record, blobid);
        return HAM_SUCCESS;
    }

    /*
     * second step: resize the blob buffer
     */
    if (!(record->flags & HAM_RECORD_USER_ALLOC)) {
        st = db_resize_record_allocdata(db, size_to_copy);
        if (st)
            return (st);
        record->data = db_get_record_allocdata(db);
    }
    else
    {
        if (!(flags & HAM_PARTIAL)
            && record->size < blobsize)
        {
            record->size = blobsize;
            return HAM_RECORDSIZE_TOO_SMALL;
        }
    }

    /*
     * third step: read the blob data
     */
    st=__read_chunk(env, page, 0,
                    blobid+sizeof(blob_t)+(flags & HAM_PARTIAL
                            ? record->partial_offset
                            : 0),
                    (ham_u8_t *)record->data, size_to_copy);
    if (st)
        return (st);

    record->size = blobsize;
    ham_record_set_rid(record, blobid);

    return HAM_SUCCESS;
}

ham_status_t
blob_overwrite(ham_env_t *env, ham_offset_t old_blobid,
        const ham_record_t *record, ham_u32_t flags,
        dev_alloc_request_info_ex_t *extra_dev_alloc_info,
        ham_offset_t *new_blobid)
{
    ham_status_t st;
    ham_size_t alloc_size;
    blob_t old_hdr;
    blob_t new_hdr;
    ham_page_t *page;
    ham_size_t size = record->size;

    /*
     * PARTIAL WRITE
     *
     * if offset+partial_size equals the full record size, then we won't
     * have any gaps. In this case we just write the full record and ignore
     * the partial parameters.
     */
    if (flags & HAM_PARTIAL) {
        if (record->partial_offset == 0
                && record->partial_offset+record->partial_size==record->size)
        {
            flags &= ~HAM_PARTIAL;
        }
    }

    /*
     * in-memory-databases: free the old blob,
     * allocate a new blob (but if both sizes are equal, just overwrite
     * the data)
     */
    if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
    {
        blob_t *nhdr;
        blob_t *phdr=(blob_t *)U64_TO_PTR(old_blobid);

        if (blob_get_size(phdr) == size)
        {
            ham_u8_t *p = blob_get_data(phdr);
            if (flags & HAM_PARTIAL)
            {
                memmove(p+record->partial_offset,
                        record->data, record->partial_size);
            }
            else
            {
                memmove(p, record->data, size);
            }
            *new_blobid=(ham_offset_t)PTR_TO_U64(phdr);
            ham_assert(*new_blobid == old_blobid, (0));
        }
        else
        {
            /*
             * when the new data is larger, allocate a fresh space for it
             * and discard the old;
             * 'overwrite' has become (delete + insert) now.
             *
             * But NOT ENTIRELY SO for PARTIAL writes: those 'augment' the original
             * data, i.e. the old record data is copied into the new record, then
             * the new PARTIAL chunk is written over that to ensure the latest
             * data resides in the record stored on disk.
             */
            st = blob_allocate(env, record, flags, extra_dev_alloc_info, phdr, new_blobid);
            if (st)
                return (st);
            nhdr=(blob_t *)U64_TO_PTR(*new_blobid);
            blob_set_flags(nhdr, blob_get_flags(phdr));

            allocator_free(env_get_allocator(env), phdr);
        }

        return HAM_SUCCESS;
    }

    ham_assert(old_blobid%DB_CHUNKSIZE==0, (0));

    /*
     * blobs are CHUNKSIZE-allocated
     */
    alloc_size = sizeof(blob_t) + size;
    alloc_size += DB_CHUNKSIZE - 1;
    alloc_size -= alloc_size % DB_CHUNKSIZE;

    /*
     * first, read the blob header; if the new blob fits into the
     * old blob, we overwrite the old blob (and add the remaining
     * space to the freelist, if there is any)
     */
    st = __read_chunk(env, 0, &page, old_blobid, (ham_u8_t *)&old_hdr,
            sizeof(old_hdr));
    if (st)
        return (st);

    ham_assert(blob_get_alloc_size(&old_hdr)%DB_CHUNKSIZE==0, (0));

    /*
     * sanity check
     */
    ham_assert(blob_get_self(&old_hdr)==old_blobid,
            ("invalid blobid %llu != %llu", blob_get_self(&old_hdr),
            old_blobid));
    if (blob_get_self(&old_hdr) != old_blobid)
        return (HAM_INTEGRITY_VIOLATED);

    /*
     * now compare the sizes; does the new data fit in the old allocated
     * space?
     */
    if (alloc_size <= blob_get_alloc_size(&old_hdr))
    {
        ham_u8_t *chunk_data[2];
        ham_size_t chunk_size[2];

        /*
         * setup the new blob header
         */
        blob_set_self(&new_hdr, blob_get_self(&old_hdr));
        blob_set_size(&new_hdr, size);
        blob_set_flags(&new_hdr, blob_get_flags(&old_hdr));
        if (blob_get_alloc_size(&old_hdr) - alloc_size > SMALLEST_CHUNK_SIZE)
            blob_set_alloc_size(&new_hdr, alloc_size);
        else
            blob_set_alloc_size(&new_hdr, blob_get_alloc_size(&old_hdr));

        /*
         * PARTIAL WRITE
         *
         * if we have a gap at the beginning, then we have to write the
         * blob header and the blob data in two steps; otherwise we can
         * write both immediately
         */
        if ((flags & HAM_PARTIAL) && (record->partial_offset)) {
            chunk_data[0]=(ham_u8_t *)&new_hdr;
            chunk_size[0]=sizeof(new_hdr);
            st=__write_chunks(env, page, blob_get_self(&new_hdr), HAM_FALSE,
                    HAM_FALSE, chunk_data, chunk_size, 1);
            if (st)
                return (st);

            chunk_data[0]=(ham_u8_t *)record->data;
            chunk_size[0]=record->partial_size;
            st=__write_chunks(env, page,
                    blob_get_self(&new_hdr)+sizeof(new_hdr)
                            +record->partial_offset,
                    HAM_FALSE, HAM_FALSE, chunk_data, chunk_size, 1);
            if (st)
                return (st);
        }
        else {
            chunk_data[0]=(ham_u8_t *)&new_hdr;
            chunk_size[0]=sizeof(new_hdr);
            chunk_data[1]=(ham_u8_t *)record->data;
            chunk_size[1]=(flags&HAM_PARTIAL)
                                ? record->partial_size
                                : size;

            st=__write_chunks(env, page, blob_get_self(&new_hdr), HAM_FALSE,
                    HAM_FALSE, chunk_data, chunk_size, 2);
            if (st)
                return (st);
        }

        /*
         * move remaining data to the freelist
         */
        if (blob_get_alloc_size(&old_hdr) != blob_get_alloc_size(&new_hdr))
        {
            st = freel_mark_free(env,
                  blob_get_self(&new_hdr)+blob_get_alloc_size(&new_hdr),
                  (ham_size_t)(blob_get_alloc_size(&old_hdr)-
                    blob_get_alloc_size(&new_hdr)), HAM_FALSE);
            if (st)
                return st;
        }

        /*
         * the old rid is the new rid
         */
        *new_blobid=blob_get_self(&new_hdr);

        return HAM_SUCCESS;
    }
    else {
        /*
         * when the new data is larger, allocate a fresh space for it
         * and discard the old;
         * 'overwrite' has become (delete + insert) now.
         *
         * But NOT ENTIRELY SO for PARTIAL writes: those 'augment' the original
         * data, i.e. the old record data is copied into the new record, then
         * the new PARTIAL chunk is written over that to ensure the latest
         * data resides in the record stored on disk.
         */
        st = blob_allocate(env, record, flags, extra_dev_alloc_info, &old_hdr, new_blobid);
        if (st)
            return (st);

        st = freel_mark_free(env, old_blobid,
                (ham_size_t)blob_get_alloc_size(&old_hdr), HAM_FALSE);
        if (st)
            return st;
    }

    return HAM_SUCCESS;
}

ham_status_t
blob_free(ham_env_t *env, ham_offset_t blobid, ham_u32_t flags)
{
    ham_status_t st;
    blob_t hdr;

    /*
     * in-memory-database: the blobid is actually a pointer to the memory
     * buffer, in which the blob is stored
     */
    if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
    {
        allocator_free(env_get_allocator(env), (void *)U64_TO_PTR(blobid));
        return HAM_SUCCESS;
    }

    ham_assert(blobid%DB_CHUNKSIZE==0, (0));

    /*
     * fetch the blob header
     */
    st=__read_chunk(env, 0, 0, blobid, (ham_u8_t *)&hdr, sizeof(hdr));
    if (st)
        return (st);

    ham_assert(blob_get_alloc_size(&hdr)%DB_CHUNKSIZE==0, (0));

    /*
     * sanity check
     */
    ham_assert(blob_get_self(&hdr)==blobid,
            ("invalid blobid %llu != %llu", blob_get_self(&hdr), blobid));
    if (blob_get_self(&hdr)!=blobid)
        return (HAM_INTEGRITY_VIOLATED);

    /*
     * move the blob to the freelist
     */
    st = freel_mark_free(env, blobid,
            (ham_size_t)blob_get_alloc_size(&hdr), HAM_FALSE);
    ham_assert(!st, ("unexpected error, at least not covered in the old code"));

    return st;
}


/**
Determines the insert position for an ordered duplicate insertion.

@param position_ref call-by-reference in/out parameter providing the initial start position
                    where to begin looking for an insert position and on successful return
                    it will contain the located insert position.

@return errorcode or HAM_SUCCESS. In case of HAM_SUCCESS, the @a position_ref in/out parameter
        will be set to the desired insert position.
*/
static ham_size_t
__get_sorted_position(ham_db_t *db, dupe_table_t *table, const ham_record_t *record,
                ham_u32_t flags, ham_size_t *position_ref)
{
    ham_duplicate_compare_func_t foo = db_get_duplicate_compare_func(db);
    ham_size_t l, r, m;
    int cmp;
    dupe_entry_t *e;
    ham_record_t item_record;
    //ham_offset_t blob_ptr;
    ham_u16_t dam;
    ham_status_t st = 0;

    /*
     * Use a slightly adapted form of binary search: as we already have our
     * initial position (as was stored in the cursor), we take that as our
     * first 'median' value and go from there.
     */
    l = 0;
    r = dupe_table_get_count(table) - 1; /* get_count() is 1 too many! */

    /*
     * Maybe Wrong Idea: sequential access/insert doesn't mean the RECORD
     * values are sequential too! They MAY be, but don't have to!
     *
     * For now, we assume they are also sequential when you're storing records
     * in duplicate-key tables (probably a secondary index table for another
     * table, this one).
     */
    dam = db_get_data_access_mode(db);
    if (dam & HAM_DAM_SEQUENTIAL_INSERT) {
        /* assume the insertion point sits at the end of the dupe table */
        m = r;
    }
    else if (dam & HAM_DAM_FAST_INSERT) {
        /*
         * can't really say how to speed these buggers up, apart from maybe
         * assuming the insertion point is at the previously known position.
         */
        m = *position_ref;
        if (m > r)
            m = r;
        /*
         * as this is only a split point, we check which side is shortest
         * and adjust the split point to the other side, so the reduction
         * can be quickened when all assumptions match.
         */
        if (r - m > m - l) {
            m++;
        }
        else if (m > 0) {
            m--;
        }
    }
    else {
        /* the bland average split point for any binary search */
        m = (l + r) / 2;
    }
    ham_assert(m <= r, (0));
    ham_assert(r >= 0, (0));
    ham_assert(r >= l, (0));

    for (;;) {
        ham_assert(r >= l, (0));
        ham_assert(m < dupe_table_get_count(table), (0));

        e = dupe_table_get_entry(table, m);
        //blob_ptr = dupe_entry_get_rid(e);

        /*
        TODO:

        memleak as blob data is read but never released, while we binary search using those blob data chunks
        */

        memset(&item_record, 0, sizeof(item_record));
        //item_record._rid=blob_ptr;
        ham_record_set_intflags(&item_record, dupe_entry_get_flags(e)
                &(KEY_BLOB_SIZE_SMALL
                 |KEY_BLOB_SIZE_TINY
                 |KEY_BLOB_SIZE_EMPTY));
#if 01 /* Christoph patch 19/jan ? */
		/*
		 * rid: same as record->_rid; however, if key is TINY/SMALL and
		 * HAM_DIRECT_ACCESS is set, we need a direct pointer to the original
		 * record ID
		 *
		*/
        st=util_read_record(db, &item_record, dupe_entry_get_rid_direct_ref(e), flags);
        if (st)
            return (st);
#else
        if (0 == (ham_record_get_intflags(&item_record) &
                        (KEY_BLOB_SIZE_TINY
                        |KEY_BLOB_SIZE_SMALL
                        |KEY_BLOB_SIZE_EMPTY)))
        {
            ham_offset_t blob_ptr = dupe_entry_get_rid(e);

            //ham_record_set_rid(&item_record, blob_ptr);   -- [i_a] now done inside blob_read()
            st = blob_read(db, blob_ptr, &item_record, flags);
            if (st)
                return (st);
        }
        else
        {
            ham_u8_t *blob_ptr = dupe_entry_get_rid_as_data(e);

            item_record.data = blob_ptr;
            if (ham_record_get_intflags(&item_record) & KEY_BLOB_SIZE_TINY) {
                item_record.size = blob_ptr[sizeof(ham_pers_rid_t)-1];
            }
            else if (ham_record_get_intflags(&item_record) & KEY_BLOB_SIZE_SMALL) {
                item_record.size = 8;
            }
            else {
                ham_assert(ham_record_get_intflags(&item_record) & KEY_BLOB_SIZE_EMPTY, (0));
                item_record.size = 0;
            }
        }
#endif

        cmp = foo(db, (const ham_u8_t *)record->data, record->size,
                        (const ham_u8_t *)item_record.data, item_record.size);
        /* item is lower than (or equal to) the left-most item of our range */
        if (m == l) {
            ham_assert(l <= r, (0));
            ham_assert(r - l <= 1, (0));
            if (cmp <= 0) {
                /* write LEQ record value in THIS slot */
                break;
            }
            else if (m == r) {
                /* write GT record value in NEXT slot */
                ham_assert(cmp > 0, (0));
                m++;
                break;
            }
            /* else: (l != r) --> has to compare with [r] before we know for sure */
            ham_assert(l < r, (0));
            l = m + 1;
        }
        else if (cmp == 0) {
            ham_assert(m < r, (0));
            ham_assert(l < r, (0));
            ham_assert(m > l, (0));
            /* write equal record value in NEXT slot */
            m++;
            break;
        }
        else if (cmp < 0) {
            ham_assert(m < r, (0));
            ham_assert(l < r, (0));
            ham_assert(m > l, (0));
            r = m - 1;
        }
        else {
            ham_assert(m < r, (0));
            ham_assert(l < r, (0));
            ham_assert(m > l, (0));
            l = m + 1;
        }
        m = (l + r) / 2;
    }

    /* now 'm' points at the insertion point in the table */
    ham_assert(m <= dupe_table_get_count(table), (0));
    *position_ref = m;

	ham_nuke_stack_space(item_record);
    return HAM_SUCCESS;
}

ham_status_t
blob_duplicate_insert(ham_db_t *db, ham_offset_t table_id,
        const ham_record_t *record, ham_size_t position, ham_u32_t flags,
        dupe_entry_t *entries, ham_size_t num_entries,
        dev_alloc_request_info_ex_t *extra_dev_alloc_info,
        ham_offset_t *rid, ham_size_t *new_position)
{
    ham_status_t st = 0;
    dupe_table_t *table = 0;
    ham_bool_t alloc_table = HAM_FALSE;
    ham_bool_t resize = HAM_FALSE;
    ham_page_t *page = 0;
    ham_env_t *env = db_get_env(db);
    ham_bool_t sort_dups = HAM_FALSE;
    ham_size_t table_count;

    ham_assert(db_get_duplicate_compare_func(db) != 0, (0));

    if (db_get_rt_flags(db) & HAM_SORT_DUPLICATES)
    {
#if 0
        if (flags & HAM_OVERWRITE)
        {
            ham_trace(("cannot specify HAM_OVERWRITE when inserting a duplicate (key+record) in an ordered-duplicate record table"));
            return HAM_INV_PARAMETER;
        }
#endif
#if 0
        if (flags & (HAM_DUPLICATE_INSERT_BEFORE
                    |HAM_DUPLICATE_INSERT_AFTER
                    |HAM_DUPLICATE_INSERT_FIRST
                    |HAM_DUPLICATE_INSERT_LAST))
        {
            ham_trace(("cannot specify HAM_DUPLICATE_INSERT_BEFORE/AFTER/FIRST/LAST when inserting a duplicate (key+record) in an ordered-duplicate record table"));
            return HAM_INV_PARAMETER;
        }
#endif
        sort_dups = HAM_TRUE;
    }

    ham_assert(record->partial_offset == 0, (0));
    ham_assert(record->partial_size == 0 || record->partial_size == record->size, (0));
    ham_assert(!(record->flags & HAM_PARTIAL), ("HAM_PARTIAL is not an accepted flag for duplicate records!"));

    /*
     * create a new duplicate table if none existed, and insert
     * the first entry
     */
    if (!table_id) {
        ham_assert(num_entries==2, (0));
        /* allocates space for 8 (!) entries */
        table=(dupe_table_t *)allocator_calloc(env_get_allocator(env),
                        sizeof(dupe_table_t) + (8-1) * sizeof(dupe_entry_t));
        if (!table)
            return HAM_OUT_OF_MEMORY;
        dupe_table_set_capacity(table, 8);
        dupe_table_set_count(table, 1);
        memcpy(dupe_table_get_entry(table, 0), &entries[0],
                        sizeof(entries[0]));

        /* skip the first entry */
        entries++;
        num_entries--;
        alloc_table = HAM_TRUE;
    }
    else {
        /*
         * otherwise load the existing table
         */
        st=__get_duplicate_table(&table, &page, env, table_id);
        ham_assert(st ? table == NULL : 1, (0));
        ham_assert(st ? page == NULL : 1, (0));
        if (!table)
            return st ? st : HAM_INTERNAL_ERROR;
        if (!page && !(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
            alloc_table = HAM_TRUE;
    }

    if (page)
    {
        st = ham_log_add_page_before(page);
        if (st)
            goto failed_dramatically;

        if (sort_dups)
        {
            page_add_ref(page); /* mandatory as sorting may possibly load 'extended keys' which reside on other pages, at least partially, which actions may deplete the cache and this page as well unless we increment its reference counter for the duration */
        }
    }

    ham_assert(num_entries==1, (0));
    table_count = dupe_table_get_count(table);

    /*
     * resize the table, if necessary
     */
    if (!(flags & HAM_OVERWRITE))
    {
        ham_assert(table_count < dupe_table_get_capacity(table), (0));
        /* old code never filled the last slot in the dupe table here :-( */
        if (table_count == dupe_table_get_capacity(table))
        {
            dupe_table_t *old = table;
            ham_size_t new_cap = dupe_table_get_capacity(table);

            if (new_cap < 3*8)
                new_cap += 8;
            else
                new_cap += new_cap/3;

            table = (dupe_table_t *)allocator_calloc(env_get_allocator(env), sizeof(dupe_table_t) +
                            (new_cap-1) * sizeof(dupe_entry_t));
            if (!table)
            {
                st = HAM_OUT_OF_MEMORY;
                table = old;
                goto failed_dramatically;
            }
            dupe_table_set_capacity(table, new_cap);
            dupe_table_set_count(table, table_count);
            memcpy(dupe_table_get_entry(table, 0), dupe_table_get_entry(old, 0),
                           table_count * sizeof(dupe_entry_t));
            if (alloc_table)
                allocator_free(env_get_allocator(env), old);

            alloc_table = HAM_TRUE;
            resize = HAM_TRUE;
        }
    }

    /*
     * insert sorted, unsorted or overwrite the entry at the requested position
     */
    if (flags & HAM_OVERWRITE)
    {
        dupe_entry_t *e = dupe_table_get_entry(table, position);

        if (!(dupe_entry_get_flags(e) & (KEY_BLOB_SIZE_SMALL
                                    |KEY_BLOB_SIZE_TINY
                                    |KEY_BLOB_SIZE_EMPTY)))
        {
            st = blob_free(env, dupe_entry_get_rid(e), 0);
            if (st)
                return st;
        }

        if (!sort_dups)
        {
            memcpy(dupe_table_get_entry(table, position),
                &entries[0], sizeof(entries[0]));
        }
        else
        {
            /*
            ensuring sort order requires we see where the overwritten
            node should end up - we don't just overwrite the item
            at 'position' here, but we have to follow a two-stage
            process: kill the item at 'position' as a first step
            of 'overwriting' it (done that already above), next we
            need to see where the new node content should end up to
            ensure proper ordering.

            Now that last bit is done as a delete + insert, so that we
            only need a single, and simple!, piece of logic to take
            care of this ordered insert/overwrite stuff.

            So we delete the entry here, then progress past the 'insert'
            section below, where the re-insertion will be taken
            care of, resulting in a 'sorted' overwrite overall.
            */
            if (position < table_count - 1 /* right-most element */ )
            {
                memmove(dupe_table_get_entry(table, position),
                        dupe_table_get_entry(table, position + 1),
                        sizeof(entries[0]) * (table_count - (position + 1)));
                table_count--;
            }
        }
    }
    else if (!sort_dups)
    {
        if (flags & HAM_DUPLICATE_INSERT_BEFORE) {
            /* do nothing, insert at the current position */
        }
        else if (flags & HAM_DUPLICATE_INSERT_AFTER) {
            position++;
            if (position > table_count)
                position = table_count;
        }
        else if (flags & HAM_DUPLICATE_INSERT_FIRST) {
            position = 0;
        }
        else if (flags & HAM_DUPLICATE_INSERT_LAST) {
            position = table_count;
        }
        else {
            position = table_count;
        }

        if (position < table_count)
        {
            memmove(dupe_table_get_entry(table, position + 1),
                    dupe_table_get_entry(table, position),
                    sizeof(entries[0]) * (table_count - position));
        }

        memcpy(dupe_table_get_entry(table, position),
                &entries[0], sizeof(entries[0]));

        table_count++;
        dupe_table_set_count(table, table_count);
    }
    else /* if (sort_dups) */
    {
        /*
        ensuring sort order requires we see where the new node should
        be inserted.

        Fake a position for now...

        Use the HAM_DUPLICATE_INSERT_xxx flags as 'hints' for the dupe insertion slot
        */
        if (flags & HAM_DUPLICATE_INSERT_BEFORE) {
            /* do nothing, insert at the current position */
        }
        else if (flags & HAM_DUPLICATE_INSERT_AFTER) {
            position++;
        }
        else if (flags & HAM_DUPLICATE_INSERT_FIRST) {
            position = 0;
        }
        else if (flags & HAM_DUPLICATE_INSERT_LAST) {
            position = table_count;
        }
        else {
            position = table_count / 2;
        }
    }

    if (sort_dups)
    {
        st = __get_sorted_position(db, table, record, flags, &position);
        if (st)
            goto failed_dramatically;
        ham_assert(position <= table_count, (0));
        if (position < table_count)
        {
            /* make room inside the array */
            memmove(dupe_table_get_entry(table, position + 1),
                dupe_table_get_entry(table, position),
                sizeof(entries[0]) * (table_count - position));
        }

        memcpy(dupe_table_get_entry(table, position),
            &entries[0], sizeof(entries[0]));

        table_count++;
        dupe_table_set_count(table, table_count);
    }

    /*
     * write the table back to disk and return the blobid of the table
     */
    if ((table_id && !page) || resize)
    {
        dev_alloc_request_info_ex_t info = *extra_dev_alloc_info;
        ham_record_t rec={0};

        ham_assert(info.db == db, (0));
        ham_assert(info.env == env, (0));
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_DUPETABLESPACE; // PAGE_TYPE_BLOB;
        info.record = record;
        info.insert_flags = flags;
        info.master = extra_dev_alloc_info;

        rec.data = (ham_u8_t *)table;
        rec.size = sizeof(dupe_table_t)
                    +(dupe_table_get_capacity(table)-1)*sizeof(dupe_entry_t);

        st = blob_overwrite(env, table_id, &rec, 0, &info, rid);
		ham_nuke_stack_space(rec);
    }
    else if (!table_id)
    {
        dev_alloc_request_info_ex_t info = *extra_dev_alloc_info;
        ham_record_t rec={0};

        ham_assert(info.db == db, (0));
        ham_assert(info.env == env, (0));
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_DUPETABLESPACE; // PAGE_TYPE_BLOB;
        info.record = record;
        info.insert_flags = flags;
        info.master = extra_dev_alloc_info;

        rec.data = (ham_u8_t *)table;
        rec.size = sizeof(dupe_table_t)
                    +(dupe_table_get_capacity(table)-1)*sizeof(dupe_entry_t);

        st = blob_allocate(env, &rec, 0, &info, NULL, rid);
		ham_nuke_stack_space(rec);
    }
    else if (table_id && page)
    {
        page_set_dirty(page, env);
    }
    else
    {
        ham_assert(!"shouldn't be here", (0));
    }

failed_dramatically:
    if (page && sort_dups)
    {
        page_release_ref(page);
    }

    if (alloc_table)
        allocator_free(env_get_allocator(env), table);

    if (new_position)
        *new_position = position;

    return (st);
}

ham_status_t
blob_duplicate_erase(ham_db_t *db, int_key_t *table_key,
        ham_size_t position, ham_u32_t flags, ham_offset_t *new_table_id)
{
    ham_status_t st;
    ham_record_t rec = {0};
    ham_size_t i;
    dupe_table_t *table;
    ham_offset_t rid;
    ham_env_t *env = db_get_env(db);
    ham_offset_t table_id;

    /* store the public record pointer, otherwise it's destroyed */
    ham_size_t rs = db_get_record_allocsize(db);
    void      *rp = db_get_record_allocdata(db);

    db_set_record_allocdata(db, 0);
    db_set_record_allocsize(db, 0);

    //memset(&rec, 0, sizeof(rec));

    table_id = key_get_ptr(table_key);
    if (new_table_id)
        *new_table_id=table_id;

    st=blob_read(db, table_id, &rec, 0);
    if (st)
        return (st);

    /* restore the public record pointer */
    db_set_record_allocsize(db, rs);
    db_set_record_allocdata(db, rp);

    table=(dupe_table_t *)rec.data;

    /*
     * if BLOB_FREE_ALL_DUPES is set *OR* if the last duplicate is deleted:
     * free the whole duplicate table
     */
    if (flags & BLOB_FREE_ALL_DUPES
            || (position==0 && dupe_table_get_count(table)==1))
    {
        for (i = 0; i < dupe_table_get_count(table); i++)
        {
            dupe_entry_t *e = dupe_table_get_entry(table, i);
            if (!(dupe_entry_get_flags(e) & (KEY_BLOB_SIZE_SMALL
                                        |KEY_BLOB_SIZE_TINY
                                        |KEY_BLOB_SIZE_EMPTY)))
            {
                st=blob_free(env, dupe_entry_get_rid(e), 0);
                if (st)
                {
                    allocator_free(env_get_allocator(env), table);
                    return (st);
                }
            }
        }
        st=blob_free(env, table_id, 0); /* [i_a] isn't this superfluous (&
                                        * dangerous), thanks to the
                                        * free_all_dupes loop above??? */
        allocator_free(env_get_allocator(env), table);
        if (st)
            return (st);

        if (new_table_id)
            *new_table_id=0;

		ham_nuke_stack_space(rec);
        return HAM_SUCCESS;
    }
    else
    {
        dupe_entry_t *e=dupe_table_get_entry(table, position);
        dev_alloc_request_info_ex_t info = {0};

        info.db = db;
        info.env = env;
        info.dam = db_get_data_access_mode(db);
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_DUPETABLESPACE; // PAGE_TYPE_BLOB;
        info.int_key = table_key;

        if (!(dupe_entry_get_flags(e)&(KEY_BLOB_SIZE_SMALL
                                    |KEY_BLOB_SIZE_TINY
                                    |KEY_BLOB_SIZE_EMPTY)))
        {
            st=blob_free(env, dupe_entry_get_rid(e), 0);
            if (st) {
                allocator_free(env_get_allocator(env), table);
                return (st);
            }
        }
        memmove(e, e+1,
            ((dupe_table_get_count(table)-position)-1)*sizeof(dupe_entry_t));
        dupe_table_set_count(table, dupe_table_get_count(table)-1);

		memset(&rec, 0, sizeof(rec));
        rec.data=(ham_u8_t *)table;
        rec.size=sizeof(dupe_table_t)
                    +(dupe_table_get_capacity(table)-1)*sizeof(dupe_entry_t);
        st=blob_overwrite(env, table_id, &rec, 0, &info, &rid);
        if (st) {
            allocator_free(env_get_allocator(env), table);
            return (st);
        }
        if (new_table_id)
            *new_table_id=rid;
    }

    /*
     * return 0 as a rid if the table is empty
     */
    if (dupe_table_get_count(table)==0)
        if (new_table_id)
            *new_table_id=0;

    allocator_free(env_get_allocator(env), table);
	ham_nuke_stack_space(rec);
    return HAM_SUCCESS;
}

ham_status_t
blob_duplicate_get_count(ham_env_t *env, ham_offset_t table_id,
        ham_size_t *count, dupe_entry_t *entry)
{
    ham_status_t st;
    dupe_table_t *table;
    ham_page_t *page=0;

    st=__get_duplicate_table(&table, &page, env, table_id);
    ham_assert(st ? table == NULL : 1, (0));
    ham_assert(st ? page == NULL : 1, (0));
    if (!table)
        return st ? st : HAM_INTERNAL_ERROR;

    *count=dupe_table_get_count(table);
    if (entry)
        memcpy(entry, dupe_table_get_entry(table, (*count)-1), sizeof(*entry));

    if (!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
    {
        if (!page)
            allocator_free(env_get_allocator(env), table);
    }
    return HAM_SUCCESS;
}

ham_status_t
blob_duplicate_get(ham_env_t *env, ham_offset_t table_id,
        ham_size_t position, dupe_entry_t *entry)
{
    ham_status_t st;
    dupe_table_t *table;
    ham_page_t *page=0;

    st = __get_duplicate_table(&table, &page, env, table_id);
    ham_assert(st ? table == NULL : 1, (0));
    ham_assert(st ? page == NULL : 1, (0));
    if (!table)
        return st ? st : HAM_INTERNAL_ERROR;

    if (position >= dupe_table_get_count(table))
    {
        if (!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
            if (!page)
                allocator_free(env_get_allocator(env), table);
        return HAM_KEY_NOT_FOUND;
    }
    memcpy(entry, dupe_table_get_entry(table, position), sizeof(*entry));

    if (!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
    {
        if (!page)
            allocator_free(env_get_allocator(env), table);
    }
    return HAM_SUCCESS;
}



/**
* @endcond
*/










