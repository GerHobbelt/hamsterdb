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

#include "internal_fwd_decl.h"


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


/**
 * a cache manager object
 */
struct ham_cache_t
{
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

#define CACHE_NOREMOVE   1      /** don't remove the page from the cache */

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



/**
 * check the cache integrity
 */
extern ham_status_t
cache_check_integrity(ham_cache_t *cache);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_CACHE_H__ */

/**
* @endcond
*/

