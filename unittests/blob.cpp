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

#include "../src/config.h"

#include <ham/hamsterdb.h>
#include "../src/blob.h"
#include "../src/db.h"
#include "../src/env.h"
<<<<<<< HEAD
#include "../src/page.h"
#include "../src/btree_key.h"
#include "../src/freelist.h"
=======
#include "../src/device.h"
#include "../src/freelist.h"
#include "../src/mem.h"
#include "../src/page.h"
#include "memtracker.h"
>>>>>>> flash-bang-grenade
#include "os.hpp"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <cstring>


using namespace bfc;

class BlobTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    BlobTest(ham_bool_t inmemory=HAM_FALSE, ham_bool_t use_txn=HAM_FALSE,
                ham_size_t cachesize=0, ham_size_t pagesize=0,
                const char *name="BlobTest")
    :   hamsterDB_fixture(name),
<<<<<<< HEAD
        m_db(0), m_inmemory(inmemory), m_use_txn(use_txn),
=======
        m_db(NULL), m_env(NULL), m_alloc(NULL), m_inmemory(inmemory), m_use_txn(use_txn),
>>>>>>> flash-bang-grenade
        m_cachesize(cachesize), m_pagesize(pagesize)
    {
        //if (name)
        //    return;
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(BlobTest, structureTest);
        BFC_REGISTER_TEST(BlobTest, dupeStructureTest);
        BFC_REGISTER_TEST(BlobTest, allocReadFreeTest);
        BFC_REGISTER_TEST(BlobTest, replaceTest);
        BFC_REGISTER_TEST(BlobTest, replaceWithBigTest);
        BFC_REGISTER_TEST(BlobTest, replaceWithSmallTest);
        BFC_REGISTER_TEST(BlobTest, replaceBiggerAndBiggerTest);
        /* negative tests are not necessary, because hamsterdb asserts that
         * blob-IDs actually exist */
        BFC_REGISTER_TEST(BlobTest, multipleAllocReadFreeTest);
        BFC_REGISTER_TEST(BlobTest, hugeBlobTest);
        BFC_REGISTER_TEST(BlobTest, smallBlobTest);
    }

protected:
    ham_db_t *m_db;
    ham_env_t *m_env;
<<<<<<< HEAD
=======
    mem_allocator_t *m_alloc;
>>>>>>> flash-bang-grenade
    ham_bool_t m_inmemory;
    ham_bool_t m_use_txn;
    ham_size_t m_cachesize;
    ham_size_t m_pagesize;

public:
    virtual void setup()
    {
        __super::setup();

        ham_parameter_t params[3]=
        {
            { HAM_PARAM_CACHESIZE, m_cachesize },
            // set pagesize, otherwise 16-bit limit bugs in freelist
            // will fire on Win32
            { HAM_PARAM_PAGESIZE, (m_pagesize ? m_pagesize : 4096) },
            { 0, 0 }
        };

        os::unlink(BFC_OPATH(".test"));

<<<<<<< HEAD
=======
        m_alloc = memtracker_new();
        ham_set_default_allocator_template(m_alloc);
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0,
                ham_create_ex(m_db, BFC_OPATH(".test"),
                    (m_inmemory
                        ? HAM_IN_MEMORY_DB
                        : (m_use_txn
                            ? HAM_ENABLE_TRANSACTIONS
                            : 0)),
                        0644, &params[0]));
<<<<<<< HEAD
        m_env=ham_get_env(m_db);
    }
    
=======
        m_env = db_get_env(m_db);
    }

>>>>>>> flash-bang-grenade
    virtual void teardown()
    {
        __super::teardown();

        /* clear the changeset, otherwise ham_close will complain */
        if (!m_inmemory && m_env)
            ((Environment *)m_env)->get_changeset().clear();

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
<<<<<<< HEAD
        ham_delete(m_db);
=======
        BFC_ASSERT_EQUAL(0, ham_delete(m_db));
        BFC_ASSERT_EQUAL(memtracker_get_leaks(ham_get_default_allocator_template()), 0U);
>>>>>>> flash-bang-grenade
    }

    void structureTest(void)
    {
        blob_t b;
        ::memset(&b, 0, sizeof(b));

        blob_set_self(&b, (ham_offset_t)0x12345ull);
        BFC_ASSERT_EQUAL((ham_offset_t)0x12345ull,
                        blob_get_self(&b));

        blob_set_alloc_size(&b, 0x789ull);
        BFC_ASSERT_EQUAL((ham_u64_t)0x789ull, blob_get_alloc_size(&b));

        blob_set_size(&b, 0x123ull);
        BFC_ASSERT_EQUAL((ham_u64_t)0x123ull, blob_get_size(&b));

        blob_set_flags(&b, 0x13);
        BFC_ASSERT_EQUAL((ham_u32_t)0x13, blob_get_flags(&b));
    }

    void dupeStructureTest(void)
    {
        dupe_table_t t;
        ::memset(&t, 0, sizeof(t));

        dupe_table_set_count(&t, 0x789ull);
        BFC_ASSERT_EQUAL((ham_u32_t)0x789ull, dupe_table_get_count(&t));

        dupe_table_set_capacity(&t, 0x123ull);
        BFC_ASSERT_EQUAL((ham_u32_t)0x123ull, dupe_table_get_capacity(&t));

        dupe_entry_t *e=dupe_table_get_entry(&t, 0);
        dupe_entry_set_flags(e, 0x13);
        BFC_ASSERT_EQUAL((ham_u8_t)0x13, dupe_entry_get_flags(e));

        /*
        setting a dupe record RID while that record has been flagged as tiny/small,
        will trigger an assertion failure, at least in debug builds.

        That's why we reset the flags above, before we do the next test.
        */
        dupe_entry_set_flags(e, 0);
        dupe_entry_set_rid(e, (ham_offset_t)0x12345ull);
        BFC_ASSERT_EQUAL((ham_offset_t)0x12345ull,
                        dupe_entry_get_rid(e));
    }

    void allocReadFreeTest(void)
    {
        ham_u8_t buffer[64];
        ham_offset_t blobid;
        ham_record_t record;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.record = &record;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_BLOB;

        ::memset(&record, 0, sizeof(record));
        ::memset(&buffer, 0x12, sizeof(buffer));

        record.size=sizeof(buffer);
        record.data=buffer;
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_allocate((Environment *)m_env, (Database *)m_db,
                                &record, 0, &blobid));
        BFC_ASSERT(blobid!=0);
=======
        ham_assert(info.space_type != PAGE_TYPE_UNKNOWN, (0)); // not necessarily a TYPE_BLOB though...
        BFC_ASSERT_EQUAL(0, blob_allocate(m_env, &record, 0, &info, 0, &blobid));
        BFC_ASSERT_NOTNULL(blobid);
>>>>>>> flash-bang-grenade

        BFC_ASSERT_EQUAL(0, blob_read((Database *)m_db, blobid, &record, 0));
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer, record.data, record.size));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_free((Environment *)m_env,
                                (Database *)m_db, blobid, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_free(m_env, blobid, 0));
>>>>>>> flash-bang-grenade
    }

    void replaceTest(void)
    {
        ham_u8_t buffer[64], buffer2[64];
        ham_offset_t blobid, blobid2;
        ham_record_t record;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.record = &record;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_BLOB;

        ::memset(&record,  0, sizeof(record));
        ::memset(&buffer,  0x12, sizeof(buffer));
        ::memset(&buffer2, 0x15, sizeof(buffer2));

        record.size=sizeof(buffer);
        record.data=buffer;
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_allocate((Environment *)m_env, (Database *)m_db,
                                &record, 0, &blobid));
        BFC_ASSERT(blobid!=0);
=======
        BFC_ASSERT_EQUAL(0, blob_allocate(m_env, &record, 0, &info, 0, &blobid));
        BFC_ASSERT_NOTNULL(blobid);
>>>>>>> flash-bang-grenade

        BFC_ASSERT_EQUAL(0, blob_read((Database *)m_db, blobid, &record, 0));
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer, record.data, record.size));

        record.size=sizeof(buffer2);
        record.data=buffer2;
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_overwrite((Environment *)m_env, (Database *)m_db,
                                blobid, &record, 0, &blobid2));
        BFC_ASSERT(blobid2!=0);

        BFC_ASSERT_EQUAL(0,
                blob_read((Database *)m_db, blobid2, &record, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_overwrite(m_env, blobid, &record, 0, &info, &blobid2));
        BFC_ASSERT_NOTNULL(blobid2);

        BFC_ASSERT_EQUAL(0, blob_read(m_db, blobid2,
                                &record, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer2));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer2, record.data, record.size));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_free((Environment *)m_env, (Database *)m_db,
                                blobid2, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_free(m_env, blobid2, 0));
>>>>>>> flash-bang-grenade
    }

    void replaceWithBigTest(void)
    {
        ham_u8_t buffer[64], buffer2[128];
        ham_offset_t blobid, blobid2;
        ham_record_t record;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.record = &record;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_BLOB;

        ::memset(&record,  0, sizeof(record));
        ::memset(&buffer,  0x12, sizeof(buffer));
        ::memset(&buffer2, 0x15, sizeof(buffer2));

        record.data=buffer;
        record.size=sizeof(buffer);
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_allocate((Environment *)m_env, (Database *)m_db,
                                &record, 0, &blobid));
        BFC_ASSERT(blobid!=0);

        BFC_ASSERT_EQUAL(0,
                blob_read((Database *)m_db, blobid, &record, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_allocate(m_env, &record, 0, &info, 0, &blobid));
        BFC_ASSERT_NOTNULL(blobid);

        BFC_ASSERT_EQUAL(0, blob_read(m_db, blobid,
                                &record, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer, record.data, record.size));

        record.size=sizeof(buffer2);
        record.data=buffer2;
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_overwrite((Environment *)m_env, (Database *)m_db, blobid,
                    &record, 0, &blobid2));
        BFC_ASSERT(blobid2!=0);

        BFC_ASSERT_EQUAL(0,
                blob_read((Database *)m_db, blobid2, &record, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_overwrite(m_env, blobid, &record, 0, &info, &blobid2));
        BFC_ASSERT_NOTNULL(blobid2);

        BFC_ASSERT_EQUAL(0, blob_read(m_db, blobid2,
                                &record, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer2));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer2, record.data, record.size));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_free((Environment *)m_env, (Database *)m_db, blobid2, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_free(m_env, blobid2, 0));
>>>>>>> flash-bang-grenade
    }

    void replaceWithSmallTest(void)
    {
        ham_status_t st;
        ham_u8_t buffer[128], buffer2[64];
        ham_offset_t blobid, blobid2;
        ham_offset_t addr;
        ham_record_t record;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.record = &record;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_BLOB;

        ::memset(&record,  0, sizeof(record));
        ::memset(&buffer,  0x12, sizeof(buffer));
        ::memset(&buffer2, 0x15, sizeof(buffer2));

        record.data=buffer;
        record.size=sizeof(buffer);
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_allocate((Environment *)m_env, (Database *)m_db, &record,
                                0, &blobid));
        BFC_ASSERT(blobid!=0);

        BFC_ASSERT_EQUAL(0,
                blob_read((Database *)m_db, blobid, &record, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_allocate(m_env, &record, 0, &info, 0, &blobid));
        BFC_ASSERT_NOTNULL(blobid);

        BFC_ASSERT_EQUAL(0, blob_read(m_db, blobid,
                                &record, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer, record.data, record.size));

        record.size=sizeof(buffer2);
        record.data=buffer2;
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                blob_overwrite((Environment *)m_env, (Database *)m_db,
                                blobid, &record, 0, &blobid2));
        BFC_ASSERT(blobid2!=0);

        BFC_ASSERT_EQUAL(0,
                blob_read((Database *)m_db, blobid2, &record, 0));
=======
        BFC_ASSERT_EQUAL(0, blob_overwrite(m_env, blobid, &record,
                    0, &info, &blobid2));
        BFC_ASSERT_NOTNULL(blobid2);

        BFC_ASSERT_EQUAL(0, blob_read(m_db, blobid2,
                                &record, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)sizeof(buffer2));
        BFC_ASSERT_EQUAL(0, ::memcmp(buffer2, record.data, record.size));

        /* make sure that at least 64bit are in the freelist */
<<<<<<< HEAD
        if (!m_inmemory) {
            ham_offset_t addr;
            BFC_ASSERT_EQUAL(0, freel_alloc_area(&addr,
                            (Environment *)m_env, (Database *)m_db, 64));
            BFC_ASSERT(addr!=0);
        }

        BFC_ASSERT_EQUAL(0,
                blob_free((Environment *)m_env,
                                (Database *)m_db, blobid2, 0));

        /* and now another 64bit should be in the freelist */
        if (!m_inmemory) {
            ham_offset_t addr;
            BFC_ASSERT_EQUAL(0, freel_alloc_area(&addr,
                        (Environment *)m_env, (Database *)m_db, 64));
            BFC_ASSERT(addr!=0);
=======
        if (!m_inmemory)
        {
            st = freel_alloc_area(&addr, 64, &info);
            BFC_ASSERT_NOTEQUAL(addr, 0u);
            BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        }

        BFC_ASSERT_EQUAL(0, blob_free(m_env, blobid2, 0));

        /* and now another 64bit should be in the freelist */
        if (!m_inmemory)
        {
            st = freel_alloc_area(&addr, 64, &info);
            BFC_ASSERT_NOTEQUAL(addr, 0u);
            BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        }
    }

    void replaceBiggerAndBiggerTest(void)
    {
        const int BLOCKS=32;
        unsigned ps=env_get_pagesize(m_env);
        ham_u8_t *buffer=(ham_u8_t *)malloc(ps*BLOCKS*2);
        ham_offset_t blobid, blobid2;
        ham_record_t record;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.record = &record;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_BLOB;

        ::memset(&record, 0, sizeof(record));
        ::memset(buffer,  0, ps*BLOCKS*2);

        /* first: create a big blob and erase it - we want to use the
         * space from the freelist */
        record.data=buffer;
        record.size=ps*BLOCKS*2;
        BFC_ASSERT_EQUAL(0, blob_allocate(m_env, &record, 0, &info, 0, &blobid));
        BFC_ASSERT_NOTNULL(blobid);

        /* verify it */
        BFC_ASSERT_EQUAL(0, blob_read(m_db, blobid, &record, 0));
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)ps*BLOCKS*2);

        /* and erase it */
        BFC_ASSERT_EQUAL(0, blob_free(m_env, blobid, 0));

        /* now use a loop to allocate the buffer, and make it bigger and
         * bigger */
        for (int i=1; i<32; i++) {
            record.size=i*ps;
            record.data=(void *)buffer;
            ::memset(buffer, i, record.size);
            if (i==1) {
                BFC_ASSERT_EQUAL(0,
                        blob_allocate(m_env, &record, 0, &info, 0, &blobid2));
            }
            else {
                BFC_ASSERT_EQUAL(0,
                        blob_overwrite(m_env, blobid,
                                    &record, 0, &info, &blobid2));
            }
            blobid=blobid2;
            BFC_ASSERT_NOTNULL(blobid);
>>>>>>> flash-bang-grenade
        }
        BFC_ASSERT_EQUAL(0, blob_free(m_env, blobid, 0));
        ::free(buffer);
    }

    void replaceBiggerAndBiggerTest(void)
    {
        const int BLOCKS=32;
        unsigned ps=((Environment *)m_env)->get_pagesize();
        ham_u8_t *buffer=(ham_u8_t *)malloc(ps*BLOCKS*2);
        ham_offset_t blobid, blobid2;
        ham_record_t record;
        ::memset(&record, 0, sizeof(record));
        ::memset(buffer,  0, ps*BLOCKS*2);

        /* first: create a big blob and erase it - we want to use the
         * space from the freelist */
        record.data=buffer;
        record.size=ps*BLOCKS*2;
        BFC_ASSERT_EQUAL(0,
                blob_allocate((Environment *)m_env, (Database *)m_db, &record, 0, &blobid));
        BFC_ASSERT(blobid!=0);

        /* verify it */
        BFC_ASSERT_EQUAL(0, blob_read((Database *)m_db, blobid, &record, 0));
        BFC_ASSERT_EQUAL(record.size, (ham_size_t)ps*BLOCKS*2);

        /* and erase it */
        BFC_ASSERT_EQUAL(0, blob_free((Environment *)m_env, (Database *)m_db, blobid, 0));

        /* now use a loop to allocate the buffer, and make it bigger and
         * bigger */
        for (int i=1; i<32; i++) {
            record.size=i*ps;
            record.data=(void *)buffer;
            ::memset(buffer, i, record.size);
            if (i==1) {
                BFC_ASSERT_EQUAL(0,
                        blob_allocate((Environment *)m_env,
                            (Database *)m_db, &record, 0, &blobid2));
            }
            else {
                BFC_ASSERT_EQUAL(0,
                        blob_overwrite((Environment *)m_env, (Database *)m_db, blobid,
                                    &record, 0, &blobid2));
            }
            blobid=blobid2;
            BFC_ASSERT(blobid!=0);
        }
        BFC_ASSERT_EQUAL(0, blob_free((Environment *)m_env, (Database *)m_db, blobid, 0));
        ::free(buffer);
    }

    void loopInsert(int loops, int factor)
    {
        ham_u8_t *buffer;
        ham_offset_t *blobid;
        ham_record_t record;
        ham_txn_t *txn = 0; /* need a txn object for the blob routines */
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.record = &record;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_BLOB;

        ::memset(&record, 0, sizeof(record));
        ::memset(&buffer, 0x12, sizeof(buffer));

        blobid=(ham_offset_t *)::malloc(sizeof(ham_offset_t)*loops);
<<<<<<< HEAD
        BFC_ASSERT(blobid!=0);
        if (!m_inmemory && m_use_txn)
            BFC_ASSERT(ham_txn_begin(&txn, m_env, 0, 0, 0)==HAM_SUCCESS);
=======
        BFC_ASSERT_NOTNULL(blobid);
        if (!m_inmemory && m_use_txn)
            BFC_ASSERT_EQUAL(ham_txn_begin(&txn, m_db, 0), HAM_SUCCESS);
>>>>>>> flash-bang-grenade

        for (int i=0; i<loops; i++) {
            buffer=(ham_u8_t *)::malloc((i+1)*factor);
            BFC_ASSERT_I(buffer!=0, i);
            ::memset(buffer, (char)i, (i+1)*factor);

            ham_record_t rec={0};
            rec.data=buffer;
            rec.size=(ham_size_t)((i+1)*factor);
            BFC_ASSERT_EQUAL(0,
<<<<<<< HEAD
                        blob_allocate((Environment *)m_env, (Database *)m_db, &rec,
                            0, &blobid[i]));
=======
                        blob_allocate(m_env, &rec, 0, &info, 0, &blobid[i]));
>>>>>>> flash-bang-grenade
            BFC_ASSERT_I(blobid[i]!=0, i);

            ::free(buffer);
        }

        for (int i=0; i<loops; i++) {
            buffer=(ham_u8_t *)::malloc((i+1)*factor);
            BFC_ASSERT_I(buffer!=0, i);
            ::memset(buffer, (char)i, (i+1)*factor);

<<<<<<< HEAD
            BFC_ASSERT_EQUAL_I(0,
                    blob_read((Database *)m_db, blobid[i], &record, 0), i);
=======
            BFC_ASSERT_EQUAL_I(0, blob_read(m_db, blobid[i],
                                &record, 0), i);
>>>>>>> flash-bang-grenade
            BFC_ASSERT_EQUAL_I(record.size, (ham_size_t)(i+1)*factor, i);
            BFC_ASSERT_I(0==::memcmp(buffer, record.data, record.size), i);

            ::free(buffer);
        }

        for (int i=0; i<loops; i++) {
<<<<<<< HEAD
            BFC_ASSERT_EQUAL_I(0,
                    blob_free((Environment *)m_env, (Database *)m_db, blobid[i], 0), i);
=======
            BFC_ASSERT_EQUAL_I(0, blob_free(m_env, blobid[i], 0), i);
>>>>>>> flash-bang-grenade
        }

        ::free(blobid);
        if (!m_inmemory && m_use_txn)
<<<<<<< HEAD
            BFC_ASSERT(ham_txn_commit(txn, 0)==HAM_SUCCESS);
=======
            BFC_ASSERT_EQUAL(ham_txn_commit(txn, 0), HAM_SUCCESS);
>>>>>>> flash-bang-grenade
    }

    void multipleAllocReadFreeTest(void)
    {
        loopInsert(20, 2048);
    }

    void hugeBlobTest(void)
    {
        loopInsert(10, 1024*1024*4);
    }

    void smallBlobTest(void)
    {
        loopInsert(20, 64);
    }

};

class FileBlobTest : public BlobTest
{
public:
    FileBlobTest(ham_size_t cachesize=1024,
                ham_size_t pagesize=0, const char *name="FileBlobTest")
    : BlobTest(HAM_FALSE, HAM_TRUE, cachesize, pagesize, name)
    {
    }
};

class FileBlobNoTxnTest : public BlobTest
{
public:
    FileBlobNoTxnTest(ham_size_t cachesize=1024,
                ham_size_t pagesize=0, const char *name="FileBlobNoTxnTest")
    : BlobTest(HAM_FALSE, HAM_FALSE, cachesize, pagesize, name)
    {
    }
};

class FileBlobTest64Kpage : public FileBlobTest
{
public:
    FileBlobTest64Kpage(ham_size_t cachesize=64*1024,
                ham_size_t pagesize=64*1024, const char *name="FileBlobTest64Kpage")
    : FileBlobTest(cachesize, pagesize, name)
    {
    }
};

class NoCacheBlobTest : public BlobTest
{
public:
    NoCacheBlobTest(ham_size_t cachesize=0,
                ham_size_t pagesize=0, const char *name="NoCacheBlobTest")
    : BlobTest(HAM_FALSE, HAM_TRUE, cachesize, pagesize, name)
    {
    }
};

class NoCacheBlobNoTxnTest : public BlobTest
{
public:
    NoCacheBlobNoTxnTest(ham_size_t cachesize=0,
                ham_size_t pagesize=0, const char *name="NoCacheBlobNoTxnTest")
    : BlobTest(HAM_FALSE, HAM_FALSE, cachesize, pagesize, name)
    {
    }
};

class NoCacheBlobTest64Kpage : public NoCacheBlobTest
{
public:
    NoCacheBlobTest64Kpage(ham_size_t cachesize=0, ham_size_t pagesize=64*1024,
           const char *name="NoCacheBlobTest64Kpage")
    : NoCacheBlobTest(cachesize, pagesize, name)
    {
    }
};


class InMemoryBlobTest : public BlobTest
{
public:
    InMemoryBlobTest(ham_size_t cachesize=0, ham_size_t pagesize=0,
            const char *name="InMemoryBlobTest")
    : BlobTest(HAM_TRUE, HAM_FALSE, cachesize, pagesize, name)
    {
    }
};

class InMemoryBlobTest64Kpage : public InMemoryBlobTest
{
public:
    InMemoryBlobTest64Kpage(ham_size_t cachesize=0, ham_size_t pagesize=64*1024,
            const char *name="InMemoryBlobTest64Kpage")
    : InMemoryBlobTest(cachesize, pagesize, name)
    {
    }
};


BFC_REGISTER_FIXTURE(FileBlobTest);
BFC_REGISTER_FIXTURE(FileBlobNoTxnTest);
BFC_REGISTER_FIXTURE(NoCacheBlobTest);
BFC_REGISTER_FIXTURE(NoCacheBlobNoTxnTest);
BFC_REGISTER_FIXTURE(InMemoryBlobTest);

/* re-run these tests with the Win32/Win64 pagesize setting as well! */
BFC_REGISTER_FIXTURE(FileBlobTest64Kpage);
BFC_REGISTER_FIXTURE(NoCacheBlobTest64Kpage);
BFC_REGISTER_FIXTURE(InMemoryBlobTest64Kpage);
