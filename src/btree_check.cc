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
 * @brief btree verification
 *
 */

#include "internal_preparation.h"

#include "btree.h"
#include "btree_classic.h"



/**
 * verify a whole level in the tree - start with "page" and traverse
 * the linked list of all the siblings
 */
static ham_status_t
my_verify_level(ham_page_t *parent, ham_page_t *page,
        ham_u32_t level, common_btree_datums_t *btdata);

/**
 * verify a single page
 */
static ham_status_t
btree_verify_single_node(ham_page_t *parent, ham_page_t *leftsib, ham_page_t *page,
        ham_u32_t level, ham_u32_t count, common_btree_datums_t *btdata);







/**
 * verify the whole tree
 *
 * @note This is a B+-tree 'backend' method.
 */
ham_status_t
btree_check_integrity(common_btree_datums_t *btdata)
{
    ham_page_t *page;
    ham_page_t *parent = NULL;
    ham_u32_t level = 0;
    btree_node_t *node;
    ham_status_t st = 0;
    ham_offset_t ptr_left;
    ham_env_t * const env = btdata->env;
    ham_db_t * const db = btdata->db;
    ham_btree_t * const be = btdata->be;

    ham_assert(btree_get_rootpage(be) != 0, ("invalid root page"));

    /* get the root page of the tree */
    st=db_fetch_page(&page, env, btree_get_rootpage(be), 0);
    ham_assert(st ? page == NULL : page != NULL, (0));
    if (!page)
    {
        return st ? st : HAM_INTERNAL_ERROR;
    }
    ham_assert(!st, (0));
    ham_assert(db, (0));
    page_set_owner(page, db);

    /* while we found a page... */
    while (page)
    {
        node = ham_page_get_btree_node(page);
        ptr_left = btree_node_get_ptr_left(node);

        /*
         * verify the page and all its siblings
         */
        st=my_verify_level(parent, page, level, btdata);
        if (st)
            break;
        parent = page;

        /*
         * follow the pointer to the smallest child
         */
        if (ptr_left)
        {
            st=db_fetch_page(&page, env, ptr_left, 0);
            ham_assert(st ? page == NULL : page != NULL, (0));
            if (!page)
            {
                return st ? st : HAM_INTERNAL_ERROR;
            }
            ham_assert(!st, (0));
            ham_assert(db, (0));
            page_set_owner(page, db);
        }
        else
        {
            page = NULL;
        }

        ++level;
    }

    return (st);
}

static ham_status_t
my_verify_level(ham_page_t *parent, ham_page_t *page,
        ham_u32_t level, common_btree_datums_t *btdata)
{
    int cmp;
    ham_u32_t count = 0;
    ham_page_t *child;
    ham_page_t *leftsib = 0;
    ham_status_t st = 0;
    btree_node_t *node = ham_page_get_btree_node(page);
    ham_db_t *db = btdata->db;
    ham_env_t * const env = btdata->env;

    ham_assert(page_get_owner(page) == db, (0));
    ham_assert(env == db_get_env(db), (0));

    /*
     * assert that the parent page's smallest item (item 0) is bigger
     * than the largest item in this page
     */
    if (parent && btree_node_get_left(node))
    {
        btree_node_t *cnode = ham_page_get_btree_node(page);

        cmp = key_compare_int_to_int(btdata, page, 0,
                    (ham_u16_t)(btree_node_get_count(cnode)-1));
        if (cmp < -1)
            return (ham_status_t)cmp;
        if (cmp < 0)
        {
            ham_logerr(("integrity check failed in page 0x%llx: parent item #0 "
                    "< item #%d\n", page_get_self(page),
                    btree_node_get_count(cnode)-1));
            return HAM_INTEGRITY_VIOLATED;
        }
    }

    while (page)
    {
        /*
         * verify the page
         */
        st = btree_verify_single_node(parent, leftsib, page, level, count, btdata);
        if (st)
            break;

        /*
         * get the right sibling
         */
        node = ham_page_get_btree_node(page);
        if (btree_node_get_right(node))
        {
            st = db_fetch_page(&child, env, btree_node_get_right(node), 0);
            ham_assert(st ? child == NULL : child != NULL, (0));
            if (!page)
            {
                return st ? st : HAM_INTERNAL_ERROR;
            }
            ham_assert(!st, (0));
            ham_assert(db, (0));
            page_set_owner(child, db);
        }
        else
        {
            child = 0;
        }
        leftsib = page;
        page = child;

        ++count;
    }

    return (st);
}



static ham_status_t
btree_verify_single_node(ham_page_t *parent, ham_page_t *leftsib, ham_page_t *page,
        ham_u32_t level, ham_u32_t sibcount, common_btree_datums_t *btdata)
{
    int cmp;
    ham_u32_t i;
    //ham_size_t count;
    int_key_t *bte;
    //int_key_t *keyarr;
    btree_node_t *node = ham_page_get_btree_node(page);

    const ham_size_t maxkeys = btdata->maxkeys;
    const ham_size_t minkeys = (maxkeys * btdata->merge_ratio + MK_HAM_FLOAT(0.5)) / HAM_FLOAT_1; // round position value
    ham_db_t * const db = btdata->db;
    ham_btree_t * const be = btdata->be;

    const ham_size_t count = btree_node_get_count(node);

    if (count == 0)
    {
        /*
         * a rootpage can be empty! check if this page is the
         * rootpage.
         */
        //ham_btree_t *be=(ham_btree_t *)db_get_backend(db);
        if (page_get_self(page) == btree_get_rootpage(be))
            return HAM_SUCCESS;

        ham_logerr(("integrity check failed in page 0x%llx: empty page!\n",
                page_get_self(page)));
        return HAM_INTEGRITY_VIOLATED;
    }

    /*
     * previous hamsterdb versions verified that at least "minkeys" keys
     * are in the page. newer hamsterdb versions relaxed these rules and
     * performed late splits and maybe will even avoid merges if pages
     * underflow.
     */
    if ((btree_node_get_left(node) != 0
        || btree_node_get_right(node) != 0)
        && !(db_get_rt_flags(db) & HAM_RECORD_NUMBER))
    {
        ham_bool_t isfew;

        isfew = btree_node_get_count(node) < minkeys-1;
        if (isfew)
        {
            ham_logwarn(("integrity check notification in page 0x%llx: few keys, which is most probably due to speed optimizations or other relaxed criteria "
                    " (usual for B+/B*-tree: %d, now %d)\n", page_get_self(page),
                    minkeys, btree_node_get_count(node)));
            // return HAM_INTEGRITY_VIOLATED;
        }
    }

    /*
     * check if the largest item of the left sibling is smaller than
     * the smallest item of this page
     */
    if (leftsib)
    {
        btree_node_t *sibnode = ham_page_get_btree_node(leftsib);
        ham_size_t sibnode_keycount = btree_node_get_count(sibnode);
        int_key_t *sibentry;

        ham_assert(sibnode_keycount > 0, (0));
        sibentry = btree_in_node_get_key_ref(btdata, leftsib, sibnode_keycount - 1);
        bte = btree_in_node_get_key_ref(btdata, page, 0);

        if ((key_get_flags(bte) != 0 && key_get_flags(bte) != KEY_IS_EXTENDED)
            && !btree_node_is_leaf(node))
        {
            ham_logerr(("integrity check failed in page 0x%llx: item #0 "
                    "has flags, but it's not a leaf page",
                    page_get_self(page)));
            return HAM_INTEGRITY_VIOLATED;
        }
        else
        {
            ham_status_t st;
            ham_key_t lhs;
            ham_key_t rhs;

            st = db_prepare_ham_key_for_compare(db, 0, sibentry, &lhs);
            if (st)
                return st;
            st = db_prepare_ham_key_for_compare(db, 1, bte, &rhs);
            if (st)
                return st;

            ham_assert(node == ham_page_get_btree_node(page), (0));
            cmp = db_compare_keys(db, &lhs, &rhs, leftsib, page);

            /* error is detected, keys will be released when database is closed, so no sweat! */
            if (cmp < -1)
                return (ham_status_t)cmp;
        }

        if (cmp >= 0)
        {
            ham_logerr(("integrity check failed in page 0x%llx: item #0 "
                    "< left sibling item #%d\n", page_get_self(page),
                    sibnode_keycount-1));
            return HAM_INTEGRITY_VIOLATED;
        }
    }

    if (count==1)
        return HAM_SUCCESS;

    ham_assert(count == btree_node_get_count(node), (0));

    /* MINOR BUGFIX: old code wouldn't check EXTKEY flags for the last key in a btree node page */
    for (i = 0; i < count; i++)
    {
        /*
         * if this is an extended key: check for a blob-id
         */
        int_key_t *bte = btree_in_node_get_key_ref(btdata, page, i);

        if (key_get_flags(bte) & KEY_IS_EXTENDED)
        {
            ham_offset_t blobid = key_get_extended_rid(db, bte);
            if (!blobid)
            {
                ham_logerr(("integrity check failed in page 0x%llx: item #%d "
                        "is extended, but has no blob",
                        page_get_self(page), i));
                return HAM_INTEGRITY_VIOLATED;
            }
        }

        if (i > 0)
        {
            int cmp = key_compare_int_to_int(btdata, page, (ham_u16_t)(i-1), (ham_u16_t)i);

            if (cmp < -1)
                return (ham_status_t)cmp;
            if (cmp >= 0)
            {
                ham_logerr(("integrity check failed in page 0x%llx: item #%u "
                        "< item #%u", page_get_self(page), i-1, i));
                return HAM_INTEGRITY_VIOLATED;
            }
        }
    }

    return HAM_SUCCESS;
}


/**
* @endcond
*/

