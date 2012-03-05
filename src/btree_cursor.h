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
 * @brief btree cursors
 *
 * A Btree-Cursor is an object which is used to traverse a Btree.
 *
 * Btree-Cursors are used in Cursor structures as defined in cursor.h. But
 * some routines still use them directly. Over time these layers will be
 * cleaned up and the separation will be improved.
 *
 * The cursor implementation is very fast. Most of the operations (i.e.
 * move previous/next) will not cause any disk access but are O(1) and
 * in-memory only. That's because a cursor is directly "coupled" to a
 * btree page (Page) that resides in memory. If the page is removed
 * from memory (i.e. because the cache decides that it needs to purge the
 * cache, or if there's a page split) then the page is "uncoupled", and a
 * copy of the current key is stored in the cursor. On first access, the
 * cursor is "coupled" again and basically performs a normal lookup of the key.
 */

#ifndef HAM_BTREE_CURSORS_H__
#define HAM_BTREE_CURSORS_H__

#include "internal_fwd_decl.h"
#include "blob.h"

<<<<<<< HEAD

/**
 * the Cursor structure for a b+tree cursor
 */
typedef struct btree_cursor_t
{
    btree_cursor_t()
    : _dupe_cache() {
        _parent=0;
        _dupe_id=0;
        _u._coupled._page=0;
        _u._coupled._index=0;
        _u._uncoupled._key=0;
    }

    /** the parent cursor */
    Cursor *_parent;

    /** the id of the duplicate key to which this cursor is coupled */
    ham_size_t _dupe_id;

    /** cached flags and record ID of the current duplicate */
=======
#ifdef __cplusplus
extern "C" {
#endif



/**
 * The cursor structure for a B+tree.
 */
struct ham_bt_cursor_t
{
    /**
     * The common declarations of all cursors.
     */
    CURSOR_DECLARATIONS(ham_bt_cursor_t);

    /**
     * Internal cursor flags.
     */
    ham_u32_t _flags;

    /**
     * The id of the duplicate key.
     */
    ham_size_t _dupe_id;

    /**
     * Cached flags and record ID of the current duplicate.
     */
>>>>>>> flash-bang-grenade
    dupe_entry_t _dupe_cache;

    /**
     * "coupled" or "uncoupled" states; coupled means that the
     * cursor points into a Page object, which is in
     * memory. "uncoupled" means that the cursor has a copy
<<<<<<< HEAD
     * of the key on which it points (i.e. because the coupled page was
     * flushed to disk and removed from the cache)
     */
    union btree_cursor_union_t {
        struct btree_cursor_coupled_t {
            /* the page we're pointing to */
            Page *_page;

            /* the offset of the key in the page */
=======
     * of the key on which it points.
     */
    union ham_bt_cursor_union_t {
        struct ham_bt_cursor_coupled_t {
            /**
             * The page we're pointing to.
             */
            ham_page_t *_page;

            /**
             * The offset of the key in the page.
             */
>>>>>>> flash-bang-grenade
            ham_size_t _index;

        } _coupled;

<<<<<<< HEAD
        struct btree_cursor_uncoupled_t {
            /* a copy of the key at which we're pointing */
=======
        struct ham_bt_cursor_uncoupled_t {
            /**
             * A copy of the key at which we're pointing.
             */
>>>>>>> flash-bang-grenade
            ham_key_t *_key;

        } _uncoupled;

    } _u;

} btree_cursor_t;

<<<<<<< HEAD
/** cursor flag: the cursor is coupled */
#define BTREE_CURSOR_FLAG_COUPLED              1

/** cursor flag: the cursor is uncoupled */
#define BTREE_CURSOR_FLAG_UNCOUPLED            2

/** get the parent cursor */
#define btree_cursor_get_parent(c)            (c)->_parent

/** set the parent cursor */
#define btree_cursor_set_parent(c, p)         (c)->_parent=p

/** get the database pointer */
#define btree_cursor_get_db(c)                (c)->_parent->get_db()

/** get the flags */
#define btree_cursor_get_flags(c)             (c)->_parent->get_flags()

/** set the flags */
#define btree_cursor_set_flags(c, f)          (c)->_parent->set_flags(f)

/** get the page we're pointing to - if the cursor is coupled */
#define btree_cursor_get_coupled_page(c)      (c)->_u._coupled._page

/** set the page we're pointing to - if the cursor is coupled */
#define btree_cursor_set_coupled_page(c, p)   (c)->_u._coupled._page=p

/** get the key index we're pointing to - if the cursor is coupled */
#define btree_cursor_get_coupled_index(c)     (c)->_u._coupled._index

/** set the key index we're pointing to - if the cursor is coupled */
#define btree_cursor_set_coupled_index(c, i)  (c)->_u._coupled._index=i

/** get the duplicate key we're pointing to - if the cursor is coupled */
#define btree_cursor_get_dupe_id(c)           (c)->_dupe_id

/** set the duplicate key we're pointing to - if the cursor is coupled */
#define btree_cursor_set_dupe_id(c, d)        (c)->_dupe_id=d

/** get the duplicate key's cache */
#define btree_cursor_get_dupe_cache(c)        (&(c)->_dupe_cache)

/** get the key we're pointing to - if the cursor is uncoupled */
#define btree_cursor_get_uncoupled_key(c)     (c)->_u._uncoupled._key

/** set the key we're pointing to - if the cursor is uncoupled */
#define btree_cursor_set_uncoupled_key(c, k)  (c)->_u._uncoupled._key=k

/** check if the cursor is coupled */
#define btree_cursor_is_coupled(c)            (btree_cursor_get_flags(c)&     \
                                              BTREE_CURSOR_FLAG_COUPLED)

/** check if the cursor is uncoupled */
#define btree_cursor_is_uncoupled(c)          (btree_cursor_get_flags(c)&     \
                                              BTREE_CURSOR_FLAG_UNCOUPLED)

/**
 * Create a new cursor
=======
/**
 * Cursor flag: the cursor is coupled.
 */
#define BT_CURSOR_FLAG_COUPLED              1

/**
 * Cursor flag: the cursor is uncoupled.
 */
#define BT_CURSOR_FLAG_UNCOUPLED            2

/**
 * Get the database pointer.
 */
#define bt_cursor_get_db(cu)                (cu)->_db

/**
 * Set the database pointer.
 */
#define bt_cursor_set_db(cu, db)            (cu)->_db=(db)

/**
 * Get the txn pointer.
 */
#define bt_cursor_get_txn(cu)               (cu)->_txn

/**
 * Set the txn pointer.
 */
#define bt_cursor_set_txn(cu, txn)          (cu)->_txn=(txn)

/**
* Get the allocator.
*/
#define bt_cursor_get_allocator(cu)         (cu)->_allocator

/**
* Set the allocator.
*/
#define bt_cursor_set_allocator(cu, a)      (cu)->_allocator=(a)

/**
 * Get the flags.
>>>>>>> flash-bang-grenade
 */
extern void
btree_cursor_create(Database *db, ham_txn_t *txn, ham_u32_t flags,
                btree_cursor_t *cursor, Cursor *parent);

/**
<<<<<<< HEAD
 * Clone an existing cursor
 * the dest structure is already allocated
=======
 * Set the flags.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_clone(btree_cursor_t *src, btree_cursor_t *dest,
                Cursor *parent);

/**
 * Set the cursor to NIL
 */
extern ham_status_t
btree_cursor_set_to_nil(btree_cursor_t *c);

/**
<<<<<<< HEAD
 * Returns true if the cursor is nil, otherwise false
=======
 * Get the page we're pointing to - if the cursor is coupled.
>>>>>>> flash-bang-grenade
 */
extern ham_bool_t
btree_cursor_is_nil(btree_cursor_t *cursor);

/**
<<<<<<< HEAD
 * Couple the cursor to the same item as another (coupled!) cursor
 *
 * @remark will assert that the other cursor is coupled; will set the
 * current cursor to nil
=======
 * Set the page we're pointing to - if the cursor is coupled.
>>>>>>> flash-bang-grenade
 */
extern void
btree_cursor_couple_to_other(btree_cursor_t *c, btree_cursor_t *other);

/**
<<<<<<< HEAD
 * Uncouple the cursor
 *
 * @remark to uncouple a page the cursor HAS to be coupled!
=======
 * Get the key index we're pointing to - if the cursor is coupled.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_uncouple(btree_cursor_t *c, ham_u32_t flags);

/**
<<<<<<< HEAD
 * flag for @ref btree_cursor_uncouple: uncouple from the page, but do not
 * call @ref Page::remove_cursor()
=======
 * Set the key index we're pointing to - if the cursor is coupled.
>>>>>>> flash-bang-grenade
 */
#define BTREE_CURSOR_UNCOUPLE_NO_REMOVE        1

/**
<<<<<<< HEAD
 * returns true if a cursor points to this btree key, otherwise false
=======
 * Get the duplicate key we're pointing to - if the cursor is coupled.
>>>>>>> flash-bang-grenade
 */
extern bool
btree_cursor_points_to(btree_cursor_t *cursor, btree_key_t *key);

/**
<<<<<<< HEAD
 * returns true if a cursor points to this external key, otherwise false
=======
 * Set the duplicate key we're pointing to - if the cursor is coupled.
>>>>>>> flash-bang-grenade
 */
extern bool
btree_cursor_points_to_key(btree_cursor_t *cursor, ham_key_t *key);

/**
<<<<<<< HEAD
 * uncouple all cursors from a page
 *
 * @remark this is called whenever the page is deleted or becoming invalid
=======
 * Get the duplicate key's cache.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_uncouple_all_cursors(Page *page, ham_size_t start);

/**
<<<<<<< HEAD
 * Inserts a key/record pair with a cursor
=======
 * Get the key we're pointing to - if the cursor is uncoupled.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_insert(btree_cursor_t *c, ham_key_t *key,
                ham_record_t *record, ham_u32_t flags);

/**
<<<<<<< HEAD
 * Positions the cursor on a key and retrieves the record (if @a record
 * is a valid pointer)
=======
 * Set the key we're pointing to - if the cursor is uncoupled.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_find(btree_cursor_t *c, ham_key_t *key, ham_record_t *record,
                ham_u32_t flags);

<<<<<<< HEAD
/**
 * Erases the key from the index; afterwards, the cursor points to NIL
=======
/*
 * Set a cursor to NIL.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_erase(btree_cursor_t *c, ham_u32_t flags);

/**
<<<<<<< HEAD
 * Moves the cursor to the first, last, next or previous element
=======
 * Couple the cursor.
 *
 * @remark To couple a page, it has to be uncoupled!
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_move(btree_cursor_t *c, ham_key_t *key,
                ham_record_t *record, ham_u32_t flags);

/**
<<<<<<< HEAD
 * Count the number of records stored with the referenced key, i.e.
 * count the number of duplicates for the current key.
=======
 * Uncouple the cursor.
 *
 * @remark To uncouple a page, it has to be coupled!
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_get_duplicate_count(btree_cursor_t *c, ham_size_t *count,
                ham_u32_t flags);

/**
<<<<<<< HEAD
 * Overwrite the record of this cursor
=======
 * Flag for @ref bt_cursor_uncouple: uncouple from the page, but do not
 * call @ref page_remove_cursor()
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_overwrite(btree_cursor_t *c, ham_record_t *record,
                ham_u32_t flags);

/**
<<<<<<< HEAD
 * retrieves the duplicate table of the current key; memory in ptable has
 * to be released by the caller.
 *
 * if key has no duplicates, *ptable is NULL.
 *
 * @warning memory has to be freed by the caller IF needs_free is true!
=======
 * Create a new cursor.
>>>>>>> flash-bang-grenade
 */
extern ham_status_t
btree_cursor_get_duplicate_table(btree_cursor_t *c, dupe_table_t **ptable,
                ham_bool_t *needs_free);

/**
<<<<<<< HEAD
 * retrieves the record size of the current record
 */
extern ham_status_t
btree_cursor_get_record_size(btree_cursor_t *c, ham_offset_t *size);

/**
 * Closes an existing cursor
 */
extern void
btree_cursor_close(btree_cursor_t *cursor);


#endif /* HAM_BTREE_CURSORS_H__ */
=======
 * Returns @ref HAM_TRUE if a cursor points to this key, @ref HAM_FALSE if it is not
 * an an negative error code when an error occurred.
 */
ham_status_t
bt_cursor_points_to(common_btree_datums_t *btdata, ham_bt_cursor_t *cursor, int_key_t *key);

/**
* Uncouple all cursors from a page.
*
* @remark This is called whenever the page is deleted or becoming invalid.
*/
ham_status_t
bt_uncouple_all_cursors(ham_page_t *page, ham_size_t start);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_BT_CURSORS_H__ */

/**
* @endcond
*/

>>>>>>> flash-bang-grenade
