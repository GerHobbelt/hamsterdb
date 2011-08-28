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

/**
* @cond ham_internals
*/

#include "internal_preparation.h"


#include "btree.h" /* btree_create() method */
#include "btree_classic.h"
#include "cuckoo_hash.h"



#if 0 /* [GerH] moved to cache.c */
#define PURGE_THRESHOLD  (500 * 1024 * 1024) /* 500 mb */
#endif





ham_status_t
db_uncouple_all_cursors(ham_page_t *page, ham_size_t start)
{
    ham_cursor_t *c=page_get_cursors(page);

    if (c)
    {
        ham_db_t *db = cursor_get_db(c);
        if (db)
        {
            ham_backend_t *be = db_get_backend(db);

            if (be)
            {
                return (*be->_fun_uncouple_all_cursors)(be, page, start);
            }
        }
    }

    return HAM_SUCCESS;
}


ham_u16_t
db_get_dbname(ham_db_t *db)
{
    ham_env_t *env;

    ham_assert(db!=0, (""));
    ham_assert(db_get_env(db), (""));

    env=db_get_env(db);

    if (env_get_header_page(env) && page_get_pers(env_get_header_page(env))) {
        db_indexdata_t *idx=env_get_indexdata_ptr(env,
            db_get_indexdata_offset(db));
        return (index_get_dbname(idx));
    }

    return (0);
}

int HAM_CALLCONV
db_default_prefix_compare(ham_db_t *db,
                   const ham_u8_t *lhs, ham_size_t lhs_length,
                   ham_size_t lhs_real_length,
                   const ham_u8_t *rhs, ham_size_t rhs_length,
                   ham_size_t rhs_real_length)
{
    int m;

    (void)db;

    /*
     * the default compare uses memcmp
     *
     * treat shorter strings as "higher"

    when one of the keys is NOT extended we don't need to request the other (extended) key:
    shorter is "higher" anyhow, and one of 'em is short enough to know already. Two scenarios
    here: the simple one and the little less simple one:

    1) when the key lengths differ, one of them is NOT extended for sure.

    2) when one of the key lengths equals their real_length, that one is NOT extended.

    Saves fetching extended keys whenever possible.

    The few extra comparisons in here outweigh the overhead of fetching one extended key by far.


    Note:

    there's a 'tiny' caveat to it all: often these comparisons are between a database key
    (int_key_t based) and a user-specified key (ham_key_t based), where the latter will always
    appear in the LHS and the important part here is: ham_key_t-based comparisons will have
    their key lengths possibly LARGER than the usual 'short' int_key_t key length as the
    ham_key_t data doesn't need extending - the result is that simply looking at the lhs_length
    is not good enough here to ensure the key is actually shorter than the other.
     */
    if (lhs_length < rhs_length)
    {
        m=memcmp(lhs, rhs, lhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);
        //ham_assert(lhs_real_length < rhs_real_length, (0));

        /* scenario (2) check: */
        if (lhs_length == lhs_real_length) {
            ham_assert(lhs_real_length < rhs_real_length, (0));
            return (-1);
        }
    }
    else if (rhs_length < lhs_length)
    {
        m=memcmp(lhs, rhs, rhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);
        //ham_assert(lhs_real_length > rhs_real_length, (0));

        /* scenario (2) check: */
        if (rhs_length == rhs_real_length) {
            ham_assert(lhs_real_length > rhs_real_length, (0));
            return (+1);
        }
    }
    else
    {
        m=memcmp(lhs, rhs, lhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);

        /* scenario (2) check: */
        if (lhs_length == lhs_real_length) {
            if (lhs_real_length < rhs_real_length)
                return (-1);
        }
        else if (rhs_length == rhs_real_length) {
            if (lhs_real_length > rhs_real_length)
                return (+1);
        }
    }

    return (HAM_PREFIX_REQUEST_FULLKEY);
}

int HAM_CALLCONV
db_default_compare(ham_db_t *db,
                   const ham_u8_t *lhs, ham_size_t lhs_length,
                   const ham_u8_t *rhs, ham_size_t rhs_length)
{
    int m;

    (void)db;

    /*
     * the default compare uses memcmp
     *
     * treat shorter strings as "higher"
     */
    if (lhs_length<rhs_length) {
        m=memcmp(lhs, rhs, lhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);
        return (-1);
    }
    else if (rhs_length<lhs_length) {
        m=memcmp(lhs, rhs, rhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);
        return (+1);
    }

    m=memcmp(lhs, rhs, lhs_length);
    if (m<0)
        return (-1);
    if (m>0)
        return (+1);
    return (0);
}

int HAM_CALLCONV
db_default_recno_compare(ham_db_t *db,
                   const ham_u8_t *lhs, ham_size_t lhs_length,
                   const ham_u8_t *rhs, ham_size_t rhs_length)
{
    ham_pers_recno_t ulhs_pers, urhs_pers;
    ham_offset_t ulhs, urhs;

    (void)db;

    ham_assert(sizeof(ulhs_pers) == 9, (0));
    memcpy(&ulhs_pers, lhs, 8);
    memcpy(&urhs_pers, rhs, 8);

    ulhs=ham_db2h_recno(ulhs_pers);
    urhs=ham_db2h_recno(urhs_pers);

    if (ulhs < urhs)
        return -1;
    if (ulhs == urhs)
        return 0;
    return 1;
}


ham_status_t HAM_CALLCONV
db_default_recno_normalize(ham_db_t *db,
        const ham_lexorder_assist_t *assist,
        const ham_u8_t *src, ham_size_t src_length,
        ham_u8_t *dst, ham_size_t dst_max_length,
        ham_bool_t normalize)
{
    if (dst != src)
    {
        ham_assert(src_length <= dst_max_length, (0));
        memcpy(dst, src, src_length);
    }
    return HAM_SUCCESS;
}

int HAM_CALLCONV
db_default_dupe_compare(ham_db_t *db,
                  const ham_u8_t *lhs, ham_size_t lhs_length,
                  const ham_u8_t *rhs, ham_size_t rhs_length)
{
    int m;

    (void)db;

    /*
    * the default compare uses memcmp
    *
    * treat shorter strings as "higher"
    */
    if (lhs_length<rhs_length) {
        m=memcmp(lhs, rhs, lhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);
        return (-1);
    }
    else if (rhs_length<lhs_length) {
        m=memcmp(lhs, rhs, rhs_length);
        if (m<0)
            return (-1);
        if (m>0)
            return (+1);
        return (+1);
    }

    m=memcmp(lhs, rhs, lhs_length);
    if (m < 0)
        return (-1);
    if (m > 0)
        return (+1);
    return (0);
}

ham_status_t
db_get_extended_key(ham_db_t *db, const ham_u8_t *key_data,
                    ham_size_t key_length, ham_u32_t key_flags,
                    ham_key_t *ext_key, const ham_page_t *refering_index_page)
{
    ham_pers_rid_t rid;
    ham_offset_t blobid;
    ham_status_t st;
    ham_size_t temp;
    ham_record_t record;
    ham_u8_t *ptr;
    ham_env_t *env = db_get_env(db);
    ham_backend_t *be = db_get_backend(db);
    ham_size_t maxkeylen4ext_key = be_get_keysize(be)-sizeof(ham_pers_rid_t);

    //ext_key->data = 0;

    ham_assert(key_flags&KEY_IS_EXTENDED, ("key is not extended"));

    if (!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
    {
        /*
         * make sure that we have an extended key-cache
         *
         * in in-memory-db, the extkey-cache doesn't lead to performance
         * advantages; it only duplicates the data and wastes memory.
         * therefore we don't use it.
         */
        if (!db_get_extkey_cache(db)) {
            extkey_cache_t *c=extkey_cache_new(db);

            db_set_extkey_cache(db, c);
            if (!c)
                return HAM_OUT_OF_MEMORY;
        }
    }

    /* effectively the same as: blobid = key_get_extended_rid(db, key); */
    memcpy(&rid, key_data + maxkeylen4ext_key, sizeof(rid));
    blobid = ham_db2h_rid(rid);

    /* fetch from the cache */
    if (!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB)) {
        st=extkey_cache_fetch(db_get_extkey_cache(db), blobid,
                        &temp, &ptr);
        if (!st) {
            ham_assert(temp==key_length, ("invalid key length"));

            if (!(ext_key->flags & HAM_KEY_USER_ALLOC)) {
                if (!ext_key->data || ext_key->size < key_length)
                {
                    if (ext_key->data)
                        allocator_free(env_get_allocator(env), ext_key->data);
                    ext_key->data = (ham_u8_t *)allocator_alloc(env_get_allocator(env), key_length);
                    if (!ext_key->data)
                    {
                        return HAM_OUT_OF_MEMORY;
                    }
                }
            }
            else if (ext_key->size < key_length)
            {
                ham_assert(ext_key->flags & HAM_KEY_USER_ALLOC, (0));
                return HAM_KEYSIZE_TOO_SMALL;
            }
            memcpy(ext_key->data, ptr, key_length);
            ext_key->size=(ham_u16_t)key_length;
            return HAM_SUCCESS;
        }
        else if (st!=HAM_KEY_NOT_FOUND)
        {
            return st;
        }
    }

    /*
     * not cached - fetch from disk;
     * we allocate the memory here to avoid that the global record
     * pointer is overwritten

     Note that the key is fetched in two parts: we already have the front part of
     the key in key_data and now we only need to fetch the blob remainder, which size
     is:
             key_length - maxkeylen4ext_key

     To prevent another round of memcpy and heap allocation here, we simply allocate
     sufficient space for the entire key as it should be passed back through (*ext_key)
     and adjust the pointer into that memory space for the faked record-based blob_read()
     below.
     */
    if (!(ext_key->flags & HAM_KEY_USER_ALLOC))
    {
        if (!ext_key->data || ext_key->size < key_length)
        {
            if (ext_key->data)
                allocator_free(env_get_allocator(env), ext_key->data);
            ext_key->data = (ham_u8_t *)allocator_alloc(env_get_allocator(env), key_length);
            if (!ext_key->data)
            {
                return HAM_OUT_OF_MEMORY;
            }
        }
    }
    else if (ext_key->size < key_length)
    {
        ham_assert(ext_key->flags & HAM_KEY_USER_ALLOC, (0));
        return HAM_KEYSIZE_TOO_SMALL;
    }

    memcpy(ext_key->data, key_data, maxkeylen4ext_key); // no need for overlapped copy support here

    /*
     * now read the remainder of the key
     */
    memset(&record, 0, sizeof(record));
    record.data = (((ham_u8_t *)ext_key->data) + maxkeylen4ext_key);
    record.size = key_length - maxkeylen4ext_key;
    record.flags = HAM_RECORD_USER_ALLOC;

    st=blob_read(db, blobid, &record, 0);
    if (st) {
        return st;
    }

    /*
     * insert the FULL key in the extkey-cache
     */
    if (db_get_extkey_cache(db))
    {
        st = extkey_cache_insert(db_get_extkey_cache(db),
                blobid, key_length, (const ham_u8_t *)ext_key->data, refering_index_page);
        if (st)
            return st;
    }

    ext_key->size = (ham_u16_t)key_length;

	ham_nuke_stack_space(rid);
	ham_nuke_stack_space(record);
    return HAM_SUCCESS;
}


ham_status_t
db_prepare_ham_key_for_compare(ham_db_t *db, int left_or_right, const int_key_t *src, ham_key_t *dest)
{
    ham_env_t *env = db_get_env(db);
    ham_backend_t *be = db_get_backend(db);
    mem_allocator_t *alloc = env_get_allocator(env);
    ham_backend_key_cmp_cache_elem_t *key_cache_elem;
    void *p;

    if (!(key_get_flags(src) & KEY_IS_EXTENDED))
    {
        dest->size = key_get_size(src);
        dest->data = (void *)key_get_key(src);  // ignore the 'const' from src: we 'know' it'll be all right anyhow...
        dest->flags = HAM_KEY_USER_ALLOC;
        ham_key_set_intflags(dest, key_get_flags(src));

        return HAM_SUCCESS;
    }

    dest->size = key_get_size(src);

    key_cache_elem = be_get_keydata(be, left_or_right);
    /*
    Do NOT invoke allocator_realloc() as it will include a (possibly large) memmove operation
    to move old data into the (larger) newly allocated space, while we are definitely NOT
    interested in such antiquated data here anyhow.

    Hence we free() + malloc() a new suitable key space when we discover that the new key is
    too large for the existing cached space.
    */
    ham_assert(key_cache_elem->data_ptr ? key_cache_elem->alloc_size > 0 : 1, (0));
    ham_assert(!key_cache_elem->data_ptr ? key_cache_elem->alloc_size == 0 : 1, (0));
    if (dest->size > key_cache_elem->alloc_size)
    {
        allocator_free(alloc, key_cache_elem->data_ptr);
        key_cache_elem->data_ptr = allocator_alloc(alloc, dest->size);
        key_cache_elem->alloc_size = dest->size;
    }
    p = key_cache_elem->data_ptr;
    if (!p)
    {
        key_cache_elem->alloc_size = 0;
        dest->data = 0;
        return HAM_OUT_OF_MEMORY;
    }
    ham_assert(be_get_keysize(be) < dest->size, (0));
    memcpy(p, key_get_key(src), be_get_keysize(be));
    dest->data = p;
    dest->flags = 0; /* [i_a] important; discard the USER_ALLOC present */

    /* set a flag that this memory has to be freed by hamsterdb */
    //dest->_flags |= KEY_IS_ALLOCATED;
    dest->_flags |= KEY_IS_EXTENDED;
    // dest->flags  |= HAM_KEY_USER_ALLOC; // GerH: you're now using two different flags for the same thing: whether the rec is allocated by hamsterdb or the user. The whole KEY_IS_ALLOCATED bit is not required. Yes, that requires a few tweaks and tugs elsewhere in the code as this is not rigorously checked in vanilla hamsterdb.

    return HAM_SUCCESS;
}



int
db_compare_keys(ham_db_t *db, ham_key_t *lhs, ham_key_t *rhs, const ham_page_t *lhs_refering_index_page, const ham_page_t *rhs_refering_index_page)
{
    ham_backend_t *be = db_get_backend(db);

    int cmp=HAM_PREFIX_REQUEST_FULLKEY;
    ham_compare_func_t foo=db_get_compare_func(db);
    ham_prefix_compare_func_t prefoo=db_get_prefix_compare_func(db);

    //db_set_error(db, 0);

    /*
     * need prefix compare? if no key is extended we can just call the
     * normal compare function
     */
    if (!(ham_key_get_intflags(lhs) & KEY_IS_EXTENDED)
        && !(ham_key_get_intflags(rhs) & KEY_IS_EXTENDED))
    {
        /*
         * no!
         */
        return (foo(db, (const ham_u8_t *)lhs->data, lhs->size, (const ham_u8_t *)rhs->data, rhs->size));
    }

    /*
     * yes! - run prefix comparison, but only if we have a prefix
     * comparison function
     */
    if (prefoo) {
        ham_size_t lhsprefixlen;
        ham_size_t rhsprefixlen;
        ham_size_t maxkeylen4ext_key = be_get_keysize(be)-sizeof(ham_pers_rid_t);

        if (ham_key_get_intflags(lhs) & KEY_IS_EXTENDED)
            lhsprefixlen=maxkeylen4ext_key;
        else
            lhsprefixlen=lhs->size;

        if (ham_key_get_intflags(rhs) & KEY_IS_EXTENDED)
            rhsprefixlen=maxkeylen4ext_key;
        else
            rhsprefixlen=rhs->size;

        cmp=prefoo(db, (const ham_u8_t *)lhs->data, lhsprefixlen, lhs->size,
                    (const ham_u8_t *)rhs->data, rhsprefixlen, rhs->size);
        if (cmp < -1 && cmp != HAM_PREFIX_REQUEST_FULLKEY)
            return cmp; /* unexpected error! */
    }

    if (cmp==HAM_PREFIX_REQUEST_FULLKEY)
    {
        ham_status_t st;

        /*
         * 1. load the first key, if needed
         */
        if (ham_key_get_intflags(lhs) & KEY_IS_EXTENDED)
        {
            st = db_get_extended_key(db, (const ham_u8_t *)lhs->data,
                    lhs->size, ham_key_get_intflags(lhs),
                    lhs, lhs_refering_index_page);
            if (st)
            {
                ham_assert(st < -1, (0));
                return st;
            }
        }

        /*
         * 2. load the second key, if needed
         */
        if (ham_key_get_intflags(rhs) & KEY_IS_EXTENDED)
        {
            st = db_get_extended_key(db, (const ham_u8_t *)rhs->data,
                    rhs->size, ham_key_get_intflags(rhs),
                    rhs, rhs_refering_index_page);
            if (st)
            {
                ham_assert(st < -1, (0));
                return st;
            }
        }

        /*
         * 3. run the comparison function
         */
        cmp=foo(db, (const ham_u8_t *)lhs->data, lhs->size, (const ham_u8_t *)rhs->data, rhs->size);
    }

    return (cmp);
}

ham_status_t
db_create_backend(ham_backend_t **backend_ref, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    *backend_ref = 0;

    /*
     * the default backend is the BTREE
     *
     * create a ham_backend_t with the size of a ham_btree_t
     */
    switch (flags & HAM_USE_DB_ALGO_MASK)
    {
    default:
    case HAM_USE_BTREE:
        return btree_create(backend_ref, db, flags, param);

    case HAM_USE_HASH:
        return spiral_hashtable_create(backend_ref, db, flags, param);

    case HAM_USE_CUSTOM_DB_ALGO:
        /*
        provide expandability of the database engine: register a custom
        database 'backend', possibly through passing along a special
        PARAM when creating/opening the db, or calling a register API
        attaching the custom backend to the specified ham_db_t.

        This way, an arbitrary number of custom backends can be used,
        which enables hamster use in special environments and for database
        research testbeds.
        */
        ham_assert(!"Should not get here YET", (0));
        return HAM_INV_PARAMETER;
    }
}


ham_status_t
db_free_page(ham_page_t *page, ham_u32_t flags)
{
    ham_status_t st;
    ham_device_t *dev = page_get_device(page);
    ham_env_t *env = device_get_env(dev);

    ham_assert(page_get_owner(page) ? device_get_env(dev) == db_get_env(page_get_owner(page)) : 1, (0));

    ham_assert(0 == (flags & ~DB_MOVE_TO_FREELIST), (0));

    st = db_uncouple_all_cursors(page, 0);
    if (st)
        return (st);

    if (env_get_cache(env))
    {
        st = cache_remove_page(env_get_cache(env), page);
        if (st)
            return (st);
    }

    /*
     * if this page has a header, and it's either a B-Tree root page or
     * a B-Tree index page: remove all extended keys from the cache,
     * and/or free their blobs
     */
    if (page_get_pers(page) &&
        !(page_get_npers_flags(page) & PAGE_NPERS_NO_HEADER))
    {
        ham_u32_t page_type = page_get_pers_type(page);

        if (page_type == PAGE_TYPE_B_ROOT || page_type == PAGE_TYPE_B_INDEX)
        {
            ham_db_t * const db = page_get_owner(page);
            ham_backend_t *be;

            ham_assert(db, ("Must be set as page owner when this is a Btree page"));
            be = db_get_backend(db);
            ham_assert(be, (0));

            st = be->_fun_free_page_extkeys(be, page, flags);
            if (st)
                return (st);
        }
    }

    /*
     * move the page to the freelist
     */
    if (flags & DB_MOVE_TO_FREELIST)
    {
        if (!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
        {
            st = freel_mark_free(env, page_get_self(page),
                    env_get_pagesize(env), HAM_TRUE);
            if (st)
                return st;
        }
    }

    /*
     * free the page; since it's deleted, we don't need to flush it
     */
    page_set_undirty(page);
    (void)page_free(page);
    (void)page_delete(page);

    return (HAM_SUCCESS);
}

ham_status_t
db_alloc_page(ham_page_t **page_ref, ham_u32_t flags,
              dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_status_t st;
    ham_offset_t tellpos=0;
    ham_page_t *page = NULL;

    ham_env_t * const env = extra_dev_alloc_info->env;
    ham_cache_t * const cache = env_get_cache(env);

    *page_ref = 0;
    ham_assert(0 == (flags & ~(PAGE_IGNORE_FREELIST
                        | PAGE_CLEAR_WITH_ZERO
                        | PAGE_DONT_LOG_CONTENT
                        | DB_NEW_PAGE_DOES_THRASH_CACHE)), (0));
    ham_assert(env_get_cache(env) != 0,
            ("This code will not realize the requested page may already exist "
             "through a previous call to this function or db_fetch_page() "
             "unless page caching is available!"));

    /* freel_alloc_page() can set an error and we want to detect
     * that unambiguously */
    //ham_assert(db_get_error(db) == 0, (0));


    /*
     * purge the cache, if necessary. if cache is unlimited, then we purge very
     * very rarely (but we nevertheless purge to avoid OUT OF MEMORY conditions
     * which can happen on Win32 (and elsewhere))
     */
    if (cache
            && !(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
    {
        //if (cache_get_cur_elements(cache)*env_get_pagesize(env)
        //        > PURGE_THRESHOLD)
        // taken care of inside env_purge_cache() itself!
        ham_size_t target = 0;
        ham_size_t cur = cache_get_cur_elements(cache);
        ham_size_t max = cache_get_max_elements(cache);
        ham_assert(max > 0, (0));

        if (cur >= max / 2)
        {
            if (env_get_rt_flags(env) & HAM_CACHE_UNLIMITED)
            {
                target = ham_log16(cur);
            }
            else
            {
                if (cur > max)
                {
                    /* try to purge all 'too many' pages plus 1/16th of the rest. This is a continuation of the 'cur <= max' case below. */
                    target = cur - max + max / 16;
                }
                else
                {
                    target = max - cur / 16;
                    ham_assert(target > 0, (0));
                    target = max / target;
                    ham_assert(target > 0, (0));
                }
            }
        }

        st = env_purge_cache(env, target);
        if (st)
            return st;
    }

    /* first, we ask the freelist for a page */
    if (!(flags & PAGE_IGNORE_FREELIST))
    {
        st = freel_alloc_page(&tellpos, extra_dev_alloc_info);
        ham_assert(st ? !tellpos : 1, (0));
        if (tellpos)
        {
            ham_assert(tellpos % env_get_pagesize(env) == 0,
                    ("page id %llu is not aligned", tellpos));
            /* try to fetch the page from the txn */
            if (env_get_txn(env))
			{
                page=txn_get_page(env_get_txn(env), tellpos);
                if (page)
                    goto done;
            }
            /* try to fetch the page from the cache */
            if (cache)
			{
                page = cache_get_page(cache, tellpos, 0);
                if (page)
                    goto done;
            }
            /* allocate a new page structure */
            ham_assert(!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
                    ? 1
                    : !!cache,
                    ("in-memory DBs MUST have a cache"));
            page=page_new(env);
            if (!page)
                return HAM_OUT_OF_MEMORY;
            page_set_self(page, tellpos);
            /* fetch the page from disk */
            st=page_fetch(page, env_get_pagesize(env));
            if (st) {
                page_delete(page);
                return st;
            }
            goto done;
        }
        else if (st)
        {
            return st;
        }
    }

    if (!page) {
        page=page_new(env);
        if (!page)
            return HAM_OUT_OF_MEMORY;
    }

    ham_assert(tellpos == 0, (0));
    st=page_alloc(page, env_get_pagesize(env), extra_dev_alloc_info);
    if (st)
        return st;

    if (env_get_txn(env))
        page_set_alloc_txn_id(page, txn_get_id(env_get_txn(env)));

    /*
     * update statistics for the freelist: we'll need to mark this one
     * down as allocated, so the statistics are up-to-date.
     *
     * This is done further below.
     */

done:
    ham_assert(!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
            ? 1
            : !!env_get_cache(env), ("in-memory DBs MUST have a cache"));

    /*
     * disable page content logging ONLY when the page is
     * completely new (contains bogus 'before' data)
    */
    if (tellpos == 0) /* [i_a] BUG! */
    {
        flags &= ~PAGE_DONT_LOG_CONTENT;
    }

    /*
     * As we re-purpose a page, we will reset its page counter as
     * well to signal its first use as the new type assigned here.
     *
     * only do this if the page is reused - otherwise page_get_pers_type()
     * accesses uninitialized memory, and valgrind complains
     */
    ham_assert(extra_dev_alloc_info, (0));
#if 0 // this is done at the end of this call...
    if (tellpos && cache && page_get_pers(page)
        && (page_get_pers_type(page) != extra_dev_alloc_info->space_type))
    {
        cache_update_page_access_counter(newroot, cache); /* bump up */
    }
#endif

    /*
    CONCERN / TO BE CHECKED

    logging or no logging, when a new page is appended to the file
    and the txn fails, that page is not removed from the file through
    the OS, yet freelist is rolled back, resulting in a 'gap' page
    which would seem allocated for ever as the freelist didn't mark
    it as used (max_bits), while subsequent page adds will append
    new pages.

    EDIT v1.1.2: this is resolved by adding database file size changes
                 to the log: aborting transactions should now abort
                 correctly by also truncating the database filesize
                 to the previously known filesize.

                 This, of course, means trouble for partitioning schemes,
                 where the database has multiple 'growth fronts': which
                 one should we truncate?

                 Answer: this is resolved in the simplest possible way:
                 new pages are flagged as such and each page is 'rewound'
                 by calling the 'shrink' device method, which will thus
                 have access to the appropriate 'rid' and any underlying
                 partitioners can take care of the routing of the resize
                 request.

                 The alternative solution: keeping the file size as is,
                 while the transaction within which it occurred, would
                 cause insurmountable trouble when the freelist is expanded
                 but at the same time the freelist needs to grow itself to
                 contain this expansion: the allocation of the freelist
                 page MAY collide with other freelist bit edits in the
                 same freelist page, so logging would not work as is. Then,
                 the option would be to 'recreate' this particular freelist
                 page allocation, but such 'regeneration' cannot be guaranteed
                 to match the original action as the transaction is rewound
                 and the freelist allocation will end up in another place.

                 Guaranteed fault scenario for regeneration: TXN START >
                 ERASE PAGE-SIZE BLOB (marks freelist section as 'free') >
                 EXPAND DB BY SEVERAL PAGES: needs to expand freelist >
                 FREELIST ALLOCS PAGE in location where the erased BLOB was.

                 Which is perfectly Okay when transaction commits, but txn
                 abort means the BLOB will exist again and the freelist
                 allocation CANNOT happen at the same spot. Subsequent
                 committing transactions would then see a b0rked freelist
                 image after regeneration-on-txn-abort.


                 WRONG! immediately after ABORT the freelist CAN be regenerated,
                 as long as those results are applied to the REWOUND database
                 AND these new edits are LOGGED after the ABORT TXN using
                 a 'derived' transaction which is COMMITTED: that TXN will only
                 contain the filesize and freelist edits then!
    */
    ham_assert(env, (0));
    if (!(flags & PAGE_DONT_LOG_CONTENT) && env_get_log(env))
    {
        st=ham_log_add_page_before(page);
        if (st)
            return st;
    }

    /* BUGFIX */
    memset(page_get_raw_payload(page), 0, page_get_persistent_header_size());

    /* this modifies the persisted part of the page; must be done AFTER writing the 'before' page image to log */
#if 0 /* old code: */
    page_set_type(page, type);
    page_set_undirty(page);
#endif
    page_set_owner(page, extra_dev_alloc_info->db);
    page_set_pers_type(page, extra_dev_alloc_info->space_type);
    page_set_dirty(page, env); /* signal write required, otherwise header edit above is lost, at least, on recovery / commit */

    /*
     * clear the page with zeroes?
     */
    if (flags & PAGE_CLEAR_WITH_ZERO)
    {
        ham_assert(((ham_u8_t *)page_get_payload(page)) - page_get_persistent_header_size() == page_get_raw_payload(page), (0));
        memset(page_get_payload(page), 0, env_get_pagesize(env) - page_get_persistent_header_size());

        /*
        BUG?

        why log this state of affairs???

        or was that to compensate for the incorrect page_set_UNdirty() earlier???
        */
        st=ham_log_add_page_after(page);
        if (st)
            return st;
    }

    if (env_get_txn(env))
	{
        st=txn_add_page(env_get_txn(env), page, HAM_TRUE);
        if (st) {
            return st;
            /* TODO memleak? */
        }
    }

    if (cache)
    {
        st=cache_put_page(cache, page);
        if (st) {
            return st;
            /* TODO memleak? */
        }
#if 0
        /*
        Some quick measurements indicate that this (and the btree lines which
        do the same: bumping the cache age of the given page) is deteriorating
        performance:

        with this it's ~ 17K refetches (reloads of previously cached pages);
        without it's ~ 16K refetches, which means the dumb version without the
        weights reloads less database pages.

        All in all, the conclusion is simple:

        Stick with the simplest cache aging system, unless we can come up with something
        truely fantastic to cut down disc I/O (which is particularly important when the
        database file is located on a remote storage disc (SAN).

        And the simplest system is...

        Count every access as one age point, i.e. age
        all pages with each cache access by 1 and dicard the oldest bugger.

        Don't bother with high/low watermarks in purging either as that didn't help
        neither.
        */
        switch (type)
        {
        default:
        case PAGE_TYPE_UNKNOWN:
        case PAGE_TYPE_HEADER:
            break;

        case PAGE_TYPE_B_ROOT:
        case PAGE_TYPE_B_INDEX:
            bump = (cache_get_max_elements(env_get_cache(env)) + 32 - 1) / 32;
            if (bump > 4)
                bump = 4;
            break;

        case PAGE_TYPE_FREELIST:
            bump = (cache_get_max_elements(env_get_cache(env)) + 8 - 1) / 8;
            break;
        }
#endif
        if (flags & DB_NEW_PAGE_DOES_THRASH_CACHE)
		{
            /* give it an 'antique' age so this one will get flushed pronto */
            page_set_cache_cntr(page, cache->_timeslot - HAM_MAX_S32 / 2);
            page_set_cache_hit_freq(page, 0);
        }
        else
		{
            cache_update_page_access_counter(page, cache);
        }
    }

    cache_check_history(env, page, -99);

    *page_ref = page;
    return HAM_SUCCESS;
}

ham_status_t
db_fetch_page(ham_page_t **page_ref, ham_env_t *env, ham_offset_t address, ham_u32_t flags)
{
    ham_page_t *page = 0;
    ham_status_t st;
    ham_cache_t * const cache = env_get_cache(env);

    ham_assert(0 == (flags & ~(DB_NEW_PAGE_DOES_THRASH_CACHE
                                | HAM_HINTS_MASK
                                | DB_ONLY_FROM_CACHE)), (0));
    ham_assert(!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
            ? 1
            : !!cache, ("in-memory DBs MUST have a cache"));
    ham_assert((env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
            ? 1
            : cache != 0,
            ("This code will not realize the requested page may already exist through"
             " a previous call to this function or db_alloc_page() unless page caching"
             " is available!"));

    *page_ref = 0;

    /*
     * check if the cache allows us to allocate another page; if not,
     * purge it
     *
     * purge the cache, if necessary. if cache is unlimited, then we purge very
     * very rarely (but we nevertheless purge to avoid OUT OF MEMORY conditions
     * which can happen on Win32 (and elsewhere))
     */
    if (!(flags & DB_ONLY_FROM_CACHE)
            && cache
            && !(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
    {
        //if (cache_get_cur_elements(cache)*env_get_pagesize(env)
        //        > PURGE_THRESHOLD)
        // taken care of inside env_purge_cache() itself!
        ham_size_t target = 0;
        ham_size_t cur = cache_get_cur_elements(cache);
        ham_size_t max = cache_get_max_elements(cache);
        ham_assert(max > 0, (0));

        if (cur >= max / 2)
        {
            if (env_get_rt_flags(env) & HAM_CACHE_UNLIMITED)
            {
                target = ham_log16(cur);
            }
            else
            {
                if (cur > max)
                {
                    /* try to purge all 'too many' pages plus 1/16th of the rest. This is a continuation of the 'cur <= max' case below. */
                    target = cur - max + max / 16;
                }
                else
                {
                    target = max - cur / 16;
                    ham_assert(target > 0, (0));
                    target = max / target;
                    ham_assert(target > 0, (0));
                }
            }
        }

        st = env_purge_cache(env, target);
        if (st)
            return st;
    }

    if (env_get_txn(env))
    {
        page=txn_get_page(env_get_txn(env), address);
        if (page)
		{
            cache_check_history(env, page, 1);
            *page_ref = page;
            ham_assert(page_get_pers(page), (0));
#if 0
            ham_assert(db ? page_get_owner(page) == db : 1, (0));
#endif
            return HAM_SUCCESS;
        }
    }

    /*
     * fetch the page from the cache
     */
    if (cache)
	{
        page = cache_get_page(cache, address, CACHE_NOREMOVE);
        if (page)
		{
            if (env_get_txn(env))
			{
                st=txn_add_page(env_get_txn(env), page, HAM_TRUE);
                if (st)
				{
                    return st;
                }
            }
            cache_check_history(env, page, 2);
            *page_ref = page;
            ham_assert(page_get_pers(page), (0));
#if 0
            ham_assert(db ? page_get_owner(page)==db : 1, (0));
#endif
            return HAM_SUCCESS;
        }
    }

    if (flags & DB_ONLY_FROM_CACHE)
        return HAM_SUCCESS;

#if HAM_DEBUG
    if (cache)
	{
        ham_assert(cache_get_page(cache, address, 0)==0, (0));
    }
#endif
    ham_assert(!(env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
            ? 1
            : !!cache, ("in-memory DBs MUST have a cache"));

    page=page_new(env);
    if (!page)
        return HAM_OUT_OF_MEMORY;

#if 0 /* [GerH] deferred to caller; hence we don't need to know about the db here */
    page_set_owner(page, db);
#endif
    page_set_self(page, address);
    st=page_fetch(page, env_get_pagesize(env));
    if (st) {
        (void)page_delete(page);
        return st;
    }

    ham_assert(page_get_pers(page), (0));

    if (env_get_txn(env))
	{
        st=txn_add_page(env_get_txn(env), page, HAM_TRUE);
        if (st) {
            (void)page_delete(page);
            return st;
        }
    }

    if (cache)
	{
        st=cache_put_page(cache, page);
        if (st)
		{
            (void)page_delete(page);
            return st;
        }
        if (flags & DB_NEW_PAGE_DOES_THRASH_CACHE)
		{
            /* give it an 'antique' age so this one will get flushed pronto */
            page_set_cache_cntr(page, cache->_timeslot - HAM_MAX_S32 / 2);
            page_set_cache_hit_freq(page, 0);
        }
        else
		{
            cache_update_page_access_counter(page, cache);
        }
    }

    cache_check_history(env, page, 3);

    *page_ref = page;
    return HAM_SUCCESS;
}

ham_status_t
db_flush_page(ham_env_t *env, ham_page_t *page, ham_u32_t flags)
{
    ham_status_t st;

    /* write the page, if it's dirty and if write-through is enabled */
    if ((env_get_rt_flags(env) & HAM_WRITE_THROUGH
         || flags & HAM_WRITE_THROUGH
         || !env_get_cache(env))
        && page_is_dirty(page))
    {
        st=page_flush(page);
        if (st)
            return (st);
    }

    /*
     * put page back into the cache; do NOT update the page_counter, as
     * this flush operation should not be considered an 'additional page
     * access' impacting the page life-time in the cache.
     */
    if (env_get_cache(env))
        return cache_put_page(env_get_cache(env), page);

    return HAM_SUCCESS;
}

ham_status_t
db_flush_all(ham_cache_t *cache, ham_u32_t flags)
{
    ham_status_t st2 = HAM_SUCCESS;
    ham_status_t st;
    ham_page_t *head;

    ham_assert(0 == (flags & ~DB_FLUSH_NODELETE), (0));

    if (!cache)
        return HAM_SUCCESS;

    head=cache_get_totallist(cache);
    while (head) {
        ham_page_t *next=page_get_next(head, PAGE_LIST_CACHED);

        /* don't touch pages which are currently in use by a transaction */
        if (page_get_refcount(head)==0)
		{
            /*
			 * don't remove the page from the cache, if flag NODELETE
             * is set (this flag is used i.e. in ham_flush())
	         */
	        if (!(flags & DB_FLUSH_NODELETE))
			{
	            ham_assert(page_get_refcount(head)==0,
	                ("page is in use, but database is closing"));
				// [i_a] Christoph's fix is better than mine; removed code here as removal-from-cache is now correctly done in db_write_page_and_delete()!
			}

	        st = db_write_page_and_delete(head, flags);
	        if (!st2) st2 = st;
		}

        head=next;
    }

    return st2;
}

ham_status_t
db_write_page_and_delete(ham_page_t *page, ham_u32_t flags)
{
    ham_status_t st;
    ham_device_t *dev = page_get_device(page);
    ham_db_t *db = page_get_owner(page);
    ham_env_t *env = device_get_env(dev);

    ham_assert(0 == (flags & ~DB_FLUSH_NODELETE), (0));

    /*
     * write page to disk if it's dirty (and if we don't have
     * an IN-MEMORY DB)
     */
    ham_assert(env, (0));
    if (page_is_dirty(page)
            && !(env_get_rt_flags(env) & HAM_IN_MEMORY_DB))
	{
        st=page_flush(page);
        if (st)
            return st;
    }

    /*
     * if the page is deleted, uncouple all cursors, then
     * free the memory of the page
     *
     * Also check the extkey cache to see whether that one needs
     * a purge by now; this is done here to reduce the number of
     * extkey cache purge invocations as purging the extkey is
     * not a cheap action.
     */
    if (!(flags & DB_FLUSH_NODELETE))
    {
        if (db && db_get_extkey_cache(db))
        {
            st = extkey_cache_purge(db_get_extkey_cache(db));
            if (st)
                return (st);
        }
        st=db_uncouple_all_cursors(page, 0);
        if (st)
            return (st);
        cache_remove_page(env_get_cache(env), page);
        st=page_free(page);
        if (st)
            return (st);
        page_delete(page);
    }

    return (HAM_SUCCESS);
}

ham_status_t
db_resize_record_allocdata(ham_db_t *db, ham_size_t size)
{
    ham_env_t *env = db_get_env(db);
    mem_allocator_t *allocator = env_get_allocator(env);

    if (size == 0)
    {
        if (db_get_record_allocdata(db))
            allocator_free(allocator, db_get_record_allocdata(db));
        db_set_record_allocdata(db, 0);
        db_set_record_allocsize(db, 0);
    }
    else if (size > db_get_record_allocsize(db))
    {
#if 0 /* huge blob test profiling --> 8.1% spent in realloc through here, while malloc is much cheaper and we don't need the content intact anyway */
		void *newdata=allocator_realloc(allocator,
                db_get_record_allocdata(db), size);
#else
		void *newdata;
		/*
		Also improve performance in race conditions (such as the blob unittests) by
		bumping the storage reserved for the record by a power curve, so that 'slow
		growth' doesn't hammer this code section.

		After all, we only allocate space for a single record here, so allocating
		a little 'too much' doesn't harm.
		*/
#if 01
		ham_size_t new_size = db_get_record_allocsize(db) * 3 / 2; // factor 1.5 instead of the more brutal 2
#else
		ham_size_t new_size = db_get_record_allocsize(db) * 2;
#endif
		if (size < new_size)
			size = new_size;

        if (db_get_record_allocdata(db))
	        allocator_free(allocator, db_get_record_allocdata(db));
		newdata = allocator_alloc(allocator, size);
#endif
        db_set_record_allocdata(db, newdata);
        db_set_record_allocsize(db, size);
        /*
        make sure the new memory pointer is set, even in error conditions,
        so that global cleanup code doesn't try to free an invalid heap pointer reference.
        */
        if (!newdata) {
            db_set_record_allocsize(db, 0);
            return (HAM_OUT_OF_MEMORY);
        }
    }

    return HAM_SUCCESS;
}

ham_status_t
db_resize_key_allocdata(ham_db_t *db, ham_size_t size)
{
    ham_env_t *env = db_get_env(db);
    mem_allocator_t *allocator = env_get_allocator(env);

    if (size == 0)
    {
        if (db_get_key_allocdata(db))
            allocator_free(allocator, db_get_key_allocdata(db));
        db_set_key_allocdata(db, 0);
        db_set_key_allocsize(db, 0);
    }
    else if (size > db_get_key_allocsize(db))
    {
        void *newdata=allocator_realloc(allocator,
                db_get_key_allocdata(db), size);
        if (!newdata)
            return (HAM_OUT_OF_MEMORY);
        db_set_key_allocdata(db, newdata);
        db_set_key_allocsize(db, size);
    }

    return HAM_SUCCESS;
}


/**
* @endcond
*/

