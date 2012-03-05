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
 * @brief the classic B-tree backend
 *
 * This backend implements a B+-tree with three notable characteristics:
 *
 * - the leaf nodes are chained to support O(1) next/prev key traversal.
 * - the leaf nodes store the keys (and the accompanying record 'RID' (pointer))
 *   in-page in an ordered array.
 * - small records (<= 8 bytes) are stored in compressed format in the key overhead,
 *   reducing disk access overhead by 1 (record page fetch).
 */

#ifndef HAM_BTREE_CLASSIC_H__
#define HAM_BTREE_CLASSIC_H__

#include "internal_fwd_decl.h"

#include "endianswap.h"

#include "backend.h"
#include "btree.h"
#include "btree_cursor.h"
#include "keys.h"
#include "page.h"
#include "statistics.h"
#include "device.h"


#ifdef __cplusplus
extern "C" {
#endif



/**
* The backend structure for a B+tree.
*/
struct ham_btree_t
{
    /**
     * the common declarations of all backends
     */
    BACKEND_DECLARATIONS(ham_btree_t, common_btree_datums_t);

    /**
     * address of the root-page
     */
    ham_offset_t _rootpage;

    /**
     * maximum keys in an internal page
     */
    ham_u16_t _maxkeys;

    ham_u16_t _alignment_padding_dummy1;
};



/**
 * get the address of the root node
 */
#define btree_get_rootpage(be)          ((be)->_rootpage) /* we're not accessing a 'persisted' member as the entire struct is never persisted, so the ham_db2h_...() operation is completely unnecessary */

/**
 * set the address of the root node
 */
#define btree_set_rootpage(be, rp)      (be)->_rootpage=(rp)

/**
 * get maximum number of keys per (internal) node
 */
#define btree_get_maxkeys(be)           ((be)->_maxkeys)

/**
 * set maximum number of keys per (internal) node
 */
#define btree_set_maxkeys(be, s)        (be)->_maxkeys=(s)



/*
  keydata: moved to the backend struct definition et al as it is generic, not btree specific
*/



#if 0
/**
 * a macro for getting the minimum number of keys
 */
#define btree_get_minkeys(maxkeys)      ((maxkeys)/2)
#endif

/**
 @defgroup btree_node_flags B+-tree Node Flags
 @{
*/

/**
Mask used to extract the count of pending deleted entries (@ref KEY_IS_DELETED)
in this node.

An extracted count equal to the mask itself means there that many <em>or more</em>
in here, waiting to be cleaned.
*/
#define BTREE_NODE_HAS_DELETED_ENTRIES_MASK         0x000F

/**
 @}
 */

#include <ham/packstart.h>

/**
 * A btree-node; it spans the persistent part of a ham_page_t:
 *
 * <pre>
 * btree_node_t *btp=(btree_node_t *)page->_u._pers.payload;
 * </pre>
 */
HAM_PACK_0 struct HAM_PACK_1 btree_node_t
{
    /**
     * @ref btree_node_flags flags of this node - flags are always the first member
     * of every page - regardless of the backend.
     *
     * Currently only used for the page type.
     *
     * @sa btree_node_flags
     */
    ham_u16_t _flags;

    /**
     * number of used entries in the node
     */
    ham_u16_t _count;

    /*
    WARNING WARNING WARNING WARNING WARNING WARNING

    ALIGNMENT ISSUE FOR DEC ALPHA CPU:

    64-bit int access requires 8-byte alignment
    */
#if defined(FIX_PEDANTIC_64BIT_ALIGNMENT_REQUIREMENT)
    ham_u32_t _reserved8;
#endif

    /**
     * address of left sibling
     */
    ham_pers_rid_t _left;

    /**
     * address of right sibling
     */
    ham_pers_rid_t _right;

    /**
     * address of child node whose items are smaller than all items
     * in this node; NIL when this is a leaf node.
     */
    ham_pers_rid_t _ptr_left;

    /**
     * the entries of this node.
     *
     * When @ref HAM_BTREE_NODES_HAVE_FAST_INDEX is set, the 'fast index' is
     * positioned before the actual key store in the node.
     */
    int_key_t _entries[1];

} HAM_PACK_2;

#include <ham/packstop.h>


/**
* get the flags of a btree-node
*/
#define btree_node_get_flags(btp)            ham_db2h16((btp)->_flags)

/**
* set the flags of a btree-node
*/
#define btree_node_set_flags(btp, f)         (btp)->_flags=ham_h2db16(f)

/**
 * get the number of entries of a btree-node
 */
#define btree_node_get_count(btp)            ham_db2h16((btp)->_count)

/**
 * set the number of entries of a btree-node
 */
#define btree_node_set_count(btp, c)         (btp)->_count=ham_h2db16(c)

/**
 * get the left sibling of a btree-node
 */
#define btree_node_get_left(btp)             ham_db2h_rid((btp)->_left)

/*
 * check if a btree node is a leaf node
 */
#define btree_node_is_leaf(btp)              (!btree_node_get_ptr_left(btp))

/**
 * set the left sibling of a btree-node
 */
#define btree_node_set_left(btp, l)          ham_h2db_rid((btp)->_left, l)

/**
 * get the right sibling of a btree-node
 */
#define btree_node_get_right(btp)            ham_db2h_rid((btp)->_right)

/**
 * set the right sibling of a btree-node
 */
#define btree_node_set_right(btp, r)         ham_h2db_rid((btp)->_right, r)

/**
 * get the ptr_left of a btree-node
 */
#define btree_node_get_ptr_left(btp)         ham_db2h_rid((btp)->_ptr_left)

/**
 * set the ptr_left of a btree-node
 */
#define btree_node_set_ptr_left(btp, r)      ham_h2db_rid((btp)->_ptr_left, r)

/**
 * get a btree_node_t from a ham_page_t
 */
#define ham_page_get_btree_node(p)      ((btree_node_t *)page_get_payload(p))


#if 0
 /**
 * "constructor" - initializes a new ham_btree_t object
 *
 * @return a pointer to a new created B+-tree backend
 *         instance in @a backend_ref
 *
 * @remark flags are from @ref ham_env_open_db() or @ref ham_env_create_db()
 */
extern ham_status_t
btree_create(ham_backend_t **backend_ref, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);
#endif


/**
* Search the btree structures for a record with key matching the given criteria.
*
* @return This function returns @ref HAM_SUCCESS and sets the cursor to the matching (key, record) position
* if the @a key was found; otherwise an error code is returned.
*/
extern ham_status_t
btree_find_cursor(common_btree_datums_t *btdata,
                  ham_key_t *key, ham_record_t *record);


/**
 * Insert a new tuple (key/record) in the tree.
 *
 * Sets the cursor position to the new item.
 */
extern ham_status_t
btree_insert_cursor(common_btree_datums_t *btdata, ham_key_t *key,
        ham_record_t *record);

/**
 * Erase a key from the tree.
 */
extern ham_status_t
btree_erase_cursor(common_btree_datums_t *btdata, ham_key_t * const key);

/**
 * enumerate all items
 */
extern ham_status_t
btree_enumerate(common_btree_datums_t *btdata, ham_enumerate_cb_t cb,
        void *context);

/**
 * verify the whole tree
 *
 * @note This is a B+-tree 'backend' method.
 */
extern ham_status_t
btree_check_integrity(common_btree_datums_t *btdata);

/**
 * find the child page for a key
 *
 * @return returns the child page in @a page_ref
 *
 * @remark if @a idxptr is a valid pointer, it will store the anchor index of the
 *      loaded page
 */
extern ham_status_t
btree_traverse_tree(ham_page_t **page_ref, ham_s32_t *idxptr,
                    common_btree_datums_t *btdata, ham_page_t *page, ham_key_t *key);

/**
 * search a leaf node for a key
 *
 * !!!
 * only works with leaf nodes!!
 *
 * @return returns the index of the key, or -1 if the key was not found, or
 *         another negative @ref ham_status_codes value when an
 *         unexpected error occurred.
 */
extern ham_s32_t
btree_node_search_by_key(common_btree_datums_t *btdata, const ham_page_t *page, ham_key_t *key,
                ham_u32_t flags);

#if 0 /* unused, except in unittests */
/**
 * get offset of entry @a i - add this to page_get_self(page) for
 * the absolute offset of the key in the file
 */
#define btree_node_get_key_offset(page, i)                          \
     (page_get_self(page)+page_get_persistent_header_size()+        \
     OFFSETOF(btree_node_t, _entries)                               \
     /* ^^^ sizeof(int_key_t) WITHOUT THE -1 !!! */ +               \
     (db_get_int_key_header_size()+db_get_keysize(page_get_owner(page)))*(i))
#endif


/**
Fetch the key stored in @a page at position @a slot.

This assumes that all keys stored in the given page (i.e. index
page/node) are indexed in the sequential order as implicitly
dictated by the comparison function @ref ham_compare_func_t
*/
static __inline int_key_t *
btree_in_node_get_key_ref(common_btree_datums_t *btdata, const ham_page_t *page, ham_u16_t slot)
{
    ham_btree_t * const be = btdata->be;
    const ham_size_t keywidth = btdata->keywidth;

#ifdef HAM_DEBUG
    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);
#endif

    btree_node_t *node = ham_page_get_btree_node(page);

    ham_assert(!has_fast_index, (0));
    {
        ham_u8_t *arr = (ham_u8_t *)node;

        ham_assert(btdata->offset_to_keyarr > 0, (0));
        arr += btdata->offset_to_keyarr;
        ham_assert(arr == (ham_u8_t *)&node->_entries[0], (0));

        ham_assert(((ham_offset_t)arr) % 8 == 0, ("Checking alignment"));
        ham_assert(slot >= 0, (0));
        ham_assert(slot <= MAX_KEYS_PER_NODE - 1, (0));
#if defined(FIX_PEDANTIC_64BIT_ALIGNMENT_REQUIREMENT)
        ham_assert(keywidth % 8 == 0u, ("Checking alignment, see int_key_t definition"));
#endif
        return (int_key_t *)&arr[slot * keywidth];
    }
}



static __inline void
btree_move_key_series(common_btree_datums_t * const btdata, ham_page_t * const lhs_page, ham_page_t * const rhs_page, int_key_t * const bte_lhs, int_key_t * const bte_rhs, ham_size_t count)
{
    ham_device_t * const dev = btdata->dev;
    ham_btree_t * const be = btdata->be;
    //ham_db_t * const db = btdata->db;
    //ham_env_t * const env = btdata->env;
    //ham_bt_cursor_t * const cursor = btdata->cursor;
    //common_hints_t * const hints = &btdata->hints;
    const ham_size_t keywidth = btdata->keywidth;
    //const ham_size_t keysize = btdata->keysize;
    btree_node_t *rhs_node = ham_page_get_btree_node(rhs_page);

    ham_bool_t has_fast_index = !!(be_get_flags(be) & HAM_BTREE_NODES_HAVE_FAST_INDEX);

    ham_assert(page_get_owner(lhs_page), (0));
    ham_assert(dev == page_get_device(lhs_page), (0));
    ham_assert(device_get_env(dev) == db_get_env(page_get_owner(lhs_page)), (0));
    ham_assert(page_get_owner(rhs_page), (0));
    ham_assert(dev == page_get_device(rhs_page), (0));
    ham_assert(device_get_env(dev) == db_get_env(page_get_owner(rhs_page)), (0));

    ham_assert(count >= 0, (0));
    ham_assert(count <= btree_node_get_count(rhs_node), (0));

    ham_assert(!has_fast_index, (0));

    memmove(bte_lhs, bte_rhs, keywidth * count);
}



extern ham_status_t
btree_in_node_enumerate(common_btree_datums_t *btdata, ham_cb_enum_data_t *cb_data, ham_enumerate_cb_t cb);



/**
 * get the slot of an element in the page
 */
extern ham_status_t
btree_get_slot(common_btree_datums_t *btdata, const ham_page_t *page,
        ham_key_t *key, ham_s32_t *slot, int *cmp);

extern ham_size_t
btree_calc_maxkeys(ham_u16_t dam, ham_size_t pagesize, ham_u32_t flags, ham_u16_t *keysize, ham_bool_t *may_have_fastindex);

extern ham_status_t
btree_close_cursors(ham_db_t *db, ham_u32_t flags);




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_BTREE_CLASSIC_H__ */

/**
* @endcond
*/

