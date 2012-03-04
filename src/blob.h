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
 * @brief BLOB processing
 *
 * Functions for reading/writing/allocating blobs (memory chunks of
 * arbitrary size)
 *
 */

#ifndef HAM_BLOB_H__
#define HAM_BLOB_H__

#include "internal_fwd_decl.h"

#include "endianswap.h"
#include "keys.h"


#ifdef __cplusplus
extern "C" {
#endif



#include <ham/packstart.h>

/**
 * a blob structure (blob_t)
 *
 * every blob has a blob_t header; it holds flags and some other
 * administrative information
 */
typedef HAM_PACK_0 struct HAM_PACK_1 blob_t
{
    /**
     * The blob ID - which generally is the absolute address/offset of this
     * blob_t structure in the file, i.e. the RID of the blob.
     */
    ham_pers_rid_t _blobid;

    /**
     * The allocated size of the blob; this is the size which is used
     * by the blob and it's header and maybe additional padding.
     */
    ham_pers_size64_t _allocated_size;

    /**
     * The actual size of the blob itself.
     */
    ham_pers_size64_t _size;

    /**
     * Additional flags. Should always be zero(0).
     */
    ham_u32_t _flags;

    /*
    WARNING WARNING WARNING WARNING WARNING WARNING

    ALIGNMENT ISSUE FOR DEC ALPHA CPU:

    64-bit int access requires 8-byte alignment: this applies to the blob data, when accessed directly.

    (The dupe_table only contains 32 bit values, but the dupe table is also an array of int_key_t and /that/
    one contains 64 bit RIDs which will be 4-byte aligned because of this blob_t layout instead of 8-byte aligned
    as they should.)
    */
#if defined(FIX_PEDANTIC_64BIT_ALIGNMENT_REQUIREMENT)
    ham_u32_t _reserved7;
#endif

} HAM_PACK_2 blob_t;

#include <ham/packstop.h>

/**
 * get the blob ID (blob start address / RID) of a @ref blob_t
 */
#define blob_get_self(b)               ham_db2h_rid((b)->_blobid)

/**
 * set the blob ID (blob start address) of a @ref blob_t
 */
#define blob_set_self(b, s)            ham_h2db_rid((b)->_blobid, s)

/**
 * get the allocated size of a blob_t
 */
#define blob_get_alloc_size(b)         ham_db2h_size64((b)->_allocated_size)

/**
 * set the allocated size of a blob_t
 */
#define blob_set_alloc_size(b, s)      ham_h2db_size64((b)->_allocated_size, s)

/**
 * get the size of a blob_t
 */
#define blob_get_size(b)               ham_db2h_size64((b)->_size)

/**
 * get the size of a blob_t
 */
#define blob_set_size(b, s)            ham_h2db_size64((b)->_size, s)

/**
 * get flags of a blob_t
 */
#define blob_get_flags(b)              ham_db2h32((b)->_flags)

/**
 * set flags of a blob_t
 */
#define blob_set_flags(b, f)           (b)->_flags=ham_h2db32(f)

/**
Return a pointer to the data following the BLOB header.
*/
#define blob_get_data(b)               ((ham_u8_t *)&((blob_t *)(b))[1])


#include <ham/packstart.h>

/**
 * A persistent storage structure for a duplicate - used in a duplicate table BLOB.
 */
typedef HAM_PACK_0 struct HAM_PACK_1 dupe_entry_t
{
    /**
     * Reserved, for padding.
     */
    ham_u8_t _padding[7];

    /**
     * The flags - same as @ref KEY_BLOB_SIZE_SMALL,
     *             @ref KEY_BLOB_SIZE_TINY and @ref KEY_BLOB_SIZE_EMPTY
     */
    ham_u8_t _flags;

    /**
     * The record id a.k.a. RID of the record data (unless it's TINY, SMALL or NULL)
     */
    ham_pers_rid_t _rid;

} HAM_PACK_2 dupe_entry_t;

#include <ham/packstop.h>

/**
 * get the flags of a duplicate entry
 */
#define dupe_entry_get_flags(e)         (e)->_flags

/**
 * set the flags of a duplicate entry
 */
#define dupe_entry_set_flags(e, f)      (e)->_flags=(f)

/**
 * Get the record id (a.k.a. RID) of a duplicate entry.
 *
 * @note
 * If EMPTY, TINY or SMALL is set, the rid is actually a char[] array;
 * in this case, we must not use endian-conversion! For those record
 * types, use @ref dupe_entry_get_rid_as_data() instead!
 *
 * @sa KEY_BLOB_SIZE_SMALL
 * @sa KEY_BLOB_SIZE_TINY
 * @sa KEY_BLOB_SIZE_EMPTY
 * @sa dupe_entry_get_rid_as_data
 * @sa dupe_entry_get_rid_direct_ref
 */
#define dupe_entry_get_rid(e)                                                 \
           ham_db2h_rid((e)->_rid)

/**
 * Get a reference to the data in the record id (a.k.a. RID) of a duplicate entry.
 *
 * @sa KEY_BLOB_SIZE_SMALL
 * @sa KEY_BLOB_SIZE_TINY
 * @sa KEY_BLOB_SIZE_EMPTY
 * @sa dupe_entry_get_rid
 * @sa dupe_entry_get_rid_direct_ref
 */
#define dupe_entry_get_rid_as_data(e)                                         \
           ((e)->_rid.rid8)

/**
 * Get a direct reference to the record id (a.k.a. RID) of a duplicate entry.
 *
 * @sa KEY_BLOB_SIZE_SMALL
 * @sa KEY_BLOB_SIZE_TINY
 * @sa KEY_BLOB_SIZE_EMPTY
 * @sa dupe_entry_get_rid
 * @sa dupe_entry_get_rid_as_data
 */
#define dupe_entry_get_rid_direct_ref(e)                                    \
           (&(e)->_rid)

/**
 * set the record id of a duplicate entry
 *
 * @warning
 * Same problems as with get_rid():
 * if TINY or SMALL is set, the rid is actually a char*-pointer;
 * in this case, we must not use endian-conversion!
 */
#define dupe_entry_set_rid(e, r)                                            \
    do {                                                                    \
        ham_assert(!(dupe_entry_get_flags(e)&(KEY_BLOB_SIZE_TINY            \
                                          |KEY_BLOB_SIZE_SMALL              \
                                          |KEY_BLOB_SIZE_EMPTY)), (0));     \
        ham_h2db_rid((e)->_rid, r);                                         \
    } while (0)

#define dupe_entry_set_rid_direct_ref(e, r)                                 \
    do {                                                                    \
        ham_assert((dupe_entry_get_flags(e)&(KEY_BLOB_SIZE_TINY             \
                                          |KEY_BLOB_SIZE_SMALL              \
                                          |KEY_BLOB_SIZE_EMPTY)), (0));     \
        (e)->_rid = (r);                                                    \
    } while (0)

#include <ham/packstart.h>

/**
 * a structure for duplicates (dupe_table_t)
 */
typedef HAM_PACK_0 struct HAM_PACK_1 dupe_table_t
{
    /**
     * the number of duplicates (used entries in this table)
     */
    ham_u32_t _count;

    /**
     * The capacity of entries in this table.

     @remark This should correlate with the table BLOB size itself, i.e. the size of the BLOB which stores this dupe table.
     */
    ham_u32_t _capacity;

    /**
     * a dynamic array of duplicate entries
     */
    dupe_entry_t _entries[1];

} HAM_PACK_2 dupe_table_t;

#include <ham/packstop.h>

/**
 * get the number of duplicates
 */
#define dupe_table_get_count(t)         (ham_db2h32((t)->_count))

/**
 * set the number of duplicates
 */
#define dupe_table_set_count(t, c)      (t)->_count=ham_h2db32(c)

/**
 * get the maximum number of duplicates
 */
#define dupe_table_get_capacity(t)      (ham_db2h32((t)->_capacity))

/**
 * set the maximum number of duplicates
 */
#define dupe_table_set_capacity(t, c)   (t)->_capacity=ham_h2db32(c)

/**
 * get a pointer to a duplicate entry @a i
 */
#define dupe_table_get_entry(t, i)      (&(t)->_entries[i])

/**
 * allocate/create a blob
 *
 * returns the blob-id (the start address of the blob header) in @a new_blobid
 */
extern ham_status_t
blob_allocate(ham_env_t *env, const ham_record_t *record,
        ham_u32_t flags,
        dev_alloc_request_info_ex_t *extra_dev_alloc_info,
        blob_t *old_hdr, ham_offset_t *new_blobid);

/**
 * read a blob
 *
 * stores the data in @a record
 *
 * flags: either 0 or @ref HAM_DIRECT_ACCESS
 */
extern ham_status_t
blob_read(ham_db_t *db, ham_offset_t blobid,
        ham_record_t *record, ham_u32_t flags);

/**
 * overwrite an existing blob
 *
 * will return an error if the blob does not exist
 * returns the blob-id (the start address of the blob header) in @a new_blobid
 */
extern ham_status_t
blob_overwrite(ham_env_t *env, ham_offset_t old_blobid,
        const ham_record_t *record, ham_u32_t flags,
        dev_alloc_request_info_ex_t *extra_dev_alloc_info,
        ham_offset_t *new_blobid);

/**
 * delete an existing blob
 */
extern ham_status_t
blob_free(ham_env_t *env, ham_offset_t blobid, ham_u32_t flags);

/**
 * create a duplicate table and insert all entries in the duplicate
 * (max. two entries are allowed; first entry will be at the first position,
 * second entry will be set depending on the flags)
 *
 * OR, if the table already exists (i.e. table_id != 0), insert the
 * entry depending on the flags (only one entry is allowed in this case)
 */
extern ham_status_t
blob_duplicate_insert(ham_db_t *db, ham_offset_t table_id,
        const ham_record_t *record, ham_size_t position, ham_u32_t flags,
        dupe_entry_t *entries, ham_size_t num_entries,
        dev_alloc_request_info_ex_t *extra_dev_alloc_info,
        ham_offset_t *rid, ham_size_t *new_position);

/**
 * delete a duplicate
 *
 * if flags == BLOB_FREE_ALL_DUPES: all duplicates and the dupe table
 * are deleted
 *
 * sets new_table_id to 0 if the table is empty
 */
extern ham_status_t
blob_duplicate_erase(ham_db_t *db, int_key_t *table_key,
        ham_size_t position, ham_u32_t flags, ham_offset_t *new_table_id);

#define BLOB_FREE_ALL_DUPES   1


/**
 * get the number of duplicates
 */
extern ham_status_t
blob_duplicate_get_count(ham_env_t *env, ham_offset_t table_id,
        ham_size_t *count, dupe_entry_t *entry);

/**
 * get a duplicate
 */
extern ham_status_t
blob_duplicate_get(ham_env_t *env, ham_offset_t table_id,
        ham_size_t position, dupe_entry_t *entry);



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_BLOB_H__ */

/**
* @endcond
*/

