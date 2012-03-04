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
 * @brief key handling
 *
 */

#ifndef HAM_KEY_H__
#define HAM_KEY_H__

#include "internal_fwd_decl.h"

#include "db.h"
#include "backend.h"


#ifdef __cplusplus
extern "C" {
#endif



#include <ham/packstart.h>

/**
 * the internal representation of a key
 *
 * Note: the names of the fields have changed in 1.1.0 to ensure the compiler
 * barfs on misuse of some macros, e.g. key_get_flags(): here flags are 8-bit,
 * while ham_key_t flags are 32-bit!
 */
HAM_PACK_0 struct HAM_PACK_1 int_key_t
{
    /**
     * the pointer of this entry: points to either the corresponding record or
     * (in case of duplicate keys) a record table BLOB which lists the pointers to
     * all related records.
     */
    ham_pers_rid_t _ptr;

    /**
     * the size of this entry
     */
    ham_u16_t _keysize;

    /**
     * flags
     */
    ham_u8_t _flags8;

#if 0
    /*
    WARNING WARNING WARNING WARNING WARNING WARNING

    ALIGNMENT ISSUE (Motorola MC68000, DEC ALPHA, ...)

    key data is positioned at an odd offset, so any word/int oriented keys will
    fault when accessed directly.

    This applies to the comparison callback methods!
    */
#if defined(FIX_PEDANTIC_64BIT_ALIGNMENT_REQUIREMENT)
    ham_u8_t _reserved10;
    ham_u32_t _reserved11;
#endif
#endif

    /**
     * the key
     */
    ham_u8_t _key[1];

} HAM_PACK_2;

#include <ham/packstop.h>

/**
 * get the size of the internal key representation header
 */
#define db_get_int_key_header_size()   OFFSETOF(int_key_t, _key)
                                       /* sizeof(int_key_t)-1 */

/**
 * get the pointer of a btree-entry
 *
 * @note
 * If @ref KEY_BLOB_SIZE_TINY, @ref KEY_BLOB_SIZE_SMALL or
 * @ref KEY_BLOB_SIZE_EMPTY is set, the key is actually not a
 * index but stores the key data itself;
 * in this case, we must not use endian-conversion! Instead, use
 * @ref key_get_ptr_as_data() to obtain a reference for such keys.
 *
 * @sa key_get_ptr_direct_ref
 * @sa key_get_ptr_as_data
 */
#define key_get_ptr(k)                                                          \
                                    ham_db2h_rid((k)->_ptr)

/**
 * get the reference to the key data itself for small keys (with have
 * any of the flags @ref KEY_BLOB_SIZE_TINY, @ref KEY_BLOB_SIZE_SMALL or
 * @ref KEY_BLOB_SIZE_EMPTY set).
 *
 * @note No endian conversion will take place.
 *
 * @sa key_get_ptr_direct_ref
 * @sa key_get_ptr
 */
#define key_get_ptr_as_data(k)                                                  \
                                    ((k)->_ptr.rid8)

/**
 * get the reference to the RID itself.
 *
 * @note No endian conversion will take place.
 *
 * @sa key_get_ptr
 * @sa key_get_ptr_as_data
 */
#define key_get_ptr_direct_ref(k)                                               \
                                    (&(k)->_ptr)

/**
 * set the pointer of an btree-entry
 *
 * !!! same problems as with key_get_ptr():
 * if TINY or SMALL is set, the key is actually a char*-pointer;
 * in this case, we must not use endian-conversion
 */
#define key_set_ptr(k, p)                                                   \
    do {                                                                    \
        ham_assert(!(key_get_flags(k)&(KEY_BLOB_SIZE_TINY                   \
                              |KEY_BLOB_SIZE_SMALL                          \
                              |KEY_BLOB_SIZE_EMPTY)), (0));                 \
        ham_h2db_rid((k)->_ptr, p);                                         \
    } while (0)

#define key_set_ptr_direct_ref(k, p)                                        \
    do {                                                                    \
        ham_assert((key_get_flags(k)&(KEY_BLOB_SIZE_TINY                    \
                              |KEY_BLOB_SIZE_SMALL                          \
                              |KEY_BLOB_SIZE_EMPTY)), (0));                 \
        (k)->_ptr = (p);                                                    \
    } while (0)

/**
 * get the size of an btree-entry
 */
#define key_get_size(bte)               (ham_db2h16((bte)->_keysize))

/**
 * set the size of an btree-entry
 */
#define key_set_size(bte, s)            (bte)->_keysize=ham_h2db16(s)

/**
 * get a pointer to the key
 */
#define key_get_key(bte)                ((bte)->_key)

/**
 * set the key data
 */
#define key_set_key(bte, ptr, len)      memcpy((bte)->_key, ptr, len)

/**
 * get the record-ID of an extended key
 */
static __inline ham_offset_t
key_get_extended_rid(ham_db_t *db, int_key_t *key)
{
    ham_pers_rid_t rid;
    ham_backend_t *be = db_get_backend(db);
    ham_size_t maxkeylen4ext_key = be_get_keysize(be)-sizeof(ham_pers_rid_t);
    memcpy(&rid, key_get_key(key) + maxkeylen4ext_key, sizeof(rid));
    return ham_db2h_rid(rid);
}

/**
 * set the record-ID of an extended key
 */
static __inline void
key_set_extended_rid(ham_db_t *db, int_key_t *key, ham_offset_t rid_int)
{
    ham_pers_rid_t rid;
    ham_backend_t *be = db_get_backend(db);
    ham_size_t maxkeylen4ext_key = be_get_keysize(be)-sizeof(ham_pers_rid_t);

    ham_h2db_rid(rid, rid_int);
    memcpy(key_get_key(key) + maxkeylen4ext_key, &rid, sizeof(rid));
}

/**
 * get the (persisted) flags of a key
 */
#define key_get_flags(bte)         (bte)->_flags8

/**
 * set the flags of a key
 *
 * Note that the ham_find/ham_cursor_find/ham_cursor_find_ex flags must be
 * defined such that those can peacefully co-exist with these; that's why
 * those public flags start at the value 0x1000 (4096).
 */
#define key_set_flags(bte, f)      (bte)->_flags8=(f)

/**
@defgroup persisted_key_flags Persisted @ref int_key_t flags
@{

Persisted @ref int_key_t flags; also used in @ref ham_key_t::_flags and @ref dupe_entry_t::_flags

These flags are used to signal how the record (or duplicate record) data is encoded in the related persistent header structure:
For sizes larger than 8 bytes, a RID (a 64-bit reference) is stored which points at (references) the actual record data; for sizes equal or less than
8 bytes the record data is stored into the space otherwise reserved for the aforementioned RID. See the explanation accompanying each of the flags to
see the various encodings for zero length, 1..7 byte length and 8 byte length data.

NOTE: persisted flags must fit within a ham_u8_t (1 byte) --> mask: 0x000000FF
*/

/** size < 8; len encoded at byte[7] of key->ptr; data is stored as a 1..7 bytes large byte[] array in key->ptr */
#define KEY_BLOB_SIZE_TINY             0x01
/** size == 8; encoded in key->ptr as a byte[8] array */
#define KEY_BLOB_SIZE_SMALL            0x02
/** size == 0; key->ptr == 0 */
#define KEY_BLOB_SIZE_EMPTY            0x04
/** an extended key; the last 8 bytes of the key stored in-page contains the 'rid' to the trailing remainder of the key.

That is: the key->ptr will store a regular RID to the record data as it would do for any other key with size larger than 8,
while the <em>key data chunk</em> stored at this RID (starting at key->_key[0] a.k.a. @ref key_get_key(x)[0] ) will have a length equal to the configured maximum key length for this Database
(see @ref HAM_PARAM_KEYSIZE). However, the <em>last 8 bytes</em> of that key data space will not contain key data but another RID instead, which
points at the continuation of the key data.

For example, a 500 byte key is considered 'extended' when the key size configured for this Database is less than 500, say for example, 100.
Then the key data will be stored as follows: key->_key[] will point at the regular key space, which will <em>occupy</em> 100 bytes yet
store <em>only</em> 100-8 = 92 bytes, plus another 8 byte RID, pointing at an 'extended' storage space (situated in another page) where
the remaining 500-92 = 408 bytes of key data are stored.

The latter part will be stored in a generic storage page (i.e. not an index page,
but a page which also may store record data and BLOBs) and will occupy an number of 'chunks' (see @ref DB_CHUNKSIZE) as such storage is
allocated on a per-chunk basis: assuming the default chunk size of 32, the remaining 408 key data bytes will therefore 'cost' ROUNDUP(408/32)=13 chunks i.e.
416 bytes.

@note The above is a hint that storing 'extended' keys, i.e. overlarge keys which surpass the initially configured maximum key size, will
      result in a slightly lower insert/update/search/erase performance as the key data has to be fetched from two different pages instead of
      only one (the index page). Nevertheless it is ill advised to just go and configure a very large maximum keysize for the Database as the
      maximum keysize has an adverse effect on performance as well as index pages reserve this space for <em>each</em> key stored in the index
      pages, resulting in a reduced span (number of keys stored per page) in the index for larger key sizes (@ref HAM_PARAM_KEYSIZE).
*/
#define KEY_IS_EXTENDED                0x08
/**
The @ref _ptr element does not point to the related record but instead points to a table BLOB which stored 'rid's for all related records,
one entry for each duplicate key/record tuple. */
#define KEY_HAS_DUPLICATES             0x10
#if 0 /* [GerH] not included as this is a redundant flag; see the blob and other code about the right way to do it, i.e. by checking the USER_ALLOC flag for key and/or record. */
#define KEY_IS_ALLOCATED               0x20  /* memory allocated in hamsterdb */
#endif
/** Marks a deferred delete, i.e. a deleted key which has not yet been removed from the page. */
#define KEY_IS_DELETED                 0x20

/**
@}
*/

/*
and the nonpersistent flag bits:

#define KEY_IS_LT                      0x00010000
#define KEY_IS_GT                      0x00020000
#define KEY_IS_APPROXIMATE             (KEY_IS_LT | KEY_IS_GT)
*/

/*
ham_key_t support internals:
*/

/**
@defgroup ham_key_internal_flags flags used with the @ref ham_key_t INTERNAL USE field _flags.
@{

Note: these flags should NOT overlap with the persisted flags for @ref int_key_t

As these flags NEVER will be persisted, they should be located outside
the range of a @ref ham_u16_t, i.e. outside the mask 0x0000FFFF.
*/

/**
Indicates that the key match was a 'LESS THAN' result, rather than a 'IS EQUAL' match.
*/
#define KEY_IS_LT                      0x00010000
/**
Indicates that the key match was a 'GREATER THAN' result, rather than a 'IS EQUAL' match.
*/
#define KEY_IS_GT                      0x00020000
/**
Indicates that the key match was an 'approximate' result, rather than a 'IS EQUAL' match.
*/
#define KEY_IS_APPROXIMATE             (KEY_IS_LT | KEY_IS_GT)

/**
 * @}
 */

/**
 * get the (non-persisted) flags of a key
 */
#define ham_key_get_intflags(key)         (key)->_flags

/**
 * set the flags of a key
 *
 * Note that the ham_find/ham_cursor_find/ham_cursor_find_ex flags must
 * be defined such that those can peacefully co-exist with these; that's
 * why those public flags start at the value 0x1000 (4096).
 */
#define ham_key_set_intflags(key, f)      (key)->_flags=(f)



/**
 * compare a public key (ham_key_t, LHS) to an internal key (int_key_t, RHS)
 *
 * @return -1, 0, +1 or higher positive values are the result of a successful
 *         key comparison (0 if both keys match, -1 when LHS < RHS key, +1
 *         when LHS > RHS key).
 *
 * @return values less than -1 are @ref ham_status_t error codes and indicate
 *         a failed comparison execution: these are listed in
 *         @ref ham_status_codes .
 *
 * @sa ham_status_codes
 */
extern int
key_compare_pub_to_int(common_btree_datums_t *btdata, const ham_page_t *page, ham_key_t *lhs, ham_u16_t rhs);

/**
 * compare two internal keys
 *
 * @return -1, 0, +1 or higher positive values are the result of a successful
 *         key comparison (0 if both keys match, -1 when LHS < RHS key, +1
 *         when LHS > RHS key).
 *
 * @return values less than -1 are @ref ham_status_t error codes and indicate
 *         a failed comparison execution: these are listed in
 *         @ref ham_status_codes .
 *
 * @sa ham_status_codes
 */
extern int
key_compare_int_to_int(common_btree_datums_t *btdata, const ham_page_t *page,
        ham_u16_t lhs_int, ham_u16_t rhs_int);

/**
 * insert an extended key
 *
 * @return the blob-id of this key in @a rid_ref
 */
extern ham_status_t
key_insert_extended(ham_offset_t *rid_ref, ham_db_t *db, ham_page_t *page, ham_key_t *key);

/**
 * Inserts / stores a record.
 *
 * flags can be
 * - @ref HAM_OVERWRITE
 * - @ref HAM_DUPLICATE_INSERT_BEFORE
 * - @ref HAM_DUPLICATE_INSERT_AFTER
 * - @ref HAM_DUPLICATE_INSERT_FIRST
 * - @ref HAM_DUPLICATE_INSERT_LAST
 * - @ref HAM_DUPLICATE
 *
 * A previously existing blob will be deleted if necessary.
 *
 * The related @a key is patched to point at the 'rid' where the record is stored in the
 * persistent storage (file, ...).
 *
 * When the record is part of a series of key/record tuples which have a duplicate key, then
 * @a position is used to indicate where the given @a record should be stored within the
 * records table (see @ref blob_duplicate_insert). The optional @a new_position call-by-reference
 * value is set to the actual position where the record is stored within the records table.
 * This may differ from the input parameter @a position value when particular hinting flags
 * have been specified as well (see @ref HAM_DUPLICATE_INSERT_BEFORE, @ref HAM_DUPLICATE_INSERT_AFTER,
 * @ref HAM_DUPLICATE_INSERT_FIRST, @ref HAM_DUPLICATE_INSERT_LAST)
 * or when the database has been configured to store the duplicates
 * in record order (see @ref HAM_SORT_DUPLICATES).
 */
extern ham_status_t
key_set_record(ham_db_t *db, int_key_t *key, const ham_record_t *record,
                ham_size_t position, ham_u32_t flags,
                ham_size_t *new_position);

/*
 * deletes a record
 *
 * flag can be BLOB_FREE_ALL_DUPES (declared in blob.h)
 */
extern ham_status_t
key_erase_record(ham_db_t *db, int_key_t *key,
                ham_size_t dupe_id, ham_u32_t flags);



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_KEY_H__ */

/**
* @endcond
*/

