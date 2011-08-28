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

typedef struct free_cb_context_t
{
    ham_db_t *db;
    ham_bool_t is_leaf;

} free_cb_context_t;

/*
 * forward decl - implemented in hamsterdb.c
 */
extern ham_status_t
__check_create_parameters(ham_env_t *env, ham_db_t *db, const char *filename,
        ham_u32_t *pflags, const ham_parameter_t *param,
        ham_size_t *ppagesize, ham_u16_t *pkeysize,
        ham_size_t *pcachesize, ham_u16_t *pdbname,
        ham_u16_t *pmaxdbs, ham_u16_t *pdata_access_mode, ham_bool_t create);

/*
 * callback function for freeing blobs of an in-memory-database, implemented
 * in db.c
 */
extern ham_status_t
free_inmemory_blobs_cb(int event, void *param1, void *param2, void *context);

static ham_status_t
__purge_cache_max20(ham_env_t *env)
{
    ham_status_t st;
    ham_page_t *page;
    ham_cache_t *cache=env_get_cache(env);
    unsigned i, max_pages=cache_get_cur_elements(cache);

    /* don't remove pages from the cache if it's an in-memory database */
    if (!cache)
        return (0);
    if ((env_get_rt_flags(env)&HAM_IN_MEMORY_DB))
        return (0);
    if (!cache_too_big(cache))
        return (0);

    /*
     * max_pages specifies how many pages we try to flush in case the
     * cache is full. some benchmarks showed that 10% is a good value.
     *
     * if STRICT cache limits are enabled then purge as much as we can
     */
    if (!(env_get_rt_flags(env)&HAM_CACHE_STRICT)) {
        max_pages/=10;
        /* but still we set an upper limit to avoid IO spikes */
        if (max_pages>20)
            max_pages=20;
    }

    /* try to free 10% of the unused pages */
    for (i=0; i<max_pages; i++) {
        page=cache_get_unused_page(cache);
        if (!page) {
            if (i==0 && (env_get_rt_flags(env)&HAM_CACHE_STRICT))
                return (HAM_CACHE_FULL);
            else
                break;
        }

        st=db_write_page_and_delete(page, 0);
        if (st)
            return (st);
    }

    if (i==max_pages && max_pages!=0)
        return (HAM_LIMITS_REACHED);
    return (0);
}

ham_status_t
env_purge_cache(ham_env_t *env)
{
    ham_status_t st;
    ham_cache_t *cache=env_get_cache(env);

    /* don't purge while txn is in progress */
    if (env_get_txn(env))
        return (0);

    do {
        st=__purge_cache_max20(env);
        if (st && st!=HAM_LIMITS_REACHED)
            return st;
    } while (st==HAM_LIMITS_REACHED && cache_too_big(cache));

    return (0);
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






