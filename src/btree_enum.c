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
 * @brief btree enumeration
 *
 */

#include "internal_preparation.h"

#include "btree.h"
#include "btree_classic.h"


/**
 * enumerate a whole level in the tree - start with "page" and traverse
 * the linked list of all the siblings
 */
static ham_status_t
my_enumerate_level(common_btree_datums_t *btdata, ham_cb_enum_data_t *cb_data,
        ham_enumerate_cb_t cb);

#if 0
/**
 * enumerate a single page
 */
static ham_status_t
my_enumerate_page(ham_page_t *page, ham_u32_t level, ham_u32_t count,
        ham_enumerate_cb_t cb, void *context);
-->
btree_in_node_enumerate()
#endif

/**
 * iterate the whole tree and enumerate every item.
 *
 * @note This is a B+-tree 'backend' method.
 */
ham_status_t
btree_enumerate(common_btree_datums_t *btdata, ham_enumerate_cb_t cb,
        void *context)
{
    ham_page_t *page;
    ham_u32_t level = 0;
    ham_offset_t ptr_left;
    btree_node_t *node;
    ham_status_t st;

    ham_btree_t * const be = btdata->be;
    ham_env_t * const env = btdata->env;
    ham_db_t * const db = btdata->db;

    ham_status_t cb_st = CB_CONTINUE;

	ham_cb_enum_data_t cb_data = { 0 };
	cb_data.context = context;

    ham_assert(btree_get_rootpage(be) != 0, ("invalid root page"));
    ham_assert(cb != 0, ("invalid parameter"));

    /* get the root page of the tree */
    st = db_fetch_page(&page, env, btree_get_rootpage(be), 0);
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
        ham_size_t count;

        node = ham_page_get_btree_node(page);
        ptr_left = btree_node_get_ptr_left(node);

        count = btree_node_get_count(node);
        /*
        WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:

        the current Btree page must be 'pinned' during each callback invocation
        during the enumeration; if you don't (by temporarily bumping up its reference count)
        callback methods MAY flush the page from the page cache without us being
        aware of such until after the fact, when the hamster will CRASH as page pointers
        and content are invalidated.

        To prevent such mishaps, all user-callback invocations in here are surrounded
        by this page 'pinning' countermeasure.
        */
        page_add_ref(page);
		cb_data.event_code = ENUM_EVENT_DESCEND;
		cb_data.page = page;
		cb_data.level = level;
		cb_data.node_count = count;
		cb_data.node_is_leaf = btree_node_is_leaf(node);
        st = cb(&cb_data);
        page_release_ref(page);
        if (st != CB_CONTINUE)
            return (st);

        /*
         * enumerate the page and all its siblings
         */
        cb_st = my_enumerate_level(btdata, &cb_data, cb);
        if (cb_st == CB_STOP || cb_st < 0 /* error */)
            break;

        /*
         * follow the pointer to the smallest child
         */
        if (ptr_left)
        {
            st = db_fetch_page(&page, env, ptr_left, 0);
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
            page = 0;
        }

        ++level;
    }

    return cb_st < 0 ? cb_st : HAM_SUCCESS;
}

static ham_status_t
my_enumerate_level(common_btree_datums_t *btdata, ham_cb_enum_data_t *cb_data,
        ham_enumerate_cb_t cb)
{
    ham_status_t st;
    ham_size_t count = 0;
    btree_node_t *node;
    ham_status_t cb_st = CB_CONTINUE;

    ham_btree_t * const be = btdata->be;
    ham_device_t * const dev = btdata->dev;
    ham_env_t * const env = btdata->env;
    ham_db_t * const db = btdata->db;

	ham_page_t *page = cb_data->page;

    while (page)
    {
        /*
         * enumerate the page
         */
		cb_data->page = page;
		cb_data->page_level_sibling_index = count;
        cb_st = be->_fun_in_node_enumerate(btdata, cb_data, cb);
        if (cb_st == CB_STOP || cb_st < 0 /* error */)
            break;

        /*
         * get the right sibling
         */
        node = ham_page_get_btree_node(page);
        if (btree_node_get_right(node))
        {
            ham_assert(page_get_owner(page), (0));
            ham_assert(dev == page_get_device(page), (0));
            ham_assert(device_get_env(dev) == db_get_env(page_get_owner(page)), (0));
            st=db_fetch_page(&page, env, btree_node_get_right(node), 0);
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
            break;

        ++count;
    }

    return (cb_st);
}

ham_status_t
btree_in_node_enumerate(common_btree_datums_t *btdata, ham_cb_enum_data_t *cb_data, ham_enumerate_cb_t cb)
{
    ham_size_t i;
    ham_size_t count;
	ham_page_t *page = cb_data->page;
    //ham_db_t *db=page_get_owner(page);
    //ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    int_key_t *bte;
    //int_key_t *keyarr;
    btree_node_t *node = ham_page_get_btree_node(page);
    ham_status_t cb_st;
    ham_status_t cb_st2;
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();

    //ham_btree_t * const be = btdata->be;
    ham_device_t * const dev = btdata->dev;

    ham_assert(page_get_owner(page), (0));
    ham_assert(dev == page_get_device(page), (0));
    ham_assert(device_get_env(dev) == db_get_env(page_get_owner(page)), (0));

    count = btree_node_get_count(node);

    /*
    WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:WARNING:

    the current Btree page must be 'pinned' during each callback invocation
    during the enumeration; if you don't (by temporarily bumping up its reference count)
    callback methods MAY flush the page from the page cache without us being
    aware of such until after the fact, when the hamster will CRASH as page pointers
    and content are invalidated.

    To prevent such mishaps, all user-callback invocations in here are surrounded
    by this page 'pinning' countermeasure.
    */
    page_add_ref(page);
	cb_data->event_code = ENUM_EVENT_PAGE_START;
	cb_data->node_count = count;
	//cb_data->page = page;
	cb_data->node_is_leaf = btree_node_is_leaf(node);
    cb_st = cb(cb_data);
    page_release_ref(page);
    if (cb_st == CB_STOP || cb_st < 0 /* error */)
        return (cb_st);

    page_add_ref(page);
    for (i = 0; (i < count) && (cb_st != CB_DO_NOT_DESCEND); i++)
    {
        bte = btree_in_node_get_key_ref(btdata, page, i);

		cb_data->event_code = ENUM_EVENT_ITEM;
		cb_data->key = bte;
		cb_data->key_index = i;
        cb_st = cb(cb_data);
        if (cb_st == CB_STOP || cb_st < 0 /* error */)
            break;
    }

	cb_data->event_code = ENUM_EVENT_PAGE_STOP;
    cb_st2 = cb(cb_data);
    page_release_ref(page);

    if (cb_st < 0 /* error */)
        return (cb_st);
    else if (cb_st == CB_STOP)
        return (CB_STOP);
    else
        return (cb_st2);
}


/**
* @endcond
*/

