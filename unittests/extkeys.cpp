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
#include "../src/db.h"
#include "../src/env.h"
<<<<<<< HEAD
=======
#include "../src/extkeys.h"
#include "../src/mem.h"
#include "memtracker.h"
#include "os.hpp"
>>>>>>> flash-bang-grenade

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <cstring>


using namespace bfc;

class ExtendedKeyTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    ExtendedKeyTest()
    :   hamsterDB_fixture("ExtendedKeyTest"), m_db(NULL), m_env(NULL), m_alloc(NULL)
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(ExtendedKeyTest, insertFetchRemoveTest);
        BFC_REGISTER_TEST(ExtendedKeyTest, negativeFetchTest);
        BFC_REGISTER_TEST(ExtendedKeyTest, bigCacheTest);
        BFC_REGISTER_TEST(ExtendedKeyTest, purgeTest);
        BFC_REGISTER_TEST(ExtendedKeyTest, fullPurgeTest);
    }

protected:
    ham_db_t *m_db;
<<<<<<< HEAD
=======
    ham_env_t *m_env;
    mem_allocator_t *m_alloc;
>>>>>>> flash-bang-grenade

public:
    virtual void setup()
    {
        __super::setup();

<<<<<<< HEAD
=======
        ham_set_default_allocator_template(m_alloc = memtracker_new());
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0, ham_create(m_db, 0, HAM_IN_MEMORY_DB, 0));
        m_env = db_get_env(m_db);

        ExtKeyCache *c=new ExtKeyCache((Database *)m_db);
        BFC_ASSERT(c!=0);
<<<<<<< HEAD
        ((Database *)m_db)->set_extkey_cache(c);
    }
    
=======
        db_set_extkey_cache(m_db, c);   /* [i_a] keycache per db */
    }

>>>>>>> flash-bang-grenade
    virtual void teardown()
    {
        __super::teardown();

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
<<<<<<< HEAD
        ham_delete(m_db);
=======
        BFC_ASSERT_EQUAL(0, ham_delete(m_db));
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));
    }

    void keyStructureTest(void)
    {
        extkey_t e;

        BFC_ASSERT_EQUAL(SIZEOF_EXTKEY_T, sizeof(extkey_t)-1);

        extkey_set_blobid(&e, (ham_offset_t)0x12345);
        BFC_ASSERT_EQUAL((ham_offset_t)0x12345,
                extkey_get_blobid(&e));

        extkey_set_txn_id(&e, (ham_u64_t)0x12345678);
        BFC_ASSERT_EQUAL((ham_u64_t)0x12345678,
                extkey_get_txn_id(&e));

        extkey_set_next(&e, (extkey_t *)0x13);
        BFC_ASSERT_EQUAL((extkey_t *)0x13, extkey_get_next(&e));

        extkey_set_size(&e, 200);
        BFC_ASSERT_EQUAL((ham_size_t)200, extkey_get_size(&e));
    }

    void cacheStructureTest(void)
    {
        ham_size_t tmp;
        extkey_cache_t *c=db_get_extkey_cache(m_db);

        extkey_cache_set_db(c, m_db);
        BFC_ASSERT_EQUAL(m_db, extkey_cache_get_db(c));

        tmp=extkey_cache_get_usedsize(c);
        extkey_cache_set_usedsize(c, 1000);
        BFC_ASSERT_EQUAL((ham_size_t)1000, extkey_cache_get_usedsize(c));
        extkey_cache_set_usedsize(c, tmp);

        tmp=extkey_cache_get_bucketsize(c);
        extkey_cache_set_bucketsize(c, 500);
        BFC_ASSERT_EQUAL((ham_size_t)500, extkey_cache_get_bucketsize(c));
        extkey_cache_set_bucketsize(c, tmp);

        for (ham_size_t i=0; i<extkey_cache_get_bucketsize(c); i++) {
            extkey_t *e;

            e=extkey_cache_get_bucket(c, i);
            BFC_ASSERT_EQUAL((extkey_t *)0, e);

            extkey_cache_set_bucket(c, i, (extkey_t *)(i+1));
            e=extkey_cache_get_bucket(c, i);
            BFC_ASSERT_EQUAL((extkey_t *)(i+1), e);

            extkey_cache_set_bucket(c, i, 0);
        }
>>>>>>> flash-bang-grenade
    }

    void insertFetchRemoveTest(void)
    {
        ExtKeyCache *c=((Database *)m_db)->get_extkey_cache();
        ham_u8_t *pbuffer, buffer[12]={0};
        ham_size_t size;

<<<<<<< HEAD
        c->insert(0x123, sizeof(buffer), buffer);

        BFC_ASSERT_EQUAL(0, c->fetch(0x123, &size, &pbuffer));
        BFC_ASSERT_EQUAL((ham_size_t)12, size);
        BFC_ASSERT(::memcmp(pbuffer, buffer, size)==0);

        c->remove(0x123);
=======
        BFC_ASSERT_EQUAL(0,
                extkey_cache_insert(c, 0x123, sizeof(buffer), buffer, NULL));

        BFC_ASSERT_EQUAL(0,
                extkey_cache_fetch(c, 0x123, &size, &pbuffer));
        BFC_ASSERT_EQUAL((ham_size_t)12, size);
        BFC_ASSERT(::memcmp(pbuffer, buffer, size)==0);

        BFC_ASSERT_EQUAL(0,
                extkey_cache_remove(c, 0x123));
>>>>>>> flash-bang-grenade
    }

    void negativeFetchTest(void)
    {
        ExtKeyCache *c=((Database *)m_db)->get_extkey_cache();
        ham_u8_t *pbuffer, buffer[12]={0};
        ham_size_t size;

<<<<<<< HEAD
        c->insert(0x123, sizeof(buffer), buffer);
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND, c->fetch(0x321, &size, &pbuffer));

        BFC_ASSERT_EQUAL(0, c->fetch(0x123, &size, &pbuffer));
        BFC_ASSERT_EQUAL((ham_size_t)12, size);
        BFC_ASSERT(::memcmp(pbuffer, buffer, size)==0);

        c->remove(0x123);
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND, c->fetch(0x123, &size, &pbuffer));
=======
        BFC_ASSERT_EQUAL(0,
                extkey_cache_insert(c, 0x123, sizeof(buffer), buffer, NULL));

        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                extkey_cache_fetch(c, 0x1234, &size, &pbuffer));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                extkey_cache_fetch(c, 0x12345, &size, &pbuffer));

        BFC_ASSERT_EQUAL(0,
                extkey_cache_remove(c, 0x123));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                extkey_cache_fetch(c, 0x123, &size, &pbuffer));
    }

    void negativeRemoveTest(void)
    {
        extkey_cache_t *c=db_get_extkey_cache(m_db);

        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                extkey_cache_remove(c, 0x12345));
>>>>>>> flash-bang-grenade
    }

    void bigCacheTest(void)
    {
        ExtKeyCache *c=((Database *)m_db)->get_extkey_cache();
        ham_u8_t *pbuffer, buffer[12]={0};
        ham_size_t size;

<<<<<<< HEAD
        for (ham_size_t i=0; i<10000; i++) {
            c->insert((ham_offset_t)i, sizeof(buffer), buffer);
        }

        for (ham_size_t i=0; i<10000; i++) {
            BFC_ASSERT_EQUAL(0,
                c->fetch((ham_offset_t)i, &size, &pbuffer));
            BFC_ASSERT_EQUAL((ham_size_t)12, size);
        }

        for (ham_size_t i=0; i<10000; i++) {
             c->remove((ham_offset_t)i);
        }

        for (ham_size_t i=0; i<10000; i++) {
            BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                        c->fetch((ham_offset_t)i, 0, 0));
=======
        for (ham_size_t i=0; i<extkey_cache_get_bucketsize(c)*4; i++) {
            BFC_ASSERT_EQUAL(0,
                extkey_cache_insert(c, (ham_offset_t)i,
                    sizeof(buffer), buffer, NULL));
        }

        for (ham_size_t i=0; i<extkey_cache_get_bucketsize(c)*4; i++) {
            BFC_ASSERT_EQUAL(0,
                extkey_cache_fetch(c, (ham_offset_t)i,
                    &size, &pbuffer));
            BFC_ASSERT_EQUAL((ham_size_t)12, size);
        }

        for (ham_size_t i=0; i<extkey_cache_get_bucketsize(c)*4; i++) {
            BFC_ASSERT_EQUAL(0,
                extkey_cache_remove(c, (ham_offset_t)i));
>>>>>>> flash-bang-grenade
        }
    }

    /* not a real test any longer as extkey cache flushing has been made dependent on page flush */
    void purgeTest(void)
    {
        ExtKeyCache *c=((Database *)m_db)->get_extkey_cache();
        ham_u8_t *pbuffer, buffer[12]={0};
        ham_size_t size;

        for (int i=0; i<20; i++) {
<<<<<<< HEAD
            c->insert((ham_offset_t)i, sizeof(buffer), buffer);
        }

        ham_env_t *env=ham_get_env(m_db);
        Environment *e=(Environment *)env;
        e->set_txn_id(e->get_txn_id()+2000);

        c->purge();

        for (int i=0; i<20; i++) {
            BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                c->fetch((ham_offset_t)i, &size, &pbuffer));
=======
            BFC_ASSERT_EQUAL(0,
                extkey_cache_insert(c, (ham_offset_t)i,
                    sizeof(buffer), buffer, NULL));
        }

        env_set_txn_id(m_env, env_get_txn_id(m_env)+2000);

#if 0
        BFC_ASSERT_EQUAL(0, extkey_cache_purge(c));

        for (int i=0; i<20; i++) {
            BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                extkey_cache_fetch(c, (ham_offset_t)i,
                    &size, &pbuffer));
>>>>>>> flash-bang-grenade
        }
#else
        for (int i=0; i<20; i++) {
            BFC_ASSERT_EQUAL(HAM_SUCCESS,
                extkey_cache_fetch(c, (ham_offset_t)i,
                &size, &pbuffer));
        }
        for (int i=20; i<30; i++) {
            BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                extkey_cache_fetch(c, (ham_offset_t)i,
                &size, &pbuffer));
        }
#endif
    }

    void fullPurgeTest(void)
    {
        ham_status_t st;
        ham_key_t key;
        ham_record_t rec;
        int i;
        ham_txn_t *txn;
        ham_parameter_t ps[] = {
            {HAM_PARAM_KEYSIZE, 256},
            {HAM_PARAM_PAGESIZE, 64 * 1024},
            {0, 0},
        };

        ham_close(m_db, 0);

        os::unlink(BFC_OPATH(".test"));

        st = ham_create_ex(m_db, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS | HAM_CACHE_UNLIMITED, 0644, ps);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);

        st = ham_txn_begin(&txn, m_db, 0);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_NOTNULL(txn);

        for (i = 0; i < 100; i++)
        {
            ham_u64_t kv[100] = {i};
            ham_u64_t rv = i * 317;

            memset(&key, 0, sizeof(key));
            memset(&rec, 0, sizeof(rec));

            kv[99] = i;
            kv[99] *= 65531;

            key.data = &kv;
            key.size = sizeof(kv);
            rec.data = &rv;
            rec.size = sizeof(rv);

            st = ham_insert(m_db, txn, &key, &rec, HAM_HINT_APPEND);
            BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        }

        /* now that we are here, we know all extkeys are in cache, thanks to unlimited cache: */

        extkey_cache_t *c=db_get_extkey_cache(m_db);
        ham_u8_t *pbuffer;
        ham_size_t size;

        for (i=0; i<100; i++)
        {
            BFC_ASSERT_EQUAL(HAM_SUCCESS,
                extkey_cache_fetch(c, i, &size, &pbuffer));

        }
    }
};

BFC_REGISTER_FIXTURE(ExtendedKeyTest);

