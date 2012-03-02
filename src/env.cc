/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

#include "internal_preparation.h"





ham_status_t
env_purge_cache(ham_env_t *env, ham_size_t purge_depth)
{
    ham_status_t st;
    ham_page_t *page;
    ham_cache_t *cache = env_get_cache(env);

    /*
     * first, try to delete unused pages from the cache
     */
    if (cache && !(env_get_rt_flags(env) & HAM_IN_MEMORY_DB) && purge_depth)
    {
        /*
        Win32 and WinCE have an issue with unlimited caches and other data structures which /may/ consume very large amounts of memory-mapped disk space: as Win32 has a 32-bit, i.e. 4GiB memory space limit (i.e. 'flat' addressing model for x86), Microsoft Windows has split this memory space in several parts, one of which is used for addressing memory-mapped I/O, among other things: the 'paged pool' space: this space is about 470MiB large (and it doesn't seem the /3G Win32 boot switch has any impact on that one).

        See also: http://msdn.microsoft.com/en-us/library/ms836325.aspx (Figure #2), where memory mapped address space is limited to the range 0x42000000-0x80000000.

        See also: http://www.windows-tech.info/17/aa6968db35d5fb2c.php and http://support.microsoft.com/kb/889654 where the 'paged pool' is listed as 470MiB (where Russinovich mentions it is limited to 491MiB at most for Win32: http://blogs.technet.com/markrussinovich/archive/2009/03/26/3211216.aspx )

        See also: http://blogs.technet.com/askperf/archive/2007/03/07/memory-management-understanding-pool-resources.aspx

        From http://blogs.technet.com/markrussinovich/archive/2009/03/26/3211216.aspx :
        "On 32-bit Windows XP, the limit is calculated based on how much
        address space is assigned other resources, most notably system PTEs, with
        an upper limit of 491MB."

        Also note that currently this may seem a Win32 specific 'issue', but it really exists on ALL operating systems. See also: http://lists.freebsd.org/pipermail/freebsd-questions/2004-June/050371.html

        Hence we take a bit of a different approach here to 'resolve' this issue, or at least to ameliorate it to a bearable level:
        since we cannot guarantee that we will be the only piece of software running in the current process which consumes memory mapping
        address space, we might have been able to guestimate some sort of best case upper bound on that limit for particular systems,
        but generally speaking we can't say when we will 'hit the roof' with any reasonable prediction quality, so we turn around
        and decide to act only once we hit that roof: that fact is simply detected through examining the operating system's error
        code when doing the memory mapping jig, so we can add a 'retry/recovery mechanism' to the device I/O for when such a limit/failure
        code occurs in our run-time.

        THAT is the time this 'purge the cache' routine will be called (again), only this time we'll require the purge code to
        do a little more work than just a superficial (and relatively fast) purge: this requirement can be enforced by passing
        a lowered 'purge_depth' into this routine as such a lower depth (== the desired number of pages purged)
        would require the purge routine to keep at it
        for a while longer, thus ensuring more pages are released/purged and hence increasing our chances of a successful retry
        action. (See page_alloc() and page_fetch(): since those are the only two routines which perform device->alloc_page() or
        device->read_page() operations, and those are the only ones where memory mapped address space usage is possibly increased,
        we can restrict the 'on error purge, then retry' activity to those routines (page_alloc() and page_fetch()).)

        As this issue isn't just a Win32 issue, but also an issue on 32 bit UNIX systems (and probably other platforms as well), we
        do NOT surround this code with
            #ifdef WIN32
        conditionals; instead, the support for this recovery scheme is now wholly dependent on the device driver code: the Win32
        memory mapped file I/O device produces the correct, distinctive error code (@ref HAM_LIMITS_REACHED) for the retry action to
        work; it is merely a question of time when the POSIX/UNIX equivalent will support this same feature.


        Furthermore, the code in here tries to help cut down the number of possible occurrences of said failure by performing
        a wee bit of purging even for the HAM_CACHE_UNLIMITED case of caching setup: in that we case we will purge the oldest
        UNUSED page from the cache; since this purge routine is called often enough this approach will keep the cache to within
        a reasonable bound for most use cases.
        */
        ham_bool_t fast_scan_mode;
        ham_size_t cur = cache_get_cur_elements(cache);
        ham_size_t max = cache_get_max_elements(cache);
        const ham_size_t orig_purge_depth = purge_depth;

        /*
        Use 'fast' mode when the requested number of pages to purge is rather few:
        if we can't get those from the garbage list, we don't want to spend a long time scanning
        the cache linked list. See also cache_get_unused_page().
        */
        fast_scan_mode = ((purge_depth <= 4)
                        && (purge_depth <= cur / 16));
                        // && (cur <= max / 2));
        ham_assert(fast_scan_mode ? cur <= max : 1, ("must be assured by the way the caller calculates purge_depth"));

        // if (env_get_rt_flags(env) & HAM_CACHE_UNLIMITED)

#if defined(HAM_DEBUG) && (HAM_LEAN_AND_MEAN_FOR_PROFILING_LEVEL < 1)
        if (!fast_scan_mode)
        {
            (void)cache_check_integrity(cache);
        }
#endif
        while (purge_depth)
        {
            page = cache_get_unused_page(cache, fast_scan_mode);
            if (!page)
            {
                if (env_get_rt_flags(env) & HAM_CACHE_STRICT)
                {
                    if (!fast_scan_mode)
                    {
				        cur = cache_get_cur_elements(cache);

						/* only yak about CACHE FULL when there really is no space left any more! */
						if (cur >= max)
						{
							return HAM_CACHE_FULL;
						}
						else
						{
							/* we still have 1 or more slots left to use... */
							return HAM_SUCCESS;
						}
                    }
                    else
                    {
                        /* use more energy to purge a page! */
                        fast_scan_mode = HAM_FALSE;
                        continue;
                    }
                }
                else
                {
                    if (!fast_scan_mode)
                    {
                        break;
                    }
                    else
                    {
                        /* should we put more effort into purging? Only when we didn't achieve anything yet! */
                        if (purge_depth != orig_purge_depth)
                        {
                            break;
                        }
                        fast_scan_mode = HAM_FALSE;
                        continue;
                    }
                }
            }
            cache_push_history(page, -100);

            st = db_write_page_and_delete(page, 0);
            if (st)
                return st;

            purge_depth--;
        }
    }

    /*
     * then free unused extended keys. We don't do this too often to
     * avoid a thrashing of the cache, and freeing unused extkeys
     * is more expensive (performance-wise) than freeing unused pages.
     *
     * We also purge the extkey cache when we flush a page above;
     * this code is only active when we have no page cache, in which
     * case we need another way to get at the extkey caches which
     * are maintained per Database.
     */
    if (!cache)
    {
        if ((env_get_txn_id(env) & 0x7) == 0)  // N MOD 8 = 0
        {
            ham_db_t *db;

            db = env_get_list(env);
            while (db) {
                if (db_get_extkey_cache(db))
                {
                    st=extkey_cache_purge(db_get_extkey_cache(db));
                    if (st)
                        return st;
                }
                db = db_get_next(db);
            }
        }
    }

    return HAM_SUCCESS;
}







ham_u16_t
env_get_max_databases(ham_env_t *env)
{
    env_header_t *hdr=(env_header_t*)(page_get_payload(env_get_header_page(env)));
    return (ham_db2h16(hdr->_max_databases));
}

ham_u8_t
env_get_version(ham_env_t *env, ham_size_t idx)
{
    env_header_t *hdr=(env_header_t*)(page_get_payload(env_get_header_page(env)));
    return (envheader_get_version(hdr, idx));
}

ham_u32_t
env_get_serialno(ham_env_t *env)
{
    env_header_t *hdr=(env_header_t*)(page_get_payload(env_get_header_page(env)));
    return (ham_db2h32(hdr->_serialno));
}

void
env_set_serialno(ham_env_t *env, ham_u32_t n)
{
    env_header_t *hdr=(env_header_t*)(page_get_payload(env_get_header_page(env)));
    hdr->_serialno=ham_h2db32(n);
}

env_header_t *
env_get_header(ham_env_t *env)
{
    return ((env_header_t*)(page_get_payload(env_get_header_page(env))));
}

ham_status_t
env_fetch_page(ham_page_t **page_ref, ham_env_t *env,
        ham_offset_t address, ham_u32_t flags)
{
    return (db_fetch_page_impl(page_ref, env, 0, address, flags));
}

ham_status_t
env_alloc_page(ham_page_t **page_ref, ham_env_t *env,
                ham_u32_t type, ham_u32_t flags)
{
    return (db_alloc_page_impl(page_ref, env, 0, type, flags));
}
ham_status_t
env_reserve_space(ham_offset_t minimum_page_count,
                  dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_status_t st = HAM_SUCCESS;
    ham_offset_t address;
    ham_env_t *env = extra_dev_alloc_info->env;

    ham_assert(env, (0));

    /*
     * this is not an activity allowed for r/o dbs!
     */
    if (env_get_rt_flags(env) & HAM_READ_ONLY) {
        return HAM_DB_READ_ONLY;
    }

    /*
    when required minimum size is 1 page or less, we don't need to bother.
    */
    if (minimum_page_count <= 1)
        return HAM_SUCCESS;

    ham_assert(env_get_cache(env) != NULL, ("ENV MUST have a cache set up or the resize will fetch dual mmap copies of old pages, which is bad"));

    /*
     * in-memory-database: don't preallocate space for the database.
     */
    if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB) {
        return HAM_SUCCESS;
    }

    ham_log_mark_db_expansion_start(env);

    /*
    The trick here is to use an UNDOCUMENTED SIDE EFFECT of the
    freelist check:

    when asked whether a certain address space in the storage is
    free, the freelist guarantees to be expanded to at least reach
    that range.

    Thus we take care of the caveat mentioned above: basically,
    we first make sure our freelist is large enough to span the
    required range, before we dimension the storage accordingly
    by allocating a tiny chunk of space just below the requested
    size.

    This is a SHORTCUT for our 1-2-3 process above...
    */
    address = minimum_page_count;
    address *= env_get_pagesize(env);
    address -= DB_CHUNKSIZE;

    st = freel_check_area_is_allocated(address, DB_CHUNKSIZE, extra_dev_alloc_info);
    if (st)
    {
        ham_log_mark_db_expansion_end(env);
        return st;
    }
    else
    {
        /*
        when this area is not yet occupied, we must temporarily
        allocate (and free) it now to ensure that the database
        storage space is expanded to the requested size.
        */
        ham_offset_t alloced_addr;

        st = freel_alloc_area_ex(&alloced_addr, DB_CHUNKSIZE, 0, address, extra_dev_alloc_info);
        if (alloced_addr)
        {
            /* the storage already is equal or larger than the requested size */
            ham_assert(!st, (0));
            st = freel_mark_free(env, alloced_addr, DB_CHUNKSIZE, HAM_FALSE);
        }
        else
        {
            /*
            the freelist may now be large enough, but that does not mean
            that all storage pages up to 'address' have already been created:
            a task for right now.
            */
            ham_page_t *page;
            ham_offset_t page_addr;

            /* ignore probable error from freel_alloc_area_ex() */

            address += DB_CHUNKSIZE;

            do
            {
                st = db_alloc_page(&page,
                            ( PAGE_IGNORE_FREELIST
                            | PAGE_DONT_LOG_CONTENT
                            | DB_NEW_PAGE_DOES_THRASH_CACHE),
                            extra_dev_alloc_info);
                ham_assert(st ? page == NULL : 1, (0));
                ham_assert(!st ? page != NULL : 1, (0));
                if (st)
                    break;
                page_addr = page_get_self(page);
                st = db_free_page(page, DB_MOVE_TO_FREELIST);
                if (st)
                    break;
                /* are we there yet? */
            } while (page_addr < address);
        }
    }

    ham_log_mark_db_expansion_end(env);

    return st;
}






