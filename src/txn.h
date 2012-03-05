/*
 * Copyright (C) 2005-2012 Christoph Rupp (chris@crupp.de).
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
#include "rb.h"

#include "page.h"

/**
 * a single operation in a transaction
 */
typedef struct txn_op_t
{
    /** flags and type of this operation; defined in txn.h */
    ham_u32_t _flags;

    /** the original flags of this operation */
    ham_u32_t _orig_flags;

    /** the referenced duplicate id (if neccessary) - used if this is
     * i.e. a ham_cursor_erase, ham_cursor_overwrite or ham_cursor_insert
     * with a DUPLICATE_AFTER/BEFORE flag
     * this is 1-based (like dupecache-index, which is also 1-based) */
    ham_u32_t _referenced_dupe;

    /** the Transaction of this operation */
    ham_txn_t *_txn;

    /** the parent node */
    struct txn_opnode_t *_node;

    /** next in linked list (managed in txn_opnode_t) */
    struct txn_op_t *_node_next;

    /** previous in linked list (managed in txn_opnode_t) */
    struct txn_op_t *_node_prev;

    /** next in linked list (managed in ham_txn_t) */
    struct txn_op_t *_txn_next;

    /** previous in linked list (managed in ham_txn_t) */
    struct txn_op_t *_txn_prev;

    /** the log serial number (lsn) of this operation */
    ham_u64_t _lsn;

    /** if this op overwrites or erases another operation in the same node,
     * then _other_lsn stores the lsn of the other txn_op (only used when
     * the other txn_op is a duplicate) */
    ham_u64_t _other_lsn;

    /** the record */
    ham_record_t _record;

    /** a linked list of cursors which are attached to this txn_op */
    struct txn_cursor_t *_cursors;

} txn_op_t;

/** a NOP operation (empty) */
#define TXN_OP_NOP          0x000000u

/** txn operation is an insert */
#define TXN_OP_INSERT       0x010000u

/** txn operation is an insert w/ overwrite */
#define TXN_OP_INSERT_OW    0x020000u

/** txn operation is an insert w/ duplicate */
#define TXN_OP_INSERT_DUP   0x040000u

/** txn operation erases the key */
#define TXN_OP_ERASE        0x080000u

/** txn operation was already flushed */
#define TXN_OP_FLUSHED      0x100000u

/** get flags */
#define txn_op_get_flags(t)                (t)->_flags

/** set flags */
#define txn_op_set_flags(t, f)             (t)->_flags=f

/** get flags original flags of ham_insert/ham_cursor_insert/ham_erase... */
#define txn_op_get_orig_flags(t)           (t)->_orig_flags

/** set flags original flags of ham_insert/ham_cursor_insert/ham_erase... */
#define txn_op_set_orig_flags(t, f)        (t)->_orig_flags=f

/** get the referenced duplicate id */
#define txn_op_get_referenced_dupe(t)      (t)->_referenced_dupe

/** set the referenced duplicate id */
#define txn_op_set_referenced_dupe(t, id)  (t)->_referenced_dupe=id

/** get the Transaction pointer */
#define txn_op_get_txn(t)                  (t)->_txn

/** set the Transaction pointer */
#define txn_op_set_txn(t, txn)             (t)->_txn=txn

/** get the parent node pointer */
#define txn_op_get_node(t)                 (t)->_node

/** set the parent node pointer */
#define txn_op_set_node(t, n)              (t)->_node=n

/** get next txn_op_t structure */
#define txn_op_get_next_in_node(t)         (t)->_node_next

/** set next txn_op_t structure */
#define txn_op_set_next_in_node(t, n)      (t)->_node_next=n

/** get previous txn_op_t structure */
#define txn_op_get_previous_in_node(t)     (t)->_node_prev

/** set previous txn_op_t structure */
#define txn_op_set_previous_in_node(t, p)  (t)->_node_prev=p

/** get next txn_op_t structure */
#define txn_op_get_next_in_txn(t)          (t)->_txn_next

/** set next txn_op_t structure */
#define txn_op_set_next_in_txn(t, n)       (t)->_txn_next=n

/** get previous txn_op_t structure */
#define txn_op_get_previous_in_txn(t)      (t)->_txn_prev

/** set next txn_op_t structure */
#define txn_op_set_previous_in_txn(t, p)   (t)->_txn_prev=p

/** get lsn */
#define txn_op_get_lsn(t)                  (t)->_lsn

/** set lsn */
#define txn_op_set_lsn(t, l)               (t)->_lsn=l

/** get lsn of other op */
#define txn_op_get_other_lsn(t)            (t)->_other_lsn

/** set lsn of other op */
#define txn_op_set_other_lsn(t, l)         (t)->_other_lsn=l

/** get record */
#define txn_op_get_record(t)               (&(t)->_record)

/** get cursor list */
#define txn_op_get_cursors(t)              (t)->_cursors

/** set cursor list */
#define txn_op_set_cursors(t, c)           (t)->_cursors=c

/**
 * add a cursor to this txn_op structure
 */
extern void
txn_op_add_cursor(txn_op_t *op, struct txn_cursor_t *cursor);

/**
 * remove a cursor from this txn_op structure
 */
extern void
txn_op_remove_cursor(txn_op_t *op, struct txn_cursor_t *cursor);

/**
 * returns true if the op is in a txn which has a conflict
 */
extern ham_bool_t
txn_op_conflicts(txn_op_t *op, ham_txn_t *current_txn);

/*
 * a node in the red-black Transaction tree (implemented in rb.h);
 * a group of Transaction operations which modify the same key
 */
typedef struct txn_opnode_t
{
    /** red-black tree stub */
    rb_node(struct txn_opnode_t) node;

    /** the database - need this pointer for the compare function */
    Database *_db;

    /** this is the key which is modified */
    ham_key_t _key;

    /** the parent tree */
    struct txn_optree_t *_tree;

    /** the linked list of operations - head is oldest operation */
    txn_op_t *_oldest_op;

    /** the linked list of operations - tail is newest operation */
    txn_op_t *_newest_op;

} txn_opnode_t;

/** get the database */
#define txn_opnode_get_db(t)                    (t)->_db

/** set the database */
#define txn_opnode_set_db(t, db)                (t)->_db=db

/** get pointer to the modified key */
#define txn_opnode_get_key(t)                   (&(t)->_key)

/** set pointer to the modified key */
#define txn_opnode_set_key(t, k)                (t)->_key=(*k)

/** get pointer to the parent tree */
#define txn_opnode_get_tree(t)                  (t)->_tree

/** set pointer to the parent tree */
#define txn_opnode_set_tree(t, tree)            (t)->_tree=tree

/** get pointer to the first (oldest) node in list */
#define txn_opnode_get_oldest_op(t)             (t)->_oldest_op

<<<<<<< HEAD
/** set pointer to the first (oldest) node in list */
#define txn_opnode_set_oldest_op(t, o)          (t)->_oldest_op=o

/** get pointer to the last (newest) node in list */
#define txn_opnode_get_newest_op(t)             (t)->_newest_op

/** set pointer to the last (newest) node in list */
#define txn_opnode_set_newest_op(t, n)          (t)->_newest_op=n


/*
 * each Transaction has one tree per Database for a fast lookup; this
 * is just a normal binary tree
 */
typedef struct txn_optree_t
{
    /* the Database for all operations in this tree */
    Database *_db;

    /* stuff for rb.h */
    txn_opnode_t *rbt_root;
    txn_opnode_t rbt_nil;

} txn_optree_t;

/** get the database handle of this txn tree */
#define txn_optree_get_db(t)        (t)->_db

/** set the database handle of this txn tree */
#define txn_optree_set_db(t, d)     (t)->_db=d
=======
#ifdef __cplusplus
extern "C" {
#endif
>>>>>>> flash-bang-grenade

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
<<<<<<< HEAD
 * a Transaction structure
 */
struct ham_txn_t
{
    /** the id of this txn */
    ham_u64_t _id;

    /** owner of this transaction */
    Environment *_env;
=======
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
>>>>>>> flash-bang-grenade

    /** the Transaction name */
    const char *_name;

    /** flags for this transaction */
    ham_u32_t _flags;

    /**
     * reference counter for cursors (how many cursors are
     * attached to this txn?)
     */
    ham_u32_t _cursor_refcount;

<<<<<<< HEAD
    /** index of the log file descriptor for this transaction [0..1] */
=======
    /**
     * index of the log file descriptor for this transaction [0..1]
     */
>>>>>>> flash-bang-grenade
    int _log_desc;

#ifdef HAM_ENABLE_REMOTE
    /** the remote database handle */
    ham_u64_t _remote_handle;
#endif

    /** linked list of all transactions */
    ham_txn_t *_newer, *_older;

    /** the linked list of operations - head is oldest operation */
    txn_op_t *_oldest_op;

<<<<<<< HEAD
    /** the linked list of operations - tail is newest operation */
    txn_op_t *_newest_op;
=======
    struct
    {
        ham_size_t alloc_size;  /**< the number of slots allocated, i.e. the size of the hash table */
        ham_size_t count;       /**< the number of occupied slots. */
        ham_page_t *index;      /**< the hash table itself, where each slot is merely a pointer into the linked list. */
    } _pagelist_hashtable;
>>>>>>> flash-bang-grenade
};

/** transaction is still alive but was aborted */
#define TXN_STATE_ABORTED               0x10000

/** transaction is still alive but was committed */
#define TXN_STATE_COMMITTED             0x20000

/** get the id */
#define txn_get_id(txn)                         (txn)->_id

/** set the id */
#define txn_set_id(txn, id)                     (txn)->_id=(id)

/** get the environment pointer */
#define txn_get_env(txn)                        (txn)->_env

/** set the environment pointer */
#define txn_set_env(txn, env)                   (txn)->_env=(env)

/** get the txn name */
#define txn_get_name(txn)                       (txn)->_name

/** set the txn name */
#define txn_set_name(txn, name)                 (txn)->_name=(name)

/** get the flags */
#define txn_get_flags(txn)                      (txn)->_flags

<<<<<<< HEAD
/** set the flags */
=======
/**
 * set the flags
 */
>>>>>>> flash-bang-grenade
#define txn_set_flags(txn, f)                   (txn)->_flags=(f)

/** get the cursor refcount */
#define txn_get_cursor_refcount(txn)            (txn)->_cursor_refcount

<<<<<<< HEAD
/** set the cursor refcount */
=======
/**
 * set the cursor refcount
 */
>>>>>>> flash-bang-grenade
#define txn_set_cursor_refcount(txn, cfc)       (txn)->_cursor_refcount=(cfc)

/** get the index of the log file descriptor */
#define txn_get_log_desc(txn)                   (txn)->_log_desc

/** set the index of the log file descriptor */
#define txn_set_log_desc(txn, desc)             (txn)->_log_desc=(desc)

/** get the remote database handle */
#define txn_get_remote_handle(txn)              (txn)->_remote_handle

/** set the remote database handle */
#define txn_set_remote_handle(txn, h)           (txn)->_remote_handle=(h)

/** get the 'newer' pointer of the linked list */
#define txn_get_newer(txn)                      (txn)->_newer

/** set the 'newer' pointer of the linked list */
#define txn_set_newer(txn, n)                   (txn)->_newer=(n)

/** get the 'older' pointer of the linked list */
#define txn_get_older(txn)                      (txn)->_older

/** set the 'older' pointer of the linked list */
#define txn_set_older(txn, o)                   (txn)->_older=(o)

/** get the oldest transaction operation */
#define txn_get_oldest_op(txn)                  (txn)->_oldest_op

/** set the oldest transaction operation */
#define txn_set_oldest_op(txn, o)               (txn)->_oldest_op=o

/** get the newest transaction operation */
#define txn_get_newest_op(txn)                  (txn)->_newest_op

/** set the newest transaction operation */
#define txn_set_newest_op(txn, o)               (txn)->_newest_op=o

/**
 * initializes the txn-tree
 */
extern void
txn_tree_init(Database *db, txn_optree_t *tree);

/**
 * traverses a tree; for each node, a callback function is executed
 */
typedef void(*txn_tree_enumerate_cb)(txn_opnode_t *node, void *data);

/**
 * retrieves the first (=smallest) node of the tree, or NULL if the
 * tree is empty
 */
extern txn_opnode_t *
txn_tree_get_first(txn_optree_t *tree);

/**
 * retrieves the last (=largest) node of the tree, or NULL if the
 * tree is empty
 */
extern txn_opnode_t *
txn_tree_get_last(txn_optree_t *tree);

extern void
txn_tree_enumerate(txn_optree_t *tree, txn_tree_enumerate_cb cb, void *data);

/**
 * get an opnode for an optree; if a node with this
 * key already exists then the existing node is returned, otherwise NULL
 *
 * flags can be HAM_FIND_GEQ_MATCH, HAM_FIND_LEQ_MATCH
 */
extern txn_opnode_t *
txn_opnode_get(Database *db, ham_key_t *key, ham_u32_t flags);

/**
 * creates an opnode for an optree; asserts that a node with this
 * key does not yet exist
 *
 * returns NULL if out of memory
 */
<<<<<<< HEAD
extern txn_opnode_t *
txn_opnode_create(Database *db, ham_key_t *key);

/**
 * insert an actual operation into the txn_tree
 */
extern txn_op_t *
txn_opnode_append(ham_txn_t *txn, txn_opnode_t *node, ham_u32_t orig_flags,
                    ham_u32_t flags, ham_u64_t lsn, ham_record_t *record);
=======
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
>>>>>>> flash-bang-grenade

/**
 * frees a txn_opnode_t structure, and removes it from its tree
 */
<<<<<<< HEAD
extern void
txn_opnode_free(Environment *env, txn_opnode_t *node);
=======
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
>>>>>>> flash-bang-grenade

/**
 * retrieves the next larger sibling of a given node, or NULL if there
 * is no sibling
 */
<<<<<<< HEAD
extern txn_opnode_t *
txn_opnode_get_next_sibling(txn_opnode_t *node);
=======
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
>>>>>>> flash-bang-grenade

/**
 * retrieves the previous larger sibling of a given node, or NULL if there
 * is no sibling
 */
<<<<<<< HEAD
extern txn_opnode_t *
txn_opnode_get_previous_sibling(txn_opnode_t *node);
=======
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
>>>>>>> flash-bang-grenade

/**
 * start a Transaction
 *
 * @remark flags are defined below
 */
extern ham_status_t
txn_begin(ham_txn_t **ptxn, Environment *env, const char *name,
                ham_u32_t flags);

/* #define HAM_TXN_READ_ONLY       1   -- already defined in hamsterdb.h */

/**
 * commit a Transaction
 */
extern ham_status_t
txn_commit(ham_txn_t *txn, ham_u32_t flags);

/**
 * abort a Transaction
 */
extern ham_status_t
txn_abort(ham_txn_t *txn, ham_u32_t flags);

/**
<<<<<<< HEAD
 * frees all nodes in the tree
 */
extern void
txn_free_optree(txn_optree_t *tree);

/**
 * frees the internal txn trees, nodes and ops
 * This function is a test gate for the unittests. do not use it.
 */
extern void
txn_free_ops(ham_txn_t *txn);

/**
 * free the txn structure
 *
 * will call txn_free_ops() and then free the txn pointer itself
 */
extern void
txn_free(ham_txn_t *txn);

=======
internal flags: signal that a transaction ABORT should NOT nuke the page statistics.

This is relevant for find operations where an internal (local) transaction is used
and the find fails: as this is a read-only operation, aborting or commiting such
a local transaction does not result in necessary different statistics.
*/
#define DO_NOT_NUKE_PAGE_STATS      0x0001

#ifdef __cplusplus
} // extern "C"
#endif
>>>>>>> flash-bang-grenade

#endif /* HAM_TXN_H__ */

/**
* @endcond
*/

