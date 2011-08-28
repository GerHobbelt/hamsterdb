/**
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
 * @brief btree inserting
 *
 */

#include "internal_preparation.h"

#include "btree.h"
#include "btree_classic.h"
#include "btree_cursor.h"






/**
 * the insert_scratchpad_t structure helps us to propagate return values
 * from the bottom of the tree to the root.
 *
 * It also transports several semi-constant datums around the call tree
 * at the cost of a single pointer instead of a series of stack pushes.
 */
typedef struct insert_scratchpad_t
{
    common_btree_datums_t *btdata;

    /**
     * A key; this is used to propagate SMOs (structure modification
     * operations) from a child page to a parent page.
     */
    ham_key_t propa_key;

    /**
     * A 'RID'; this is used to propagate SMOs (structure modification
     * operations) from a child page to a parent page.
     */
    ham_offset_t rid;

    /**
     * A pointer to a cursor; if this is a valid pointer, then this
     * cursor will point to the new inserted item.
     */
    ham_bt_cursor_t *cursor;

} insert_scratchpad_t;

/**
 * @ref __insert_recursive B+-tree split requirement signaling
 * return value.
 *
 * @note Shares the value space with the error codes
 * listed in @ref ham_status_codes .
 */
#define SPLIT     1

/*
 * flags for __insert_nosplit()
 */
/* #define NOFLUSH   0x1000    -- unused */

/**
 * this is the function which does most of the work - traversing to a
 * leaf, inserting the key using __insert_in_page()
 * and performing necessary SMOs. It works recursively.
 */
static ham_status_t
__insert_recursive(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, insert_scratchpad_t *scratchpad);

/**
 * this function inserts a key in a page
 */
static ham_status_t
__insert_in_page(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, insert_scratchpad_t *scratchpad);

/**
 * insert a key in a page; the page MUST have free slots
 */
static ham_status_t
__insert_nosplit(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, const ham_record_t *record,
        insert_scratchpad_t *scratchpad);

/**
 * split a page and insert the new element
 */
static ham_status_t
__insert_split(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, insert_scratchpad_t *scratchpad);

static ham_status_t
__insert_cursor(insert_scratchpad_t *scratchpad, ham_key_t *key,
        ham_record_t *record);



static ham_status_t
__append_key(insert_scratchpad_t *scratchpad, ham_key_t *key,
        ham_record_t *record)
{
    ham_status_t st=0;
    ham_page_t *page;
    btree_node_t *node;

    common_btree_datums_t * const btdata = scratchpad->btdata;
    ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    ham_bt_cursor_t * const cursor = btdata->cursor;
    common_hints_t * const hints = &btdata->hints;

#ifdef HAM_DEBUG
    if (cursor && !bt_cursor_is_nil(cursor))
    {
        ham_assert(be_get_db(be) == bt_cursor_get_db(cursor), (0));
    }
#endif

    //db = be_get_db(be);

    ham_assert(hints->try_fast_track, (0));

    /*
     * see if we get this btree leaf; if not, revert to regular scan
     *
     * As this is a speed-improvement hint re-using recent material, the page
     * should still sit in the cache, or we're using old info, which should be
     * discarded.
     */
    st = db_fetch_page(&page, env, hints->leaf_page_addr, DB_ONLY_FROM_CACHE);
    ham_assert(st ? page == NULL : 1, (0));
    if (st)
        return st;
    if (!page)
    {
        //hints->flags &=~ HAM_HINT_APPEND;
        hints->force_append = HAM_FALSE;
        hints->force_prepend = HAM_FALSE;
        return __insert_cursor(scratchpad, key, record);
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(page, db);

    page_add_ref(page);
    node=ham_page_get_btree_node(page);
    ham_assert(btree_node_is_leaf(node), ("iterator points to internal node"));

    /*
     * if the page is already full OR this page is not the right-most page
     * when we APPEND or the left-most node when we PREPEND
     * OR the new key is not the highest key: perform a normal insert
     */
    if ((hints->force_append && btree_node_get_right(node))
        || (hints->force_prepend && btree_node_get_left(node))
        || btree_node_get_count(node) >= btree_get_maxkeys(be))
    {
        page_release_ref(page);
        //hints->flags &=~ HAM_HINT_APPEND;
        hints->force_append = HAM_FALSE;
        hints->force_prepend = HAM_FALSE;
        return __insert_cursor(scratchpad, key, record);
    }

    /*
     * if the page is not empty: check if we append the key at the end / start
     * (depending on force_append/force_prepend),
     * or if it's actually inserted in the middle (when neither force_append
     * or force_prepend is specified: that'd be SEQUENTIAL insertion
     * hinting somewhere in the middle of the total key range.
     */
    if (btree_node_get_count(node)!=0)
    {
        int cmp_hi;
        int cmp_lo;

        if (!hints->force_prepend)
        {
            cmp_hi = key_compare_pub_to_int(btdata, page, key, btree_node_get_count(node)-1);
            /* did the key comparison result in a error? */
            if (cmp_hi < -1)
            {
                page_release_ref(page);
                return (ham_status_t)cmp_hi;
            }
            /* key is larger than the largest on this page */
            if (cmp_hi > 0)
            {
                if (btree_node_get_right(node))
                {
                    /* not at top end of the btree, so we can't do the fast track */
                    page_release_ref(page);
                    //hints->flags &= ~HAM_HINT_APPEND;
                    hints->force_append = HAM_FALSE;
                    hints->force_prepend = HAM_FALSE;
                    return __insert_cursor(scratchpad, key, record);
                }

                hints->force_append = HAM_TRUE;
                hints->force_prepend = HAM_FALSE;
            }
        }
        else
        {
            /* not bigger than the right-most node while we were trying to APPEND */
            cmp_hi = -1;
        }

        if (!hints->force_append)
        {
            cmp_lo = key_compare_pub_to_int(btdata, page, key, 0);
            /* did the key comparison result in a error? */
            if (cmp_lo < -1)
            {
                page_release_ref(page);
                return (ham_status_t)cmp_lo;
            }
            /* key is smaller than the smallest on this page */
            if (cmp_lo < 0)
            {
                if (btree_node_get_left(node))
                {
                    /* not at bottom end of the btree, so we can't do the fast track */
                    page_release_ref(page);
                    //hints->flags &= ~HAM_HINT_PREPEND;
                    hints->force_append = HAM_FALSE;
                    hints->force_prepend = HAM_FALSE;
                    return __insert_cursor(scratchpad, key, record);
                }

                hints->force_append = HAM_FALSE;
                hints->force_prepend = HAM_TRUE;
            }
        }
        else
        {
            /* not smaller than the left-most node while we were trying to PREPEND */
            cmp_lo = +1;
        }

        /* handle inserts in the middle range */
        if (cmp_lo >= 0 && cmp_hi <= 0)
        {
            /*
             * Depending on where we are in the btree, the current key either
             * is going to end up in the middle of the given node/page,
             * OR the given key is out of range of the given leaf node.
             */
            if (hints->force_append || hints->force_prepend)
            {
                /*
                 * when prepend or append is FORCED, we are expected to
                 * add keys ONLY at the beginning or end of the btree
                 * key range. Clearly the current key does not fit that
                 * criterium.
                 */
                page_release_ref(page);
                //hints->flags &= ~HAM_HINT_PREPEND;
                hints->force_append = HAM_FALSE;
                hints->force_prepend = HAM_FALSE;
                return __insert_cursor(scratchpad, key, record);
            }

            /*
             * we discovered that the key must be inserted in the middle
             * of the current leaf.
             *
             * It does not matter whether the current leaf is at the start or
             * end of the btree range; as we need to add the key in the middle
             * of the current leaf, that info alone is enough to continue with
             * the fast track insert operation.
             */
            ham_assert(!hints->force_prepend && !hints->force_append, (0));
        }

        ham_assert((hints->force_prepend + hints->force_append) < 2,
                ("Either APPEND or PREPEND flag MAY be set, but not both"));
    }
    else
    {
        /* empty page: force insertion in slot 0 */
        hints->force_append = HAM_FALSE;
        hints->force_prepend = HAM_TRUE;
    }

    /*
     * the page will be changed - write it to the log (if a log exists)
     */
    st=ham_log_add_page_before(page);
    if (st) {
        page_release_ref(page);
        return (st);
    }

    /*
     * OK - we're really appending/prepending the new key.
     */
    //ham_assert(hints->force_append || hints->force_prepend, (0));
    st=__insert_nosplit(page, key, 0, record, scratchpad);
#if 0
    scratchpad->cursor = 0; /* don't overwrite cursor if __insert_nosplit
                             is called again */
#endif

    page_release_ref(page);
    return (st);
}


static ham_status_t
__insert_cursor(insert_scratchpad_t *scratchpad, ham_key_t *key,
        ham_record_t *record)
{
    ham_status_t st;
    ham_page_t *root;

    common_btree_datums_t * const btdata = scratchpad->btdata;
    ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    common_hints_t * const hints = &btdata->hints;

    ham_assert(hints->force_append == HAM_FALSE, (0));
    ham_assert(hints->force_prepend == HAM_FALSE, (0));

    /*
     * get the root-page...
     */
    ham_assert(btree_get_rootpage(be) != 0, ("btree has no root page"));
    st=db_fetch_page(&root, env, btree_get_rootpage(be), 0);
    ham_assert(st ? root == NULL : root != NULL, (0));
    if (!root)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(root, db);

    /*
     * ... and start the recursion
     */
    st=__insert_recursive(root, key, 0, scratchpad);

    /*
     * if the root page was split, we have to create a new
     * root page.
     */
    if (st == SPLIT)
    {
        ham_page_t *newroot;
        btree_node_t *node;
        dev_alloc_request_info_ex_t info = {0};

        /*
         * the root-page will be changed...
         */
        st=ham_log_add_page_before(root);
        if (st)
            return (st);

        /*
         * allocate a new root page
         */
        info.db = db;
        info.env = env;
        info.dam = db_get_data_access_mode(db);
        info.entire_page = HAM_TRUE;
        info.space_type = PAGE_TYPE_B_ROOT;
        info.key = key;
        info.record = record;

        st=db_alloc_page(&newroot, 0, &info);
        ham_assert(st ? newroot == NULL : 1, (0));
        ham_assert(!st ? newroot != NULL : 1, (0));
        if (st)
            return st;
        /* clear the node header */
        memset(page_get_payload(newroot), 0, sizeof(btree_node_t));
        page_set_owner(newroot, db);

        stats_page_is_nuked(btdata, root, REASON_SPLIT);

        /*
         * insert the pivot element and the ptr_left
         */
        node=ham_page_get_btree_node(newroot);
        btree_node_set_ptr_left(node, btree_get_rootpage(be));
        st=__insert_nosplit(newroot, &scratchpad->propa_key,
                scratchpad->rid, record, scratchpad);
        ham_assert(!(scratchpad->propa_key.flags & HAM_KEY_USER_ALLOC), (0));
        scratchpad->cursor = 0; /* don't overwrite cursor if __insert_nosplit
                                is called again */
        if (st) {
            ham_assert(!(scratchpad->propa_key.flags & HAM_KEY_USER_ALLOC), (0));
            if (scratchpad->propa_key.data)
                allocator_free(env_get_allocator(env), scratchpad->propa_key.data);
            return (st);
        }

        /*
         * set the new root page
         *
         * !!
         * do NOT delete the old root page - it's still in use!
         *
         * also don't forget to flush the backend - otherwise the header
         * page of the database will not contain the updated information.
         * The backend is flushed when the database is closed, but if
         * recovery is enabled then the flush here is critical.
         */
        btree_set_rootpage(be, page_get_self(newroot));
        be_set_dirty(be, HAM_TRUE);
        env_set_dirty(env);
		/* [i_a] Christoph added the flush + txn_add_page below, but somehow I get the feeling that he's been
		         patching the side-effect of other bugs through enforced flush; my code had
				 the env_set_dirty() and his didn't; also I seem to recall that 1.1.6 (which
				 was when I wrote this code, not this comment!) had several spots where
				 the dirty-marking of the ENV was neglected and a flush also corrects
				 those mistakes, unless I am mistaken.

				 Anyway, merged. We'll see what happens en when I've got time again, we'll do
				 another flow review all over again.
		*/

        be->_fun_flush(be);

        if (env_get_cache(env) && (page_get_pers_type(root) != PAGE_TYPE_B_INDEX))
        {
            /*
             * As we re-purpose a page, we do NOT reset its pagecounter
             * as well as the cache is merely interested in how 'important'
			 * a page is, irrespective of purpose.
             */
            cache_update_page_access_counter(root, env_get_cache(env));
        }
        page_set_pers_type(root, PAGE_TYPE_B_INDEX);
        page_set_dirty(root, env);
        page_set_dirty(newroot, env);

        /* the root page was modified (btree_set_rootpage) - make sure that
         * it's logged */
        if (env_get_rt_flags(env) & HAM_ENABLE_RECOVERY)
		{
            st=txn_add_page(env_get_txn(env), env_get_header_page(env),
                    HAM_TRUE);
            if (st)
                return (st);
        }
    }

    /*
     * release the scratchpad-memory and return to caller
     */
    ham_assert(!(scratchpad->propa_key.flags & HAM_KEY_USER_ALLOC), (0));
    if (scratchpad->propa_key.data)
        allocator_free(env_get_allocator(env), scratchpad->propa_key.data);

    return (st);
}

ham_status_t
btree_insert_cursor(common_btree_datums_t *btdata, ham_key_t *key,
        ham_record_t *record)
{
    ham_status_t st;

    //ham_btree_t * const be = btdata->be;
    //ham_db_t * const db = btdata->db;
    //ham_env_t * const env = btdata->env;
    ham_bt_cursor_t * const cursor = btdata->cursor;
    common_hints_t * const hints = &btdata->hints;

    insert_scratchpad_t scratchpad =
    {
        btdata,
        {0},
        0,
        cursor
    };

    db_update_global_stats_insert_query(btdata, key->size, record->size);

    btree_insert_get_hints(btdata);

    /*
     * append the key? __append_key() will try to append the key; if it
     * fails because the key is NOT the highest key in the database or
     * because the current page is already full, it will remove the
     * HINT_APPEND flag and call btree_insert_cursor() again
     */
    if (hints->try_fast_track)
    {
        //ham_assert(!bt_cursor_is_nil(cursor), (0)); -- [i_a] no need for a cursor anymore
        st = __append_key(&scratchpad, key, record);
    }
    else
    {
        ham_assert(!hints->force_append && !hints->force_prepend, (0));
        //hints.force_append = HAM_FALSE;
        //hints.force_prepend = HAM_FALSE;
        st = __insert_cursor(&scratchpad, key, record);
    }

    if (st)
    {
        stats_update_insert_fail(btdata);
    }
    else
    {
        stats_update_insert(btdata, hints->processed_leaf_page);
        stats_update_any_bound(btdata, hints->processed_leaf_page, key, hints->flags, hints->processed_slot);
    }

    return (st);
}


static ham_status_t
__insert_recursive(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, insert_scratchpad_t *scratchpad)
{
    ham_status_t st;
    ham_page_t *child;
    //ham_db_t *db=page_get_owner(page);
    btree_node_t *node=ham_page_get_btree_node(page);

    common_btree_datums_t * const btdata = scratchpad->btdata;
    //ham_btree_t * const be = btdata->be;
    //ham_db_t * const db = btdata->db;
    //ham_env_t * const env = btdata->env;
    //ham_bt_cursor_t * const cursor = btdata->cursor;
    common_hints_t * const hints = &btdata->hints;

    /*
     * if we've reached a leaf: insert the key
     */
    if (btree_node_is_leaf(node))
    {
        return __insert_in_page(page, key, rid, scratchpad);
    }

    /*
     * otherwise traverse the root down to the leaf
     */
    st = btree_traverse_tree(&child, 0, btdata, page, key);
    if (!child)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(st == HAM_SUCCESS, (0));

    /*
     * and call this function recursively
     */
    st = __insert_recursive(child, key, rid, scratchpad);
    switch (st)
    {
    /*
     * if we're done, we're done
     */
    case HAM_SUCCESS:
        break;

    /*
     * if we tried to insert a duplicate key, we're done, too
     */
    case HAM_DUPLICATE_KEY:
        break;

    /*
     * the child was split, and we have to insert a new key/rid-pair.
     */
    case SPLIT:
        hints->flags |= HAM_OVERWRITE;
        ham_assert(!(ham_key_get_intflags(&scratchpad->propa_key) & KEY_IS_EXTENDED), (0));
        st = __insert_in_page(page, &scratchpad->propa_key,
                    scratchpad->rid, scratchpad);
        ham_assert(!(scratchpad->propa_key.flags & HAM_KEY_USER_ALLOC), (0));
        hints->flags = hints->original_flags;
        break;

    /*
     * every other return value is an error
     */
    default:
        ham_assert(st < 0, (0));
        break;
    }

    return (st);
}

static ham_status_t
__insert_in_page(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, insert_scratchpad_t *scratchpad)
{
    ham_status_t st;
    btree_node_t *node = ham_page_get_btree_node(page);
    //ham_size_t maxkeys=btree_get_maxkeys(scratchpad->be);

    common_btree_datums_t * const btdata = scratchpad->btdata;
    //ham_btree_t * const be = btdata->be;
    //ham_db_t * const db = btdata->db;
    //ham_env_t * const env = btdata->env;
    //ham_bt_cursor_t * const cursor = btdata->cursor;
    //const ham_size_t keywidth = btdata->keywidth;
    common_hints_t * const hints = &btdata->hints;
    const ham_size_t maxkeys = btdata->maxkeys;
    const ham_record_t * const record = btdata->in_rec;

    ham_assert(maxkeys > 1,
            ("invalid result of db_get_maxkeys(): %d", maxkeys));
    ham_assert(hints->force_append == HAM_FALSE, (0));
    ham_assert(hints->force_prepend == HAM_FALSE, (0));

    /*
     * prepare the page for modifications
     */
    st=ham_log_add_page_before(page);
    if (st)
        return (st);

    /*
     * if we can insert the new key without splitting the page:
     * __insert_nosplit() will do the work for us
     */
    if (btree_node_get_count(node) < maxkeys)
    {
        st = __insert_nosplit(page, key, rid, record, scratchpad);
        scratchpad->cursor = 0; /* don't overwrite cursor if __insert_nosplit
                                 is called again */
        return (st);
    }

    /*
     * otherwise, we have to split the page.
     * but BEFORE we split, we check if the key already exists!
     */
    if (btree_node_is_leaf(node))
    {
        ham_s32_t idx;

        idx = btree_node_search_by_key(btdata, page, key, HAM_FIND_EXACT_MATCH);
        /* key exists! */
        if (idx >= 0)
        {
            ham_assert((hints->flags & (HAM_DUPLICATE_INSERT_BEFORE
                                    |HAM_DUPLICATE_INSERT_AFTER
                                    |HAM_DUPLICATE_INSERT_FIRST
                                    |HAM_DUPLICATE_INSERT_LAST))
                        ? (hints->flags & HAM_DUPLICATE)
                        : 1, (0));
            if (!(hints->flags & (HAM_OVERWRITE | HAM_DUPLICATE)))
                return (HAM_DUPLICATE_KEY);
                st = __insert_nosplit(page, key, rid, record, scratchpad);
                /* don't overwrite cursor if __insert_nosplit is called again */
                scratchpad->cursor = 0;
                return st;
        }
        else if (idx < -1)
        {
            ham_assert(!"this shouldn't ever happen, right?", (0));
            return (ham_status_t)idx;
        }
    }

    return __insert_split(page, key, rid, scratchpad);
}

static ham_status_t
__insert_nosplit(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, const ham_record_t *record,
        insert_scratchpad_t *scratchpad)
{
    common_btree_datums_t * const btdata = scratchpad->btdata;
    ham_device_t * const dev = btdata->dev;
    ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    ham_bt_cursor_t * const cursor = btdata->cursor;
    common_hints_t * const hints = &btdata->hints;
    const ham_size_t keywidth = btdata->keywidth;
    const ham_size_t keysize = btdata->keysize;

    ham_status_t st;
    ham_u16_t count;
    ham_size_t new_dupe_id = 0;
    int_key_t *bte = 0;
    //int_key_t *keyarr;
    btree_node_t *node;
    //ham_db_t *db=page_get_owner(page);
    //ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_bool_t exists = HAM_FALSE;
    ham_s32_t slot;
    //ham_size_t keywidth;

    ham_assert(page_get_owner(page), (0));
    ham_assert(dev == page_get_device(page), (0));
    ham_assert(device_get_env(dev) == db_get_env(page_get_owner(page)), (0));

    //keywidth = be_get_keysize(be) + db_get_int_key_header_size();
    node = ham_page_get_btree_node(page);
    count = btree_node_get_count(node);
    //keysize = be_get_keysize(be);

    if (btree_node_get_count(node)==0)
    {
        slot = 0;
    }
    else if (hints->force_append)
    {
        slot = count;
    }
    else if (hints->force_prepend)
    {
        /* insert at beginning; shift all up by one */
        slot = 0;
    }
    else
    {
        int cmp;

        st=btree_get_slot(btdata, page, key, &slot, &cmp);
        if (st)
            return st;

        /* insert the new key at the beginning? */
        if (slot == -1)
        {
            slot = 0;
        }
        else
        {
            /*
             * key exists already
             */
            if (cmp == 0)
            {
                if (hints->flags & HAM_OVERWRITE)
                {
                    /*
                     * no need to overwrite the key - it already exists!
                     * however, we have to overwrite the data!
                     */
                    if (!btree_node_is_leaf(node))
                    {
                        return HAM_SUCCESS;
                    }
                }
                else if (!(hints->flags & HAM_DUPLICATE))
                {
                    return HAM_DUPLICATE_KEY;
                }

                /* do NOT shift keys up to make room; just overwrite the current [slot] */
                exists = HAM_TRUE;
            }
            else
            {
                /*
                 * otherwise, if the new key is > then the slot key, move to
                 * the next slot
                 */
                if (cmp > 0)
                {
                    slot++;
                }
            }
        }
    }

    /*
     * in any case, uncouple the cursors and see if we must shift any elements to the
     * right
     */
    bte = btree_in_node_get_key_ref(btdata, page, slot);
    ham_assert(bte, (0));

    if (!exists)
    {
        ham_assert(count < btree_get_maxkeys(be), (0));

        if (count > slot)
        {
            /* uncouple all cursors & shift any elements following [slot] */
            st=bt_uncouple_all_cursors(page, slot);
            if (st)
                return st;

#if 0
			memmove(((char *)bte)+keywidth, bte, keywidth * (count-slot));
#else
			{
				int_key_t *bte_lhs = btree_in_node_get_key_ref(btdata, page, slot + 1);

				btree_move_key_series(btdata, page, page, bte_lhs, bte, count - slot); // memmove(((char *)bte)+keywidth, bte, keywidth * (count-slot));
			}
#endif
		}

        /*
         * if a new key is created or inserted: initialize it with zeroes
         */
        memset(bte, 0, keywidth);
    }

    /*
     * if we're in the leaf: insert, overwrite or append the blob
     * (depends on the flags)
     */
    if (btree_node_is_leaf(node))
    {
        ham_status_t st;

        ham_assert(new_dupe_id == 0, (0));
        st=key_set_record(db, bte, record,
                        cursor
                            ? bt_cursor_get_dupe_id(cursor)
                            : 0,
                        hints->flags, &new_dupe_id);
        if (st)
            return st;

        hints->processed_leaf_page = page;
        hints->processed_slot = slot;
    }
    else
    {
        key_set_ptr(bte, rid);
    }

    page_set_dirty(page, env);
    key_set_size(bte, key->size);

    /*
     * set a flag if the key is extended, and does not fit into the
     * btree
     */
    if (key->size > keysize)
        key_set_flags(bte, key_get_flags(bte) | KEY_IS_EXTENDED);

    /*
     * if we have a cursor: couple it to the new key
     *
     * the cursor always points to NIL.
     */
    if (cursor)
    {
        st=bt_cursor_set_to_nil(cursor);
        if (st)
            return st;

        ham_assert(!(bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_UNCOUPLED),
                ("coupling an uncoupled cursor, but need a nil-cursor"));
        ham_assert(!(bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_COUPLED),
                ("coupling a coupled cursor, but need a nil-cursor"));
        bt_cursor_set_flags(cursor,
                bt_cursor_get_flags(cursor) | BT_CURSOR_FLAG_COUPLED);
        bt_cursor_set_coupled_page(cursor, page);
        bt_cursor_set_coupled_index(cursor, slot);
        bt_cursor_set_dupe_id(cursor, new_dupe_id);
        memset(bt_cursor_get_dupe_cache(cursor), 0, sizeof(dupe_entry_t));
        page_add_cursor(page, (ham_cursor_t *)cursor);
    }

    /*
     * if we've overwritten a key: no need to continue, we're done
     */
    if (exists)
        return HAM_SUCCESS;

    /*
     * we insert the extended key, if necessary
     */
    key_set_key(bte, key->data,
            be_get_keysize(be) < key->size ? be_get_keysize(be) : key->size);

    /*
     * if we need an extended key, allocate a blob and store
     * the blob-id in the key
     */
    if (key->size > be_get_keysize(be))
    {
        ham_offset_t blobid;

        key_set_key(bte, key->data, be_get_keysize(be));

        st = key_insert_extended(&blobid, db, page, key);
        ham_assert(st ? blobid == 0 : 1, (0));
        if (!blobid)
            return st ? st : HAM_INTERNAL_ERROR;

        key_set_extended_rid(db, bte, blobid);
    }

    /*
     * update the btree node-header
     */
    count++;
    btree_node_set_count(node, count);

    return HAM_SUCCESS;
}

static ham_status_t
__insert_split(ham_page_t *page, ham_key_t *key,
        ham_offset_t rid, insert_scratchpad_t *scratchpad)
{
    common_btree_datums_t * const btdata = scratchpad->btdata;
    ham_device_t * const dev = btdata->dev;
    //ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_bt_cursor_t * const cursor = btdata->cursor;
    common_hints_t * const hints = &btdata->hints;
    const ham_size_t keywidth = btdata->keywidth;
    //const ham_size_t keysize = btdata->keysize;
    const ham_float_t split_ratio = btdata->split_ratio;

    int cmp;
    ham_status_t st;
    ham_page_t *newpage;
    ham_page_t *oldsib;
    int_key_t *nbte;
    int_key_t *obte;
    //int_key_t *nkeyarr;
    //int_key_t *okeyarr;
    //int_key_t *keyarr;
    btree_node_t *nbtp;
    btree_node_t *obtp;
    btree_node_t *sbtp;
    ham_size_t count;
    //ham_size_t keysize;
    //ham_db_t *db=page_get_owner(page);
    //ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    //ham_env_t *env = env;
    ham_key_t pivotkey;
    ham_key_t oldkey;
    ham_offset_t pivotrid;
    ham_u16_t pivot;
    ham_bool_t pivot_at_end=HAM_FALSE;
    dev_alloc_request_info_ex_t info = {0};
    //ham_size_t keywidth;

    ham_assert(page_get_owner(page), (0));
    ham_assert(dev == page_get_device(page), (0));
    ham_assert(device_get_env(dev) == db_get_env(page_get_owner(page)), (0));

    ham_assert(hints->force_append == HAM_FALSE, (0));

    //keysize=be_get_keysize(be);
    //keywidth = be_get_keysize(be) + db_get_int_key_header_size();

    /*
     * allocate a new page
     */
    info.db = db;
    info.env = env;
    info.dam = db_get_data_access_mode(db);
    info.entire_page = HAM_TRUE;
    info.space_type = PAGE_TYPE_B_INDEX;
    info.key = key;
    info.record = btdata->in_rec;

    st = db_alloc_page(&newpage, 0, &info);
    ham_assert(st ? newpage == NULL : 1, (0));
    ham_assert(!st ? newpage != NULL : 1, (0));
    if (st)
        return st;
    /* clear the node header */
    memset(page_get_payload(newpage), 0, sizeof(btree_node_t));
    page_set_owner(newpage, db);

    stats_page_is_nuked(btdata, page, REASON_SPLIT);

    /*
     * move half of the key/rid-tuples to the new page
     *
     * !! recno: keys are sorted; we do a "lazy split".
     *
     * for databases with sequential access (this includes recno databases):
     * do not split in the middle, but at the very end of the page
     *
     *
     * The pivot is moved from the M/2 midpoint to either side, depending on
     * the db statistics regarding append/prepend activity: when a lot of
     * activity is append or prepend insert actitivy, a performance
     * improvement can possibly be obtained by shifting the pivot anticipating
     * near-future activity of the same type: that way the fill rate of the
     * Btree index may improve resulting in fewer splits, which are costly
     * in both CPU load and storage space terms. Otherwise, with a (semi-)ordered
     * append or prepend insert feed, the fill rates of the pages would
     * not surpass the worst case value of M/2.
     *
     * TODO: check the EMA's in the statistics to decide which way the pivot should shift.
     */
    nbtp=ham_page_get_btree_node(newpage);
    nbte = btree_in_node_get_key_ref(btdata, newpage, 0);

    obtp=ham_page_get_btree_node(page);
    obte = btree_in_node_get_key_ref(btdata, page, 0);
    count=btree_node_get_count(obtp);

#if 0
    /*
     * for databases with sequential access (this includes recno databases):
     * do not split in the middle, but at the very end of the page
     *
     * if this page is the right-most page in the index, and this key is
     * inserted at the very end, then we select the same pivot as for
     * sequential access
     *
     * TODO GerH: this should be hinter-based work!!!
     */
    if (db_get_data_access_mode(db)&HAM_DAM_SEQUENTIAL_INSERT)
    {
        pivot_at_end=HAM_TRUE;
    }
    else if (btree_node_get_right(obtp)==0)
    {
        ham_assert(!(db_get_rt_flags(db) & HAM_RECORD_NUMBER), (0));
        cmp=key_compare_pub_to_int(db, page, key, btree_node_get_count(obtp)-1);
        if (cmp>0)
            pivot_at_end=HAM_TRUE;
    }
    else
    {
        ham_assert(!(db_get_rt_flags(db) & HAM_RECORD_NUMBER), (0));
    }
#endif

    /*
     * internal pages set the count of the new page to count-pivot-1 (because
     * the pivot element will become ptr_left of the new page).
     * by using pivot=count-2 we make sure that at least 1 element will remain
     * in the new node.
     */
    if (pivot_at_end)
    {
        ham_assert(count > 2, (0));
        ham_assert(count - 2 < HAM_MAX_U16, (0));
        pivot = count - 2;
    }
    else
    {
        ham_assert((count * split_ratio) / HAM_FLOAT_1 < HAM_MAX_U16, (0));
        ham_assert(split_ratio == MK_HAM_FLOAT(0.5), (0)); // old 1/2 ratio
        pivot = (count * split_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round the value
		if (btree_node_is_leaf(obtp))
		{
			if (pivot < 2)
			{
				pivot = 2;
			}
			else if (count < pivot + 2)
			{
				pivot = count - 2;
			}
		}
		else
		{
			if (pivot < 2)
			{
				pivot = 2;
			}
			else if (count < pivot + 2 + 1)
			{
				pivot = count - 3;
			}
		}
    }

    /*
     * uncouple all cursors
     */
    st = bt_uncouple_all_cursors(page, pivot);
    if (st)
        return st;

    /*
     * if we split a leaf, we'll insert the pivot element in the leaf
     * page, too. in internal nodes, we don't insert it, but propagate
     * it to the parent node only.
     */
    if (btree_node_is_leaf(obtp))
    {
#if 0
		memcpy((char *)nbte,
               ((char *)obte)+keywidth*pivot,
               keywidth*(count-pivot));
#else
        int_key_t *bte_rhs = btree_in_node_get_key_ref(btdata, page, pivot);
		btree_move_key_series(btdata, newpage, page, nbte, bte_rhs, count - pivot);
#endif
	}
    else
    {
#if 0
		memcpy((char *)nbte,
               ((char *)obte)+keywidth*(pivot+1),
               keywidth*(count-pivot-1));
#else
        int_key_t *bte_rhs = btree_in_node_get_key_ref(btdata, page, pivot + 1);
		btree_move_key_series(btdata, newpage, page, nbte, bte_rhs, count - pivot - 1);
#endif
	}

    /*
     * store the pivot element, we'll need it later to propagate it
     * to the parent page
     */
    nbte = btree_in_node_get_key_ref(btdata, page, pivot);

    memset(&pivotkey, 0, sizeof(pivotkey));
    memset(&oldkey, 0, sizeof(oldkey));
    oldkey.data = key_get_key(nbte);
    oldkey.size = key_get_size(nbte);
    ham_key_set_intflags(&oldkey, key_get_flags(nbte));
    st = util_copy_key(db, &oldkey, &pivotkey, page);
    if (st)
    {
        (void)db_free_page(newpage, DB_MOVE_TO_FREELIST);
        goto fail_dramatically;
    }
    pivotrid=page_get_self(newpage);

    /*
     * adjust the page count
     */
    if (btree_node_is_leaf(obtp))
    {
        btree_node_set_count(obtp, pivot);
        btree_node_set_count(nbtp, count - pivot);
    }
    else
    {
        btree_node_set_count(obtp, pivot);
        btree_node_set_count(nbtp, count - pivot - 1);
    }

    /*
     * if we're in an internal page: fix the ptr_left of the new page
     * (it points to the ptr of the pivot key)
     */
    if (!btree_node_is_leaf(obtp))
    {
        /*
         * nbte still contains the pivot key
         */
        btree_node_set_ptr_left(nbtp, key_get_ptr(nbte));
    }

    /*
     * insert the new element
     */
    cmp = key_compare_pub_to_int(btdata, page, key, pivot);
    if (cmp < -1)
    {
        st = (ham_status_t)cmp;
        goto fail_dramatically;
    }

    if (cmp >= 0)
    {
        st = __insert_nosplit(newpage, key, rid, btdata->in_rec, scratchpad);
    }
    else
    {
        st = __insert_nosplit(page, key, rid, btdata->in_rec, scratchpad);
    }
    if (st)
    {
        goto fail_dramatically;
    }
    scratchpad->cursor = 0; /* don't overwrite cursor if __insert_nosplit
                             is called again */

    /*
     * fix the double-linked list of pages, and mark the pages as dirty
     */
    if (btree_node_get_right(obtp))
    {
        st=db_fetch_page(&oldsib, env, btree_node_get_right(obtp), 0);
        ham_assert(st ? oldsib == NULL : oldsib != NULL, (0));
        if (st)
            goto fail_dramatically;
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(oldsib, db);
    }
    else
    {
        oldsib=0;
    }

    if (oldsib)
    {
        ham_assert(btdata->db && page_get_owner(oldsib) == btdata->db, (0));
        st = ham_log_add_page_before(oldsib);
        if (st)
            goto fail_dramatically;
    }

    btree_node_set_left (nbtp, page_get_self(page));
    btree_node_set_right(nbtp, btree_node_get_right(obtp));
    btree_node_set_right(obtp, page_get_self(newpage));
    if (oldsib)
    {
        sbtp = ham_page_get_btree_node(oldsib);
        btree_node_set_left(sbtp, page_get_self(newpage));
        page_set_dirty(oldsib, env);
    }
    page_set_dirty(newpage, env);
    page_set_dirty(page, env);

    /*
     * propagate the pivot key to the parent page
     */
    ham_assert(!(scratchpad->propa_key.flags & HAM_KEY_USER_ALLOC), (0));
    if (scratchpad->propa_key.data)
        allocator_free(env_get_allocator(env), scratchpad->propa_key.data);
    scratchpad->propa_key = pivotkey;
    scratchpad->rid = pivotrid;
    ham_assert(!(scratchpad->propa_key.flags & HAM_KEY_USER_ALLOC), (0));
    ham_assert(!(ham_key_get_intflags(&scratchpad->propa_key) & KEY_IS_EXTENDED), (0));

    return (SPLIT);

fail_dramatically:
    ham_assert(!(pivotkey.flags & HAM_KEY_USER_ALLOC), (0));
    if (pivotkey.data)
       allocator_free(env_get_allocator(env), pivotkey.data);
    return st;
}


/**
* @endcond
*/

