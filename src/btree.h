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
 * @brief Generic interface items for B-tree backends
 *
 */

#ifndef HAM_BTREE_H__
#define HAM_BTREE_H__

#include "internal_fwd_decl.h"

#include "endianswap.h"

#include "backend.h"
#include "btree_cursor.h"
#include "keys.h"
#include "page.h"
#include "statistics.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
This structure stores all 'constant' datums related to the current Database to be passed around
the backend methods.

The intent here is to calculate these items once, upon entering a backend method (callback) and
then only taking up a single pointer pushed on the stack with each internal function call in the
backend, while enabling all these functions easy and fast access to this information.

Some items (the @ref ham_key_t and @ref ham_record_t references for instance) are stored in here
to allow the backend to pass these pieces of information to the RID-assigning methods of the underlying
device (which may employ this information for partitioning and other storage layout schemes), while
other items are just kept in here to cut down on the number of dereferences through pages, cursors, etc.
to get at them.

The prime directive for this structure is always: attempt to cut down on the run-time cost in the
backend, either through nested or repetitive dereferences in one of more internal functions or through
reducing the stack frame construction effort as a result of pushing many function arguments on the stack
as those are passed around.
*/
struct common_btree_datums_t
{
    ham_btree_t *be;

    ham_db_t *db;

    ham_env_t *env;

    ham_device_t *dev;

    ham_key_t *in_key;

    ham_record_t *in_rec;

    /**
     * A pointer to a cursor; if this is a valid pointer, then this
     * cursor will point to the new inserted / located (found) item.
     */
    ham_bt_cursor_t *cursor;

    ham_u32_t flags;

    ham_u16_t keysize;

    ham_u16_t maxkeys;

    ham_size_t keywidth;

    unsigned has_fast_index : 1;

	unsigned _alignment_padding_dummy1 : 31;

    ham_size_t offset_to_fastindex;

    ham_size_t offset_to_keyarr;

    common_hints_t hints;

    /** The ratio where a node is split: (maxkeys * ratio)
	 *
     * Classic HamsterDB/B+-tree: value = MK_HAM_FLOAT(0.5), i.e. split at midpoint.
	 */
    ham_float_t split_ratio;

    /** The ratio where a node is merged: (maxkeys * ratio) */
    ham_float_t merge_ratio;
};


/**
* defines the maximum number of keys per node
*/
#define MAX_KEYS_PER_NODE               0xFFFFU /* max(ham_u16_t) */



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


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_BTREE_H__ */

/**
* @endcond
*/

