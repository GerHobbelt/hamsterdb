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

/**
 * @brief btree cursors - implementation
 *
 */

#include "internal_preparation.h"

#include "btree.h"
#include "btree_classic.h"
#include "btree_cursor.h"



static ham_status_t
bt_cursor_find(ham_bt_cursor_t *c, ham_key_t *key, ham_record_t *record,
            ham_u32_t flags);


static ham_status_t
my_move_first(common_btree_datums_t *btdata, ham_bt_cursor_t *c, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *page;
    btree_node_t *node;
    //ham_db_t *db=cursor_get_db(c);
    //ham_env_t *env = db_get_env(db);

    ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;

    /*
     * get a NIL cursor
     */
    st=bt_cursor_set_to_nil(c);
    if (st)
        return (st);

    /*
    TODO

    use the hinter to get a reference to the smallest key leaf page, if that page is known.
    */




    /*
     * get the root page
     */
    if (!btree_get_rootpage(be))
        return HAM_KEY_NOT_FOUND;
    st=db_fetch_page(&page, env, btree_get_rootpage(be), 0);
    ham_assert(st ? page == NULL : page != NULL, (0));
    if (!page)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(page, db);

    /*
     * while we've not reached the leaf: pick the smallest element
     * and traverse down
     */
    for(;;) {
        node=ham_page_get_btree_node(page);
        /* check for an empty root page */
        if (btree_node_get_count(node)==0)
            return HAM_KEY_NOT_FOUND;
        /* leave the loop when we've reached the leaf page */
        if (btree_node_is_leaf(node))
            break;

        st=db_fetch_page(&page, env, btree_node_get_ptr_left(node), 0);
        ham_assert(st ? page == NULL : page != NULL, (0));
        if (!page)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(page, db);
    }

    /*
     * couple this cursor to the smallest key in this page
     */
    page_add_cursor(page, (ham_cursor_t *)c);
    bt_cursor_set_coupled_page(c, page);
    bt_cursor_set_coupled_index(c, 0);
    bt_cursor_set_flags(c,
            bt_cursor_get_flags(c)|BT_CURSOR_FLAG_COUPLED);
    bt_cursor_set_dupe_id(c, 0);

    return HAM_SUCCESS;
}

static ham_status_t
my_move_next(common_btree_datums_t *btdata, ham_bt_cursor_t *c, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *page;
    btree_node_t *node;
    //ham_db_t *db=cursor_get_db(c);
    //ham_env_t *env = db_get_env(db);
    int_key_t *entry;
    //int_key_t *keyarr;
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();

    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    /*
     * uncoupled cursor: couple it
     */
    if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED)
    {
        st=bt_cursor_couple(c);
        if (st)
            return (st);
    }
    else if (!(bt_cursor_get_flags(c)&BT_CURSOR_FLAG_COUPLED))
    {
        return (HAM_CURSOR_IS_NIL);
    }

    page = bt_cursor_get_coupled_page(c);
    node = ham_page_get_btree_node(page);
    entry = btree_in_node_get_key_ref(btdata, page, bt_cursor_get_coupled_index(c));

    /*
     * if this key has duplicates: get the next duplicate; otherwise
     * (and if there's no duplicate): fall through
     */
    if (key_get_flags(entry) & KEY_HAS_DUPLICATES
            && (!(flags & HAM_SKIP_DUPLICATES)))
    {
        ham_status_t st;
        bt_cursor_set_dupe_id(c, bt_cursor_get_dupe_id(c)+1);
        page_add_ref(page);
        st=blob_duplicate_get(env, key_get_ptr(entry),
                        bt_cursor_get_dupe_id(c),
                        bt_cursor_get_dupe_cache(c));
        page_release_ref(page);
        if (st) {
            bt_cursor_set_dupe_id(c, bt_cursor_get_dupe_id(c)-1);
            if (st!=HAM_KEY_NOT_FOUND)
                return (st);
        }
        else if (!st)
            return HAM_SUCCESS;
    }

    /*
     * don't continue if ONLY_DUPLICATES is set
     */
    if (flags&HAM_ONLY_DUPLICATES)
        return (HAM_KEY_NOT_FOUND);

    /*
     * if the index+1 is still in the coupled page, just increment the
     * index
     */
    if (bt_cursor_get_coupled_index(c) + 1 < btree_node_get_count(node))
    {
        bt_cursor_set_coupled_index(c, bt_cursor_get_coupled_index(c) + 1);
        bt_cursor_set_dupe_id(c, 0);
        return HAM_SUCCESS;
    }

    /*
     * otherwise uncouple the cursor and load the right sibling page
     */
    if (!btree_node_get_right(node))
        return (HAM_KEY_NOT_FOUND);

    page_remove_cursor(page, (ham_cursor_t *)c);
    bt_cursor_set_flags(c, bt_cursor_get_flags(c)&(~BT_CURSOR_FLAG_COUPLED));

    st=db_fetch_page(&page, env, btree_node_get_right(node), 0);
    ham_assert(st ? page == NULL : page != NULL, (0));
    if (!page)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(page, db);
    node = ham_page_get_btree_node(page);

    /*
     * couple this cursor to the smallest key in this page
     */
    page_add_cursor(page, (ham_cursor_t *)c);
    bt_cursor_set_coupled_page(c, page);
    bt_cursor_set_coupled_index(c, 0);
    bt_cursor_set_flags(c,
            bt_cursor_get_flags(c)|BT_CURSOR_FLAG_COUPLED);
    bt_cursor_set_dupe_id(c, 0);

    return HAM_SUCCESS;
}

static ham_status_t
my_move_previous(common_btree_datums_t *btdata, ham_bt_cursor_t *c, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *page;
    btree_node_t *node;
    //ham_db_t *db=cursor_get_db(c);
    //ham_env_t *env = db_get_env(db);
    int_key_t *entry;
    //int_key_t *keyarr;
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();

    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    /*
     * uncoupled cursor: couple it
     */
    if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED) {
        st=bt_cursor_couple(c);
        if (st)
            return (st);
    }
    else if (!(bt_cursor_get_flags(c)&BT_CURSOR_FLAG_COUPLED))
        return (HAM_CURSOR_IS_NIL);

    page = bt_cursor_get_coupled_page(c);
    node = ham_page_get_btree_node(page);
    entry = btree_in_node_get_key_ref(btdata, page, bt_cursor_get_coupled_index(c));

    /*
     * if this key has duplicates: get the previous duplicate; otherwise
     * (and if there's no duplicate): fall through
     */
    if (key_get_flags(entry)&KEY_HAS_DUPLICATES
            && (!(flags&HAM_SKIP_DUPLICATES))
            && bt_cursor_get_dupe_id(c)>0)
    {
        ham_status_t st;
        bt_cursor_set_dupe_id(c, bt_cursor_get_dupe_id(c)-1);
        page_add_ref(page);
        st=blob_duplicate_get(env, key_get_ptr(entry),
                        bt_cursor_get_dupe_id(c),
                        bt_cursor_get_dupe_cache(c));
        page_release_ref(page);
        if (st) {
            bt_cursor_set_dupe_id(c, bt_cursor_get_dupe_id(c)+1);
            if (st!=HAM_KEY_NOT_FOUND)
                return (st);
        }
        else if (!st)
            return HAM_SUCCESS;
    }

    /*
     * don't continue if ONLY_DUPLICATES is set
     */
    if (flags&HAM_ONLY_DUPLICATES)
        return (HAM_KEY_NOT_FOUND);

    /*
     * if the index-1 is till in the coupled page, just decrement the
     * index
     */
    if (bt_cursor_get_coupled_index(c)!=0) {
        bt_cursor_set_coupled_index(c, bt_cursor_get_coupled_index(c)-1);
        entry = btree_in_node_get_key_ref(btdata, page, bt_cursor_get_coupled_index(c));
    }
    /*
     * otherwise load the left sibling page
     */
    else {
        if (!btree_node_get_left(node))
            return HAM_KEY_NOT_FOUND;

        page_remove_cursor(page, (ham_cursor_t *)c);
        bt_cursor_set_flags(c,
                bt_cursor_get_flags(c)&(~BT_CURSOR_FLAG_COUPLED));

        st=db_fetch_page(&page, env, btree_node_get_left(node), 0);
        ham_assert(st ? page == NULL : page != NULL, (0));
        if (!page)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(page, db);
        node=ham_page_get_btree_node(page);

        /*
         * couple this cursor to the highest key in this page
         */
        page_add_cursor(page, (ham_cursor_t *)c);
        bt_cursor_set_coupled_page(c, page);
        bt_cursor_set_coupled_index(c, btree_node_get_count(node)-1);
        bt_cursor_set_flags(c,
                bt_cursor_get_flags(c)|BT_CURSOR_FLAG_COUPLED);
        entry = btree_in_node_get_key_ref(btdata, page, bt_cursor_get_coupled_index(c));
    }
    bt_cursor_set_dupe_id(c, 0);

    /*
     * if duplicates are enabled: move to the end of the duplicate-list
     */
    if (key_get_flags(entry)&KEY_HAS_DUPLICATES
            && !(flags&HAM_SKIP_DUPLICATES))
    {
        ham_size_t count;
        ham_status_t st;

        page_add_ref(page);
        st=blob_duplicate_get_count(env, key_get_ptr(entry),
                        &count, bt_cursor_get_dupe_cache(c));
        page_release_ref(page);
        if (st)
            return st;
        bt_cursor_set_dupe_id(c, count-1);
    }

    return HAM_SUCCESS;
}

static ham_status_t
my_move_last(common_btree_datums_t *btdata, ham_bt_cursor_t *c, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *page;
    btree_node_t *node;
    int_key_t *entry;
    //int_key_t *keyarr;
    //ham_db_t *db=cursor_get_db(c);
    //ham_env_t *env = db_get_env(db);

    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    /*
     * get a NIL cursor
     */
    st=bt_cursor_set_to_nil(c);
    if (st)
        return (st);

    /*
    TODO

    use the hinter to get a reference to the leaf page of the last key.
    */






    /*
     * get the root page
     */
    if (!btree_get_rootpage(be))
        return HAM_KEY_NOT_FOUND;
    st=db_fetch_page(&page, env, btree_get_rootpage(be), 0);
    ham_assert(st ? page == NULL : page != NULL, (0));
    if (!page)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(page, db);

    /*
     * while we've not reached the leaf: pick the largest element
     * and traverse down
     */
    for(;;)
    {
        int_key_t *key;

        node=ham_page_get_btree_node(page);
        /* check for an empty root page */
        if (btree_node_get_count(node)==0)
            return HAM_KEY_NOT_FOUND;
        /* leave the loop when we've reached a leaf page */
        if (btree_node_is_leaf(node))
            break;

        key = btree_in_node_get_key_ref(btdata, page, btree_node_get_count(node) - 1);

        st=db_fetch_page(&page, env, key_get_ptr(key), 0);
        ham_assert(st ? page == NULL : page != NULL, (0));
        if (!page)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(page, db);
    }

    /*
     * couple this cursor to the largest key in this page
     */
    page_add_cursor(page, (ham_cursor_t *)c);
    bt_cursor_set_coupled_page(c, page);
    bt_cursor_set_coupled_index(c, btree_node_get_count(node)-1);
    bt_cursor_set_flags(c,
            bt_cursor_get_flags(c)|BT_CURSOR_FLAG_COUPLED);
    entry = btree_in_node_get_key_ref(btdata, page, bt_cursor_get_coupled_index(c));
    bt_cursor_set_dupe_id(c, 0);

    /*
     * if duplicates are enabled: move to the end of the duplicate-list
     */
    if (key_get_flags(entry) & KEY_HAS_DUPLICATES
            && !(flags & HAM_SKIP_DUPLICATES))
    {
        ham_size_t count;
        ham_status_t st;
        page_add_ref(page);
        st=blob_duplicate_get_count(env, key_get_ptr(entry),
                        &count, bt_cursor_get_dupe_cache(c));
        page_release_ref(page);
        if (st)
            return st;
        bt_cursor_set_dupe_id(c, count-1);
    }

    return HAM_SUCCESS;
}

ham_status_t
bt_cursor_set_to_nil(ham_bt_cursor_t *c)
{
    ham_env_t *env = db_get_env(cursor_get_db(c));

    /*
     * uncoupled cursor: free the cached pointer
     */
    if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED)
    {
        ham_key_t *key = bt_cursor_get_uncoupled_key(c);
        if (key->data)
            allocator_free(env_get_allocator(env), key->data);
        allocator_free(env_get_allocator(env), key);
        bt_cursor_set_uncoupled_key(c, 0);
        bt_cursor_set_flags(c,
                bt_cursor_get_flags(c) & ~BT_CURSOR_FLAG_UNCOUPLED);
    }
    /*
     * coupled cursor: uncouple, remove from page
     */
    else if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_COUPLED)
    {
        page_remove_cursor(bt_cursor_get_coupled_page(c),
                (ham_cursor_t *)c);
        bt_cursor_set_flags(c,
                bt_cursor_get_flags(c)&(~BT_CURSOR_FLAG_COUPLED));
    }

    bt_cursor_set_dupe_id(c, 0);
    memset(bt_cursor_get_dupe_cache(c), 0, sizeof(dupe_entry_t));

    return HAM_SUCCESS;
}

ham_status_t
bt_cursor_couple(ham_bt_cursor_t *c)
{
    ham_key_t key;
    ham_status_t st;
    ham_db_t *db = cursor_get_db(c);
    ham_env_t *env = db_get_env(db);
    ham_u32_t dupe_id;

    ham_assert(env_get_txn(env) != 0,
            ("coupling a cursor without a txn object"));
    ham_assert(bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED,
            ("coupling a cursor which is not uncoupled"));

    /*
     * make a 'find' on the cached key; if we succeed, the cursor
     * is automatically coupled
     *
     * the dupe ID is overwritten in bt_cursor_find, therefore save it
     * and restore it afterwards
     */
    memset(&key, 0, sizeof(key));

    ham_assert(!(ham_key_get_intflags(bt_cursor_get_uncoupled_key(c)) & KEY_IS_EXTENDED), (0));
    st = util_copy_key(db, bt_cursor_get_uncoupled_key(c), &key, NULL);
    if (st)
    {
        if (key.data)
            allocator_free(env_get_allocator(env), key.data);
        return st;
    }

    dupe_id=bt_cursor_get_dupe_id(c);

    st=bt_cursor_find(c, &key, NULL, 0);

    bt_cursor_set_dupe_id(c, dupe_id);

    /*
     * free the cached key
     */
    if (key.data)
        allocator_free(env_get_allocator(env), key.data);

    return (st);
}

ham_status_t
bt_cursor_uncouple(ham_bt_cursor_t *c, ham_u32_t flags)
{
    ham_status_t st;
    btree_node_t *node;
    ham_page_t *page;
    int_key_t *entry;
    //int_key_t *keyarr;
    ham_key_t *key;

    ham_db_t *db=bt_cursor_get_db(c);
    ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_env_t *env = db_get_env(db);
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        NULL,
        c,
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
        {flags, flags, (ham_cursor_t *)c, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED)
        return HAM_SUCCESS;
    if (!(bt_cursor_get_flags(c) & BT_CURSOR_FLAG_COUPLED))
        return HAM_SUCCESS;

    ham_assert(bt_cursor_get_coupled_page(c) != 0,
            ("uncoupling a cursor which has no coupled page"));

    /*
     * get the btree-entry of this key
     */
    page = bt_cursor_get_coupled_page(c);
    node = ham_page_get_btree_node(page);
    ham_assert(btree_node_is_leaf(node), ("iterator points to internal node"));
    entry = btree_in_node_get_key_ref(&btdata, page, bt_cursor_get_coupled_index(c));

    /*
     * copy the key
     */
    key=(ham_key_t *)allocator_calloc(env_get_allocator(env), sizeof(*key));
    if (!key)
        return HAM_OUT_OF_MEMORY;
    st = util_copy_key_int2pub(db, entry, key, page);
    if (st)
    {
        if (key->data)
            allocator_free(env_get_allocator(env), key->data);
        allocator_free(env_get_allocator(env), key);
        return st;
    }

    /*
     * uncouple the page
     */
    if (!(flags & BT_CURSOR_UNCOUPLE_NO_REMOVE))
    {
        page_remove_cursor(page, (ham_cursor_t *)c);
    }

    /*
     * set the flags and the uncoupled key
     */
    bt_cursor_set_flags(c, bt_cursor_get_flags(c)&(~BT_CURSOR_FLAG_COUPLED));
    bt_cursor_set_flags(c, bt_cursor_get_flags(c)|BT_CURSOR_FLAG_UNCOUPLED);
    ham_assert(!(ham_key_get_intflags(key) & KEY_IS_EXTENDED), (0));
    bt_cursor_set_uncoupled_key(c, key);

    return HAM_SUCCESS;
}



/* [i_a] bt_cursor_create moved to bottom of source file so all the static scope functions are known */



/**
 * clone an existing cursor

 @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_clone(ham_bt_cursor_t *old, ham_bt_cursor_t **newc)
{
    ham_status_t st;
    ham_bt_cursor_t *c;
    ham_db_t *db=bt_cursor_get_db(old);
    ham_env_t *env = db_get_env(db);

    *newc=0;

    c=(ham_bt_cursor_t *)allocator_alloc(env_get_allocator(env), sizeof(*c));
    if (!c)
        return HAM_OUT_OF_MEMORY;
    memcpy(c, old, sizeof(*c));
    cursor_set_next_in_page(c, 0);
    cursor_set_previous_in_page(c, 0);

    /* fix the linked list of cursors */
    cursor_set_previous((ham_cursor_t *)c, 0);
    cursor_set_next((ham_cursor_t *)c, db_get_cursors(db));
    ham_assert(db_get_cursors(db)!=0, (0));
    cursor_set_previous(db_get_cursors(db), (ham_cursor_t *)c);
    db_set_cursors(db, (ham_cursor_t *)c);

    /*
     * if the old cursor is coupled: couple the new cursor, too
     */
    if (bt_cursor_get_flags(old)&BT_CURSOR_FLAG_COUPLED) {
         ham_page_t *page=bt_cursor_get_coupled_page(old);
         page_add_cursor(page, (ham_cursor_t *)c);
         bt_cursor_set_coupled_page(c, page);
    }
    else if (bt_cursor_get_flags(old)&BT_CURSOR_FLAG_UNCOUPLED)
    {
        /*
         * if the old cursor is uncoupled: copy the key
         */
        ham_key_t *key;

        key=(ham_key_t *)allocator_calloc(env_get_allocator(env), sizeof(*key));
        if (!key)
            return HAM_OUT_OF_MEMORY;

        ham_assert(!(ham_key_get_intflags(bt_cursor_get_uncoupled_key(old)) & KEY_IS_EXTENDED), (0));
        st = util_copy_key(bt_cursor_get_db(c),
            bt_cursor_get_uncoupled_key(old), key, NULL);
        if (st)
        {
            if (key->data)
                allocator_free(env_get_allocator(env), key->data);
            allocator_free(env_get_allocator(env), key);
            return st;
        }
        ham_assert(!(ham_key_get_intflags(key) & KEY_IS_EXTENDED), (0));
        bt_cursor_set_uncoupled_key(c, key);
    }

    bt_cursor_set_dupe_id(c, bt_cursor_get_dupe_id(old));

    *newc=c;

    return HAM_SUCCESS;
}

/**
 * close an existing cursor
 *
 * @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_close(ham_bt_cursor_t *c)
{
    ham_db_t *db=bt_cursor_get_db(c);

    /* fix the linked list of cursors */
    ham_cursor_t *p=(ham_cursor_t *)cursor_get_previous(c);
    ham_cursor_t *n=(ham_cursor_t *)cursor_get_next(c);

    if (p)
        cursor_set_next(p, n);
    else
        db_set_cursors(db, n);

    if (n)
        cursor_set_previous(n, p);

    cursor_set_next(c, 0);
    cursor_set_previous(c, 0);

    (void)bt_cursor_set_to_nil(c);

    return HAM_SUCCESS;
}

/**
 * overwrite the record of this cursor

 @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_overwrite(ham_bt_cursor_t *c, ham_record_t *record,
            ham_u32_t flags)
{
    ham_status_t st;
    btree_node_t *node;
    int_key_t *key;
    //int_key_t *keyarr;
    //ham_db_t *db=bt_cursor_get_db(c);
    //ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    //ham_env_t *env = db_get_env(db);
    ham_page_t *page;
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();

    ham_db_t *db=bt_cursor_get_db(c);
    ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_env_t *env = db_get_env(db);
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        record,
        c,
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
        {flags, flags, (ham_cursor_t *)c, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    /*
     * uncoupled cursor: couple it
     */
    if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED)
    {
        st=bt_cursor_couple(c);
        if (st)
            return (st);
        page = bt_cursor_get_coupled_page(c);
    }
    /*
     * coupled cursor: the coupled page must be stored in the txn!
     */
    else if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_COUPLED)
    {
        page = bt_cursor_get_coupled_page(c);
        st = txn_add_page(env_get_txn(env), page, HAM_TRUE);
        if (st)
            return (st);
    }
    else
    {
        return (HAM_CURSOR_IS_NIL);
    }

    /*
     * delete the cache of the current duplicate
     */
    memset(bt_cursor_get_dupe_cache(c), 0, sizeof(dupe_entry_t));

    /*
     * make sure that the page is not unloaded
     */
    page_add_ref(page);

    /*
     * prepare page for logging
     */
    st=ham_log_add_page_before(page);
    if (st) {
        page_release_ref(page);
        return (st);
    }

    /*
     * get the btree node entry
     */
    node = ham_page_get_btree_node(page);
    ham_assert(btree_node_is_leaf(node), ("iterator points to internal node"));
    key = btree_in_node_get_key_ref(&btdata, page, bt_cursor_get_coupled_index(c));

    /*
     * copy the key flags, and remove all flags concerning the key size
     */
    st=key_set_record(db, key, record,
            bt_cursor_get_dupe_id(c), flags|HAM_OVERWRITE, NULL);
    if (st) {
        page_release_ref(page);
        return (st);
    }

    page_set_dirty(page, env);
    page_release_ref(page);

    return HAM_SUCCESS;
}

/**
 * move the cursor
 *
 * set the cursor to the first item in the database when the cursor has not yet
 * been positioned through a previous find/move/insert/erase operation.
 *
 * @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_move(ham_bt_cursor_t *c, ham_key_t *key,
            ham_record_t *record, ham_u32_t flags)
{
    ham_status_t st = 0;
    ham_page_t *page;
    btree_node_t *node;
    //ham_db_t *db=bt_cursor_get_db(c);
    //ham_env_t *env = db_get_env(db);
    //ham_btree_t *be=(ham_btree_t *)db_get_backend(db);
    int_key_t *entry;
    //int_key_t *keyarr;
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();


    ham_db_t *db=bt_cursor_get_db(c);
    ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_env_t *env = db_get_env(db);
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        key,
        record,
        c,
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
        {flags, flags, (ham_cursor_t *)c, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    ham_assert(be, (0));

    /*
     * if the cursor is NIL, and the user requests a NEXT, we set it to FIRST;
     * if the user requests a PREVIOUS, we set it to LAST, resp.
     */
    if (bt_cursor_is_nil(c))
    {
        if (flags & HAM_CURSOR_NEXT)
        {
            flags &= ~HAM_CURSOR_NEXT;
            flags |= HAM_CURSOR_FIRST;
        }
        else if (flags & HAM_CURSOR_PREVIOUS)
        {
            flags &= ~HAM_CURSOR_PREVIOUS;
            flags |= HAM_CURSOR_LAST;
        }
    }

    /*
     * delete the cache of the current duplicate
     */
    memset(bt_cursor_get_dupe_cache(c), 0, sizeof(dupe_entry_t));

    if (flags & HAM_CURSOR_FIRST)
        st=my_move_first(&btdata, c, flags);
    else if (flags & HAM_CURSOR_LAST)
        st=my_move_last(&btdata, c, flags);
    else if (flags & HAM_CURSOR_NEXT)
        st=my_move_next(&btdata, c, flags);
    else if (flags & HAM_CURSOR_PREVIOUS)
        st=my_move_previous(&btdata, c, flags);
    /* no move, but cursor is nil? return error */
    else if (bt_cursor_is_nil(c))
    {
        if (key || record)
            return HAM_CURSOR_IS_NIL;
        else
            return HAM_SUCCESS;
    }
    /* no move, but cursor is not coupled? couple it */
    else if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED)
    {
        st=bt_cursor_couple(c);
    }

    if (st)
        return (st);

    /*
     * during util_read_key and util_read_record, new pages might be needed,
     * and the page at which we're pointing could be moved out of memory;
     * that would mean that the cursor would be uncoupled, and we're losing
     * the 'entry'-pointer. therefore we 'lock' the page by incrementing
     * the reference counter
     */
    ham_assert(bt_cursor_get_flags(c)&BT_CURSOR_FLAG_COUPLED,
            ("move: cursor is not coupled"));
    page = bt_cursor_get_coupled_page(c);
    page_add_ref(page);
    node = ham_page_get_btree_node(page);
    ham_assert(btree_node_is_leaf(node), ("iterator points to internal node"));
    entry = btree_in_node_get_key_ref(&btdata, page, bt_cursor_get_coupled_index(c));

    if (key) {
        st=util_read_key(db, entry, key, page);
        if (st) {
            page_release_ref(page);
            return (st);
        }
    }

    if (record)
    {
        ham_pers_rid_t *ridptr = 0;

        if (key_get_flags(entry) & KEY_HAS_DUPLICATES
                && bt_cursor_get_dupe_id(c))
        {
            dupe_entry_t *e = bt_cursor_get_dupe_cache(c);
            if (!dupe_entry_get_rid(e))
            {
                st=blob_duplicate_get(env, key_get_ptr(entry),
                        bt_cursor_get_dupe_id(c),
                        e);
                if (st)
                {
                    page_release_ref(page);
                    return st;
                }
            }
            ham_record_set_intflags(record, dupe_entry_get_flags(e));
            ham_record_set_rid(record, dupe_entry_get_rid(e));
            ridptr = dupe_entry_get_rid_direct_ref(e);
        }
        else
        {
            ham_record_set_intflags(record, key_get_flags(entry));
            ham_record_set_rid(record, key_get_ptr(entry));
            ridptr = key_get_ptr_direct_ref(entry);
        }
        st=util_read_record(db, record, ridptr, flags /* & HAM_DIRECT_ACCESS */ );
        if (st) {
            page_release_ref(page);
            return (st);
        }
    }

    page_release_ref(page);
    return HAM_SUCCESS;
}

/**
 * find a key in the index and positions the cursor
 * on this key

 @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_find(ham_bt_cursor_t *c, ham_key_t *key, ham_record_t *record,
            ham_u32_t flags)
{
    ham_assert(key, (0));
    /* 'c' may be NULL */
    /* 'record' may be NULL */

    {
        ham_status_t st;
        ham_db_t *db = bt_cursor_get_db(c);
        ham_env_t *env = db_get_env(db);
        ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
        ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

        common_btree_datums_t btdata =
        {
            be,
            db,
            env,
            env_get_device(env),
            key,
            record,
            (ham_bt_cursor_t *)c,
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
            {flags, flags, (ham_cursor_t *)c, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
            MK_HAM_FLOAT(0.5),
            MK_HAM_FLOAT(0.33) // i.e. 1/3
        };

        ham_assert(be, (0));
        ham_assert(key, ("invalid parameter"));

        st=bt_cursor_set_to_nil(c);
        if (st)
            return (st);

        st=btree_find_cursor(&btdata, key, record);
        if (st) {
            /* cursor is now NIL */
            return (st);
        }
    }

    return HAM_SUCCESS;
}

/**
 * insert (or update) a key in the index
 *
 * @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_insert(ham_bt_cursor_t *c, ham_key_t *key,
            ham_record_t *record, ham_u32_t flags)
{
    ham_status_t st;
    ham_db_t *db = bt_cursor_get_db(c);
    ham_env_t *env = db_get_env(db);
    ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        key,
        record,
        (ham_bt_cursor_t *)c,
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
        {flags, flags, (ham_cursor_t *)c, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    ham_assert(be, (0));
    ham_assert(key, (0));
    ham_assert(record, (0));

    /*
     * call the btree insert function
     */
    st=btree_insert_cursor(&btdata, key, record);
    return (st);
}

/**
 * erases the key from the index; afterwards, the cursor points to NIL
 *
 * @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_erase(ham_bt_cursor_t *c, ham_u32_t flags)
{
    ham_status_t st;

    /*
     * coupled cursor: uncouple it
     */
    if (bt_cursor_get_flags(c) & BT_CURSOR_FLAG_COUPLED)
    {
        st=bt_cursor_uncouple(c, 0);
        if (st)
            return (st);
    }
    else if (!(bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED))
    {
        return (HAM_CURSOR_IS_NIL);
    }

    ham_assert((bt_cursor_get_flags(c) & BT_CURSOR_FLAG_UNCOUPLED), (0));

    {
        ham_db_t *db = bt_cursor_get_db(c);
        ham_env_t *env = db_get_env(db);
        ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
        ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

        common_btree_datums_t btdata =
        {
            be,
            db,
            env,
            env_get_device(env),
            bt_cursor_get_uncoupled_key(c),
            NULL,
            (ham_bt_cursor_t *)c,
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
            {flags, flags, (ham_cursor_t *)c, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
            MK_HAM_FLOAT(0.5),
            MK_HAM_FLOAT(0.33) // i.e. 1/3
        };

        ham_assert(be, (0));

        st = btree_erase_cursor(&btdata, bt_cursor_get_uncoupled_key(c));
        if (st)
            return (st);
    }

    /*
     * set cursor to nil
     */
    st=bt_cursor_set_to_nil(c);
    return (st);
}

ham_status_t
bt_cursor_points_to(common_btree_datums_t *btdata, ham_bt_cursor_t *cursor, int_key_t *key)
{
    ham_status_t st;
    //ham_db_t *db = bt_cursor_get_db(cursor);
    //ham_btree_t *be = (ham_btree_t *)db_get_backend(db);

    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    //ham_db_t * const db = btdata->db;
    //ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    if (bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_UNCOUPLED)
    {
        st=bt_cursor_couple(cursor);
        if (st)
            return (st);
    }

    if (bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_COUPLED)
    {
        ham_page_t *page=bt_cursor_get_coupled_page(cursor);
        //btree_node_t *node=ham_page_get_btree_node(page);
        int_key_t *entry = btree_in_node_get_key_ref(btdata, page, bt_cursor_get_coupled_index(cursor));

        if (entry == key)
            return HAM_TRUE;
    }

    return HAM_FALSE;
}

/**
 * Count the number of records stored with the referenced key, i.e.
 * count the number of duplicates for the current key.

 @note This is a B+-tree cursor 'backend' method.
 */
static ham_status_t
bt_cursor_get_duplicate_count(ham_cursor_t *db_cursor,
                ham_size_t *count, ham_u32_t flags)
{
    ham_bt_cursor_t *cursor = (ham_bt_cursor_t *)db_cursor;
    ham_status_t st;
    //ham_db_t *db=bt_cursor_get_db(cursor);
    //ham_env_t *env = db_get_env(db);
    //ham_btree_t *be=(ham_btree_t *)db_get_backend(db);
    ham_page_t *page;
    btree_node_t *node;
    int_key_t *entry;
    //int_key_t *keyarr;

    ham_db_t *db = bt_cursor_get_db(cursor);
    ham_env_t *env = db_get_env(db);
    ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    common_btree_datums_t btdata =
    {
        be,
        db,
        env,
        env_get_device(env),
        NULL,
        NULL,
        cursor,
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
        {flags, flags, db_cursor, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
        MK_HAM_FLOAT(0.5),
        MK_HAM_FLOAT(0.33) // i.e. 1/3
    };

    ham_assert(be, (0));

    /*
     * uncoupled cursor: couple it
     */
    if (bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_UNCOUPLED)
    {
        st=bt_cursor_couple(cursor);
        if (st)
            return (st);
    }
    else if (!(bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_COUPLED))
    {
        return (HAM_CURSOR_IS_NIL);
    }

    page = bt_cursor_get_coupled_page(cursor);
    node = ham_page_get_btree_node(page);
    entry = btree_in_node_get_key_ref(&btdata, page, bt_cursor_get_coupled_index(cursor));

    if (!(key_get_flags(entry) & KEY_HAS_DUPLICATES))
    {
        *count = 1;
    }
    else
    {
        st=blob_duplicate_get_count(env, key_get_ptr(entry), count, 0);
        if (st)
            return (st);
    }

    return HAM_SUCCESS;
}



ham_status_t
bt_uncouple_all_cursors(ham_page_t *page, ham_size_t start)
{
    ham_status_t st;
    ham_bool_t skipped=HAM_FALSE;
    ham_cursor_t *n;
    ham_cursor_t *c=page_get_cursors(page);

    while (c) {
        ham_bt_cursor_t *btc=(ham_bt_cursor_t *)c;
        n=cursor_get_next_in_page(c);

        /*
         * ignore all cursors which are already uncoupled
         */
        if (bt_cursor_get_flags(btc)&BT_CURSOR_FLAG_COUPLED) {
            /*
             * skip this cursor if its position is < start
             */
            if (bt_cursor_get_coupled_index(btc)<start) {
                c=n;
                skipped=HAM_TRUE;
                continue;
            }

            /*
             * otherwise: uncouple it
             */
            st=bt_cursor_uncouple((ham_bt_cursor_t *)c, 0);
            if (st)
                return (st);
            cursor_set_next_in_page(c, 0);
            cursor_set_previous_in_page(c, 0);
        }

        c=n;
    }

    if (!skipped)
        page_set_cursors(page, 0);

    return HAM_SUCCESS;
}




ham_status_t
bt_cursor_create(ham_db_t *db, ham_txn_t *txn, ham_u32_t flags,
            ham_bt_cursor_t **cu)
{
    ham_env_t *env = db_get_env(db);
    ham_bt_cursor_t *c;

    (void)flags;

    *cu=0;

    c=(ham_bt_cursor_t *)allocator_calloc(env_get_allocator(env), sizeof(*c));
    if (!c)
        return HAM_OUT_OF_MEMORY;

    c->_fun_clone=bt_cursor_clone;
    c->_fun_close=bt_cursor_close;
    c->_fun_overwrite=bt_cursor_overwrite;
    c->_fun_move=bt_cursor_move;
    c->_fun_find=bt_cursor_find;
    c->_fun_insert=bt_cursor_insert;
    c->_fun_erase=bt_cursor_erase;
    c->_fun_get_duplicate_count=bt_cursor_get_duplicate_count;

    bt_cursor_set_allocator(c, env_get_allocator(env));
    bt_cursor_set_db(c, db);
    bt_cursor_set_txn(c, txn);

    /* fix the linked list of cursors */
    cursor_set_next((ham_cursor_t *)c, db_get_cursors(db));
    if (db_get_cursors(db))
        cursor_set_previous(db_get_cursors(db), (ham_cursor_t *)c);
    db_set_cursors(db, (ham_cursor_t *)c);

    *cu=c;
    return HAM_SUCCESS;
}

/**
* @endcond
*/

