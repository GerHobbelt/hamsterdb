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
 * @brief btree erasing
 *
 */

#include "internal_preparation.h"

#include "btree.h"
#include "btree_classic.h"



/**
 * the erase_scratchpad_t structure helps us to propagate return values
 * from the bottom of the tree to the root.
 *
 * It also transports several semi-constant datums around the call tree
 * at the cost of a single pointer instead of a series of stack pushes.
 */
typedef struct erase_scratchpad_t
{
    /**
     * the common input datums
     */
    common_btree_datums_t *btdata;

    /**
     * a page which needs rebalancing
     */
    ham_page_t *mergepage;

#if 0
    /**
     * a coupled cursor (can be NULL)
     */
    ham_bt_cursor_t *cursor;
#endif

} erase_scratchpad_t;

/**
 * recursively descend down the tree, delete the item and re-balance
 * the tree on the way back up
 *
 * returns the page which is deleted, if available
 */
static ham_status_t
my_erase_recursive(ham_page_t ** const page_ref, ham_page_t * const page, ham_offset_t left, ham_offset_t right,
                   ham_offset_t lanchor, ham_offset_t ranchor, ham_page_t * const parent,
                   erase_scratchpad_t * const scratchpad);

/**
 * collapse the root node
 */
static ham_status_t
my_collapse_root(ham_page_t *root, erase_scratchpad_t *scratchpad);

/**
 * rebalance a page - either shifts elements to a sibling, or merges
 * the page with a sibling
 */
static ham_status_t
my_rebalance(ham_page_t **newpage_ref, ham_page_t *page, ham_offset_t left, ham_offset_t right,
             ham_offset_t lanchor, ham_offset_t ranchor, ham_page_t *parent,
             erase_scratchpad_t *scratchpad);

/**
 * merge two pages
 */
static ham_status_t
my_merge_pages(ham_page_t **newpage_ref, ham_page_t *page, ham_page_t *sibling, ham_offset_t anchor,
        erase_scratchpad_t *scratchpad);

/**
 * shift items from a sibling to this page, till both pages have an equal
 * number of items
 *
 * @remark @a newpage_ref reference will always be set to NULL.
 *
 * TODO has been checked twice against old code and this is true. Hence shift_pages
 *      does NOT need the newpage_ref and callers could set *newpage_ref=NULL
 *      themselves.
 */
static ham_status_t
my_shift_pages(ham_page_t **newpage_ref, ham_page_t *page, ham_page_t *sibpage, ham_offset_t anchor,
        erase_scratchpad_t *scratchpad);

/**
 * copy a key
 */
static ham_status_t
my_copy_key(common_btree_datums_t *btdata, int_key_t *dst, int_key_t *src);

/**
 * replace the key in a page at index @a slot with the key referenced by @a newentry .
 *
 * When @a flags is set to @ref INTERNAL_KEY then the key written into index @a slot
 * will have all its 'blob flags' reset:

 - @ref KEY_BLOB_SIZE_TINY
 - @ref KEY_BLOB_SIZE_SMALL
 - @ref KEY_BLOB_SIZE_EMPTY
 - @ref KEY_HAS_DUPLICATES
 */
static ham_status_t
my_replace_key(ham_page_t *page, ham_s32_t slot,
        int_key_t *newentry, ham_u32_t flags, common_btree_datums_t *btdata);

/**
 * remove an item from a page
 */
static ham_status_t
my_remove_entry(ham_page_t *page, ham_s32_t slot,
        erase_scratchpad_t *scratchpad);

/*
 * flags for my_replace_key
 */
/* #define NOFLUSH 1  -- unused */
#define INTERNAL_KEY 2


/**
 * erase a key in the index
 *
 * @note This is a B+-tree 'backend' method.
 */
ham_status_t
btree_erase_cursor(common_btree_datums_t *btdata, ham_key_t * const key)
{
    ham_status_t st;
    ham_page_t *root;
    ham_page_t *p;
    ham_offset_t rootaddr;

    common_hints_t * const hints = &btdata->hints;
    ham_btree_t * const be = btdata->be;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    ham_bt_cursor_t * const cursor = btdata->cursor;
	const ham_u32_t flags = btdata->flags;

    erase_scratchpad_t scratchpad =
    {
        btdata,
        NULL
    };

    ham_assert(cursor ? (bt_cursor_get_flags(cursor) & BT_CURSOR_FLAG_UNCOUPLED) : 1, (0));

    ham_assert(scratchpad.btdata->in_key, (0));

    db_update_global_stats_erase_query(btdata, key->size);

    btree_erase_get_hints(btdata);

    if (hints->key_is_out_of_bounds)
    {
        stats_update_erase_fail_oob(btdata);
        return HAM_KEY_NOT_FOUND;
    }

    if (hints->try_fast_track)
    {
        /* TODO */
        ham_assert(1,(0));
    }

    /*
     * get the root-page...
     */
    rootaddr = btree_get_rootpage(be);
    if (!rootaddr)
    {
        stats_update_erase_fail(btdata);
        return HAM_KEY_NOT_FOUND;
    }
    st=db_fetch_page(&root, env, rootaddr, flags);
    ham_assert(st ? root == NULL : 1, (0));
    if (!root)
    {
        stats_update_erase_fail(btdata);
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(root, db);

    /*
     * ... and start the recursion
     */
    st = my_erase_recursive(&p, root, 0, 0, 0, 0, 0, &scratchpad);
    if (st)
    {
        stats_update_erase_fail(btdata);
        return st;
    }

    if (p)
    {
        /*
         * delete the old root page
         */
        st = bt_uncouple_all_cursors(root, 0);
        if (st)
        {
            stats_update_erase_fail(btdata);
            return (st);
        }

        st=my_collapse_root(p, &scratchpad);
        if (st)
        {
            stats_update_erase_fail(btdata);
            return (st);
        }

        stats_page_is_nuked(btdata, root, REASON_ERASE);

        st=txn_free_page(env_get_txn(env), root);
        if (st)
        {
            stats_update_erase_fail(btdata);
            return (st);
        }
    }

    stats_update_erase(btdata, hints->processed_leaf_page);
    stats_update_any_bound(btdata, hints->processed_leaf_page, key, hints->flags,
                    hints->processed_slot);

    return HAM_SUCCESS;
}

static ham_status_t
my_erase_recursive(ham_page_t ** const page_ref, ham_page_t * const page, ham_offset_t left, ham_offset_t right,
        ham_offset_t lanchor, ham_offset_t ranchor, ham_page_t * const parent,
        erase_scratchpad_t * const scratchpad)
{
    ham_s32_t slot;
    ham_bool_t isfew;
    ham_status_t st;
    ham_page_t *newme;
    btree_node_t *node = ham_page_get_btree_node(page);
    ham_u16_t node_keycount = btree_node_get_count(node);

    common_btree_datums_t * const btdata = scratchpad->btdata;
    const ham_size_t maxkeys = btdata->maxkeys;
    const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    common_hints_t * const hints = &btdata->hints;
    ham_key_t * const key = btdata->in_key;

    ham_assert(page_get_owner(page) == scratchpad->btdata->db, (0));

    *page_ref = 0;

    /*
     * empty node? then most likely we're in the empty root page.
     */
    if (node_keycount == 0)
    {
        return HAM_KEY_NOT_FOUND;
    }

    /*
     * mark the nodes which may need rebalancing.
	 *
     * Always trigger rebalancing when the page contains 1 key (or rather: 'minkeys').
     */
#if 0
    if (btree_get_rootpage(be) == page_get_self(page))
        isfew = (btree_node_get_count(node) <= 1);
    else
        isfew = (btree_node_get_count(node) < minkeys);
#else
    isfew = (node_keycount < minkeys);
#endif

    if (!isfew) /* [i_a] name does not represent value; cf. code in btree_check */
        scratchpad->mergepage = 0;
    else if (!scratchpad->mergepage)
        scratchpad->mergepage = page;

    /*
     * if this page is not a leaf: recursively descend down the tree
     */
    if (!btree_node_is_leaf(node))
    {
        ham_offset_t next_lanchor;
        ham_offset_t next_ranchor;
        ham_offset_t next_left;
        ham_offset_t next_right;
	    ham_page_t *child;

        st = btree_traverse_tree(&child, &slot, btdata, page, key);
        ham_assert(child != 0, ("guru meditation error"));
        if (!child)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }

        /*
         * calculate neighbor and anchor nodes
         */
        if (slot==-1) {
            if (!left)
                next_left=0;
            else {
                //int_key_t *keyarr;
                int_key_t *bte;
                btree_node_t *n;
			    ham_page_t *tempp;

                st=db_fetch_page(&tempp, env, left, 0);
                ham_assert(st ? tempp == NULL : tempp != NULL, (0));
                if (!page)
                {
                    return st ? st : HAM_INTERNAL_ERROR;
                }
                ham_assert(!st, (0));
                ham_assert(db, (0));
                page_set_owner(tempp, db);
                n = ham_page_get_btree_node(tempp);
                bte = btree_in_node_get_key_ref(btdata, page, btree_node_get_count(n) - 1);
                next_left = key_get_ptr(bte);
            }
            next_lanchor=lanchor;
        }
        else {
            if (slot==0)
                next_left=btree_node_get_ptr_left(node);
            else {
                //int_key_t *keyarr;
                int_key_t *bte;

				ham_assert(slot > 0, (0));
                bte = btree_in_node_get_key_ref(btdata, page, (ham_u16_t)(slot - 1));
                next_left=key_get_ptr(bte);
            }
            next_lanchor=page_get_self(page);
        }

        ham_assert(node_keycount == btree_node_get_count(node), (0));
		ham_assert(node_keycount > 0, (0));
        if (slot == node_keycount - 1)
        {
            if (!right)
            {
                next_right = 0;
            }
            else
            {
                //int_key_t *keyarr;
                int_key_t *bte;
                btree_node_t *n;
			    ham_page_t *tempp;

                st=db_fetch_page(&tempp, env, right, 0);
                ham_assert(st ? tempp == NULL : tempp != NULL, (0));
                if (!page)
                {
                    return st ? st : HAM_INTERNAL_ERROR;
                }
                ham_assert(!st, (0));
                ham_assert(db, (0));
                page_set_owner(tempp, db);
                n = ham_page_get_btree_node(tempp);
                bte = btree_in_node_get_key_ref(btdata, tempp, 0);
                next_right=key_get_ptr(bte);
            }
            next_ranchor=ranchor;
        }
        else
        {
            //int_key_t *keyarr;
            int_key_t *bte;

            bte = btree_in_node_get_key_ref(btdata, page, slot + 1);
            next_right = key_get_ptr(bte);
            next_ranchor = page_get_self(page);
        }

        st = my_erase_recursive(&newme, child, next_left, next_right, next_lanchor,
                    next_ranchor, page, scratchpad);
        if (st)
            return st;
		//ham_assert(newme ? newme == child : 1, (0));
    }
    else
    {
        /*
         * otherwise (page is a leaf) delete the key...
         *
         * first, check if this entry really exists
         */
        st=btree_get_slot(btdata, page, key, &slot, 0);
        if (st) {
            return st;
        }

        newme = 0;
        if (slot != -1)
        {
            int cmp;
            //int_key_t *keyarr;
            int_key_t *bte;
            ham_key_t rhs;

			ham_assert(slot >= 0, (0));
            bte = btree_in_node_get_key_ref(btdata, page, (ham_u16_t)slot);

            st = db_prepare_ham_key_for_compare(db, 1, bte, &rhs);
            if (st)
                return st;

            ham_assert(!(ham_key_get_intflags(key) & KEY_IS_EXTENDED), (0));
            cmp = db_compare_keys(db, key, &rhs, NULL, page);

            if (cmp < -1)
                return (ham_status_t)cmp;

            if (cmp==0)
            {
                newme = page;
            }
            else
            {
                return HAM_KEY_NOT_FOUND;
            }
        }
        if (!newme)
        {
            scratchpad->mergepage = 0;
            return HAM_KEY_NOT_FOUND;
        }
    }

    /*
     * ... and rebalance the tree, if necessary
     */
    if (newme)
    {
        //ham_assert(slot != -1, (0)); /* [i_a] shouldn't ever happen, right? */

        if (slot==-1)
            slot=0;
        st=my_remove_entry(page, slot, scratchpad);
        if (st)
            return st;
    }

    /*
     * No need to rebalance in case of an error.
	 *
	 * Nor do we rebalance when we're in the middle of an 'uber fast' erase operation:
	 * that'll save us time NOW (and possibly bite us in the ass later on, but alas...)
	 *
	 * Note that we MUST rebalance -- in order to prevent /serious/ trouble later on -- when
	 * the node has become close to empty.
	 *
	 * Also note that we don't have to rebalance anything when all the erase did was
	 * remove one of the duplicates for a given key, i.e. when the key count of the node
	 * remains unchanged (after all, that count does not count duplicate keys).
     */
    ham_assert(!st, (0));

	if (hints->flags & (HAM_HINT_SEQUENTIAL | HAM_HINT_UBER_FAST_ACCESS))
	{
	    ham_u16_t node_keycount_new = btree_node_get_count(node);

		/*
		heuristic:

		1) allow non-leaf pages to 'deteriorate' while their key count is still above 1/4 MAX.

		2) allow leaf pages to stay as is while their key count is >= 3, unless the hints are
		   NOT 'super fast' in which case we allow a dip down to 1/4 MAX.

		3) as mentioned in the notes above: do not rebalance when only a duplicate was removed.

		Basically this means we're very aggressive in 'super fast' mode where it comes to
		leaf pages: having those remain almost completely empty is considered 'okay', but we
		are a little more conservative, i.e. desire a situation that's closer to maintaining
		a fully balanced tree, where non-leaf (branch) nodes are concerned.
		*/
		if (node_keycount_new == node_keycount)
		{
			*page_ref = 0;
			scratchpad->mergepage = 0;
			return HAM_SUCCESS; /* case #3 */
		}
		else
		{
			const ham_size_t minkeys_quart = ((!btree_node_is_leaf(node) || (hints->flags & HAM_HINT_UBER_FAST_ACCESS) == 0)
											? (maxkeys * MK_HAM_FLOAT(0.25) + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1		// round position value
											: 3 - 1); // case #1

			/* keep in mind that the hinter might have produced a skewed merge_split ratio! */
			if ((minkeys_quart < minkeys ? node_keycount_new > minkeys_quart : node_keycount_new > minkeys)
				&& node_keycount_new >= 3)
			{
				*page_ref = 0;
				scratchpad->mergepage = 0;
				return HAM_SUCCESS; /* case #1+#2('unless ...') */
			}
		}
	}

    return my_rebalance(page_ref, page, left, right, lanchor, ranchor, parent, scratchpad);
}

static ham_status_t
my_collapse_root(ham_page_t *newroot, erase_scratchpad_t *scratchpad)
{
    common_btree_datums_t * const btdata = scratchpad->btdata;
    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t keywidth = btdata->keywidth;
    //ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    ham_btree_t * const be = btdata->be;

    btree_set_rootpage(be, page_get_self(newroot));
    be_set_dirty(be, HAM_TRUE);

    be->_fun_flush(be);

    ham_assert(page_get_owner(newroot), (0));
    //env = db_get_env(page_get_owner(newroot));
    env_set_dirty(env);

    /* the root page was modified (btree_set_rootpage) - make sure that
     * it's logged */
    if (env_get_rt_flags(env) & HAM_ENABLE_RECOVERY) {
        ham_status_t st=txn_add_page(env_get_txn(env), env_get_header_page(env),
                HAM_TRUE);
        if (st)
            return (st);
    }

    if (env_get_cache(env) && (page_get_pers_type(newroot) != PAGE_TYPE_B_ROOT))
    {
        ham_cache_t *cache = env_get_cache(env);
        /*
         * As we re-purpose a page, we will reset its pagecounter as
         * well to signal its first use as the new type assigned here.
         */
        ham_assert(cache, (0));
        //page_set_cache_cntr(newroot, cache->_timeslot++);
        cache_update_page_access_counter(newroot, cache); /* bump up */
    }
    page_set_pers_type(newroot, PAGE_TYPE_B_ROOT);
    return HAM_SUCCESS;
}

static ham_status_t
my_rebalance(ham_page_t **newpage_ref, ham_page_t *page, ham_offset_t left, ham_offset_t right,
        ham_offset_t lanchor, ham_offset_t ranchor, ham_page_t *parent,
        erase_scratchpad_t *scratchpad)
{
    ham_status_t st;
    btree_node_t *node = ham_page_get_btree_node(page);
    ham_page_t *leftpage;
    ham_page_t *rightpage;
    btree_node_t *leftnode = NULL;
    btree_node_t *rightnode = NULL;
    ham_bool_t fewleft = HAM_FALSE;
    ham_bool_t fewright = HAM_FALSE;

    common_btree_datums_t * const btdata = scratchpad->btdata;
    const ham_size_t maxkeys = btdata->maxkeys;
    const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    ham_device_t * const dev = btdata->dev;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;

    ham_assert(page_get_owner(page) == db, (0));
    ham_assert(dev == page_get_device(page), (0));
    ham_assert(page_get_owner(page) ? device_get_env(dev) == db_get_env(page_get_owner(page)) : 1, (0));

    *newpage_ref = 0;
    if (!scratchpad->mergepage)
    {
        return HAM_SUCCESS;
    }

    ham_assert(device_get_env(dev) == env, (0));

    /*
     * get the left and the right sibling of this page
     */
    if (left)
    {
        st = db_fetch_page(&leftpage, env,
                        btree_node_get_left(node), 0);
        ham_assert(st ? leftpage == NULL : leftpage != NULL, (0));
        if (!leftpage)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(leftpage, db);
        if (leftpage)
        {
            leftnode = ham_page_get_btree_node(leftpage);
            fewleft = (btree_node_get_count(leftnode) <= minkeys);
        }
    }
    else
    {
        leftpage = NULL;
    }
    if (right)
    {
        st = db_fetch_page(&rightpage, env,
                        btree_node_get_right(node), 0);
        ham_assert(st ? rightpage == NULL : rightpage != NULL, (0));
        if (!rightpage)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(rightpage, db);
        if (rightpage)
        {
            rightnode = ham_page_get_btree_node(rightpage);
            fewright = (btree_node_get_count(rightnode) <= minkeys);
        }
    }
    else
    {
        rightpage = NULL;
    }

    /*
     * if we have no siblings, then we're rebalancing the root page
     */
    if (!leftpage && !rightpage)
    {
        if (btree_node_is_leaf(node))
        {
            return HAM_SUCCESS;
        }
        else
        {
            st = db_fetch_page(newpage_ref, env,
                        btree_node_get_ptr_left(node), 0);
            ham_assert(st ? *newpage_ref == NULL : *newpage_ref != NULL, (0));
            if (!*newpage_ref)
            {
                return st ? st : HAM_INTERNAL_ERROR;
            }
            ham_assert(!st, (0));
            ham_assert(db, (0));
            page_set_owner(*newpage_ref, db);
            return HAM_SUCCESS;
        }
    }

    /*
     * if one of the siblings is missing, or both of them are
     * too empty, we have to merge them
     */
    if ((!leftpage || fewleft) && (!rightpage || fewright))
    {
        if (parent && lanchor != page_get_self(parent))
        {
            return my_merge_pages(newpage_ref, page, rightpage, ranchor, scratchpad);
        }
        else
        {
            return my_merge_pages(newpage_ref, leftpage, page, lanchor, scratchpad);
        }
    }

    /*
     * otherwise choose the better of a merge or a shift
     */
    if (leftpage && fewleft && rightpage && !fewright)
    {
        if (parent && (!(ranchor == page_get_self(parent)) &&
                (page_get_self(page) == page_get_self(scratchpad->mergepage))))
        {
            return my_merge_pages(newpage_ref, leftpage, page, lanchor, scratchpad);
        }
        else
        {
            return my_shift_pages(newpage_ref, page, rightpage, ranchor, scratchpad);
        }
    }

    /*
     * ... still choose the better of a merge or a shift...
     */
    if (leftpage && !fewleft && rightpage && fewright)
    {
        if (parent && (!(lanchor == page_get_self(parent)) &&
                (page_get_self(page) == page_get_self(scratchpad->mergepage))))
        {
            return my_merge_pages(newpage_ref, page, rightpage, ranchor, scratchpad);
        }
        else
        {
            return my_shift_pages(newpage_ref, leftpage, page, lanchor, scratchpad);
        }
    }

    /*
     * choose the more effective of two shifts
     */
    if (lanchor == ranchor)
    {
        if (btree_node_get_count(leftnode) <= btree_node_get_count(rightnode))
        {
            return my_shift_pages(newpage_ref, page, rightpage, ranchor, scratchpad);
        }
        else
        {
            return my_shift_pages(newpage_ref, leftpage, page, lanchor, scratchpad);
        }
    }

    /*
     * choose the shift with more local effect
     */
    if (parent && lanchor == page_get_self(parent))
    {
        return my_shift_pages(newpage_ref, leftpage, page, lanchor, scratchpad);
    }
    else
    {
        return my_shift_pages(newpage_ref, page, rightpage, ranchor, scratchpad);
    }
}

static ham_status_t
my_merge_pages(ham_page_t **newpage_ref, ham_page_t *page, ham_page_t *sibpage, ham_offset_t anchor,
        erase_scratchpad_t *scratchpad)
{
    ham_status_t st;
    ham_size_t c;
    //ham_size_t keysize;
    //ham_db_t *db = page_get_owner(page);
    //ham_btree_t *be = be;
    ham_page_t *ancpage;
    btree_node_t *node;
    btree_node_t *sibnode;
    btree_node_t *ancnode;
    int_key_t *bte_lhs;
    int_key_t *bte_rhs;
    //int_key_t *sibkeyarr;
    //int_key_t *keyarr;
    //int_key_t *anckeyarr;
    ham_size_t sibnode_keycount;
    ham_size_t node_keycount;

    common_btree_datums_t * const btdata = scratchpad->btdata;
    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t keywidth = btdata->keywidth;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    common_hints_t * const hints = &btdata->hints;

    ham_assert(db, (0));
    //env = env;

    node = ham_page_get_btree_node(page);
    sibnode = ham_page_get_btree_node(sibpage);

    sibnode_keycount = btree_node_get_count(sibnode);
    node_keycount = btree_node_get_count(node);

    if (anchor)
    {
        st=db_fetch_page(&ancpage, env, anchor, 0);
        ham_assert(st ? ancpage == NULL : ancpage != NULL, (0));
        if (!ancpage)
        {
            return st ? st : HAM_INTERNAL_ERROR;
        }
        ham_assert(!st, (0));
        ham_assert(db, (0));
        page_set_owner(ancpage, db);
        ancnode = ham_page_get_btree_node(ancpage);
    }
    else
    {
        ancpage=0;
        ancnode=0;
    }

    *newpage_ref = 0;

    /*
     * prepare all pages for the log
     */
    st = ham_log_add_page_before(page);
    if (st)
        return st;
    st = ham_log_add_page_before(sibpage);
    if (st)
        return st;
    if (ancpage)
    {
        st = ham_log_add_page_before(ancpage);
        if (st)
            return st;
    }

    /*
     * uncouple all cursors
     */
    st = bt_uncouple_all_cursors(page, 0);
    if (st)
        return st;
    st = bt_uncouple_all_cursors(sibpage, 0);
    if (st)
        return st;
    if (ancpage)
    {
        st = bt_uncouple_all_cursors(ancpage, 0);
        if (st)
            return st;
    }

    /*
     * internal node: append the anchornode separator value to
     * this node
     */
    if (!btree_node_is_leaf(node))
    {
        int_key_t *bte;
        ham_key_t key;
	    ham_s32_t slot;

        bte = btree_in_node_get_key_ref(btdata, sibpage, 0);
        memset(&key, 0, sizeof(key));
        key._flags = key_get_flags(bte);
        key.data = key_get_key(bte);
        key.size = key_get_size(bte);

        st=btree_get_slot(btdata, ancpage, &key, &slot, 0);
        if (st) {
            return st;
        }

		ham_assert(slot >= 0, (0));
        ham_assert(node_keycount == btree_node_get_count(node), (0));
        bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount);
        bte_rhs = btree_in_node_get_key_ref(btdata, ancpage, slot);

        st=my_copy_key(btdata, bte_lhs, bte_rhs);
        if (st) {
            return st;
        }
        key_set_ptr(bte_lhs, btree_node_get_ptr_left(sibnode));
        node_keycount++;
        btree_node_set_count(node, node_keycount);
    }

    ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
    c = sibnode_keycount;
    ham_assert(node_keycount == btree_node_get_count(node), (0));
    bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount);
    bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, 0);

    /*
     * shift items from the sibling to this page
     */
    btree_move_key_series(btdata, page, sibpage, bte_lhs, bte_rhs, c); // memcpy(bte_lhs, bte_rhs, keywidth * c);

    /*
     * as sibnode is merged into node, we will also need to ensure that our
     * statistics node/page tracking is corrected accordingly: what was in sibnode, is now in
     * node. And sibnode will be destroyed at the end.
     */
    if (sibpage == hints->processed_leaf_page)
    {
        /* sibnode slot 0 has become node slot 'bte_lhs' */
        ham_assert(node_keycount == btree_node_get_count(node), (0));
        hints->processed_slot += node_keycount;
        hints->processed_leaf_page = page;
    }

    page_set_dirty(page, env);
    page_set_dirty(sibpage, env);
    ham_assert(node_keycount == btree_node_get_count(node), (0));
    ham_assert(node_keycount + c <= MAX_KEYS_PER_NODE, (0));
    btree_node_set_count(node, node_keycount + c);
    btree_node_set_count(sibnode, 0);

    /*
     * update the linked list of pages
     */
    if (btree_node_get_left(node) == page_get_self(sibpage))
    {
        if (btree_node_get_left(sibnode))
        {
            ham_page_t *p;
            btree_node_t *n;

            st=db_fetch_page(&p, env,
                    btree_node_get_left(sibnode), 0);
            ham_assert(st ? p == NULL : p != NULL, (0));
            if (!p)
            {
                return st ? st : HAM_INTERNAL_ERROR;
            }
            ham_assert(!st, (0));
            ham_assert(db, (0));
            page_set_owner(p, db);
            n=ham_page_get_btree_node(p);
            btree_node_set_right(n, btree_node_get_right(sibnode));
            btree_node_set_left(node, btree_node_get_left(sibnode));
            page_set_dirty(p, env);
        }
        else
        {
            btree_node_set_left(node, 0);
        }
    }
    else if (btree_node_get_right(node) == page_get_self(sibpage))
    {
        if (btree_node_get_right(sibnode))
        {
            ham_page_t *p;
            btree_node_t *n;

            st=db_fetch_page(&p, env,
                    btree_node_get_right(sibnode), 0);
            ham_assert(st ? p == NULL : p != NULL, (0));
            if (!p)
            {
                return st ? st : HAM_INTERNAL_ERROR;
            }
            ham_assert(!st, (0));
            ham_assert(db, (0));
            page_set_owner(p, db);
            n = ham_page_get_btree_node(p);

            btree_node_set_right(node, btree_node_get_right(sibnode));
            btree_node_set_left(n, btree_node_get_left(sibnode));
            page_set_dirty(p, env);
        }
        else
        {
            btree_node_set_right(node, 0);
        }
    }

    /*
     * return this page for deletion
     */
    if (scratchpad->mergepage &&
           (page_get_self(scratchpad->mergepage) == page_get_self(page) ||
            page_get_self(scratchpad->mergepage) == page_get_self(sibpage)))
    {
        scratchpad->mergepage=0;
    }

    stats_page_is_nuked(btdata, sibpage, REASON_MERGE);

    /*
     * delete the page
     */
    ham_assert(hints->processed_leaf_page != sibpage, (0));
    st=txn_free_page(env_get_txn(env), sibpage);
    if (st) {
        return st;
    }

    *newpage_ref = sibpage;
    return HAM_SUCCESS;
}

static ham_status_t
my_shift_pages(ham_page_t **newpage_ref, ham_page_t *page, ham_page_t *sibpage, ham_offset_t anchor,
        erase_scratchpad_t *scratchpad)
{
    ham_s32_t slot = 0;
    ham_status_t st;
    ham_bool_t intern;
    ham_size_t s;
    ham_size_t c;
    ham_page_t *ancpage;
    btree_node_t *node;
    btree_node_t *sibnode;
    btree_node_t *ancnode;
    int_key_t *bte_lhs;
    int_key_t *bte_rhs;
    //int_key_t *sibkeyarr;
    //int_key_t *keyarr;
    //int_key_t *anckeyarr;
    ham_size_t node_keycount;
    ham_size_t sibnode_keycount;

    common_btree_datums_t * const btdata = scratchpad->btdata;
    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t keywidth = btdata->keywidth;
    //const ham_size_t keysize = btdata->keysize;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    node = ham_page_get_btree_node(page);
    node_keycount = btree_node_get_count(node);
    sibnode = ham_page_get_btree_node(sibpage);
    sibnode_keycount = btree_node_get_count(sibnode);

    intern = !btree_node_is_leaf(node);
    st = db_fetch_page(&ancpage, env, anchor, 0);
    ham_assert(st ? ancpage == NULL : ancpage != NULL, (0));
    if (!ancpage)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(ancpage, db);
    ancnode = ham_page_get_btree_node(ancpage);

    ham_assert(node_keycount != sibnode_keycount, (0));

    *newpage_ref = 0;

    /*
     * prepare all pages for the log
     */
    st = ham_log_add_page_before(page);
    if (st)
        return st;
    st = ham_log_add_page_before(sibpage);
    if (st)
        return st;
    if (ancpage)
    {
        st = ham_log_add_page_before(ancpage);
        if (st)
            return st;
    }

    /*
     * uncouple all cursors
     */
    st = bt_uncouple_all_cursors(page, 0);
    if (st)
        return st;
    st = bt_uncouple_all_cursors(sibpage, 0);
    if (st)
        return st;
    if (ancpage)
    {
        st=bt_uncouple_all_cursors(ancpage, 0);
        if (st)
            return st;
    }

    /*
     * shift from sibling to this node
     */
    ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
    if (sibnode_keycount >= node_keycount)
    {
        /*
         * internal node: insert the anchornode separator value to
         * this node
         */
        if (intern)
        {
            int_key_t *bte;
            ham_key_t key;

            bte = btree_in_node_get_key_ref(btdata, sibpage, 0);
            memset(&key, 0, sizeof(key));
            key._flags = key_get_flags(bte);
            key.data = key_get_key(bte);
            key.size = key_get_size(bte);
            st=btree_get_slot(btdata, ancpage, &key, &slot, 0);
            if (st) {
                return st;
            }

            /*
             * append the anchor node to the page
             */
            bte_rhs = btree_in_node_get_key_ref(btdata, ancpage, slot);
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount);

            st=my_copy_key(btdata, bte_lhs, bte_rhs);
            if (st) {
                return st;
            }

            /*
             * the pointer of this new node is ptr_left of the sibling
             */
            key_set_ptr(bte_lhs, btree_node_get_ptr_left(sibnode));

            /*
             * new pointer left of the sibling is sibling[0].ptr
             */
            btree_node_set_ptr_left(sibnode, key_get_ptr(bte));

            /*
             * update the anchor node with sibling[0]
             */
            (void)my_replace_key(ancpage, slot, bte, INTERNAL_KEY, btdata);

            /*
             * shift the remainder of sibling to the left
             */
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
            ham_assert(sibnode_keycount > 0, (0));

            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 1);
            btree_move_key_series(btdata, sibpage, sibpage, bte_lhs, bte_rhs, sibnode_keycount - 1); // memmove(bte_lhs, bte_rhs, keywidth * (sibnode_keycount-1));

            /*
             * adjust counters
             */
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
            node_keycount++;
            btree_node_set_count(node, node_keycount);
            sibnode_keycount--;
            btree_node_set_count(sibnode, sibnode_keycount);
        }

        // split_ratio ???
		//
		// Hm, this is part of the rebalancing act, where keys are equally distributed among both pages.
		// Question is: do we still want 'equally distributed' here when we've got hints/statistics?
        ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
        c = (sibnode_keycount - node_keycount) / 2;
        if (c==0)
            goto cleanup;
        if (intern)
            c--;
        if (c==0)
            goto cleanup;

        /*
         * internal node: append the anchor key to the page
         */
        if (intern)
        {
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount);
            bte_rhs = btree_in_node_get_key_ref(btdata, ancpage, slot);

            st=my_copy_key(btdata, bte_lhs, bte_rhs);
            if (st) {
                return st;
            }

            key_set_ptr(bte_lhs, btree_node_get_ptr_left(sibnode));
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            node_keycount++;
            btree_node_set_count(node, node_keycount);
        }

        /*
         * shift items from the sibling to this page, then
         * delete the shifted items
         */
        ham_assert(node_keycount == btree_node_get_count(node), (0));
        bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount);
        bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, 0);

        btree_move_key_series(btdata, page, sibpage, bte_lhs, bte_rhs, c); // memmove(bte_lhs, bte_rhs, keywidth * c);

        bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
        bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, c);
        btree_move_key_series(btdata, sibpage, sibpage, bte_lhs, bte_rhs, sibnode_keycount - c); // memmove(bte_lhs, bte_rhs, keywidth * (sibnode_keycount - c));

        /*
         * internal nodes: don't forget to set ptr_left of the sibling, and
         * replace the anchor key
         */
        if (intern)
        {
            int_key_t *bte;
            bte = btree_in_node_get_key_ref(btdata, sibpage, 0);
            btree_node_set_ptr_left(sibnode, key_get_ptr(bte));
            if (anchor)
            {
                ham_key_t key = {0};
                //memset(&key, 0, sizeof(key));
                key._flags = key_get_flags(bte);
                key.data = key_get_key(bte);
                key.size = key_get_size(bte);
                st=btree_get_slot(btdata, ancpage, &key, &slot, 0);
                if (st) {
                    return st;
                }
                /* replace the key */
                st=my_replace_key(ancpage, slot, bte, INTERNAL_KEY, btdata);
                if (st) {
                    return st;
                }
            }
            /*
             * shift once more
             */
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
#if 0
            bte_lhs = sibkeyarr + 0;
            bte_rhs = sibkeyarr + 1;
#else
            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
            bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, 1);
#endif
            btree_move_key_series(btdata, sibpage, sibpage, bte_lhs, bte_rhs, sibnode_keycount - 1); // memmove(bte_lhs, bte_rhs, keywidth * (sibnode_keycount - 1));
        }
        else
        {
            /*
             * in a leaf - update the anchor
             */
            ham_key_t key = {0};
            int_key_t *bte;
#if 0
            bte = sibkeyarr + 0;
#else
            bte = btree_in_node_get_key_ref(btdata, sibpage, 0);
#endif
            //memset(&key, 0, sizeof(key));
            key._flags = key_get_flags(bte);
            key.data = key_get_key(bte);
            key.size = key_get_size(bte);
            st=btree_get_slot(btdata, ancpage, &key, &slot, 0);
            if (st) {
                return st;
            }
            /* replace the key */
            st=my_replace_key(ancpage, slot, bte, INTERNAL_KEY, btdata);
            if (st) {
                return st;
            }
        }

        /*
         * update the page counter
         */
        ham_assert(node_keycount == btree_node_get_count(node), (0));
        ham_assert(node_keycount + c <= MAX_KEYS_PER_NODE, (0));
        ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
        ham_assert(sibnode_keycount - c - (intern ? 1 : 0) <= MAX_KEYS_PER_NODE, (0));
        node_keycount += c;
        btree_node_set_count(node, node_keycount);
        sibnode_keycount -= c + (intern ? 1 : 0);
        btree_node_set_count(sibnode, sibnode_keycount);
    }
    else
    {
        /*
         * shift from this node to the sibling
         */

        /*
        * internal node: insert the anchornode separator value to
        * this node
        */
        if (intern)
        {
            int_key_t *bte;
            ham_key_t key = {0};

            bte = btree_in_node_get_key_ref(btdata, sibpage, 0);
            //memset(&key, 0, sizeof(key));
            key._flags = key_get_flags(bte);
            key.data = key_get_key(bte);
            key.size = key_get_size(bte);
            st=btree_get_slot(btdata, ancpage, &key, &slot, 0);
            if (st) {
                return st;
            }

            /*
             * shift entire sibling by 1 to the right
             */
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 1);
            bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
            btree_move_key_series(btdata, sibpage, sibpage, bte_lhs, bte_rhs, sibnode_keycount); // memmove(bte_lhs, bte_rhs, keywidth * (sibnode_keycount));

            /*
             * copy the old anchor element to sibling[0]
             */
            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
            bte_rhs = btree_in_node_get_key_ref(btdata, ancpage, slot);

            st=my_copy_key(btdata, bte_lhs, bte_rhs);
            if (st) {
                return st;
            }

            /*
             * sibling[0].ptr = sibling.ptr_left
             */
            key_set_ptr(bte_lhs, btree_node_get_ptr_left(sibnode));

            /*
             * sibling.ptr_left = node[node.count-1].ptr
             */
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount - 1);
            btree_node_set_ptr_left(sibnode, key_get_ptr(bte_lhs));

            /*
             * new anchor element is node[node.count-1].key
             */
            st=my_replace_key(ancpage, slot, bte_lhs, INTERNAL_KEY, btdata);
            if (st) {
                return st;
            }

            /*
             * page: one item less; sibling: one item more
             */
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
            node_keycount--;
            btree_node_set_count(node, node_keycount);
            sibnode_keycount++;
            btree_node_set_count(sibnode, sibnode_keycount);
        }

        // split_ratio ???
		//
		// Hm, this is part of the rebalancing act, where keys are equally distributed among both pages.
		// Question is: do we still want 'equally distributed' here when we've got hints/statistics?
        ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
        c = (node_keycount - sibnode_keycount)/2;
        if (c==0)
            goto cleanup;
        if (intern)
            c--;
        if (c==0)
            goto cleanup;

        /*
         * internal pages: insert the anchor element
         */
        if (intern)
        {
            /*
             * shift entire sibling by 1 to the right
             */
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));

            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 1);
            bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
            btree_move_key_series(btdata, sibpage, sibpage, bte_lhs, bte_rhs, sibnode_keycount); // memmove(bte_lhs, bte_rhs, keywidth * sibnode_keycount);

            bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
            bte_rhs = btree_in_node_get_key_ref(btdata, ancpage, slot);

            /* clear the key - we don't want my_replace_key to free
             * an extended block which is still used by sibnode[1] */
            memset(bte_lhs, 0, sizeof(*bte_lhs));

            st=my_replace_key(sibpage, 0, bte_rhs,
                    (btree_node_is_leaf(node) ? 0 : INTERNAL_KEY),
                    btdata);
            if (st) {
                return st;
            }

            key_set_ptr(bte_lhs, btree_node_get_ptr_left(sibnode));
            ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
            sibnode_keycount++;
            btree_node_set_count(sibnode, sibnode_keycount);
        }

        s = node_keycount - c - 1;

        /*
         * shift items from this page to the sibling, then delete the
         * items from this page
         */
        ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
        bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, c);
        bte_rhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
        ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
        btree_move_key_series(btdata, sibpage, sibpage, bte_lhs, bte_rhs, sibnode_keycount); // memmove(bte_lhs, bte_rhs, keywidth * sibnode_keycount);

        bte_lhs = btree_in_node_get_key_ref(btdata, sibpage, 0);
        bte_rhs = btree_in_node_get_key_ref(btdata, page, s + 1);
        btree_move_key_series(btdata, sibpage, page, bte_lhs, bte_rhs, c); // memmove(bte_lhs, bte_rhs, keywidth * c);

        ham_assert(node_keycount == btree_node_get_count(node), (0));
        ham_assert(node_keycount - c <= MAX_KEYS_PER_NODE, (0));
        ham_assert(sibnode_keycount == btree_node_get_count(sibnode), (0));
        ham_assert(sibnode_keycount + c <= MAX_KEYS_PER_NODE, (0));
        node_keycount -= c;
        btree_node_set_count(node, node_keycount);
        sibnode_keycount += c;
        btree_node_set_count(sibnode, sibnode_keycount);

        /*
         * internal nodes: the pointer of the highest item
         * in the node will become the ptr_left of the sibling
         */
        if (intern)
        {
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            bte_lhs = btree_in_node_get_key_ref(btdata, page, node_keycount - 1);
            btree_node_set_ptr_left(sibnode, key_get_ptr(bte_lhs));

            /*
             * free the extended blob of this key
             */
            if (key_get_flags(bte_lhs)&KEY_IS_EXTENDED) {
                ham_offset_t blobid=key_get_extended_rid(db, bte_lhs);
                ham_assert(blobid, (0));

                st=extkey_remove(db, blobid);
                if (st)
                    return st;
            }
            ham_assert(node_keycount == btree_node_get_count(node), (0));
            node_keycount--;
            btree_node_set_count(node, node_keycount);
        }

        /*
         * replace the old anchor key with the new anchor key
         */
        if (anchor)
        {
            int_key_t *bte;
            ham_key_t key = {0};
            //memset(&key, 0, sizeof(key));

            if (intern)
            {
                bte = btree_in_node_get_key_ref(btdata, page, s);
            }
            else
            {
                bte = btree_in_node_get_key_ref(btdata, sibpage, 0);
            }

            key._flags = key_get_flags(bte);
            key.data = key_get_key(bte);
            key.size = key_get_size(bte);

            st=btree_get_slot(btdata, ancpage, &key, &slot, 0);
            if (st) {
                return st;
            }

            st=my_replace_key(ancpage, slot+1, bte, INTERNAL_KEY, btdata);
            if (st) {
                return st;
            }
        }
    }

cleanup:
    /*
     * mark pages as dirty
     */
    page_set_dirty(page, env);
    page_set_dirty(ancpage, env);
    page_set_dirty(sibpage, env);

    scratchpad->mergepage=0;

    return st;
}

static ham_status_t
my_copy_key(common_btree_datums_t *btdata, int_key_t *lhs, int_key_t *rhs)
{
    //ham_backend_t *be = db_get_backend(db);

    //const ham_size_t maxkeys = btdata->maxkeys;
    const ham_size_t keywidth = btdata->keywidth;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    memcpy(lhs, rhs, keywidth);

    /*
     * if the key is extended, we copy the extended blob; otherwise, we'd
     * have to add reference counting to the blob, because two keys are now
     * using the same blobid. this would be too complicated.
	 *
	 * [GHo] comment about that: my_copy_key() is only used to duplicate keys
	 *       from child to parent nodes; you do NOT need reference counting
	 *       when you simply make all extended keys in non-leaf nodes point
	 *       to the extended key storage for the leaf node: after all, any
	 *       key occurring in a non-leaf node MUST also appear in a leaf
	 *       node somewhere.
	 *       This approach would also improve merge/split speed a little
	 *       as you don't need to copy the extended key data, ever.
	 *
	 *       Wicked scenarios with fail risk: when a key is 'updated', it's
	 *       basically a 'delete-insert' cycle, so any key changes will
	 *       be applied to all instances of that key in the B+-tree.
	 *       Unless, that is, we introduce new optimized 'update' code
	 *       which 'rewrites' a key in that special circumstance where changing
	 *       it does not alter its place in the sort order, i.e. position
	 *       in the tree. Now THAT would be an optimization for VERY RARE
	 *       circumstances; I don't think think it's useful to implement,
	 *       as the rather more generic 'hold off tree rebalancing' idea
	 *       will produce the same effect, and then some.
	 *
	 *       Conclusion: we can safely assume non-leaf node keys have their
	 *       extkey data pointing at the same space as the same key in a
	 *       leaf node.
	 *
	 *       Now all we need is a 'backwards compatible' approach which
	 *       copes with older databases where each key would carry its own
	 *       extkey copy... maybe a flag bit to signal the extkey data is
	 *       NOT duplicated? Yeah, that should be enough...
     */
    if (key_get_flags(rhs) & KEY_IS_EXTENDED)
    {
        ham_status_t st;
        ham_record_t record = {0};
        ham_offset_t rhsblobid;
        ham_offset_t lhsblobid;
        dev_alloc_request_info_ex_t info = {0};
        ham_key_t key_for_info;

        info.db = db;
        info.env = env;
        info.dam = db_get_data_access_mode(db);
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_EXTKEYSPACE; // PAGE_TYPE_BLOB;
        info.int_key = rhs;

        //memset(&record, 0, sizeof(record));

        rhsblobid = key_get_extended_rid(db, rhs);
        st=blob_read(db, rhsblobid, &record, 0);
        if (st)
            return (st);

        key_for_info.data = record.data;
        key_for_info.size = record.size;
        key_for_info.flags = HAM_KEY_USER_ALLOC;
        info.key = &key_for_info;

        st = blob_allocate(env, &record, 0, &info, NULL, &lhsblobid);
        if (st)
            return (st);
        key_set_extended_rid(db, lhs, lhsblobid);
		ham_nuke_stack_space(record);
    }

    return HAM_SUCCESS;
}

static ham_status_t
my_replace_key(ham_page_t *page, ham_s32_t slot,
        int_key_t *rhs, ham_u32_t flags, common_btree_datums_t *btdata)
{
    int_key_t *lhs;
    //int_key_t *keyarr;
    ham_status_t st;
    //btree_node_t *node = ham_page_get_btree_node(page);

    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t keywidth = btdata->keywidth;
    const ham_size_t keysize = btdata->keysize;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    //common_hints_t * const hints = &btdata->hints;

    /*
     * prepare page for the log
     */
    st = ham_log_add_page_before(page);
    if (st)
        return (st);

    /*
     * uncouple all cursors
     */
    st = bt_uncouple_all_cursors(page, 0);
    if (st)
        return st;

    lhs = btree_in_node_get_key_ref(btdata, page, slot);

    /*
     * if we overwrite an extended key: delete the existing extended blob
     */
    if (key_get_flags(lhs) & KEY_IS_EXTENDED)
    {
        ham_offset_t blobid = key_get_extended_rid(db, lhs);
        ham_assert(blobid, (0));

        st = extkey_remove(db, blobid);
        if (st)
            return (st);
    }

    key_set_flags(lhs, key_get_flags(rhs));
    memcpy(key_get_key(lhs), key_get_key(rhs), keysize /* key_get_size(rhs) */ );

    /*
     * internal keys are not allowed to have blob-flags, because only the
     * leaf-node can manage the blob. Therefore we have to disable those
     * flags if we modify an internal key.
     */
    if (flags & INTERNAL_KEY)
    {
        key_set_flags(lhs, key_get_flags(lhs)&
                ~(KEY_BLOB_SIZE_TINY
                    |KEY_BLOB_SIZE_SMALL
                    |KEY_BLOB_SIZE_EMPTY
                    |KEY_HAS_DUPLICATES));
    }

    /*
     * if this key is extended, we copy the extended blob; otherwise, we'd
     * have to add reference counting to the blob, because two keys are now
     * using the same blobid. this would be too complicated.
     */
    if (key_get_flags(rhs) & KEY_IS_EXTENDED)
    {
        ham_status_t st;
        ham_record_t record = {0};
        ham_offset_t rhsblobid;
        ham_offset_t lhsblobid;
        dev_alloc_request_info_ex_t info = {0};
        ham_key_t key_for_info;

        info.db = db;
        info.env = env;
        info.dam = db_get_data_access_mode(db);
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_EXTKEYSPACE; // PAGE_TYPE_BLOB;
        info.int_key = rhs;

        //memset(&record, 0, sizeof(record));

        rhsblobid=key_get_extended_rid(db, rhs);
        st=blob_read(db, rhsblobid, &record, 0);
        if (st)
            return (st);

        key_for_info.data = record.data;
        key_for_info.size = record.size;
        key_for_info.flags = HAM_KEY_USER_ALLOC;
        info.key = &key_for_info;

        st = blob_allocate(env, &record, 0, &info, NULL, &lhsblobid);
        if (st)
            return (st);
        key_set_extended_rid(db, lhs, lhsblobid);
		ham_nuke_stack_space(record);
    }

    key_set_size(lhs, key_get_size(rhs));

    page_set_dirty(page, env);

    return (HAM_SUCCESS);
}

static ham_status_t
my_remove_entry(ham_page_t *page, ham_s32_t slot,
        erase_scratchpad_t *scratchpad)
{
    ham_status_t st;
    int_key_t *bte_lhs;
    int_key_t *bte_rhs;
    int_key_t *bte;
    //int_key_t *keyarr;
    btree_node_t *node;
    //ham_db_t *db;
    //ham_btree_t *be;
    //ham_size_t keywidth;
    ham_size_t node_keycount;

    common_btree_datums_t * const btdata = scratchpad->btdata;
    //const ham_size_t maxkeys = btdata->maxkeys;
    //const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    //const ham_size_t keywidth = btdata->keywidth;
    //const ham_size_t keysize = btdata->keysize;
    ham_db_t * const db = btdata->db;
    ham_env_t * const env = btdata->env;
    //ham_btree_t * const be = btdata->be;
    common_hints_t * const hints = &btdata->hints;
    //const ham_key_t * const key = btdata->in_key;

    //db = page_get_owner(page);
    node = ham_page_get_btree_node(page);
    node_keycount = btree_node_get_count(node);

    bte = btree_in_node_get_key_ref(btdata, page, slot);

    /*
     * prepare page for the log
     */
    st = ham_log_add_page_before(page);
    if (st)
        return (st);

    /*
     * uncouple all cursors
     */
    st = bt_uncouple_all_cursors(page, 0);
    if (st)
        return st;

    ham_assert(slot >= 0, ("invalid slot %ld", (long)slot));
    ham_assert(node_keycount == btree_node_get_count(node), (0));
    ham_assert(slot < (ham_s32_t)node_keycount, ("invalid slot %ld", (long)slot));

    /*
     * leaf page: get rid of the record
     *
     * if duplicates are enabled and a cursor exists: remove the duplicate
     *
     * otherwise remove the full key with all duplicates
     */
    if (btree_node_is_leaf(node))
    {
        ham_bt_cursor_t *c = (ham_bt_cursor_t *)db_get_cursors(db);
        const ham_bt_cursor_t *cursor = btdata->cursor;

        hints->processed_leaf_page = page;
        hints->processed_slot = slot;

        if (key_get_flags(bte) & KEY_HAS_DUPLICATES && cursor)
        {
            st = key_erase_record(db, bte, bt_cursor_get_dupe_id(cursor), 0);
            if (st)
                return st;

            /*
             * if the last duplicate was erased (ptr and flags==0):
             * remove the entry completely
             */
            if (key_get_ptr(bte) == 0 && key_get_flags(bte) == 0)
                goto free_all;

            /*
             * make sure that no cursor is pointing to this dupe, and shift
             * all other cursors
             */
            while (c && cursor)
            {
                ham_bt_cursor_t *next = (ham_bt_cursor_t *)cursor_get_next(c);
                if (c != cursor)
                {
                    if (bt_cursor_get_dupe_id(c) == bt_cursor_get_dupe_id(cursor))
                    {
                        st = bt_cursor_points_to(btdata, c, bte);
                        if (st == HAM_TRUE)
                        {
                            st = bt_cursor_set_to_nil(c);
                            if (st)
                                return st;
                        }
                        else if (st)
                        {
                            return st;
                        }
                    }
                    else if (bt_cursor_get_dupe_id(c) > bt_cursor_get_dupe_id(cursor))
                    {
                        bt_cursor_set_dupe_id(c, bt_cursor_get_dupe_id(c) - 1);
                        memset(bt_cursor_get_dupe_cache(c), 0, sizeof(dupe_entry_t));
                    }
                }
                c = next;
            }

            /*
             * return immediately
             */
            return HAM_SUCCESS;
        }
        else
        {
            ham_bt_cursor_t *c;

            st = key_erase_record(db, bte, 0, BLOB_FREE_ALL_DUPES);
            if (st)
                return st;

free_all:
            c = (ham_bt_cursor_t *)db_get_cursors(db);

            /*
             * make sure that no cursor is pointing to this key
             */
            while (c)
            {
                ham_bt_cursor_t *next = (ham_bt_cursor_t *)cursor_get_next(c);
                if (c != cursor)
                {
                    st = bt_cursor_points_to(btdata, c, bte);
                    if (st == HAM_TRUE)
                    {
                        st = bt_cursor_set_to_nil(c);
                        if (st)
                            return st;
                    }
                    else if (st)
                    {
                        return st;
                    }
                }
                c = next;
            }
        }
    }

    /*
     * get rid of the extended key (if there is one)
     *
     * also remove the key from the cache
     */
    if (key_get_flags(bte) & KEY_IS_EXTENDED)
    {
        ham_offset_t blobid = key_get_extended_rid(db, bte);
        ham_assert(blobid, (0));

        st = extkey_remove(db, blobid);
        if (st)
            return (st);
    }

    /*
     * if we delete the last item, it's enough to decrement the item
     * counter and return...
     */
    ham_assert(node_keycount == btree_node_get_count(node), (0));
    if (slot != node_keycount - 1)
    {
        bte_lhs = btree_in_node_get_key_ref(btdata, page, slot);
        bte_rhs = btree_in_node_get_key_ref(btdata, page, slot + 1);
        btree_move_key_series(btdata, page, page, bte_lhs, bte_rhs, (node_keycount - slot - 1)); // memmove(bte_lhs, bte_rhs, (keywidth) * (node_keycount - slot - 1));
    }

    ham_assert(node_keycount == btree_node_get_count(node), (0));
    node_keycount--;
    btree_node_set_count(node, node_keycount);

    page_set_dirty(page, env);

    return HAM_SUCCESS;
}


/**
* @endcond
*/

