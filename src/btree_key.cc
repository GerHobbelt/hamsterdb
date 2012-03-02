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

#include "btree.h"
#include "btree_classic.h"


int
key_compare_pub_to_int(common_btree_datums_t *btdata, const ham_page_t *page,
        ham_key_t *lhs, ham_u16_t rhs_int)
{
    int_key_t *r;
    //int_key_t *keyarr;
    //btree_node_t *node=ham_page_get_btree_node(page);
    ham_key_t rhs;
    int cmp;
    ham_status_t st;

    ham_db_t * const db = btdata->db;
    //ham_btree_t * const be = btdata->be;
    //const ham_size_t keywidth = btdata->keywidth;

    ham_assert(db == page_get_owner(page), (0));

    r = btree_in_node_get_key_ref(btdata, page, rhs_int);

    st = db_prepare_ham_key_for_compare(db, 1, r, &rhs);
    if (st) {
        ham_assert(st<-1, (0));
        return st;
    }
    ham_assert(!(ham_key_get_intflags(lhs) & KEY_IS_EXTENDED), (0));
    cmp = db_compare_keys(db, lhs, &rhs, NULL, page);

    /* key is always released when the database is closed; errors will be detected by caller */
    return (cmp);
}



int
key_compare_int_to_int(common_btree_datums_t *btdata, const ham_page_t *page,
        ham_u16_t lhs_int, ham_u16_t rhs_int)
{
    int_key_t *l;
    int_key_t *r;
    //btree_node_t *node = ham_page_get_btree_node(page);
    ham_key_t lhs;
    ham_key_t rhs;
    int cmp;
    //ham_btree_t *be = (ham_btree_t *)db_get_backend(db);
    ham_status_t st;
    //ham_size_t keywidth = be_get_keysize(be) + db_get_int_key_header_size();

    ham_db_t * const db = btdata->db;
    //ham_btree_t * const be = btdata->be;

    ham_assert(page_get_owner(page) == db, (0));
    l = btree_in_node_get_key_ref(btdata, page, lhs_int);
    r = btree_in_node_get_key_ref(btdata, page, rhs_int);

    st = db_prepare_ham_key_for_compare(db, 0, l, &lhs);
    if (st)
    {
        ham_assert(st < -1, (0));
        return st;
    }
    st = db_prepare_ham_key_for_compare(db, 1, r, &rhs);
    if (st)
    {
        ham_assert(st < -1, (0));
        return st;
    }

    cmp = db_compare_keys(db, &lhs, &rhs, page, page);

    /* keys are always released when the database is closed; errors will be detected by caller */
    return cmp;
}


ham_status_t
key_insert_extended(ham_offset_t *rid_ref, ham_db_t *db, ham_page_t *page,
        ham_key_t *key)
{
    ham_offset_t blobid;
    ham_u8_t *data_ptr=(ham_u8_t *)key->data;
    ham_status_t st;
    ham_env_t *env = db_get_env(db);
    ham_backend_t *be = db_get_backend(db);
    ham_size_t maxkeylen4ext_key = be_get_keysize(be) - sizeof(ham_pers_rid_t);
    ham_record_t rec={0};
    dev_alloc_request_info_ex_t info = {0};

    info.db = db;
    info.env = env;
    info.dam = db_get_data_access_mode(db);
    info.entire_page = HAM_FALSE;
    info.space_type = PAGE_TYPE_EXTKEYSPACE; // PAGE_TYPE_BLOB;
    info.key = key;

    ham_assert(key->size > be_get_keysize(be), ("invalid keysize"));

    *rid_ref = 0;

    rec.data = data_ptr + maxkeylen4ext_key;
    rec.size = key->size - maxkeylen4ext_key;

    st = blob_allocate(env, &rec, 0, &info, NULL, &blobid);
    if (st)
        return st;

    if (db_get_extkey_cache(db))
    {
        st = extkey_cache_insert(db_get_extkey_cache(db), blobid,
                key->size, (const ham_u8_t *)key->data, page);
        if (st)
            return st;
    }

    *rid_ref = blobid;

	ham_nuke_stack_space(rec);
    return HAM_SUCCESS;
}

ham_status_t
key_set_record(ham_db_t *db, int_key_t *key, const ham_record_t *record,
                ham_size_t position, ham_u32_t flags,
                ham_size_t *new_position)
{
    ham_status_t st;
    ham_env_t *env = db_get_env(db);
    //ham_offset_t rid = 0;
    ham_offset_t ptr = key_get_ptr(key);
    ham_u8_t oldflags = key_get_flags(key);
    dev_alloc_request_info_ex_t info = {0};

    info.db = db;
    info.env = env;
    info.dam = db_get_data_access_mode(db);
    info.entire_page = HAM_FALSE;
    info.space_type = PAGE_TYPE_RECDATASPACE; // PAGE_TYPE_BLOB;
    info.record = record;
    info.int_key = key;
    info.insert_flags = flags;

    key_set_flags(key,
            oldflags&~(KEY_BLOB_SIZE_SMALL
                |KEY_BLOB_SIZE_TINY
                |KEY_BLOB_SIZE_EMPTY));

    /*
     * no existing record, just create a new record (but not a duplicate)?
     */
    if (!ptr
            && !(oldflags&(KEY_BLOB_SIZE_SMALL
                          |KEY_BLOB_SIZE_TINY
                          |KEY_BLOB_SIZE_EMPTY)))
    {
        if (record->size <= sizeof(ham_pers_rid_t))
        {
            ham_pers_rid_t rid = {0};

            if (record->flags & HAM_PARTIAL)
            {
                ham_trace(("flag HAM_PARTIAL is not allowed if the record is tiny (size <= %u bytes)", (unsigned)sizeof(ham_pers_rid_t)));
                return HAM_INV_PARAMETER;
            }

            ham_assert(record->partial_offset == 0, (0));
            ham_assert(record->partial_size == 0 || record->partial_size == record->size, (0));
            ham_assert(!(record->flags & HAM_PARTIAL), ("HAM_PARTIAL is not an accepted flag for tiny records!"));

            if (record->size==0)
            {
                ham_assert(!record->data, (0));
                key_set_flags(key, key_get_flags(key)|KEY_BLOB_SIZE_EMPTY);
            }
            else if (record->size < sizeof(ham_pers_rid_t))
            {
                ham_u8_t *p=(ham_u8_t *)&rid;
                p[sizeof(rid)-1]=(ham_u8_t)record->size;
                ham_assert(record->data, (0));
                memcpy(&rid, record->data, record->size);
                key_set_flags(key, key_get_flags(key)|KEY_BLOB_SIZE_TINY);
            }
            else
            {
                ham_assert(record->data, (0));
                ham_assert(record->size == sizeof(rid), (0));
                memcpy(&rid, record->data, sizeof(rid));
                key_set_flags(key, key_get_flags(key)|KEY_BLOB_SIZE_SMALL);
            }
            key_set_ptr_direct_ref(key, rid);
			ham_nuke_stack_space(rid);
        }
        else
        {
            ham_offset_t rid = 0;

            st = blob_allocate(env, record, 0, &info, NULL, &rid);
            if (st)
                return st;
            key_set_ptr(key, rid);
        }
    }
    else if (!(oldflags&KEY_HAS_DUPLICATES)
            && !(flags&(HAM_DUPLICATE
                        |HAM_DUPLICATE_INSERT_BEFORE
                        |HAM_DUPLICATE_INSERT_AFTER
                        |HAM_DUPLICATE_INSERT_FIRST
                        |HAM_DUPLICATE_INSERT_LAST)))
    {
        /*
        * an existing record, which is overwritten
        */
        if (record->size > sizeof(ham_pers_rid_t))
        {
            ham_offset_t rid = 0;

            /*
            * an existing record, which is overwritten with a big record
            *
            * Note that the case where old record is EMPTY (!ptr) or
            * SMALL (size = 8, but content = 00000000 --> !ptr) are caught here
            * and in the next branch, as they should.
            */
            if (oldflags&(KEY_BLOB_SIZE_SMALL
                         |KEY_BLOB_SIZE_TINY
                         |KEY_BLOB_SIZE_EMPTY))
            {
                if ((record->flags & HAM_PARTIAL) && record->partial_offset != 0)
                {
                    ham_trace(("flag HAM_PARTIAL is not allowed if the record was tiny (size <= %u bytes) and is increased in size when the partial_offset isn't zero(0)", (unsigned)sizeof(ham_pers_rid_t)));
                    return HAM_INV_PARAMETER;
                }

                ham_assert(record->partial_offset == 0, (0));
                ham_assert(record->partial_size == 0 || record->partial_size == record->size, (0));
                ham_assert(!(record->flags & HAM_PARTIAL), ("HAM_PARTIAL is not an accepted flag for /formerly/ tiny records!"));

                st = blob_allocate(env, record, 0, &info, NULL, &rid);
                ham_assert(st ? rid == 0 : 1, (0));
                ham_assert(!st ? rid != 0 : 1, (0));
                if (!rid)
                    return st ? st : HAM_INTERNAL_ERROR;
                key_set_ptr(key, rid);
            }
            else
            {
                st = blob_overwrite(env, ptr, record, 0, &info, &rid);
                ham_assert(st ? rid == 0 : 1, (0));
                ham_assert(!st ? rid != 0 : 1, (0));
                if (!rid)
                    return st ? st : HAM_INTERNAL_ERROR;
                key_set_ptr(key, rid);
            }
        }
        else
        {
            ham_pers_rid_t rid = {0};

            ham_assert(record->size <= sizeof(ham_pers_rid_t), (0));

            if (record->flags & HAM_PARTIAL)
            {
                ham_trace(("flag HAM_PARTIAL is not allowed if the record is tiny (size <= %u bytes)", (unsigned)sizeof(ham_pers_rid_t)));
                return HAM_INV_PARAMETER;
            }

            ham_assert(record->partial_offset == 0, (0));
            ham_assert(record->partial_size == 0 || record->partial_size == record->size, (0));
            ham_assert(!(record->flags & HAM_PARTIAL), ("HAM_PARTIAL is not an accepted flag for tiny records!"));

            /*
             * an existing key which is overwritten with a small record
             */
            if (!(oldflags&(KEY_BLOB_SIZE_SMALL
                           |KEY_BLOB_SIZE_TINY
                           |KEY_BLOB_SIZE_EMPTY)))
            {
                st=blob_free(env, ptr, 0);
                if (st)
                    return st;
            }
            if (record->size==0)
            {
                ham_assert(!record->data, (0));
                key_set_flags(key, key_get_flags(key)|KEY_BLOB_SIZE_EMPTY);
            }
            else if (record->size < sizeof(ham_pers_rid_t))
            {
                ham_u8_t *p=(ham_u8_t *)&rid;
                p[sizeof(rid)-1]=(ham_u8_t)record->size;
                ham_assert(record->data, (0));
                memcpy(&rid, record->data, record->size);
                key_set_flags(key, key_get_flags(key)|KEY_BLOB_SIZE_TINY);
            }
            else
            {
                ham_assert(record->data, (0));
                ham_assert(record->size == sizeof(rid), (0));
                memcpy(&rid, record->data, sizeof(rid));
                key_set_flags(key, key_get_flags(key)|KEY_BLOB_SIZE_SMALL);
            }
            key_set_ptr_direct_ref(key, rid);
			ham_nuke_stack_space(rid);
        }
    }
    else
    {
        /*
         * a duplicate of an existing key - always insert it at the end of
         * the duplicate list (unless the DUPLICATE flags say otherwise OR
         * when we have a duplicate-record comparison function for
         * ordered insertion of duplicate records)
         *
         * create a duplicate list, if it does not yet exist
         */
        dupe_entry_t entries[2] = {{0}};
        int i=0;

        if (record->flags & HAM_PARTIAL)
        {
            ham_trace(("flag HAM_PARTIAL is not allowed if the record is a duplicate"));
            return HAM_INV_PARAMETER;
        }

        ham_assert(record->partial_offset == 0, (0));
        ham_assert(record->partial_size == 0 || record->partial_size == record->size, (0));
        ham_assert(!(record->flags & HAM_PARTIAL), (0));

        ham_assert((flags&(HAM_DUPLICATE
                          |HAM_DUPLICATE_INSERT_BEFORE
                          |HAM_DUPLICATE_INSERT_AFTER
                          |HAM_DUPLICATE_INSERT_FIRST
                          |HAM_DUPLICATE_INSERT_LAST
                          |HAM_OVERWRITE)), (0));
        //memset(entries, 0, sizeof(entries));
        if (!(oldflags & KEY_HAS_DUPLICATES))
        {
            ham_assert((flags&(HAM_DUPLICATE
                              |HAM_DUPLICATE_INSERT_BEFORE
                              |HAM_DUPLICATE_INSERT_AFTER
                              |HAM_DUPLICATE_INSERT_FIRST
                              |HAM_DUPLICATE_INSERT_LAST)), (0));
            dupe_entry_set_flags(&entries[i],
                        oldflags&(KEY_BLOB_SIZE_SMALL
                                |KEY_BLOB_SIZE_TINY
                                |KEY_BLOB_SIZE_EMPTY));

            if (oldflags & (KEY_BLOB_SIZE_SMALL
                          |KEY_BLOB_SIZE_TINY
                          |KEY_BLOB_SIZE_EMPTY))
            {
                ham_pers_rid_t *rptr = key_get_ptr_direct_ref(key);
                dupe_entry_set_rid_direct_ref(&entries[i], *rptr);
            }
            else
            {
                dupe_entry_set_rid(&entries[i], ptr);
            }
            i++;
        }

        if (record->size <= sizeof(ham_pers_rid_t))
        {
            ham_pers_rid_t rid = {0};

            if (record->size==0)
            {
                ham_assert(!record->data, (0));
                dupe_entry_set_flags(&entries[i], KEY_BLOB_SIZE_EMPTY);
            }
            else if (record->size < sizeof(ham_pers_rid_t))
            {
                ham_u8_t *p=(ham_u8_t *)&rid;
                p[sizeof(rid)-1]=(ham_u8_t)record->size;
                ham_assert(record->data, (0));
                memcpy(&rid, record->data, record->size);
                dupe_entry_set_flags(&entries[i], KEY_BLOB_SIZE_TINY);
            }
            else
            {
                ham_assert(record->data, (0));
                ham_assert(record->size == sizeof(rid), (0));
                memcpy(&rid, record->data, sizeof(rid));
                dupe_entry_set_flags(&entries[i], KEY_BLOB_SIZE_SMALL);
            }
            dupe_entry_set_rid_direct_ref(&entries[i], rid);
			ham_nuke_stack_space(rid);
        }
        else
        {
            ham_offset_t rid = 0;

            st = blob_allocate(env, record, 0, &info, NULL, &rid);
            ham_assert(st ? rid == 0 : 1, (0));
            ham_assert(!st ? rid != 0 : 1, (0));
            if (!rid)
                return st ? st : HAM_INTERNAL_ERROR;
            dupe_entry_set_flags(&entries[i], 0);
            dupe_entry_set_rid(&entries[i], rid);
        }
        i++;

        {
            ham_offset_t rid = 0;

            st = blob_duplicate_insert(db, (i==2 ? 0 : ptr),
                    record, position, flags,
                    &entries[0], i, &info, &rid, new_position);
            if (st)
            {
                /* don't leak memory through the blob allocation above */
                ham_assert((!(dupe_entry_get_flags(&entries[i-1])
                                & (KEY_BLOB_SIZE_SMALL
                                   | KEY_BLOB_SIZE_TINY
                                   | KEY_BLOB_SIZE_EMPTY)))
                            == (record->size > sizeof(ham_pers_rid_t)), (0));

                if (record->size > sizeof(ham_pers_rid_t))
                {
                    (void)blob_free(env, dupe_entry_get_rid(&entries[i-1]), 0);
                }
                return st;
            }

            key_set_flags(key, key_get_flags(key)|KEY_HAS_DUPLICATES);
            if (rid)
                key_set_ptr(key, rid);
        }
		ham_nuke_stack_space(entries);
    }

    return HAM_SUCCESS;
}

ham_status_t
key_erase_record(ham_db_t *db, int_key_t *key,
                ham_size_t dupe_id, ham_u32_t flags)
{
    ham_status_t st;
    ham_env_t *env = db_get_env(db);
    ham_offset_t rid;

    if (!(key_get_flags(key)&(KEY_BLOB_SIZE_SMALL
                        |KEY_BLOB_SIZE_TINY
                        |KEY_BLOB_SIZE_EMPTY)))
    {
        if (key_get_flags(key) & KEY_HAS_DUPLICATES)
        {
            /* delete one (or all) duplicates */
            st=blob_duplicate_erase(db, key, dupe_id, flags,
                    &rid);
            if (st)
                return (st);
            if (flags & BLOB_FREE_ALL_DUPES)
            {
                key_set_flags(key, key_get_flags(key)&~KEY_HAS_DUPLICATES);
                key_set_ptr(key, 0);
            }
            else {
                key_set_ptr(key, rid);
                if (!rid) /* rid == 0: the last duplicate was deleted */
                    key_set_flags(key, 0);
            }
        }
        else {
            /* delete the blob */
            st=blob_free(env, key_get_ptr(key), 0);
            if (st)
                return (st);
            key_set_ptr(key, 0);
        }
    }
    else {
        key_set_flags(key, key_get_flags(key)&~(KEY_BLOB_SIZE_SMALL
                    | KEY_BLOB_SIZE_TINY
                    | KEY_BLOB_SIZE_EMPTY
                    | KEY_HAS_DUPLICATES));
        key_set_ptr(key, 0);
    }

    return HAM_SUCCESS;
}

#if 0 /* [i_a] inlined */

ham_offset_t
key_get_extended_rid(ham_db_t *db, int_key_t *key)
{
    ham_pers_rid_t rid;
    ham_backend_t *be = db_get_backend(db);
    ham_size_t maxkeylen4ext_key = be_get_keysize(be)-sizeof(ham_pers_rid_t);
    memcpy(&rid, key_get_key(key) + maxkeylen4ext_key, sizeof(rid));
    return ham_db2h_rid(rid);
}

void
key_set_extended_rid(ham_db_t *db, int_key_t *key, ham_offset_t rid_int)
{
    ham_pers_rid_t rid;
    ham_backend_t *be = db_get_backend(db);
    ham_size_t maxkeylen4ext_key = be_get_keysize(be)-sizeof(ham_pers_rid_t);

    ham_h2db_rid(rid, rid_int);
    memcpy(key_get_key(key) + maxkeylen4ext_key, &rid, sizeof(rid));
}

#endif

/**
* @endcond
*/

