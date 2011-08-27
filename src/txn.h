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
 * @brief transactions
 *
 */

#ifndef HAM_TXN_H__
#define HAM_TXN_H__

#include "internal_fwd_decl.h"

#include "page.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * a transaction page list hashtable entry.
 *
 * The transaction page list is managed in a linked list, but an additional hash table is also maintained to reduce the O(N) search cost of the
 * linked list to O(1) of the hash-based search. We do this to improve txn_get_page() performance which profiling turned up as THE major cost
 * factor in current hamsterdb.
 *
 * Per hash entry we keep track of how many linked list nodes (the 'hash collisions') are related to that entry, while page insertion into
 * the linked list is done in such a way as to keep the page hash in sort order, so that we can dynamically expand the hash table without
 * the need for hash (entry) recalculations and accompanying linked list reshuffling in a way inspired by [Johansson].
 *
 * The tough cookie here, compared to [Johansson], is to find a suitable and FAST hash function which produces a hash modulo 2^N, which
 * suggest we cannot use the easy modulo p (where p is prime) approach to convert page addresses (which are sequential numbers modulo
 * system-page-size ~ 1..64K themselves) to hash values with a flat distribution.
 *
 * Another thing we do not wish to do (as it takes extra time) is bit-reverse the hashes as was done in the [Johansson] proposal. Instead,
 * we 'fake' the bit-reversal and the related ordering of the pages in the linked list by starting at the MSBit of the hash value instead of
 * the LSBit as suggested by [Johansson]. Consequently, we reduce the bit reversal operation cost to a single bit-shift operation.
 *
 * Re hash function: we won't know how many pages (input values) there will be, nor do we the exact values of the page addresses as they come in,
 * but we do know that those will be aligned to some allocation unit (Win32/64: 16 bytes) as the addresses are a result of a malloc() call.
 * Coming up with an optimal ('perfect') FAST hash function is a bit of a bother.
 *
 * Instead, we CAN pick a SUBOPTIMAL hash function, which will, at worst case, deliver O(2) instead of O(1), when we make sure
 * that our hash table size is at least as large as the total number of pages stored in the linked list (which was the condition required
 * for O(1) in a perfect hash situation as well, so not much of a change except the realization that we'll only reach 50% of the ideal
 * performance of the hash table. Given that the cost to traverse a SHORT linked list to match the page which is being searched for in
 * txn_get_page() is rather small, we're okay with such a suboptimal hash function.
 *
 * The trick required to make the [Johansson] idea work is to have a basic hash which does not change while the hash table size changes
 * dynamically. The SUBOPTIMAL hash solution giving us O(2) instead of O(1) is to stick with the basic modulo hash approach - where the
 * prime p is a large as the largest expected number of pages accessed (stored) in a single transaction, or larger. A very good
 * approximation of that number is simply using a hash value which is close to MAX_UINT32 and larger than MAX_UINT31, so that we can
 * peruse hash bits 0..30 without much bother, except the realization that we will have a 'layered' distribution of the hash values where
 * one part will contain twice as many input entries as the other part of all possible hash values. This x2 layered ditribution will
 * propagate to our smaller sized (bit length reduced) hash tables when we store fewer pages.
 *
 * All in all, we expect to reduce linked list search O(N) to something close to O(2), while having a dynamically sized hash table, which
 * expands when the number of pages stored in the transaction page list increases.
 *
 * We stick with the knowingly-suboptimal modulo prime hash approach because the alternative for fast hashes (XOR-shift-add, [Ramakrishna]) is probably
 * marginally faster/slower, while other 'fast hashes' are doubtful in their output distribution given the 32-bit or 64-bit input values
 * we expect which will have only few differing bits (as page structs will be allocated from a limited address space in the computer).
 *
 * [Ramakrishna] costs 2 adds, 2 shifts and 1 xor per byte, clocking in 5 * 4 + 3 extra shifts = 23 cycles while a single mod~div for that size takes 14-25 cycles
 * if I read the Intel cycle charts correctly.
 *
 * post-birth note: unfortunately both hash methods have very little change in the MSBits for small changes in the input, so we have
 * a significant risk on encountering collisions either way; the decision to forego the bit-reversal phase in [Johansson] is therefor
 * a very bad choice. bit reversal takes about 25 cycles (2 shifts, 2 masks, 1 or, repeated 5 times).
 *
 * References:
 *
 * [Johansson] Split-ordered lists: Lock-free extensible hash tables, Gunnar Johansson, 2007
 *
 * [Ramakrishna] Performance in practice of string hash functions, M.V. Ramakrishna & Justin Zobel, 1997
 */

/**
 * a transaction structure
 */
struct ham_txn_t
{
    /**
     * the id of this txn
     */
    ham_txn_id_t _id;

    /**
     * owner of this transaction
     */
    ham_env_t *_env;

    /**
     * flags for this transaction
     */
    ham_u32_t _flags;

    /**
     * reference counter for cursors (how many cursors are
     * attached to this txn?)
     */
    ham_u32_t _cursor_refcount;

    /**
     * index of the log file descriptor for this transaction [0..1]
     */
    int _log_desc;

#ifdef HAM_ENABLE_REMOTE
    /** the remote database handle */
    ham_u64_t _remote_handle;
#endif

    /**
     * linked list of all transactions
     */
    ham_txn_t *_next, *_previous;

    /**
     * a list of pages which are referenced by this transaction
     */
    ham_page_t *_pagelist;

	struct
	{
		ham_size_t alloc_size;	/**< the number of slots allocated, i.e. the size of the hash table */
		ham_size_t count;       /**< the number of occupied slots. */
	    ham_page_t *index;		/**< the hash table itself, where each slot is merely a pointer into the linked list. */
	} _pagelist_hashtable;
};

/**
 * get the id
 */
#define txn_get_id(txn)                         (txn)->_id

/**
 * set the id
 */
#define txn_set_id(txn, id)                     (txn)->_id=(id)

/**
 * get the environment pointer
 */
#define txn_get_env(txn)                         (txn)->_env

/**
* set the environment pointer
*/
#define txn_set_env(txn, env)                     (txn)->_env=(env)

/**
 * get the flags
 */
#define txn_get_flags(txn)                      (txn)->_flags

/**
 * set the flags
 */
#define txn_set_flags(txn, f)                   (txn)->_flags=(f)

/**
 * get the cursor refcount
 */
#define txn_get_cursor_refcount(txn)            (txn)->_cursor_refcount

/**
 * set the cursor refcount
 */
#define txn_set_cursor_refcount(txn, cfc)       (txn)->_cursor_refcount=(cfc)

/**
 * get the index of the log file descriptor
 */
#define txn_get_log_desc(txn)                   (txn)->_log_desc

/**
 * set the index of the log file descriptor
 */
#define txn_set_log_desc(txn, desc)             (txn)->_log_desc=(desc)

/**
 * get the remote database handle
 */
#define txn_get_remote_handle(txn)              (txn)->_remote_handle

/**
 * set the remote database handle
 */
#define txn_set_remote_handle(txn, h)           (txn)->_remote_handle=(h)

/**
 * get the 'next' pointer of the linked list
 */
#define txn_get_next(txn)                       (txn)->_next

/**
 * set the 'next' pointer of the linked list
 */
#define txn_set_next(txn, n)                    (txn)->_next=(n)

/**
 * get the 'previous' pointer of the linked list
 */
#define txn_get_previous(txn)                   (txn)->_previous

/**
 * set the 'previous' pointer of the linked list
 */
#define txn_set_previous(txn, p)                (txn)->_previous=(p)

/**
 * get the page list
 */
#define txn_get_pagelist(txn)                   (txn)->_pagelist

/**
 * set the page list
 */
#define txn_set_pagelist(txn, pl)               (txn)->_pagelist=(pl)

/**
 * get the page list hash table. This hash table points into the linked list to speed up txn_get_page().
 */
#define txn_get_pagelist_hashtable(txn)         (txn)->_pagelist_hashtable

/**
 * set the page list hash table. This hash table points into the linked list to speed up txn_get_page().
 */
#define txn_set_pagelist_hashtable(txn, ht)     (txn)->_pagelist_hashtable=(ht)

#if 01 // the old code, inlined. The #else branch carries the new hashtable-assisted version...

/**
 * get a page from the transaction's pagelist; returns 0 if the page
 * is not in the list
 */
static __inline ham_page_t *
txn_get_page(ham_txn_t *txn, ham_offset_t address)
{
    ham_page_t *p=txn_get_pagelist(txn);

#ifdef HAM_DEBUG
    ham_page_t *start=p;
#endif

    while (p) {
        ham_offset_t o=page_get_self(p);
        if (o==address)
            return (p);
        p=page_get_next(p, PAGE_LIST_TXN);
        ham_assert(start!=p, ("circular reference in page-list"));
    }

    return NULL;
}

/**
 * add a page to the transaction's pagelist; does not check if the page already exists in the list.
 */
static __inline void
txn_add_page_nocheck(ham_txn_t *txn, ham_page_t *page)
{
    page_add_ref(page);

    ham_assert(!page_is_in_list(txn_get_pagelist(txn), page, PAGE_LIST_TXN), (0));
    txn_set_pagelist(txn, page_list_insert(txn_get_pagelist(txn), PAGE_LIST_TXN, page));
}

/**
 * add a page to the transaction's pagelist; does not insert the page a second time when
 * @a ignore_if_inserted is non-zero (HAM_TRUE) and the page is found to be present in the
 * list already.
 */
static __inline ham_status_t
txn_add_page(ham_txn_t *txn, ham_page_t *page, ham_bool_t ignore_if_inserted)
{
    /*
     * don't re-insert, if 'ignore_if_inserted' is true
     */
    if (ignore_if_inserted && txn_get_page(txn, page_get_self(page)))
        return HAM_SUCCESS;

#ifdef HAM_DEBUG
    /*
     * check if the page is already in the transaction's pagelist -
     * that would be a bug
     */
    ham_assert(txn_get_page(txn, page_get_self(page))==0,
            ("page 0x%llx is already in the txn", page_get_self(page)));
#endif

    /*
     * not found? add the page
     */
	txn_add_page_nocheck(txn, page);

    return (HAM_SUCCESS);
}

/**
 * mark a page in the transaction as 'deleted'
 * it will be deleted when the transaction is committed
 */
static __inline ham_status_t
txn_free_page(ham_txn_t *txn, ham_page_t *page)
{
    ham_assert(!(page_get_npers_flags(page)&PAGE_NPERS_DELETE_PENDING), (0));
    ham_assert(page_get_cursors(page)==0, (0));

    page_add_npers_flags(page, PAGE_NPERS_DELETE_PENDING);

    return (HAM_SUCCESS);
}

/**
 * remove a page from the transaction's pagelist
 */
static __inline ham_status_t
txn_remove_page(ham_txn_t *txn, struct ham_page_t *page)
{
    ham_assert(page_is_in_list(txn_get_pagelist(txn), page, PAGE_LIST_TXN), (0));
    txn_set_pagelist(txn, page_list_remove(txn_get_pagelist(txn),
            PAGE_LIST_TXN, page));

    page_release_ref(page);

    return HAM_SUCCESS;
}

#else

/**
 * get a page from the transaction's pagelist; returns 0 if the page
 * is not in the list
 */
static __inline ham_page_t *
txn_get_page(ham_txn_t *txn, ham_offset_t address)
{
    ham_page_t *p=txn_get_pagelist(txn);

#ifdef HAM_DEBUG
    ham_page_t *start=p;
#endif

    while (p) {
        ham_offset_t o=page_get_self(p);
        if (o==address)
            return (p);
        p=page_get_next(p, PAGE_LIST_TXN);
        ham_assert(start!=p, ("circular reference in page-list"));
    }

    return NULL;
}

/**
 * add a page to the transaction's pagelist; does not check if the page already exists in the list.
 */
static __inline void
txn_add_page_nocheck(ham_txn_t *txn, ham_page_t *page)
{
    page_add_ref(page);

    ham_assert(!page_is_in_list(txn_get_pagelist(txn), page, PAGE_LIST_TXN), (0));
    txn_set_pagelist(txn, page_list_insert(txn_get_pagelist(txn), PAGE_LIST_TXN, page));
}

/**
 * add a page to the transaction's pagelist; does not insert the page a second time when
 * @a ignore_if_inserted is non-zero (HAM_TRUE) and the page is found to be present in the
 * list already.
 */
static __inline ham_status_t
txn_add_page(ham_txn_t *txn, ham_page_t *page, ham_bool_t ignore_if_inserted)
{
    /*
     * don't re-insert, if 'ignore_if_inserted' is true
     */
    if (ignore_if_inserted && txn_get_page(txn, page_get_self(page)))
        return HAM_SUCCESS;

#ifdef HAM_DEBUG
    /*
     * check if the page is already in the transaction's pagelist -
     * that would be a bug
     */
    ham_assert(txn_get_page(txn, page_get_self(page))==0,
            ("page 0x%llx is already in the txn", page_get_self(page)));
#endif

    /*
     * not found? add the page
     */
	txn_add_page_nocheck(txn, page);

    return (HAM_SUCCESS);
}

/**
 * mark a page in the transaction as 'deleted'
 * it will be deleted when the transaction is committed
 */
static __inline ham_status_t
txn_free_page(ham_txn_t *txn, ham_page_t *page)
{
    ham_assert(!(page_get_npers_flags(page)&PAGE_NPERS_DELETE_PENDING), (0));
    ham_assert(page_get_cursors(page)==0, (0));

    page_add_npers_flags(page, PAGE_NPERS_DELETE_PENDING);

    return (HAM_SUCCESS);
}

/**
 * remove a page from the transaction's pagelist
 */
static __inline ham_status_t
txn_remove_page(ham_txn_t *txn, struct ham_page_t *page)
{
    ham_assert(page_is_in_list(txn_get_pagelist(txn), page, PAGE_LIST_TXN), (0));
    txn_set_pagelist(txn, page_list_remove(txn_get_pagelist(txn),
            PAGE_LIST_TXN, page));

    page_release_ref(page);

    return HAM_SUCCESS;
}

#endif

/**
 * start a transaction
 *
 * @remark flags are defined below
 */
extern ham_status_t
txn_begin(ham_txn_t *txn, ham_env_t *env, ham_u32_t flags);

/* #define HAM_TXN_READ_ONLY       1   -- already defined in hamsterdb.h */

/**
 * commit a transaction
 */
extern ham_status_t
txn_commit(ham_txn_t *txn, ham_u32_t flags);

/* #define TXN_FORCE_WRITE         1   -- moved to hamsterdb.h */

/**
 * abort a transaction
 */
extern ham_status_t
txn_abort(ham_txn_t *txn, ham_u32_t flags);

/**
internal flags: signal that a transaction ABORT should NOT nuke the page statistics.

This is relevant for find operations where an internal (local) transaction is used
and the find fails: as this is a read-only operation, aborting or commiting such
a local transaction does not result in necessary different statistics.
*/
#define DO_NOT_NUKE_PAGE_STATS      0x0001

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_TXN_H__ */

/**
* @endcond
*/

