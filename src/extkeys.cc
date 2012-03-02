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



/** EXTKEY_CACHE_BUCKETSIZE should be a prime number or similar, as it is
 * used in a MODULO hash scheme */
#define EXTKEY_CACHE_BUCKETSIZE         179
/**
 * The age of extended keys (in terms of the number of transactions since they were used last)
 * at which they are eligible for purging from the extended key cache.
 */
#define EXTKEY_MAX_AGE                    5
/**
 * The maximum chain length for any hash bucket a.k.a. hash line.
 *
 * When using HamsterDB with long-running transactions (and large numbers of extended keys),
 * all these will be cached in the (smallish) extended-key hash table and produce long
 * linked-list chains in there, thus slowing us down.
 *
 * By already purging 'old' extended keys which are at the tail end of the linked list
 * in a bucket, we can improve the cleaning of the extkey hash table without risk of
 * failure: only the last key compare, i.e. the last two keys in the transaction, are critical.
 */
#define EXTKEY_MAX_CHAIN_LENGTH           3


extkey_cache_t *
extkey_cache_new(ham_db_t *db)
{
    extkey_cache_t *c;
    int memsize;

    memsize = SIZEOF_EXTKEY_CACHE_T + EXTKEY_CACHE_BUCKETSIZE * sizeof(extkey_t *);
    c = (extkey_cache_t *)allocator_calloc(env_get_allocator(db_get_env(db)), memsize);
    if (!c)
    {
        // HAM_OUT_OF_MEMORY;
        return NULL;
    }

    extkey_cache_set_db(c, db);
    extkey_cache_set_bucketsize(c, EXTKEY_CACHE_BUCKETSIZE);

    return (c);
}

void
extkey_cache_destroy(extkey_cache_t *cache)
{
    ham_size_t i;
    extkey_t *e, *n;
    ham_db_t *db = extkey_cache_get_db(cache);
    ham_env_t *env = db_get_env(db);

    /*
     * make sure that all entries are empty
     */
    for (i = extkey_cache_get_bucketsize(cache); i-- > 0 ; )
    {
        e = extkey_cache_get_bucket(cache, i);
        while (e)
        {
#if HAM_DEBUG
            /*
             * make sure that the extkey-cache is empty - only for in-memory
             * databases and DEBUG builds.
             */
            if (env_get_rt_flags(env) & HAM_IN_MEMORY_DB)
                ham_assert(!"extkey-cache is not empty!", (0));
#endif
            n = extkey_get_next(e);
            allocator_free(env_get_allocator(env), e);
            e = n;
        }
    }

    allocator_free(env_get_allocator(env), cache);
}






/**
 * delete all entries from one 'cache line' (i.e. entry in extkey hash table)
 * which are "too old" (i.e. were not used in the last @ref EXTKEY_MAX_AGE transactions)
 */
static ham_status_t
__extkey_cache_purge_one_cacheline(extkey_cache_t *cache, ham_size_t line_index)
{
    extkey_t *base = extkey_cache_get_bucket(cache, line_index);
    ham_env_t *env = db_get_env(extkey_cache_get_db(cache));

    if (base)
    {
        extkey_t *e;
        extkey_t *next;
        extkey_t *prev;
		extkey_t *sentinel;

        /*
        The oldest entries in the list are at the back, so traverse the 'prev' chain,
        starting with the oldest entry.

        This is why the double-linked list 'prev' chain should always be cyclic:
        it takes O(1) to get at the oldest entry now!
        */
        e = extkey_get_prev(base);
		ham_assert(extkey_get_next(e) == NULL, (0)); // oldest must have NULL next.
		if (EXTKEY_MAX_CHAIN_LENGTH > 2)
		{
			int i;

			sentinel = base;
			for (i = 0; i < EXTKEY_MAX_CHAIN_LENGTH && sentinel; i++)
			{
				sentinel = extkey_get_next(sentinel);
			}
		}
		else
		{
			sentinel = NULL;
		}

        for (;;)
        {
            if (env_get_txn_id(env) - extkey_get_txn_id(e) <= EXTKEY_MAX_AGE)
            {
                /*
				further entries are added later so will belong to a more recent TXN: skip 'em.

				Unless, that is, when we've set a limit to the maximum chain length as well:
				*/
				if (!sentinel)
	                break;
            }

			if (sentinel == e)
			{
				sentinel = NULL;
			}

            next = extkey_get_next(e);
            prev = extkey_get_prev(e);
            ham_assert(prev, (0));
            ham_assert(!next, (0));
            extkey_set_next(prev, next);
            if (next)
            {
                extkey_set_prev(next, prev);
            }
            else
            {
                /* remove entry at end of list: update base->prev instead */
                extkey_set_prev(base, prev);
            }

            allocator_free(env_get_allocator(env), e);

            if (base == e)
            {
                extkey_cache_set_bucket(cache, line_index, next);
                break;
            }
            ham_assert(prev != e, (0));
            e = prev;
        }
    }

    return HAM_SUCCESS;
}





#define extkey_cache_calc_hash(cache, o)                                    \
    ((ham_size_t)(extkey_cache_get_bucketsize(cache) == 0                   \
        ? 0                                                                 \
        : (((o) % (cache_get_bucketsize(cache))))))

ham_status_t
extkey_cache_insert(extkey_cache_t *cache, ham_offset_t blobid,
            ham_size_t size, const ham_u8_t *data, const ham_page_t *refering_index_page)
{
    ham_status_t st;
    ham_size_t h = extkey_cache_calc_hash(cache, blobid);
    extkey_t *e;
    extkey_t *base;
    ham_db_t *db = extkey_cache_get_db(cache);
    ham_env_t *env = db_get_env(db);

    /*
     * DEBUG build: make sure that the item is not inserted twice!
     */
#ifdef HAM_DEBUG
    e = extkey_cache_get_bucket(cache, h);
    while (e) {
        ham_assert(extkey_get_blobid(e) != blobid, (0));
        e = extkey_get_next(e);
    }
#endif

    e = (extkey_t *)allocator_alloc(env_get_allocator(env), SIZEOF_EXTKEY_T + size);
    if (!e)
        return HAM_OUT_OF_MEMORY;
    extkey_set_blobid(e, blobid);
    extkey_set_txn_id(e, env_get_txn_id(env));

    /*
    in order to keep the extkey cache fresh and fast, we purge the current cache line
    for each insert and remove: this is /probably/ faster than purging large sets
    of dusty entries once in a while as it speeds up fetch + erase this way: shorter lists
    to scan.
    */
    st = __extkey_cache_purge_one_cacheline(cache, h);
    if (st)
        return st;

    base = extkey_cache_get_bucket(cache, h);
    extkey_set_next(e, base);
    /*
    Set up the 'prev' link chain as a cyclic chain for O(1) instead of O(N)
    extkey purging in these linked lists.
    */
    if (base)
    {
        extkey_set_prev(e, extkey_get_prev(base));
        extkey_set_prev(base, e);
    }
    else
    {
        extkey_set_prev(e, e);
    }
    extkey_cache_set_bucket(cache, h, e);

    extkey_set_size(e, size);
    memcpy(extkey_get_data(e), data, size);

    extkey_cache_set_usedsize(cache, extkey_cache_get_usedsize(cache)+size);
    extkey_cache_inc_count(cache);

    return HAM_SUCCESS;
}

ham_status_t
extkey_cache_remove(extkey_cache_t *cache, ham_offset_t blobid)
{
    ham_db_t *db = extkey_cache_get_db(cache);
    ham_env_t *env = db_get_env(db);
    ham_size_t h = extkey_cache_calc_hash(cache, blobid);
    extkey_t *e;
    extkey_t *base;
    extkey_t *next;
    extkey_t *prev;

    /*
    we assume a regular 'remove' will remove the most recent entries most
    often, so we traverse the 'next' chain
    */
    base = e = extkey_cache_get_bucket(cache, h);
    while (e) {
        if (extkey_get_blobid(e) == blobid)
            break;
        e = extkey_get_next(e);
    }

    if (!e)
        return (HAM_KEY_NOT_FOUND);

    next = extkey_get_next(e);
    prev = extkey_get_prev(e);
    ham_assert(prev, (0));
    extkey_set_next(prev, next);
    if (next)
    {
        extkey_set_prev(next, prev);
    }
    else
    {
        /* remove entry at end of list: update base->prev instead */
        extkey_set_prev(base, prev);
    }

    if (base == e)
    {
        extkey_cache_set_bucket(cache, h, next);
    }

    extkey_cache_set_usedsize(cache,
            extkey_cache_get_usedsize(cache)-extkey_get_size(e));
    extkey_cache_dec_count(cache);

    allocator_free(env_get_allocator(env), e);

    return HAM_SUCCESS;
}

ham_status_t
extkey_cache_fetch(extkey_cache_t *cache, ham_offset_t blobid,
            ham_size_t *size, ham_u8_t **data)
{
    ham_size_t h = extkey_cache_calc_hash(cache, blobid);
    extkey_t *e;
    extkey_t *top;

    /*
    we assume a regular 'fetch' will look for the most recent entries most
    often, so we traverse the 'next' chain
    */
    e=extkey_cache_get_bucket(cache, h);
    while (e) {
        if (extkey_get_blobid(e) == blobid)
            break;
        e = extkey_get_next(e);
    }

    if (!e)
        return (HAM_KEY_NOT_FOUND);

	// and place the extkey entry at the top of the linked list:
    top = extkey_cache_get_bucket(cache, h);
	if (e != top)
	{
		extkey_t *ep = extkey_get_prev(e);
		extkey_t *en = extkey_get_next(e);
		extkey_t *tp = extkey_get_prev(top);

		// drop 'e' from the chain ep -- e -- en:
		ham_assert(ep, (0));
		if (en)
		{
			// Note that for a top -- e -- .. chain, ep == top. But we're okay that way in here.
			extkey_set_next(ep, en);
			extkey_set_prev(en, ep);

			// inject e into the top chain as  tp -- e -- top -- tn:
			ham_assert(tp, (0));
			extkey_set_next(tp, NULL /* e */); // forward chain is NOT a loop!
			extkey_set_next(e, top);
			ham_assert(top, (0));
			extkey_set_prev(top, e);
			extkey_set_prev(e, tp);
		}
        else
        {
			/*
			As en == NULL, we know that e == tp

			We MAY also encounter the scenario where the LL has only two nodes in all:
			  top -- e

			Then ep == top, en == NULL, tp == e, tn == e
			*/
			ham_assert(e == tp, (0));

       		extkey_set_next(ep, NULL /* en */);
			/* remove entry at end of list: update NEW BASE->prev instead */
            //extkey_set_prev(e, ep);

			// inject e into the top chain as  tp -- e -- top -- tn:
			ham_assert(tp, (0));
			extkey_set_next(e, top);
        }

		// and update the 'most recent' a.k.a. start pointer for the chain:
		extkey_cache_set_bucket(cache, h, e);
	}

    *size = extkey_get_size(e);
    *data = extkey_get_data(e);
    extkey_set_txn_id(e, env_get_txn_id(db_get_env(extkey_cache_get_db(cache))));

    return HAM_SUCCESS;
}




ham_status_t
extkey_cache_purge(extkey_cache_t *cache)
{
    ham_status_t st;
    ham_size_t i;

    ham_assert(extkey_cache_get_db(cache), (0));

    /*
     * delete all entries which are "too old" (were not
     * used in the last EXTKEY_MAX_AGE transactions)
     */
    for (i = extkey_cache_get_bucketsize(cache); i-- > 0; )
    {
        st = __extkey_cache_purge_one_cacheline(cache, i);
        if (st)
            return st;
    }

    return HAM_SUCCESS;
}



ham_status_t
extkey_remove(ham_db_t *db, ham_offset_t blobid)
{
    ham_status_t st;

    if (db_get_extkey_cache(db))
    {
        st = extkey_cache_remove(db_get_extkey_cache(db), blobid);
        if (st && st != HAM_KEY_NOT_FOUND)
            return st;
    }

    return blob_free(db_get_env(db), blobid, 0);
}

/**
* @endcond
*/

