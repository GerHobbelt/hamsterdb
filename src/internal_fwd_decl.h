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
 * @brief provides forward declarations of internally used types
 *
 * This header file provides these forward declarations to prevent several
 * cyclic dependencies; in particular, the @ref ham_page_t is a type used
 * throughout, but a @ref ham_page_t contains several other types, which
 * again reference @ref ham_page_t pointers either directly or indirectly.
 *
 * To solve this self-referential issue once and for all, all major hamster
 * internal types are forward declared here; when the code requires the actual
 * implementation of a type it can include the related header file any time it
 * wishes.
 *
 * @remark This way of solving the cyclic dependency conundrum has the added
 *         benefit of having a single, well known spot where the actual
 *         'typedef' lines reside, so there's zero risk of 'double defined types'.
 */

#ifndef HAM_FWD_DECL_CMN_TYPES_H__
#define HAM_FWD_DECL_CMN_TYPES_H__

#include <ham/hamsterdb.h>
#include <ham/hamsterdb_int.h>

#include "config.h"


#ifdef __cplusplus
extern "C" {
#endif



struct ham_page_t;
typedef struct ham_page_t ham_page_t;

struct mem_allocator_t;
typedef struct mem_allocator_t mem_allocator_t;

//struct ham_cursor_t;
//typedef struct ham_cursor_t ham_cursor_t;

struct ham_bt_cursor_t;
typedef struct ham_bt_cursor_t ham_bt_cursor_t;

//struct ham_backend_t;
//typedef struct ham_backend_t ham_backend_t;

//struct ham_device_t;
//typedef struct ham_device_t ham_device_t;

//ham_txn_t

struct freelist_cache_t;
typedef struct freelist_cache_t freelist_cache_t;

struct freelist_common_datums_t;
typedef struct freelist_common_datums_t freelist_common_datums_t;

struct ham_cache_t;
typedef struct ham_cache_t ham_cache_t;

struct ham_log_t;
typedef struct ham_log_t ham_log_t;

struct extkey_t;
typedef struct extkey_t extkey_t;

struct extkey_cache_t;
typedef struct extkey_cache_t extkey_cache_t;


struct freelist_entry_t;
typedef struct freelist_entry_t freelist_entry_t;

//struct freelist_cache_t;
//typedef struct freelist_cache_t freelist_cache_t;

struct freelist_hints_t;
typedef struct freelist_hints_t freelist_hints_t;


struct runtime_statistics_pagedata_t;
typedef struct runtime_statistics_pagedata_t runtime_statistics_pagedata_t;

struct freelist_global_hints_t;
typedef struct freelist_global_hints_t freelist_global_hints_t;



struct common_hints_t;
typedef struct common_hints_t common_hints_t;



/* dummy decl for the backend 'abstract class' definition */
struct common_backend_datums_t;
typedef struct common_backend_datums_t common_backend_datums_t;




struct common_btree_datums_t;
typedef struct common_btree_datums_t common_btree_datums_t;




struct dev_alloc_request_info_ex_t;
typedef struct dev_alloc_request_info_ex_t dev_alloc_request_info_ex_t;



#include <ham/packstart.h>

HAM_PACK_0 struct HAM_PACK_1 freelist_payload_t HAM_PACK_2;
typedef /* HAM_PACK_0 */ struct /* HAM_PACK_1 */ freelist_payload_t /* HAM_PACK_2 */ freelist_payload_t;

#include <ham/packstop.h>


#include <ham/packstart.h>

HAM_PACK_0 struct HAM_PACK_1 int_key_t HAM_PACK_2;
typedef /* HAM_PACK_0 */ struct /* HAM_PACK_1 */ int_key_t /* HAM_PACK_2 */ int_key_t;

#include <ham/packstop.h>


#include <ham/packstart.h>

struct ham_btree_t;
typedef struct ham_btree_t ham_btree_t;

#include <ham/packstop.h>


#include <ham/packstart.h>

HAM_PACK_0 struct HAM_PACK_1 btree_node_t HAM_PACK_2;
typedef /* HAM_PACK_0 */ struct /* HAM_PACK_1 */ btree_node_t /* HAM_PACK_2 */ btree_node_t;

#include <ham/packstop.h>





struct ham_spiral_hashtable_t;
typedef struct ham_spiral_hashtable_t ham_spiral_hashtable_t;

struct common_hashtable_datums_t;
typedef struct common_hashtable_datums_t common_hashtable_datums_t;

struct ham_ch_cursor_t;
typedef struct ham_ch_cursor_t ham_ch_cursor_t;

struct ham_cuckoo_hash_t;
typedef struct ham_cuckoo_hash_t ham_cuckoo_hash_t;


#include <ham/packstart.h>

HAM_PACK_0 struct HAM_PACK_1 hashed_key_t HAM_PACK_2;
typedef /* HAM_PACK_0 */ struct /* HAM_PACK_1 */ hashed_key_t /* HAM_PACK_2 */ hashed_key_t;

#include <ham/packstop.h>





#ifdef __cplusplus
} // extern "C"
#endif

#endif

/**
* @endcond
*/

