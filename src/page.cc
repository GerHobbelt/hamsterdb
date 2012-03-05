/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
<<<<<<< HEAD
 *
 */

#include "config.h"

#include <string.h>

#include "cache.h"
#include "cursor.h"
#include "db.h"
#include "device.h"
#include "env.h"
#include "error.h"
#include "freelist.h"
#include "log.h"
#include "mem.h"
#include "os.h"
#include "page.h"


int Page::sizeof_persistent_header=(OFFSETOF(page_data_t, _s._payload));

Page::Page(Environment *env, Database *db)
  : m_self(0), m_db(db), m_device(0), m_flags(0), m_dirty(false),
    m_cursors(0), m_pers(0)
{
#if defined(HAM_OS_WIN32) || defined(HAM_OS_WIN64)
    m_win32mmap=0;
#endif
    if (env)
        m_device=env->get_device();
    memset(&m_prev[0], 0, sizeof(m_prev));
    memset(&m_next[0], 0, sizeof(m_next));
}

Page::~Page()
{
    ham_assert(get_pers()==0, (0));
    ham_assert(get_cursors()==0, (0));
}

ham_status_t
Page::allocate()
{
    return (get_device()->alloc_page(this));
}

ham_status_t
Page::fetch(ham_offset_t address)
{
    set_self(address);
    return (get_device()->read_page(this));
}

ham_status_t
Page::flush()
{
    if (!is_dirty())
        return (HAM_SUCCESS);

    ham_status_t st=get_device()->write_page(this);
    if (st)
        return (st);

    set_dirty(false);
    return (HAM_SUCCESS);
}

ham_status_t
Page::free()
{
    ham_assert(get_cursors()==0, (0));

    return (get_device()->free_page(this));
}

void
Page::add_cursor(Cursor *cursor)
{
    if (get_cursors()) {
        cursor->set_next_in_page(get_cursors());
        cursor->set_previous_in_page(0);
        get_cursors()->set_previous_in_page(cursor);
    }
    set_cursors(cursor);
}

void
Page::remove_cursor(Cursor *cursor)
{
    Cursor *n, *p;

    if (cursor==get_cursors()) {
        n=cursor->get_next_in_page();
        if (n)
            n->set_previous_in_page(0);
        set_cursors(n);
    }
    else {
        n=cursor->get_next_in_page();
        p=cursor->get_previous_in_page();
        if (p)
            p->set_next_in_page(n);
        if (n)
            n->set_previous_in_page(p);
    }

    cursor->set_next_in_page(0);
    cursor->set_previous_in_page(0);
}

ham_status_t
Page::uncouple_all_cursors(ham_size_t start)
{
    Cursor *c=get_cursors();

    if (c) {
        Database *db=c->get_db();
        if (db) {
            ham_backend_t *be=db->get_backend();
            
            if (be)
                return (*be->_fun_uncouple_all_cursors)(be, this, start);
        }
    }

    return (HAM_SUCCESS);
}

=======
 */

/**
* @cond ham_internals
*/

#include "internal_preparation.h"





void
page_add_cursor(ham_page_t *page, ham_cursor_t *cursor)
{
    if (page_get_cursors(page)) {
        cursor_set_next_in_page(cursor, page_get_cursors(page));
        cursor_set_previous_in_page(cursor, 0);
        cursor_set_previous_in_page(page_get_cursors(page), cursor);
    }
    page_set_cursors(page, cursor);
}

void
page_remove_cursor(ham_page_t *page, ham_cursor_t *cursor)
{
    ham_cursor_t *n, *p;

    if (cursor==page_get_cursors(page)) {
        n=cursor_get_next_in_page(cursor);
        if (n)
            cursor_set_previous_in_page(n, 0);
        page_set_cursors(page, n);
    }
    else {
        n=cursor_get_next_in_page(cursor);
        p=cursor_get_previous_in_page(cursor);
        if (p)
            cursor_set_next_in_page(p, n);
        if (n)
            cursor_set_previous_in_page(n, p);
    }

    cursor_set_next_in_page(cursor, 0);
    cursor_set_previous_in_page(cursor, 0);
}

ham_page_t *
page_new(ham_env_t *env)
{
    ham_page_t *page;
    mem_allocator_t *alloc;
    ham_cache_t *cache;

    ham_assert(env, (0));
    alloc=env_get_allocator(env);

    page=(ham_page_t *)allocator_calloc(alloc, sizeof(*page));
    if (!page) {
        //db_set_error(db, HAM_OUT_OF_MEMORY);
        return NULL;
    }

    page_set_allocator(page, alloc);

    //page_set_owner(page, db);
    // TODO ^^^ make sure the backend takes care of this...

    page_set_device(page, env_get_device(env));

    /*
     * initialize the cache counter,
     * see also cache_increment_page_counter()
     */
    cache = env_get_cache(env);
    /*
    do NOT increment timeslot; the real cache count+freq update happens elsewhere, this is just the
    initial init and only sticks for pages which do not belong to any
    database, e.g. the environment root page.
    */
    page_set_cache_cntr(page, (cache ? cache->_timeslot : 0));
    page_set_cache_hit_freq(page, 0);

    return (page);
}

void
page_delete(ham_page_t *page)
{
    ham_assert(page!=0, (0));
    ham_assert(page_get_refcount(page)==0, (0));
    ham_assert(page_get_pers(page)==0, (0));
    ham_assert(page_get_cursors(page)==0, (0));

    allocator_free(page_get_allocator(page), page);
}

ham_status_t
page_alloc(ham_page_t *page, ham_size_t size, dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_device_t *dev=page_get_device(page);
    ham_status_t st;

    ham_assert(dev, (0));
    st = dev->alloc_page(dev, page, size, extra_dev_alloc_info);
    if (st == HAM_LIMITS_REACHED)
    {
        /*
        'retry' plan for when the memory mapped space turns out to be exhausted: purge the page cache
        and retry the call. If it fails again, we abort/panic.
        */
        ham_env_t *env = extra_dev_alloc_info->env;

        ham_assert(env, (0));
        ham_assert(env == device_get_env(dev), (0));

        ham_assert(env,(0));
        if (env_get_cache(env))
        {
            ham_cache_t *cache = env_get_cache(env);

            /*
            Purge the cache, if allowed to.

            Make sure never to flush the currently addressed page!
            */
            page_add_ref(page);
            st = env_purge_cache(env, cache_get_cur_elements(cache) / 2);
            page_release_ref(page);
            if (!st)
            {
                st = dev->alloc_page(dev, page, size, extra_dev_alloc_info);
            }
        }
    }

    return st;
}

ham_status_t
page_fetch(ham_page_t *page, ham_size_t size)
{
    ham_device_t *dev=page_get_device(page);
    ham_status_t st;

    ham_assert(dev, (0));
    st = dev->read_page(dev, page, size);
    if (st == HAM_LIMITS_REACHED)
    {
        /*
        'retry' plan for when the memory mapped space turns out to be exhausted: purge the page cache
        and retry the call. If it fails again, we abort/panic.
        */
        ham_env_t *env = device_get_env(dev);

        ham_assert(env,(0));
        if (env_get_cache(env))
        {
            ham_cache_t *cache = env_get_cache(env);

            /*
            Purge the cache, if allowed to.

            Make sure never to flush the currently addressed page!
            */
            page_add_ref(page);
            st = env_purge_cache(env, cache_get_cur_elements(cache) / 2);
            page_release_ref(page);
            if (!st)
            {
                st = dev->read_page(dev, page, size);
            }
        }
    }

    return st;
}

ham_status_t
page_flush(ham_page_t *page)
{
    ham_status_t st;
    ham_env_t *env;
    ham_device_t *dev=page_get_device(page);

    if (!page_is_dirty(page))
        return (HAM_SUCCESS);

    ham_assert(dev, (0));
    env = device_get_env(dev);
    ham_assert(env, (0));
    ham_assert(page_get_owner(page) ? env == db_get_env(page_get_owner(page)) : 1, (0));

    ham_assert(page_get_refcount(page)==0, (0));

    /*
     * as we are about to write a modified page to disc, we MUST flush
     * the log before we do write the page in order to assure crash
     * recovery:
     *
     * as this page belongs to us, it may well be a page which was modified
     * in the pending transaction and any such edits should be REWINDable
     * after a crash when that page has just been written.
     */
    if (env
        && env_get_log(env)
        && !(log_get_state(env_get_log(env))&LOG_STATE_CHECKPOINT))
    {
        st=ham_log_append_flush_page(env_get_log(env), page);
        if (st)
            return st;
    }

    st=dev->write_page(dev, page);
    if (st)
        return st;

    page_set_undirty(page);
    return (HAM_SUCCESS);
}

ham_status_t
page_free(ham_page_t *page)
{
    ham_device_t *dev=page_get_device(page);

    ham_assert(dev, (0));
    ham_assert(page_get_cursors(page)==0, (0));

    return (dev->free_page(dev, page));
}




/**
* @endcond
*/

>>>>>>> flash-bang-grenade
