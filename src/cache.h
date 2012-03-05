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
 * @brief the cache manager
 *
 */

#ifndef HAM_CACHE_H__
#define HAM_CACHE_H__

#include <vector>

#include "internal_fwd_decl.h"
#include "env.h"

<<<<<<< HEAD

/** CACHE_BUCKET_SIZE should be a prime number or similar, as it is used in
 * a MODULO hash scheme */
#define CACHE_BUCKET_SIZE    10317
=======

#ifdef __cplusplus
extern "C" {
#endif

#if 0 /* [GerH] moved to cache.c: dimension hash table based on user cachesize config value */
/**
 * CACHE_BUCKET_SIZE should be a prime number or similar, as it is used in
 * a MODULO hash scheme
 */
#define CACHE_BUCKET_SIZE    10313 /* 10317 is not prime ;) */
#define CACHE_MAX_ELEM       8192 /**< a power of 2 *below* CACHE_BUCKET_SIZE */
#endif
>>>>>>> flash-bang-grenade


/**
 * the cache manager
 */
class Cache
{
<<<<<<< HEAD
  public:
    /** don't remove the page from the cache */
    static const int NOREMOVE=1;

    /** the default constructor
     * @remark max_size is in bytes!
     */
    Cache(Environment *env,
                ham_u64_t capacity_bytes=HAM_DEFAULT_CACHESIZE);

    /**
     * the destructor closes and destroys the cache manager object
     * @remark this will NOT flush the cache!
     */
    ~Cache() {
    }
    
    /**
     * get an unused page (or an unreferenced page, if no unused page
     * was available
     *
     * @remark if the page is dirty, it's the caller's responsibility to
     * write it to disk!
     *
     * @remark the page is removed from the cache
     */
    Page *get_unused_page(void) {
        Page *page;
        Page *oldest;

        /* get the chronologically oldest page */
        oldest=m_totallist_tail;
        if (!oldest)
            return (0);

        /* now iterate through all pages, starting from the oldest
         * (which is the tail of the "totallist", the list of ALL cached
         * pages) */
        page=oldest;
        do {
            /* pick the first unused page (not in a changeset) */
            if (!page->is_in_list(m_env->get_changeset().get_head(),
                        Page::LIST_CHANGESET))
                break;
        
            page=page->get_previous(Page::LIST_CACHED);
            ham_assert(page!=oldest, (0));
        } while (page && page!=oldest);
    
        if (!page)
            return (0);

        /* remove the page from the cache and return it */
        remove_page(page);

        return (page);
    }

    /**
     * get a page from the cache
     *
     * @remark the page is removed from the cache
     *
     * @return 0 if the page was not cached
     */
    Page *get_page(ham_offset_t address, ham_u32_t flags=0) {
        Page *page;
        ham_u64_t hash=calc_hash(address);

        page=m_buckets[hash];
        while (page) {
            if (page->get_self()==address)
                break;
            page=page->get_next(Page::LIST_BUCKET);
        }

        /* not found? then return */
        if (!page)
            return (0);

        /* otherwise remove the page from the cache */
        remove_page(page);

        /* if the flag NOREMOVE is set, then re-insert the page.
         *
         * The remove/insert trick causes the page to be inserted at the
         * head of the "totallist", and therefore it will automatically move
         * far away from the tail. And the pages at the tail are highest
         * candidates to be deleted when the cache is purged. */
        if (flags&Cache::NOREMOVE)
            put_page(page);

        return (page);
    }

    /**
     * store a page in the cache
     */
    void put_page(Page *page) {
        ham_u64_t hash=calc_hash(page->get_self());

        ham_assert(page->get_pers(), (""));

        /* first remove the page from the cache, if it's already cached
         *
         * we re-insert the page because we want to make sure that the
         * cache->_totallist_tail pointer is updated and that the page
         * is inserted at the HEAD of the list
         */
        if (page->is_in_list(m_totallist, Page::LIST_CACHED))
            remove_page(page);

        /* now (re-)insert into the list of all cached pages, and increment
         * the counter */
        ham_assert(!page->is_in_list(m_totallist, Page::LIST_CACHED), (0));
        m_totallist=page->list_insert(m_totallist, Page::LIST_CACHED);

        m_cur_elements++;

        /*
         * insert it in the cache buckets
         * !!!
         * to avoid inserting the page twice, we first remove it from the
         * bucket
         */
        if (page->is_in_list(m_buckets[hash], Page::LIST_BUCKET))
            m_buckets[hash]=page->list_remove(m_buckets[hash],
                            Page::LIST_BUCKET);
        ham_assert(!page->is_in_list(m_buckets[hash], Page::LIST_BUCKET), (0));
        m_buckets[hash]=page->list_insert(m_buckets[hash], Page::LIST_BUCKET);

        /* is this the chronologically oldest page? then set the pointer */
        if (!m_totallist_tail)
            m_totallist_tail=page;

        ham_assert(check_integrity()==0, (""));
    }
    
    /**
     * remove a page from the cache
     */
    void remove_page(Page *page) {
        ham_bool_t removed = HAM_FALSE;

        /* are we removing the chronologically oldest page? then
         * update the pointer with the next oldest page */
        if (m_totallist_tail==page)
            m_totallist_tail=page->get_previous(Page::LIST_CACHED);

        /* remove the page from the cache buckets */
        if (page->get_self()) {
            ham_u64_t hash=calc_hash(page->get_self());
            if (page->is_in_list(m_buckets[hash], Page::LIST_BUCKET)) {
                m_buckets[hash]=page->list_remove(m_buckets[hash],
                        Page::LIST_BUCKET);
            }
        }

        /* remove it from the list of all cached pages */
        if (page->is_in_list(m_totallist, Page::LIST_CACHED)) {
            m_totallist=page->list_remove(m_totallist, Page::LIST_CACHED);
            removed = HAM_TRUE;
        }

        /* decrease the number of cached elements */
        if (removed)
            m_cur_elements--;

        ham_assert(check_integrity()==0, (""));
    }

    /**
     * returns true if the caller should purge the cache
     */
    bool is_too_big(void) {
        return (m_cur_elements*m_env->get_pagesize()>m_capacity);
    }

    /**
     * get number of currently stored pages
     */
    ham_u64_t get_cur_elements(void) {
        return (m_cur_elements);
    }

    /**
     * set the HEAD of the global page list
     */
    void set_totallist(Page *l) {
        m_totallist=l;
    }

    /**
     * retrieve the HEAD of the global page list
     */
    Page *get_totallist(void) {
        return (m_totallist);
    }

    /**
     * decrease number of current elements
     * TODO this should be private, but db.c needs it
     */
    void dec_cur_elements() {
        m_cur_elements--;
    }

    /**
     * get the capacity (in bytes)
     */
    ham_u64_t get_capacity(void) {
        return (m_capacity);
    }
=======
    /** the maximum number of cached elements */
    ham_size_t _max_elements;

    /** the current number of cached elements */
    ham_size_t _cur_elements;

    /** the number of buckets */
    ham_size_t _bucketsize;

    /** linked list of ALL cached pages */
    ham_page_t *_totallist;

    /**
     * a 'timer' counter used to set/check the age of cache entries:
     * higher values represent newer / more important entries, where
     * 'higher' is regarding this number as an UNSIGNED entity.
     */
    ham_s32_t _timeslot;

    /**
    the buckets - an array of linked lists of ham_page_t pointers.

    This essentially is a fixed width hash table with external chaining to
    store an arbitrary number of pages.
    */
    ham_page_t *_buckets[1];
};

/*
 * get the maximum number of elements
 */
#define cache_get_max_elements(cm)             (cm)->_max_elements

/*
 * set the maximum number of elements
 */
#define cache_set_max_elements(cm, s)          (cm)->_max_elements=(s)

/*
 * get the current number of elements
 */
#define cache_get_cur_elements(cm)             (cm)->_cur_elements

/*
 * set the current number of elements
 */
#define cache_set_cur_elements(cm, s)          (cm)->_cur_elements=(s)

/*
 * get the bucket-size
 */
#define cache_get_bucketsize(cm)               (cm)->_bucketsize

/*
 * set the bucket-size
 */
#define cache_set_bucketsize(cm, s)            (cm)->_bucketsize=(s)

/*
 * get the linked list of all pages
 */
#define cache_get_totallist(cm)                (cm)->_totallist

/*
 * set the linked list of all pages
 */
#define cache_set_totallist(cm, l)             (cm)->_totallist=(l)

/*
 * get a bucket
 */
#define cache_get_bucket(cm, i)                (cm)->_buckets[i]

/*
 * set a bucket
 */
#define cache_set_bucket(cm, i, p)             (cm)->_buckets[i]=p

/**
 * initialize a cache manager object
 *
 * max_size is in bytes!
 */
extern ham_cache_t *
cache_new(ham_env_t *env, ham_size_t max_size);

/**
 * close and destroy a cache manager object
 *
 * @remark this will NOT flush the cache!
 */
extern void
cache_delete(ham_env_t *env, ham_cache_t *cache);

/**
 * Clear the cache hash table and reset the current fill count to zero.
 */
extern void
cache_reset(ham_cache_t *cache);

/**
 * get an unused page (or an unreferenced page, if no unused page
 * was available
 *
 * @remark if the page is dirty, it's the caller's responsibility to
 * write it to disk!
 *
 * @remark the page is removed from the cache
 *
 * @param fast only deliver unused pages which are located quickly when this parameter is set to
 *        HAM_TRUE; in effect this means only pages from the garbage list are returned as scanning
 *        the entire cache for the oldest unreferenced page takes too long for 'fast' mode.
 */
extern ham_page_t *
cache_get_unused_page(ham_cache_t *cache, ham_bool_t fast);

/**
 * get a page from the cache
 *
 * @remark the page is removed from the cache, unless you specify @ref CACHE_NOREMOVE in the @a flags
 *
 * @return 0 if the page was not cached
 */
extern ham_page_t *
cache_get_page(ham_cache_t *cache, ham_offset_t address, ham_u32_t flags);
>>>>>>> flash-bang-grenade

    /**
     * check the cache integrity
     */
    ham_status_t check_integrity(void);

<<<<<<< HEAD
  private:
    ham_u64_t calc_hash(ham_offset_t o) {
        return (o%CACHE_BUCKET_SIZE);
    }

    /** the current Environment */
    Environment *m_env;

    /** the capacity (in bytes) */
    ham_u64_t m_capacity;

    /** the current number of cached elements */
    ham_u64_t m_cur_elements;
=======
/**
 * store a page in the cache
 */
extern ham_status_t
cache_put_page(ham_cache_t *cache, ham_page_t *page);

/**
 * Update the 'access counter' for a page in the cache.
 * (The page is assumed to exist in the cache!)
 *
 * In order to improve cache activity for access patterns such as
 * AAB.AAB. where a fetch at the '.' would rate both pages A and B as
 * high, we use an increment-counter approach which will cause page A to
 * be rated higher than page B over time as A is accessed/needed more
 * often.
 */
static __inline void
cache_update_page_access_counter(ham_page_t *page, ham_cache_t *cache)
{
    cache->_timeslot++;
    page_set_cache_cntr(page, cache->_timeslot);
    page_increment_cache_hit_freq(page);
}

/**
 * remove a page from the cache
 */
extern ham_status_t
cache_remove_page(ham_cache_t *cache, ham_page_t *page);


>>>>>>> flash-bang-grenade

    /** linked list of ALL cached pages */
    Page *m_totallist;

    /** the tail of the linked "totallist" - this is the oldest element,
     * and therefore the highest candidate for a flush */
    Page *m_totallist_tail;

<<<<<<< HEAD
    /** the buckets - a linked list of Page pointers */
    std::vector<Page *> m_buckets;
};
=======
#ifdef __cplusplus
} // extern "C"
#endif
>>>>>>> flash-bang-grenade

#endif /* HAM_CACHE_H__ */

/**
* @endcond
*/

