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
 * @brief a base-"class" for a generic database backend
 *
 */

#ifndef HAM_BACKEND_H__
#define HAM_BACKEND_H__

#include "internal_fwd_decl.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
* @defgroup ham_cb_status hamsterdb Backend Node/Page Enumerator Status Codes
* @{
*/

/** continue with the traversal */
#define CB_CONTINUE         0
/** do not not descend another level (or descend from page to key traversal) */
#define CB_DO_NOT_DESCEND   1
/** stop the traversal entirely */
#define CB_STOP             2

/**
 * @}
 */


/**
 * hamsterdb Backend Node/Page Enumerator State Codes
 *
 * @sa ham_cb_enum_data_t
 */
typedef enum ham_cb_event_t
{
    /**
     * descend one level;
     * @ref ham_cb_enum_data_t.level is an integer value with the new level,
     * while @ref ham_cb_enum_data_t.node_count contains the number of keys stored
     * in the current node, which is located in the given @ref ham_cb_enum_data_t.page
     */
    ENUM_EVENT_DESCEND      = 1,

    /** start of a new page; @ref ham_cb_enum_data_t.page points to the page */
    ENUM_EVENT_PAGE_START   = 2,

    /** end of a new page; @ref ham_cb_enum_data_t.page points to the page */
    ENUM_EVENT_PAGE_STOP    = 3,

    /** an item in the page; @ref ham_cb_enum_data_t.key points to the key; @ref ham_cb_enum_data_t.key_index is the index
     * of the key in the page */
    ENUM_EVENT_ITEM         = 4
} ham_cb_event_t;


/**
 * Structure which contains the datums which are passed on to the callback by the backend enumerator.
 *
 * @sa ham_cb_event_t
 * @sa ham_enumerate_cb_t
 */
typedef struct ham_cb_enum_data_t
{
    ham_u32_t level;

    /** one of the @ref ham_cb_event_t state codes */
    ham_cb_event_t event_code;

    ham_bool_t node_is_leaf;
    ham_u16_t _alignment_padding_dummy1;
    ham_size_t node_count;
    ham_size_t page_level_sibling_index;
    ham_page_t *page;

    int_key_t *key;
    ham_size_t key_index;

    /** equals the context pointer passed by the caller of the _fun_enumerate backend method. */
    void *context;
} ham_cb_enum_data_t;

/**
 * a callback function for enumerating the index nodes/pages using the
 * @ref ham_backend_t::_fun_enumerate callback/method.
 *
 * @param data
 *
 * @return one of the @ref ham_cb_status values or a @ref ham_status_codes
 *         error code when an error occurred.
 */
typedef ham_status_t (*ham_enumerate_cb_t)(ham_cb_enum_data_t *data);



/**
 * A single cache slot for managing key data; these slots (pointers) are used to
 * avoid frequent malloc/free cycles in @ref key_compare_pub_to_int(), etc.
 *
 * @sa db_prepare_ham_key_for_compare
 */
typedef struct
{
    void *data_ptr;
    ham_size_t alloc_size;
} ham_backend_key_cmp_cache_elem_t;



/**
 * the backend structure - these functions and members are "inherited"
 * by every other backend (i.e. btree, hashdb etc).
 *
 * @remark To prevent the need for structure packing, the order of the elements
 * in this definition is critical: the @a _recno element <em>must</em> be the
 * last element in this definition as that is the largest type available on
 * any machine: with Microsoft Visual C++ 8, alignment issues would otherwise exist with
 * several backend structures which use this definition as a 'base class' template.
 */
#define BACKEND_DECLARATIONS(clss, int_clss)                            \
    /**                                                                 \
     * create and initialize a new backend                              \
     *                                                                  \
     * @remark this function is called after the @a ham_db_t structure  \
     * and the file were created                                        \
     *                                                                  \
     * the @a flags are stored in the database; only transfer           \
     * the persistent flags!                                            \
     */                                                                 \
    ham_status_t (*_fun_create)(clss *be, ham_u16_t keysize,            \
            ham_u32_t flags);                                           \
                                                                        \
    /**                                                                 \
     * open and initialize a backend                                    \
     *                                                                  \
     * @remark this function is called after the ham_db_structure       \
     * was allocated and the file was opened                            \
     */                                                                 \
    ham_status_t (*_fun_open)(clss *be, ham_u32_t flags);               \
                                                                        \
    /**                                                                 \
     * close the backend                                                \
     *                                                                  \
     * @remark this function is called before the file is closed        \
     */                                                                 \
    ham_status_t (*_fun_close)(clss *be);                               \
                                                                        \
    /**                                                                 \
     * flush the backend                                                \
     *                                                                  \
     * @remark this function is called during ham_flush                 \
     */                                                                 \
    ham_status_t (*_fun_flush)(clss *be);                               \
                                                                        \
    /**                                                                 \
     * find a key in the index                                          \
     */                                                                 \
    ham_status_t (*_fun_find)(clss *be, ham_key_t *key,                 \
            ham_record_t *record, ham_u32_t flags);                     \
                                                                        \
    /**                                                                 \
     * insert (or update) a key in the index                            \
     *                                                                  \
     * the backend is responsible for inserting or updating the         \
     * record. (see blob.h for blob management functions)               \
     */                                                                 \
    ham_status_t (*_fun_insert)(clss *be,                               \
            ham_key_t *key, ham_record_t *record, ham_u32_t flags);     \
                                                                        \
    /**                                                                 \
     * erase a key in the index                                         \
     */                                                                 \
    ham_status_t (*_fun_erase)(clss *be, ham_key_t *key,                \
            ham_u32_t flags);                                           \
                                                                        \
    /**                                                                 \
     * iterate the whole tree and enumerate every item                  \
     */                                                                 \
    ham_status_t (*_fun_enumerate)(clss *be,                            \
            ham_enumerate_cb_t cb, void *context);                      \
                                                                        \
    /**                                                                 \
     * verify the whole tree                                            \
     */                                                                 \
    ham_status_t (*_fun_check_integrity)(clss *be);                     \
                                                                        \
    /**                                                                 \
     * free all allocated resources                                     \
     *                                                                  \
     * @remark this function is called after _fun_close()               \
     */                                                                 \
    ham_status_t (*_fun_delete)(clss *be);                              \
                                                                        \
    /**                                                                 \
     * estimate the number of keys per page, given the keysize          \
     */                                                                 \
    ham_status_t (*_fun_calc_keycount_per_page)(clss *be,               \
                  ham_size_t *keycount, ham_u16_t keysize);             \
                                                                        \
    /**                                                                 \
     * Create a new cursor instance.                                    \
     */                                                                 \
    ham_status_t (*_fun_cursor_create)(clss *be,                        \
                ham_db_t *db, ham_txn_t *txn,                           \
                ham_u32_t flags, ham_cursor_t **cu);                    \
                                                                        \
    /**                                                                 \
     * Close (and free) all cursors related to this database table.     \
     */                                                                 \
    ham_status_t (*_fun_close_cursors)(clss *be, ham_u32_t flags);      \
                                                                        \
    /**                                                                 \
     * uncouple all cursors from a page                                 \
     *                                                                  \
     * @remark this is called whenever the page is deleted or           \
     * becoming invalid                                                 \
     */                                                                 \
    ham_status_t (*_fun_uncouple_all_cursors)(clss *be,                 \
                ham_page_t *page, ham_size_t start);                    \
                                                                        \
    /**                                                                 \
     * Remove all extended keys for the given @a page from the          \
     * extended key cache.                                              \
     */                                                                 \
    ham_status_t (*_fun_free_page_extkeys)(clss *be,                    \
                ham_page_t *page, ham_u32_t flags);                     \
                                                                        \
    /**                                                                 \
    Nuke the given page's statistics for the given @a reason.           \
    */                                                                  \
    void (*_fun_nuke_statistics)(clss *be,                              \
                ham_page_t *page, ham_u32_t reason);                    \
                                                                        \
    /**                                                                 \
     * enumerate a single page                                          \
     */                                                                 \
    ham_status_t (*_fun_in_node_enumerate)(int_clss *btdata,            \
                ham_cb_enum_data_t *cb_data,                            \
                ham_enumerate_cb_t cb);                                 \
                                                                        \
    /**                                                                 \
     * the keysize of this backend index                                \
     */                                                                 \
    ham_u16_t _keysize;                                                 \
                                                                        \
    /**                                                                 \
     * flag if this backend has to be written to disk                   \
     */                                                                 \
    unsigned short _dirty: 1;                                           \
                                                                        \
    /**                                                                 \
     * flag if this backend has been fully initialized                  \
     */                                                                 \
    unsigned short _is_active: 1;                                       \
                                                                        \
    /**                                                                 \
     * the persistent flags of this backend index                       \
     */                                                                 \
    ham_u32_t _flags;                                                   \
                                                                        \
    /**                                                                 \
     * two pointers for managing key data; these pointers are used to   \
     * avoid frequent mallocs in key_compare_pub_to_int(), etc.         \
     *                                                                  \
     * @sa db_prepare_ham_key_for_compare                               \
     */                                                                 \
    ham_backend_key_cmp_cache_elem_t _keydata[2];                       \
                                                                        \
    /**                                                                 \
     * pointer to the database object                                   \
     */                                                                 \
    ham_db_t *_db;                                                      \
                                                                        \
    /**                                                                 \
     * the last used record number                                      \
     */                                                                 \
    ham_recno_t _recno


/**
* A generic backend structure, which has the same memory layout as
* all other backends.
*
* @remark We're pre-declaring struct ham_backend_t and the typedef
* to avoid syntax errors in @ref BACKEND_DECLARATIONS .
*/
struct ham_backend_t
{
    BACKEND_DECLARATIONS(ham_backend_t, common_backend_datums_t);
};

/**
 * convenience macro to get the database pointer of a ham_backend_t-structure
 */
#define be_get_db(be)                       (be)->_db

/**
 * get the keysize
 */
#define be_get_keysize(be)                  (be)->_keysize

/**
 * set the keysize
 */
#define be_set_keysize(be, ks)              (be)->_keysize=(ks)

/**
 * get the flags
 */
#define be_get_flags(be)                    (be)->_flags

/**
 * set the flags
 */
#define be_set_flags(be, f)                 (be)->_flags=(f)

/**
 * get the last used record number
 */
#define be_get_recno(be)                    (be)->_recno

/**
 * set the last used record number
 */
#define be_set_recno(be, rn)                (be)->_recno=(rn)

/**
* increment the last recno value
*/
#define be_inc_recno(be)                    (be)->_recno++

/**
 * get the dirty-flag
 */
#define be_is_dirty(be)                     (be)->_dirty

/**
 * set the dirty-flag
 */
#define be_set_dirty(be, d)                 (be)->_dirty=!!(d)

/**
 * get the active-flag
 */
#define be_is_active(be)                    (be)->_is_active

/**
 * set the active-flag
 */
#define be_set_active(be, d)                (be)->_is_active=!!(d)

/**
 * getter/setter for keydata
 */
#define be_get_keydata(be, idx)             (&(be)->_keydata[idx])



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_BACKEND_H__ */

/**
* @endcond
*/

