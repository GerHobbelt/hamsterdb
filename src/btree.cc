/*
<<<<<<< HEAD
 * Copyright (C) 2005-2012 Christoph Rupp (chris@crupp.de).
=======
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
>>>>>>> flash-bang-grenade
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

/**
<<<<<<< HEAD
=======
* @cond ham_internals
*/

/**
>>>>>>> flash-bang-grenade
 * @brief implementation of btree.h
 *
 */

<<<<<<< HEAD
#include "config.h"

#include <string.h>

#include "btree.h"
#include "db.h"
#include "env.h"
#include "error.h"
#include "extkeys.h"
#include "btree_key.h"
#include "mem.h"
#include "page.h"
#include "txn.h"
#include "cursor.h"
=======
#include "internal_preparation.h"

#include "btree.h"
#include "btree_classic.h"


>>>>>>> flash-bang-grenade


/**
 * perform a binary search for the *smallest* element, which is >= the
 * key
 */
ham_status_t
<<<<<<< HEAD
btree_get_slot(Database *db, Page *page,
                ham_key_t *key, ham_s32_t *slot, int *pcmp)
{
    int cmp = -1;
    btree_node_t *node = page_get_btree_node(page);
=======
btree_get_slot(common_btree_datums_t *btdata, const ham_page_t *page,
        ham_key_t *key, ham_s32_t *slot, int *pcmp)
{
    int cmp = -1;
    btree_node_t *node = ham_page_get_btree_node(page);
>>>>>>> flash-bang-grenade
    ham_s32_t r = btree_node_get_count(node)-1;
    ham_s32_t l = 1;
    ham_s32_t i;
    ham_s32_t last = MAX_KEYS_PER_NODE + 1;

<<<<<<< HEAD
    ham_assert(btree_node_get_count(node)>0, ("node is empty"));

    /* only one element in this node?  */
    if (r==0) {
        cmp=btree_compare_keys(db, page, key, 0);
        if (cmp < -1)
            return (ham_status_t)cmp;
        *slot=cmp<0 ? -1 : 0;
        goto bail;
    }

    for (;;) {
=======
    ham_assert(btree_node_get_count(node) > 0, ("node is empty"));

    /*
     * only one element in this node?
     */
    if (r == 0)
    {
        cmp = key_compare_pub_to_int(btdata, page, key, 0);
        if (cmp < -1)
            return (ham_status_t)cmp;
        *slot = (cmp < 0 ? -1 : 0);
        goto bail;
    }

    for (;;)
    {
        /* [i_a] compare is not needed     (while (r>=0)) */

>>>>>>> flash-bang-grenade
        /* get the median item; if it's identical with the "last" item,
         * we've found the slot */
        i=(l+r)/2;

<<<<<<< HEAD
        if (i==last) {
            *slot=i;
            cmp=1;
            ham_assert(i>=0, (0));
            ham_assert(i<(int)MAX_KEYS_PER_NODE+1, (0));
            break;
        }
        
        /* compare it against the key */
        cmp=btree_compare_keys(db, page, key, (ham_u16_t)i);
=======
        if (i == last)
        {
            *slot = i;
            cmp = 1;
            ham_assert(i >= 0, (0));
            ham_assert(i < MAX_KEYS_PER_NODE + 1, (0));
            break;
        }

        /* compare it against the key */
        cmp=key_compare_pub_to_int(btdata, page, key, (ham_u16_t)i);
>>>>>>> flash-bang-grenade
        if (cmp < -1)
            return (ham_status_t)cmp;

        /* found it? */
<<<<<<< HEAD
        if (cmp==0) {
            *slot=i;
=======
        if (cmp == 0)
        {
            *slot = i;
>>>>>>> flash-bang-grenade
            break;
        }

        /* if the key is bigger than the item: search "to the left" */
<<<<<<< HEAD
        if (cmp<0) {
            if (r==0) {
                ham_assert(i==0, (0));
                *slot=-1;
                break;
            }
            r=i-1;
        }
        else {
            last=i;
            l=i+1;
        }
    }
    
bail:
    if (pcmp)
        *pcmp=cmp;

    return (0);
}

ham_size_t
btree_calc_maxkeys(ham_size_t pagesize, ham_u16_t keysize)
{
    ham_size_t p, k, max;
=======
        if (cmp < 0)
        {
            if (r == 0)
            {
                ham_assert(i == 0, (0));
                *slot = -1;
                break;
            }
            r = i-1;
        }
        else
        {
            last = i;
            l = i+1;
        }
    }

bail:
    if (pcmp /* && *slot!=-1 */)
    {
        /*
           [i_a] reduced the total number of key comparisons; this one is not
                 needed any more, as it was only really required to
                 compensate for the (i==last) conditional jump above.
                 So we can simply use 'cmp' as-is.
        */
        *pcmp = cmp;
    }

    return HAM_SUCCESS;
}

ham_size_t
btree_calc_maxkeys(ham_u16_t dam, ham_size_t pagesize, ham_u32_t flags, ham_u16_t *keysize_ref, ham_bool_t *should_have_fastindex_ref)
{
    ham_size_t p;
    ham_size_t k;
    ham_size_t max;
    ham_u16_t keysize = (keysize_ref ? *keysize_ref : 1);
    ham_bool_t fast_index = (should_have_fastindex_ref ? *should_have_fastindex_ref : HAM_FALSE);
>>>>>>> flash-bang-grenade

    /*
     * a btree page is always P bytes long, where P is the pagesize of
     * the database.
     */
<<<<<<< HEAD
    p=pagesize;

    /* every btree page has a header where we can't store entries */
    p-=OFFSETOF(btree_node_t, _entries);

    /* every page has a header where we can't store entries */
    p-=Page::sizeof_persistent_header;

    /* compute the size of a key, k.  */
    k=keysize+db_get_int_key_header_size();
=======
    p = pagesize;

    /* every btree page has a header where we can't store entries */
    p -= OFFSETOF(btree_node_t, _entries);

    /* every page has a header where we can't store entries */
    p -= page_get_persistent_header_size();

    /*
     * compute the size of a key, k.
     */
    k = keysize + db_get_int_key_header_size() + (fast_index ? sizeof(ham_u16_t) : 0);
>>>>>>> flash-bang-grenade

    /*
     * make sure that MAX is an even number, otherwise we can't calculate
     * MIN (which is MAX/2)
     */
<<<<<<< HEAD
    max=p/k;
    return (max&1 ? max-1 : max);
=======
    max = p / k;
    max = (max & 1 ? max-1 : max);

    if (fast_index)
    {
        ham_size_t index_width = max * sizeof(ham_u16_t);

        /* round the size of the index indirection array to ensure 8-byte alignment of the keys that follow it: */
        index_width += 7;
        index_width &= ~(8-1);
        p -= index_width;
        k -= sizeof(ham_u16_t);

        /* recalculate the maximum number of keys that fit in the page: we may be off due to the alignment requirement of the indirection array: */
        max = p / k;
        max = (max & 1 ? max-1 : max);
    }

    /*
     * Now that we know how many keys we can store in the page, advise the caller about the
     * optimum (~ maximum) keysize for that setting: after all, it's a shame to waste
     * those storage bytes. ;-)
     */
    if (max > 0)
    {
        k = p / max;

        /*
         * and make sure the advised keysize produces the desired int_key_t alignment at 8 bytes:
         */
        k &= ~(8 - 1);
    }

    k -= db_get_int_key_header_size();

    /*
     * Determine whether we would benefit from having a 'fast index' using a simple heuristic:
     *
     * When we can expect to memmove() a lot of keys in a page on insert/delete, a 'fast index'
     * is advised. When you've specified that you expect 'sequential insert' to happen, we also
     * assume you won't be deleting a lot of 'random' keys either and then we won't be
     * memmove()ing a lot of key data anyway, so a 'fast index' would then only slow us down,
     * thanks to the extra level of indirection it would introduce.
     */
    if ((max >= 16 || k >= 512) && !fast_index && (dam & HAM_DAM_SEQUENTIAL_INSERT) == 0)
    {
        fast_index = HAM_TRUE;
    }

    if (should_have_fastindex_ref)
        *should_have_fastindex_ref = fast_index;

    if (keysize_ref)
        *keysize_ref = k;

    return max;
>>>>>>> flash-bang-grenade
}

/**
 * estimate the number of keys per page, given the keysize
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_calc_keycount_per_page(ham_btree_t *be, ham_size_t *maxkeys,
                ham_u16_t keysize)
{
    Database *db=be_get_db(be);

    if (keysize == 0) {
        *maxkeys=btree_get_maxkeys(be);
    }
    else {
        /* prevent overflow - maxkeys only has 16 bit! */
        *maxkeys=btree_calc_maxkeys(db->get_env()->get_pagesize(), keysize);
        if (*maxkeys>MAX_KEYS_PER_NODE) {
            ham_trace(("keysize/pagesize ratio too high"));
            return (HAM_INV_KEYSIZE);
        }
        else if (*maxkeys==0) {
            ham_trace(("keysize too large for the current pagesize"));
            return (HAM_INV_KEYSIZE);
        }
    }

    return (0);
=======
 *
 * @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_calc_keycount_per_page(ham_btree_t *be, ham_size_t *maxkeys, ham_u16_t keysize)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);

    if (keysize == 0)
    {
        *maxkeys = btree_get_maxkeys(be);
    }
    else
    {
        /*
         * prevent overflow - maxkeys only has 16 bit!
         */
        *maxkeys=btree_calc_maxkeys(db_get_data_access_mode(db), env_get_pagesize(env), db_get_rt_flags(db), &keysize, NULL);
        if (*maxkeys > MAX_KEYS_PER_NODE) {
            ham_trace(("pagesize/keysize ratio too high"));
            return HAM_INV_KEYSIZE;
        }
        else if (*maxkeys == 0) {
            ham_trace(("keysize too large for the current pagesize"));
            return HAM_INV_KEYSIZE;
        }
    }

    return HAM_SUCCESS;
>>>>>>> flash-bang-grenade
}

/**
 * create and initialize a new backend
 *
<<<<<<< HEAD
 * @remark this function is called after the @a Database structure
=======
 * @remark this function is called after the @a ham_db_t structure
>>>>>>> flash-bang-grenade
 * and the file were created
 *
 * the @a flags are stored in the database; only transfer
 * the persistent flags!
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_create(ham_btree_t *be, ham_u16_t keysize, ham_u32_t flags)
{
    ham_status_t st;
    Page *root;
    ham_size_t maxkeys;
    Database *db=be_get_db(be);
    db_indexdata_t *indexdata=db->get_env()->get_indexdata_ptr(
                                db->get_indexdata_offset());
    if (be_is_active(be)) {
        ham_trace(("backend has alread been initialized before!"));
        return (HAM_ALREADY_INITIALIZED);
    }

    /* prevent overflow - maxkeys only has 16 bit! */
    maxkeys=btree_calc_maxkeys(db->get_env()->get_pagesize(), keysize);
    if (maxkeys>MAX_KEYS_PER_NODE) {
        ham_trace(("keysize/pagesize ratio too high"));
        return (HAM_INV_KEYSIZE);
    }
    else if (maxkeys==0) {
        ham_trace(("keysize too large for the current pagesize"));
        return (HAM_INV_KEYSIZE);
    }

    /* allocate a new root page */
    st=db_alloc_page(&root, db, Page::TYPE_B_ROOT, PAGE_IGNORE_FREELIST);
    ham_assert(st ? root == NULL : 1, (0));
    ham_assert(!st ? root != NULL : 1, (0));
    if (!root)
        return (st ? st : HAM_INTERNAL_ERROR);

    memset(root->get_raw_payload(), 0,
            sizeof(btree_node_t)+sizeof(page_data_t));
    root->set_type(Page::TYPE_B_ROOT);

    /*
     * calculate the maximum number of keys for this page,
     * and make sure that this number is even
     */
=======
 *
 * @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_create(ham_btree_t *be, ham_u16_t keysize, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *root;
    ham_size_t maxkeys;
    ham_u16_t advised_keysize = keysize;
    dev_alloc_request_info_ex_t info = {0};
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    db_indexdata_t *indexdata=env_get_indexdata_ptr(env,
                                db_get_indexdata_offset(db));

    if (be_is_active(be))
    {
        ham_trace(("backend has alread been initialized before!"));
        /* HAM_INTERNAL_ERROR -- not really, when keeping custom
         * backends in mind */
        return HAM_ALREADY_INITIALIZED;
    }

    /*
     * calculate the maximum number of keys for this page
     *
     * prevent overflow - maxkeys only has 16 bit!
     */
    maxkeys=btree_calc_maxkeys(db_get_data_access_mode(db), env_get_pagesize(env), flags, &advised_keysize, NULL);
    if (maxkeys > MAX_KEYS_PER_NODE) {
        ham_trace(("keysize/pagesize ratio too high"));
        return HAM_INV_KEYSIZE;
    }
    else if (maxkeys==0) {
        ham_trace(("keysize too large for the current pagesize"));
        return HAM_INV_KEYSIZE;
    }

    /*
     * allocate a new root page
     */
    info.db = db;
    info.env = env;
    info.dam = db_get_data_access_mode(db);
    info.entire_page = HAM_TRUE;
    info.space_type = PAGE_TYPE_B_ROOT;
    st=db_alloc_page(&root, PAGE_IGNORE_FREELIST, &info);
    ham_assert(st ? root == NULL : 1, (0));
    ham_assert(!st ? root != NULL : 1, (0));
    if (!root)
        return st ? st : HAM_INTERNAL_ERROR;

    /* BUGFIX: this destroyed the persisted page header too; that part is cleared inside db_alloc_page() */
    memset(page_get_payload(root), 0, sizeof(btree_node_t));
    ham_assert(page_get_owner(root) == db, (0));

>>>>>>> flash-bang-grenade
    btree_set_maxkeys(be, (ham_u16_t)maxkeys);
    be_set_dirty(be, HAM_TRUE);
    be_set_keysize(be, keysize);
    be_set_flags(be, flags);

<<<<<<< HEAD
    btree_set_rootpage(be, root->get_self());
=======
    btree_set_rootpage(be, page_get_self(root));
>>>>>>> flash-bang-grenade

    index_clear_reserved(indexdata);
    index_set_max_keys(indexdata, (ham_u16_t)maxkeys);
    index_set_keysize(indexdata, keysize);
<<<<<<< HEAD
    index_set_self(indexdata, root->get_self());
    index_set_flags(indexdata, flags);
    index_set_recno(indexdata, 0);
    index_clear_reserved(indexdata);

    db->get_env()->set_dirty(true);
    be_set_active(be, HAM_TRUE);

    return (0);
=======
    index_set_self(indexdata, page_get_self(root));
    index_set_flags(indexdata, flags);
    index_set_recno(indexdata, 0);
    index_clear_reserved(indexdata);
    // if (!db_is_mgt_mode_set(db_get_data_access_mode(db),
    //                      HAM_DAM_ENFORCE_PRE110_FORMAT))

    env_set_dirty(env);

    be_set_active(be, HAM_TRUE);

    return HAM_SUCCESS;
>>>>>>> flash-bang-grenade
}

/**
 * open and initialize a backend
 *
 * @remark this function is called after the ham_db_structure
 * was allocated and the file was opened
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_open(ham_btree_t *be, ham_u32_t flags)
{
    ham_offset_t rootadd;
    ham_offset_t recno;
    ham_u16_t maxkeys;
    ham_u16_t keysize;
    Database *db=be_get_db(be);
    db_indexdata_t *indexdata=db->get_env()->get_indexdata_ptr(
                                db->get_indexdata_offset());
=======

 @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_open(ham_btree_t *be, ham_u32_t flags)
{
    ham_offset_t rootadd;
    ham_recno_t recno;
    ham_u16_t maxkeys;
    ham_u16_t keysize;
    ham_db_t * db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    db_indexdata_t *indexdata=env_get_indexdata_ptr(env,
                                    db_get_indexdata_offset(db));
>>>>>>> flash-bang-grenade

    /*
     * load root address and maxkeys (first two bytes are the
     * database name)
     */
    maxkeys = index_get_max_keys(indexdata);
    keysize = index_get_keysize(indexdata);
    rootadd = index_get_self(indexdata);
    flags = index_get_flags(indexdata);
    recno = index_get_recno(indexdata);

    btree_set_rootpage(be, rootadd);
    btree_set_maxkeys(be, maxkeys);
    be_set_keysize(be, keysize);
    be_set_flags(be, flags);
    be_set_recno(be, recno);

    be_set_active(be, HAM_TRUE);

<<<<<<< HEAD
    return (0);
=======
    return HAM_SUCCESS;
>>>>>>> flash-bang-grenade
}

/**
 * flush the backend
 *
 * @remark this function is called during ham_flush
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_flush(ham_btree_t *be)
{
    Database *db=be_get_db(be);
    db_indexdata_t *indexdata=db->get_env()->get_indexdata_ptr(
                                db->get_indexdata_offset());

    /* nothing to do if the backend was not touched */
    if (!be_is_dirty(be))
        return (0);

    index_set_max_keys(indexdata, btree_get_maxkeys(be));
    index_set_keysize(indexdata, be_get_keysize(be));
    index_set_self(indexdata, btree_get_rootpage(be));
    index_set_flags(indexdata, be_get_flags(be));
    index_set_recno(indexdata, be_get_recno(be));
    index_clear_reserved(indexdata);

    db->get_env()->set_dirty(true);
    be_set_dirty(be, HAM_FALSE);

    return (0);
=======

 @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_flush(ham_btree_t *be)
{
    ham_db_t * db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    db_indexdata_t *indexdata=env_get_indexdata_ptr(env,
                        db_get_indexdata_offset(db));

    /*
     * nothing to do if the backend was not touched
     *

     WARNING WARNING WARNING WARNING WARNING WARNING WARNING

     This check is more important than it seems and EACH AND EVERY backend
     should have it: when we work with environments, we will open a
     'dummy db' at environment create/open times
     (see ham_env_create/ham_env_open) and such 'databases' ONLY serve to
     act as a somewhat compatible conduit to fetch the header page from
     storage, after which they are closed the usual way (ham_close(db)):
     in there this function is invoked and the only way to NOT B0RK on such
     very immature 'dummy backends' is to flee through the escape hatch
     called the 'dirty bit'. Which will be NOT SET for such 'dummy db's
     as they do not 'edit' (i.e. 'dirty') the storage content.
     */
    if (!be_is_dirty(be))
        return HAM_SUCCESS;

    /*
     * store root address and maxkeys (first two bytes are the
     * database name) -- if changed!
     */
    if(index_get_max_keys(indexdata) != btree_get_maxkeys(be)
        || index_get_keysize(indexdata) != be_get_keysize(be)
        || index_get_self(indexdata) != btree_get_rootpage(be)
        || index_get_flags(indexdata) != be_get_flags(be)
        || index_get_recno(indexdata) != be_get_recno(be)
#if 0
        || indexdata->b._reserved1 != 0
        || indexdata->b._reserved2 != 0
#endif
        )
    {
        index_set_max_keys(indexdata, btree_get_maxkeys(be));
        index_set_keysize(indexdata, be_get_keysize(be));
        index_set_self(indexdata, btree_get_rootpage(be));
        index_set_flags(indexdata, be_get_flags(be));
        index_set_recno(indexdata, be_get_recno(be));
        // if (!db_is_mgt_mode_set(db_get_data_access_mode(db),
        //                        HAM_DAM_ENFORCE_PRE110_FORMAT))
        index_clear_reserved(indexdata);

        env_set_dirty(env);
    }

    be_set_dirty(be, HAM_FALSE);

    return HAM_SUCCESS;
>>>>>>> flash-bang-grenade
}

/**
 * close the backend
 *
 * @remark this function is called before the file is closed
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_close(ham_btree_t *be)
{
    ham_status_t st;
    Allocator *alloc=be_get_db(be)->get_env()->get_allocator();

    /* only flush the backend info if it's dirty */
    st=btree_fun_flush(be);
=======

 @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_close(ham_btree_t *be)
{
    ham_status_t st;
    ham_env_t *env=db_get_env(be_get_db(be));
    int i;

    /*
     * just flush the backend info if it's dirty
     */
    st = my_fun_flush(be);
>>>>>>> flash-bang-grenade

    /* even when an error occurred, the backend has now been de-activated */
    be_set_active(be, HAM_FALSE);

<<<<<<< HEAD
    if (btree_get_keydata1(be)) {
        alloc->free(btree_get_keydata1(be));
        btree_set_keydata1(be, 0);
    }
    if (btree_get_keydata2(be)) {
        alloc->free(btree_get_keydata2(be));
        btree_set_keydata2(be, 0);
    }

    return (st);
=======
    /* free allocated storage */
    for (i = 0; i < sizeof(be->_keydata) / sizeof(be->_keydata[0]); i++)
    {
        ham_backend_key_cmp_cache_elem_t *key_cache_elem = be_get_keydata(be, i);

        if (key_cache_elem->data_ptr)
        {
            allocator_free(env_get_allocator(env), key_cache_elem->data_ptr);
            key_cache_elem->data_ptr = NULL;
            key_cache_elem->alloc_size = 0;
        }
    }

    return st;
>>>>>>> flash-bang-grenade
}

/**
 * free all allocated resources
 *
 * @remark this function is called after _fun_close()
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_delete(ham_btree_t *be)
{
    /* nothing to do */
    return (HAM_SUCCESS);
}

=======

 @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_delete(ham_btree_t *be)
{
    /*
     * nothing to do
     */
    return HAM_SUCCESS;
}


/**
* search the btree structures for a record
*
* @remark this function returns HAM_SUCCESS and returns
* the record ID @a rid, if the @a key was found; otherwise
* an error code is returned
*
* @remark this function is exported through the backend structure.
*/
static ham_status_t
my_fun_find(ham_btree_t *be, ham_key_t *key, ham_record_t *record, ham_u32_t flags)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        key,
        record,
        NULL,
        flags,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {flags, flags, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    ham_assert(key, ("invalid parameter"));

    return btree_find_cursor(&btdata, key, record);
}


/**
* insert (or update) a key in the index
*
* the backend is responsible for inserting or updating the
* record. (see blob.h for blob management functions)

@note This is a B+-tree 'backend' method.
*/
static ham_status_t
my_fun_insert(ham_btree_t *be, ham_key_t *key, ham_record_t *record, ham_u32_t flags)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        key,
        record,
        NULL,
        flags,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {flags, flags, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    return btree_insert_cursor(&btdata, key, record);
}


static ham_status_t
my_fun_erase(ham_btree_t *be, ham_key_t *key, ham_u32_t flags)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        key,
        NULL,
        NULL,
        flags,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {flags, flags, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    return btree_erase_cursor(&btdata, key);
}

static ham_status_t
my_fun_enumerate(ham_btree_t *be, ham_enumerate_cb_t cb, void *context)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        NULL,
        NULL,
        0,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {0, 0, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    return btree_enumerate(&btdata, cb, context);
}

static ham_status_t
my_fun_check_integrity(ham_btree_t *be)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        NULL,
        NULL,
        0,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {0, 0, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    return btree_check_integrity(&btdata);
}



/**
 Create a new cursor instance.

 @note This is a B+-tree 'backend' method.
*/
static ham_status_t
my_fun_cursor_create(ham_btree_t *be, ham_db_t *db, ham_txn_t *txn, ham_u32_t flags, ham_cursor_t **cu)
{
    return bt_cursor_create(db, txn, flags, (ham_bt_cursor_t **)cu);
}


>>>>>>> flash-bang-grenade
/**
 * uncouple all cursors from a page
 *
 * @remark this is called whenever the page is deleted or
 * becoming invalid
<<<<<<< HEAD
 */
static ham_status_t
btree_fun_uncouple_all_cursors(ham_btree_t *be, Page *page,
                    ham_size_t start)
{
    return (btree_uncouple_all_cursors(page, start));
}

/**
 * Close (and free) all cursors related to this database table.
 */
static ham_status_t
btree_fun_close_cursors(ham_btree_t *be, ham_u32_t flags)
{
    Database *db=be_get_db(be);
    ham_assert(db, (0));
    return (btree_close_cursors(db, flags));
}

/**
 * Remove all extended keys for the given @a page from the
 * extended key cache.
 */
static ham_status_t
btree_fun_free_page_extkeys(ham_btree_t *be, Page *page, ham_u32_t flags)
{
    Database *db=be_get_db(be);
    
    ham_assert(page->get_db() == db, (0));
    
    ham_assert(0 == (flags & ~DB_MOVE_TO_FREELIST), (0));

    /*
     * if this page has a header, and it's either a B-Tree root page or
     * a B-Tree index page: remove all extended keys from the cache,
     * and/or free their blobs
     */
    if (page->get_pers()
            && (!(page->get_flags()&Page::NPERS_NO_HEADER))
            && (page->get_type()==Page::TYPE_B_ROOT
                || page->get_type()==Page::TYPE_B_INDEX)) {
        ham_size_t i;
        ham_offset_t blobid;
        btree_key_t *bte;
        btree_node_t *node=page_get_btree_node(page);
        ExtKeyCache *c;

        ham_assert(db, ("Must be set as page owner when this is a Btree page"));
        ham_assert(db==page->get_db(), (""));
        c=db->get_extkey_cache();

        for (i=0; i<btree_node_get_count(node); i++) {
            bte=btree_node_get_key(db, node, i);
            if (key_get_flags(bte)&KEY_IS_EXTENDED) {
                blobid=key_get_extended_rid(db, bte);
                if (db->get_env()->get_flags()&HAM_IN_MEMORY_DB) {
                    /* delete the blobid to prevent that it's freed twice */
                    *(ham_offset_t *)(key_get_key(bte)+
                        (db_get_keysize(db)-sizeof(ham_offset_t)))=0;
                }
                if (c)
                    c->remove(blobid);
            }
=======

 @note This is a B+-tree 'backend' method.
 */
static ham_status_t
my_fun_uncouple_all_cursors(ham_btree_t *be, ham_page_t *page, ham_size_t start)
{
    return bt_uncouple_all_cursors(page, start);
}

/**
Close (and free) all cursors related to this database table.

 @note This is a B+-tree 'backend' method.
*/
static ham_status_t
my_fun_close_cursors(ham_btree_t *be, ham_u32_t flags)
{
    ham_db_t *db=be_get_db(be);

    ham_assert(db, (0));
    return btree_close_cursors(db, flags);
}


static ham_status_t
free_extkey_in_node_cb(ham_cb_enum_data_t *cb_data)
{
    int_key_t *bte;
    common_btree_datums_t *btdata = (common_btree_datums_t *)cb_data->context;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    extkey_cache_t *c;

    ham_assert(db, ("Must be set as page owner when this is a Btree page"));
    c = db_get_extkey_cache(db);

    switch (cb_data->event_code)
    {
    case ENUM_EVENT_PAGE_START:
        //context->page = param1;
        return CB_CONTINUE;

    case ENUM_EVENT_ITEM:
        bte = cb_data->key;

        if (key_get_flags(bte) & KEY_IS_EXTENDED)
        {
            ham_offset_t blobid = key_get_extended_rid(db, bte);

            if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
            {
                /* delete the blobid to prevent that it's freed twice */
                key_set_extended_rid(db, bte, 0);
            }
            (void)key_erase_record(db, bte, 0, BLOB_FREE_ALL_DUPES);
            (void)extkey_cache_remove(c, blobid);
        }
        return CB_CONTINUE;

    default:
        return CB_CONTINUE;
    }
}


/**
 * Remove all extended keys for the given @a page from the
 * extended key cache.
 *
 * This method will be invoked when a B-tree index page is removed from storage: see @ref db_free_page().
 */
static ham_status_t
my_fun_free_page_extkeys(ham_btree_t *be, ham_page_t *page, ham_u32_t flags)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        NULL,
        NULL,
        flags,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {flags, flags, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };


    ham_assert(page_get_owner(page) == db, (0));

    ham_assert(0 == (flags & ~DB_MOVE_TO_FREELIST), (0));

    /*
    * if this page has a header, and it's either a B-Tree root page or
    * a B-Tree index page: remove all extended keys from the cache,
    * and/or free their blobs
    */
    if (page_get_pers(page) &&
        0 == (page_get_npers_flags(page) & PAGE_NPERS_NO_HEADER))
    {
        ham_u32_t page_type = page_get_pers_type(page);

        if (page_type == PAGE_TYPE_B_ROOT || page_type == PAGE_TYPE_B_INDEX)
        {
            extkey_cache_t *c;
            ham_cb_enum_data_t cb_data = {0};

            ham_assert(db, ("Must be set as page owner when this is a Btree page"));
            ham_assert(db == page_get_owner(page), (0));
            c = db_get_extkey_cache(db);

            cb_data.page = page;
            cb_data.context = &btdata;
            ham_assert(cb_data.level == 0, (0));
            ham_assert(cb_data.node_count == 0, (0));
            return be->_fun_in_node_enumerate(&btdata, &cb_data, free_extkey_in_node_cb);
>>>>>>> flash-bang-grenade
        }
    }

    return (HAM_SUCCESS);
}

<<<<<<< HEAD
ham_backend_t *
btree_create(Database *db, ham_u32_t flags)
{
    ham_btree_t *btree=(ham_btree_t *)
                    db->get_env()->get_allocator()->calloc(sizeof(*btree));
    if (!btree)
        return (0);

    /* initialize the backend */
    btree->_db=db;
    btree->_fun_create=btree_fun_create;
    btree->_fun_open=btree_fun_open;
    btree->_fun_close=btree_fun_close;
    btree->_fun_flush=btree_fun_flush;
    btree->_fun_delete=btree_fun_delete;
    btree->_fun_find=btree_find;
    btree->_fun_insert=btree_insert;
    btree->_fun_erase=btree_erase;
    btree->_fun_enumerate=btree_enumerate;
    btree->_fun_check_integrity=btree_check_integrity;
    btree->_fun_calc_keycount_per_page=btree_fun_calc_keycount_per_page;
    btree->_fun_close_cursors=btree_fun_close_cursors;
    btree->_fun_uncouple_all_cursors=btree_fun_uncouple_all_cursors;
    btree->_fun_free_page_extkeys=btree_fun_free_page_extkeys;

    return ((ham_backend_t *)btree);
}

ham_status_t
btree_traverse_tree(Page **page_ref, ham_s32_t *idxptr,
                    Database *db, Page *page, ham_key_t *key)
{
    ham_status_t st;
    ham_s32_t slot;
    btree_key_t *bte;
    btree_node_t *node=page_get_btree_node(page);
=======

static void
my_fun_nuke_statistics(ham_btree_t *be, ham_page_t *page, ham_u32_t reason)
{
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        NULL,
        NULL,
        0,
        be_get_keysize(be),
        btree_get_maxkeys(be),
        be_get_keysize(be) + db_get_int_key_header_size(),
        has_fast_index,
        0,
        OFFSETOF(btree_node_t, _entries),
        OFFSETOF(btree_node_t, _entries)
        + (has_fast_index
        ? btree_get_maxkeys(be) * sizeof(ham_u16_t)
        : 0),
        {0, 0, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    stats_page_is_nuked(&btdata, page, reason);
}



#if 0
static ham_status_t
my_fun_in_node_shift_to_page(common_btree_datums_t *btdata, ham_page_t *dst_page, ham_u16_t dst_slot, ham_page_t *src_page, ham_u16_t src_slot, ham_u16_t count)
{
    int_key_t *bte_dst;
    int_key_t *bte_src;

    btree_node_t *dst_node = ham_page_get_btree_node(dst_page);
    btree_node_t *src_node = ham_page_get_btree_node(src_page);
    const ham_size_t dstnode_keycount = btree_node_get_count(dst_node);
    ham_size_t srcnode_keycount = btree_node_get_count(src_node);

    const ham_size_t keywidth = btdata->keywidth;
    ham_btree_t * const be = btdata->be;
    common_hints_t * const hints = &btdata->hints;

    ham_assert(count > 0, (0));

    /* shift right side of DST, if any, 'count' positions outward: make room for the new ones */
    ham_assert(dstnode_keycount >= dst_slot, (0));
    if (dstnode_keycount > dst_slot)
    {
        bte_dst = btree_in_node_get_key_ref(btdata, dst_page, dst_slot);
        bte_src = btree_in_node_get_key_ref(btdata, dst_page, dst_slot + count);

        memmove(bte_dst, bte_src, keywidth * (dstnode_keycount - dst_slot));
    }

    /* move count keys into dst */
    bte_dst = btree_in_node_get_key_ref(btdata, dst_page, dst_slot);
    bte_src = btree_in_node_get_key_ref(btdata, src_page, src_slot);

    memmove(bte_dst, bte_src, keywidth * count);

    /* shift right side of SRC, if any, inward == remove the 'count' keys in SRC */
    ham_assert(srcnode_keycount >= count, (0));
    srcnode_keycount -= count;
    if (srcnode_keycount > 0)
    {
        bte_dst = btree_in_node_get_key_ref(btdata, src_page, src_slot);
        bte_src = btree_in_node_get_key_ref(btdata, src_page, src_slot + count);

        memmove(bte_dst, bte_src, keywidth * srcnode_keycount);
    }

    btree_node_set_count(dst_node, dstnode_keycount + count);
    btree_node_set_count(src_node, srcnode_keycount);

    return HAM_SUCCESS;
}
#endif




ham_status_t
btree_create(ham_backend_t **backend_ref, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    ham_btree_t *btree;
    ham_env_t *env = db_get_env(db);

    *backend_ref = 0;

    btree = (ham_btree_t *)allocator_calloc(env_get_allocator(env), sizeof(*btree));
    if (!btree)
    {
        return HAM_OUT_OF_MEMORY;
    }

    /* initialize the backend */
    btree->_db=db;
    btree->_fun_create=my_fun_create;
    btree->_fun_open=my_fun_open;
    btree->_fun_close=my_fun_close;
    btree->_fun_flush=my_fun_flush;
    btree->_fun_delete=my_fun_delete;
    btree->_fun_find=my_fun_find;
    btree->_fun_insert=my_fun_insert;
    btree->_fun_erase=my_fun_erase;
    btree->_fun_enumerate=my_fun_enumerate;
    btree->_fun_check_integrity=my_fun_check_integrity;
    btree->_fun_calc_keycount_per_page=my_fun_calc_keycount_per_page;
    btree->_fun_cursor_create = my_fun_cursor_create;
    btree->_fun_close_cursors = my_fun_close_cursors;
    btree->_fun_uncouple_all_cursors = my_fun_uncouple_all_cursors;
    btree->_fun_free_page_extkeys = my_fun_free_page_extkeys;
    btree->_fun_nuke_statistics = my_fun_nuke_statistics;

    //btree->_fun_in_node_get_key_ref = btree_in_node_get_key_ref;
    //btree->_fun_in_node_shift_to_page = my_fun_in_node_shift_to_page;
    btree->_fun_in_node_enumerate = btree_in_node_enumerate;
    //btree->_fun_in_node_verify_keys = btree_in_node_verify_keys;

    *backend_ref = (ham_backend_t *)btree;
    return HAM_SUCCESS;
}

ham_status_t
btree_traverse_tree(ham_page_t **page_ref, ham_s32_t *idxptr,
                    common_btree_datums_t *btdata, ham_page_t *page, ham_key_t *key)
{
    ham_status_t st;
    ham_s32_t slot;
    int_key_t *bte;
    btree_node_t *node = ham_page_get_btree_node(page);

    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
>>>>>>> flash-bang-grenade

    /*
     * make sure that we're not in a leaf page, and that the
     * page is not empty
     */
    ham_assert(btree_node_get_count(node)>0, (0));
<<<<<<< HEAD
    ham_assert(btree_node_get_ptr_left(node)!=0, (0));

    st=btree_get_slot(db, page, key, &slot, 0);
    if (st) {
        *page_ref = 0;
        return (st);
    }

    if (idxptr)
        *idxptr=slot;

    if (slot==-1)
        return (db_fetch_page(page_ref, db, btree_node_get_ptr_left(node), 0));
    else {
        bte=btree_node_get_key(db, node, slot);
        ham_assert(key_get_flags(bte)==0 || key_get_flags(bte)==KEY_IS_EXTENDED,
                ("invalid key flags 0x%x", key_get_flags(bte)));
        return (db_fetch_page(page_ref, db, key_get_ptr(bte), 0));
=======
    ham_assert(!btree_node_is_leaf(node), (0));

    st = btree_get_slot(btdata, page, key, &slot, 0);
    if (st)
    {
        *page_ref = 0;
        return st;
    }

    if (idxptr)
        *idxptr = slot;

    if (slot == -1)
    {
        st = db_fetch_page(page_ref, env, btree_node_get_ptr_left(node), 0);
        ham_assert(st ? *page_ref == NULL : *page_ref != NULL, (0));
        if (!*page_ref)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(*page_ref, db);
        return HAM_SUCCESS;
    }
    else
    {
        ham_assert(slot >= 0, (0));
        ham_assert(slot < btree_node_get_count(node), (0));
        bte = btree_in_node_get_key_ref(btdata, page, (ham_u16_t)slot);
        ham_assert(key_get_flags(bte) == 0 ||
                key_get_flags(bte) == KEY_IS_EXTENDED,
                ("invalid key flags 0x%x", key_get_flags(bte)));
        st = db_fetch_page(page_ref, env, key_get_ptr(bte), 0);
        ham_assert(st ? *page_ref == NULL : *page_ref != NULL, (0));
        if (!*page_ref)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(*page_ref, db);
        return HAM_SUCCESS;
>>>>>>> flash-bang-grenade
    }
}

ham_s32_t
<<<<<<< HEAD
btree_node_search_by_key(Database *db, Page *page, ham_key_t *key,
                    ham_u32_t flags)
{
    int cmp;
    ham_s32_t slot;
    ham_status_t st;
    btree_node_t *node=page_get_btree_node(page);
=======
btree_node_search_by_key(common_btree_datums_t *btdata, const ham_page_t *page, ham_key_t *key,
                    ham_u32_t flags)
{
    int cmp; /* [i_a] */
    ham_s32_t slot;
    ham_status_t st;
    const btree_node_t *node = ham_page_get_btree_node(page);
>>>>>>> flash-bang-grenade

    /* ensure the approx flag is NOT set by anyone yet */
    ham_key_set_intflags(key, ham_key_get_intflags(key) & ~KEY_IS_APPROXIMATE);

    if (btree_node_get_count(node)==0)
        return (-1);

<<<<<<< HEAD
    st=btree_get_slot(db, page, key, &slot, &cmp);
    if (st) {
        ham_assert(st < -1, (0));
        return (st);
=======
    st=btree_get_slot(btdata, page, key, &slot, &cmp);
    if (st) {
        ham_assert(st < -1, (0));
        return st;
>>>>>>> flash-bang-grenade
    }

    /*
       'approximate matching'

        When we get here and cmp != 0 and we're looking for LT/GT/LEQ/GEQ
        key matches, this is where we need to do our prep work.

        Yes, due to the flag tweak in a caller when we have (the usual)
        multi-page DB table B+tree, both LT and GT flags are 'ON' here,
        but let's not get carried way and assume that is always
        like that. To elaborate a bit here: it may seem like doing something
        simple the hard way, but in here, we do NOT know if there are
        adjacent pages, so 'edge cases' like the scenarios 1, 2, and 5 below
        should NOT return an error KEY_NOT_FOUND but instead produce a
        valid slot AND, most important, the accompanying 'sign' (LT/GT) flags
        for that slot, so that the outer call can analyze our response and
        shift the key index into the left or right adjacent page, when such
        is available. We CANNOT see that here, so we always should work with
        both LT+GT enabled here.
        And to make matters a wee bit more complex still: the one exception
        to the above is when we have a single-page table: then we get
        the actual GT/LT flags in here, as we're SURE there won't be any
        left or right neighbour pages for us to shift into when the need
        arrises.

        Anyway, the purpose of the next section is to see if we have a
        matching 'approximate' key AND feed the 'sign' (i.e. LT(-1) or
        GT(+1)) back to the caller, who knows _exactly_ what the
        user asked for and can thus take the proper action there.

        Here, we are only concerned about determining which key index we
        should produce, IFF we should produce a matching key.

        Assume the following page layout, with two keys (values 2 and 4):

      * index:
      *    [0]   [1]
      * +-+---+-+---+-+
      * | | 2 | | 4 | |
      * +-+---+-+---+-+

        Various scenarios apply. For the key search (key ~ 1) i.e. (key=1,
        flags=NEAR), we get this:

        cmp = -1;
        slot = -1;

        hence we point here:

      *  |
      *  V
      * +-+---+-+---+-+
      * | | 2 | | 4 | |
      * +-+---+-+---+-+

        which is not a valid spot. Should we return a key? YES, since no key
        is less than '1', but there exists a key '2' which fits as NEAR allows
        for both LT and GT. Hence, this should be modified to become

        slot=0
        sign=GT

      *     | ( slot++ )
      *     V
      * +-+---+-+---+-+
      * | | 2 | | 4 | |
      * +-+---+-+---+-+


        Second scenario: key <= 1, i.e. (key=1, flags=LEQ)
        which gives us the same as above:

       cmp = -1;
       slot = -1;

        hence we point here:

      *  |
      *  V
      * +-+---+-+---+-+
      * | | 2 | | 4 | |
      * +-+---+-+---+-+

        Should we return a valid slot by adjusting? Your common sense says
        NO, but the correct answer is YES, since (a) we do not know if the
        user asked this, as _we_ see it in here as 'key ~ 1' anyway and
        we must allow the caller to adjust the slot by moving it into the
        left neighbour page -- an action we cannot do as we do not know,
        in here, whether there's more pages adjacent to this one we're
        currently looking at.

        EXCEPT... the common sense answer 'NO' is CORRECT when we have a
        single-page db table in our hands; see the remark at the top of this
        comment section; in that case, we can safely say 'NO' after all.

        Third scenario: key ~ 3
        which gives us either:

       cmp = -1;
       slot = 1;

         or

       cmp = 1;
       slot = 0;

         As we check for NEAR instead of just LT or GT, both are okay like
         that, no adjustment needed.
         All we need to do is make sure sure we pass along the proper LT/GT
         'sign' flags for outer level result processing.

<<<<<<< HEAD
      
=======

>>>>>>> flash-bang-grenade
        Fourth scenario: key < 3

        again, we get either:

       cmp = -1;
       slot = 1;

         or

       cmp = 1;
       slot = 0;

        but this time around, since we are looking for LT, we'll need to
        adjust the second result, when that happens by slot++ and sending
        the appropriate 'sign' flags.
<<<<<<< HEAD
    
=======

>>>>>>> flash-bang-grenade
      Fifth scenario: key ~ 5

        which given us:

       cmp = -1;
       slot = 1;

         hence we point here:

      *           |
      *           V
      * +-+---+-+---+-+
      * | | 2 | | 4 | |
      * +-+---+-+---+-+

        Should we return this valid slot? Yup, as long as we mention that
        it's an LT(less than) key; the caller can see that we returned the
        slot as the upper bound of this page and adjust accordingly when
        the actual query was 'key > 5' instead of 'key ~ 5' which is how we
        get to see it.
    */
    /*
      Note that we have a 'preference' for LT answers in here; IFF the user'd
        asked NEAR questions, most of the time that would give him LT answers,
        i.e. the answers to NEAR ~ LT questions -- mark the word 'most' in
        there: this is not happening when we're ending up at a page's lower
        bound.
     */
<<<<<<< HEAD
    if (cmp) {
        /*
         * When slot == -1, you're in a special situation: you do NOT know what
         * the comparison with slot[-1] delivers, because there _is_ _no_ slot
         * -1, but you _do_ know what slot[0] delivered: 'cmp' is the
         * value for that one then.
=======
    if (cmp)
    {
        /*
         When slot == -1, you're in a special situation: you do NOT know what
         the comparison with slot[-1] delivers, because there _is_ _no_ slot
         -1, but you _do_ know what slot[0] delivered: 'cmp' is the
         value for that one then.
>>>>>>> flash-bang-grenade
         */
        if (slot < 0)
            slot = 0;

        ham_assert(slot <= btree_node_get_count(node) - 1, (0));

<<<<<<< HEAD
        if (flags & HAM_FIND_LT_MATCH) {
            if (cmp < 0) {
                /* key @ slot is LARGER than the key we search for ... */
                if (slot > 0) {
                    slot--;
                    ham_key_set_intflags(key, ham_key_get_intflags(key)
                                        | KEY_IS_LT);
                    cmp = 0;
                }
                else if (flags & HAM_FIND_GT_MATCH) {
                    ham_assert(slot == 0, (0));
                    ham_key_set_intflags(key, ham_key_get_intflags(key)
                                        | KEY_IS_GT);
                    cmp = 0;
                }
            }
            else {
                /* key @ slot is SMALLER than the key we search for */
                ham_assert(cmp > 0, (0));
                ham_key_set_intflags(key, ham_key_get_intflags(key)
                                        | KEY_IS_LT);
                cmp = 0;
            }
        }
        else if (flags&HAM_FIND_GT_MATCH)   {
            /* When we get here, we're sure HAM_FIND_LT_MATCH is NOT set... */
            ham_assert(!(flags&HAM_FIND_LT_MATCH), (0));

            if (cmp < 0) {
                /* key @ slot is LARGER than the key we search for ... */
                ham_key_set_intflags(key, ham_key_get_intflags(key)
                                        | KEY_IS_GT);
=======
        if (flags & HAM_FIND_LT_MATCH)
        {
            if (cmp < 0)
            {
                /* key @ slot is LARGER than the key we search for ... */
                if (slot > 0)
                {
                    slot--;
                    ham_key_set_intflags(key, ham_key_get_intflags(key) | KEY_IS_LT);
                    cmp = 0;
                }
                else if (flags & HAM_FIND_GT_MATCH)
                {
                    ham_assert(slot == 0, (0));
                    ham_key_set_intflags(key, ham_key_get_intflags(key) | KEY_IS_GT);
                    cmp = 0;
                }
            }
            else
            {
                /* key @ slot is SMALLER than the key we search for */
                ham_assert(cmp > 0, (0));
                ham_key_set_intflags(key, ham_key_get_intflags(key) | KEY_IS_LT);
                cmp = 0;
            }
        }
        else if (flags & HAM_FIND_GT_MATCH)
        {
            /*
             When we get here, we're sure HAM_FIND_LT_MATCH is NOT set...
             */
            ham_assert(!(flags & HAM_FIND_LT_MATCH), (0));

            if (cmp < 0)
            {
                /* key @ slot is LARGER than the key we search for ... */
                ham_key_set_intflags(key, ham_key_get_intflags(key) | KEY_IS_GT);
>>>>>>> flash-bang-grenade
                cmp = 0;
            }
            else
            {
                /* key @ slot is SMALLER than the key we search for */
                ham_assert(cmp > 0, (0));
<<<<<<< HEAD
                if (slot < btree_node_get_count(node) - 1) {
                    slot++;
                    ham_key_set_intflags(key, ham_key_get_intflags(key)
                                        | KEY_IS_GT);
=======
                if (slot < btree_node_get_count(node) - 1)
                {
                    slot++;
                    ham_key_set_intflags(key, ham_key_get_intflags(key) | KEY_IS_GT);
>>>>>>> flash-bang-grenade
                    cmp = 0;
                }
            }
        }
    }

    if (cmp)
        return (-1);

    ham_assert(slot >= -1, (0));
    return (slot);
}

<<<<<<< HEAD
/**
 * Always make sure the db cursor set is released, no matter what happens.
 */
ham_status_t
btree_close_cursors(Database *db, ham_u32_t flags)
=======


/**
 Always make sure the db cursor set is released, no matter what happens.
*/
ham_status_t
btree_close_cursors(ham_db_t *db, ham_u32_t flags)
>>>>>>> flash-bang-grenade
{
    ham_status_t st = HAM_SUCCESS;
    ham_status_t st2 = HAM_SUCCESS;

<<<<<<< HEAD
    /* auto-cleanup cursors? */
    if (db->get_cursors()) {
        Cursor *c=db->get_cursors();
        while (c) {
            Cursor *next=c->get_next();
            if (flags&HAM_AUTO_CLEANUP)
                db->close_cursor(c);
            else
                c->close();
            if (st) {
=======
    /*
    * auto-cleanup cursors?
    */
    if (db_get_cursors(db))
    {
        ham_bt_cursor_t *c = (ham_bt_cursor_t *)db_get_cursors(db);
        while (c)
        {
            ham_bt_cursor_t *next = (ham_bt_cursor_t *)cursor_get_next(c);
            if (flags & HAM_AUTO_CLEANUP)
            {
                st = ham_cursor_close((ham_cursor_t *)c);
            }
            else
            {
                //st=bt_cursor_close(c);
                st = c->_fun_close(c);
            }
            if (st)
            {
>>>>>>> flash-bang-grenade
                if (st2 == 0) st2 = st;
                /* continue to try to close the other cursors, though */
            }
            c=next;
        }
<<<<<<< HEAD
        db->set_cursors(0);
    }
    
    return (st2);
}

ham_status_t
btree_prepare_key_for_compare(Database *db, int which,
                btree_key_t *src, ham_key_t *dest)
{
    ham_btree_t *be=(ham_btree_t *)db->get_backend();
    Allocator *alloc=be_get_db(be)->get_env()->get_allocator();
    void *p;

    if (!(key_get_flags(src) & KEY_IS_EXTENDED)) {
        dest->size = key_get_size(src);
        dest->data = key_get_key(src);
        dest->flags = HAM_KEY_USER_ALLOC;
        dest->_flags = key_get_flags(src);
        return (0);
    }

    dest->size = key_get_size(src);
    p = which ? btree_get_keydata2(be) : btree_get_keydata1(be);
    p = alloc->realloc(p, dest->size);
    if (which)
        btree_set_keydata2(be, p);
    else
        btree_set_keydata1(be, p);

    if (!p) {
        dest->data = 0;
        return (HAM_OUT_OF_MEMORY);
    }

    memcpy(p, key_get_key(src), db_get_keysize(db));
    dest->data    = p;
    dest->_flags |= KEY_IS_EXTENDED;
    dest->flags  |= HAM_KEY_USER_ALLOC;

    return (0);
}

int
btree_compare_keys(Database *db, Page *page,
        ham_key_t *lhs, ham_u16_t rhs_int)
{
    btree_key_t *r;
    btree_node_t *node=page_get_btree_node(page);
    ham_key_t rhs={0};
    ham_status_t st;

    ham_assert(db==page->get_db(), (0));

    r=btree_node_get_key(db, node, rhs_int);

    /* for performance reasons, we follow two branches:
     * if the key is not extended, then immediately compare it.
     * otherwise (if it's extended) use btree_prepare_key_for_compare()
     * to allocate the extended key and compare it.
     */
    if (!(key_get_flags(r)&KEY_IS_EXTENDED)) {
        rhs.size=key_get_size(r);
        rhs.data=key_get_key(r);
        rhs.flags=HAM_KEY_USER_ALLOC;
        rhs._flags=key_get_flags(r);
        return (db->compare_keys(lhs, &rhs));
    }

    /* otherwise continue for extended keys */
    st=btree_prepare_key_for_compare(db, 0, r, &rhs);
    if (st)
        return (st);

    return (db->compare_keys(lhs, &rhs));
}

ham_status_t
btree_read_key(Database *db, btree_key_t *source, ham_key_t *dest)
{
    Allocator *alloc=db->get_env()->get_allocator();

    /*
     * extended key: copy the whole key, not just the
     * overflow region!
     */
    if (key_get_flags(source)&KEY_IS_EXTENDED) {
        ham_u16_t keysize = key_get_size(source);
        ham_status_t st=db->get_extended_key(key_get_key(source),
                    keysize, key_get_flags(source),
                    dest);
        if (st) {
            if (!(dest->flags & HAM_KEY_USER_ALLOC)) {
                /*
                 * key data can be allocated in db->get_extended_key():
                 * prevent that heap memory leak
                 */
                if (dest->data && (db->get_key_allocdata()!=dest->data))
                    alloc->free(dest->data);
                dest->data=0;
            }
            return st;
        }
        ham_assert(dest->data != 0, ("invalid extended key"));

        ham_assert(sizeof(key_get_size(source)) == sizeof(keysize), (0));
        ham_assert(sizeof(dest->size) == sizeof(keysize), (0));
        ham_assert(keysize == dest->size, (0));
        ham_assert(keysize ? dest->data != 0 : 1, (0));

        if (dest->flags & HAM_KEY_USER_ALLOC) {
            ham_assert(dest->size == keysize, (0));
        }
        else {
            if (keysize) {
                (void)db->resize_key_allocdata(0);
                db->set_key_allocdata(dest->data);
                db->set_key_allocsize(keysize);
            }
            else {
                dest->data=0;
            }
        }
    }
    else {
        /*
         * otherwise (non-extended key)...
         */
        ham_u16_t keysize = key_get_size(source);

        ham_assert(sizeof(key_get_size(source)) == sizeof(keysize), (0));
        if (keysize) {
            if (dest->flags & HAM_KEY_USER_ALLOC) {
                memcpy(dest->data, key_get_key(source), keysize);
            }
            else {
                if (keysize>db->get_key_allocsize()) {
                    ham_status_t st=db->resize_key_allocdata(keysize);
                    if (st)
                        return (st);
                    else
                        db->set_key_allocsize(keysize);
                }
                dest->data = db->get_key_allocdata();
                memcpy(dest->data, key_get_key(source), keysize);
            }
        }
        else {
            if (!(dest->flags & HAM_KEY_USER_ALLOC)) {
                dest->data=0;
            }
        }

        ham_assert(sizeof(dest->size) == sizeof(keysize), (0));
        dest->size = keysize;
    }

    /*
     * recno databases: recno is stored in db-endian!
     */
    if (db->get_rt_flags()&HAM_RECORD_NUMBER) {
        ham_u64_t recno;
        ham_assert(dest->data!=0, ("this should never happen."));
        ham_assert(dest->size==sizeof(ham_u64_t), (0));
        if (dest->data==0 || dest->size!=sizeof(ham_u64_t))
            return (HAM_INTERNAL_ERROR);
        recno=*(ham_u64_t *)dest->data;
        recno=ham_db2h64(recno);
        memcpy(dest->data, &recno, sizeof(ham_u64_t));
    }

    return (HAM_SUCCESS);
}

ham_status_t
btree_read_record(Database *db, ham_record_t *record, ham_u64_t *ridptr,
            ham_u32_t flags)
{
    ham_status_t st;
    ham_bool_t noblob=HAM_FALSE;
    ham_size_t blobsize;

    /*
     * if this key has duplicates: fetch the duplicate entry
     */
    if (record->_intflags&KEY_HAS_DUPLICATES) {
        dupe_entry_t entry;
        ham_status_t st=blob_duplicate_get(db->get_env(), record->_rid,
                                            0, &entry);
        if (st)
            return st;
        record->_intflags=dupe_entry_get_flags(&entry);
        record->_rid     =dupe_entry_get_rid(&entry);
        /* ridptr must not point to entry._rid because it's on the stack! */
        ridptr           =&record->_rid;
    }

    /*
     * if the record size is small enough there's
     * no blob available, but the data is stored compressed in the record's
     * offset.
     */
    if (record->_intflags&KEY_BLOB_SIZE_TINY) {
        /* the highest byte of the record id is the size of the blob */
        char *p=(char *)ridptr;
        blobsize = p[sizeof(ham_offset_t)-1];
        noblob=HAM_TRUE;
    }
    else if (record->_intflags&KEY_BLOB_SIZE_SMALL) {
        /* record size is sizeof(ham_offset_t) */
        blobsize = sizeof(ham_offset_t);
        noblob=HAM_TRUE;
    }
    else if (record->_intflags&KEY_BLOB_SIZE_EMPTY) {
        /* record size is 0 */
        blobsize = 0;
        noblob=HAM_TRUE;
    }
    else {
        /* set to a dummy value, so the third if-branch is executed */
        blobsize = 0xffffffff;
    }

    if (noblob && blobsize == 0) {
        record->size = 0;
        record->data = 0;
    }
    else if (noblob && blobsize > 0) {
        if (flags&HAM_PARTIAL) {
            ham_trace(("flag HAM_PARTIAL is not allowed if record->size "
                        "<= 8"));
            return (HAM_INV_PARAMETER);
        }

        if (!(record->flags&HAM_RECORD_USER_ALLOC)
                && (flags&HAM_DIRECT_ACCESS)) {
            record->data=ridptr;
            record->size=blobsize;
        }
        else {
            if (!(record->flags&HAM_RECORD_USER_ALLOC)) {
                st=db->resize_record_allocdata(blobsize);
                if (st)
                    return (st);
                record->data = db->get_record_allocdata();
            }
            memcpy(record->data, ridptr, blobsize);
            record->size=blobsize;
        }
    }
    else if (!noblob && blobsize != 0) {
        return (blob_read(db, record->_rid, record, flags));
    }

    return (HAM_SUCCESS);
}

ham_status_t
btree_copy_key_int2pub(Database *db, const btree_key_t *source, ham_key_t *dest)
{
    Allocator *alloc=db->get_env()->get_allocator();

    /*
     * extended key: copy the whole key
     */
    if (key_get_flags(source)&KEY_IS_EXTENDED) {
        ham_status_t st=db->get_extended_key((ham_u8_t *)key_get_key(source),
                    key_get_size(source), key_get_flags(source), dest);
        if (st) {
            return st;
        }
        ham_assert(dest->data!=0, ("invalid extended key"));
        /* dest->size is set by db->get_extended_key() */
        ham_assert(dest->size==key_get_size(source), (0));
    }
    else if (key_get_size(source)) {
        if (!(dest->flags & HAM_KEY_USER_ALLOC)) {
            if (!dest->data || dest->size < key_get_size(source)) {
                if (dest->data)
                    alloc->free(dest->data);
                dest->data = (ham_u8_t *)alloc->alloc(key_get_size(source));
                if (!dest->data)
                    return HAM_OUT_OF_MEMORY;
            }
        }

        memcpy(dest->data, key_get_key(source), key_get_size(source));
        dest->size=key_get_size(source);
    }
    else {
        /* key.size is 0 */
        if (!(dest->flags & HAM_KEY_USER_ALLOC)) {
            if (dest->data)
                alloc->free(dest->data);
            dest->data=0;
        }
        dest->size=0;
        dest->data=0;
    }

    dest->flags=0;

    return (HAM_SUCCESS);
}
=======
        db_set_cursors(db, 0);
    }

    return st2;
}



/**
* @endcond
*/
>>>>>>> flash-bang-grenade

