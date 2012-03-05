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
#include "../src/btree.h"
#include "../src/btree_classic.h"
#include "../src/cache.h"
#include "../src/db.h"
#include "../src/device.h"
#include "../src/env.h"
#include "../src/freelist.h"
<<<<<<< HEAD
=======
#include "../src/log.h"
#include "../src/mem.h"
#include "../src/page.h"
#include "../src/txn.h"
#include "memtracker.h"
>>>>>>> flash-bang-grenade

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>

using namespace bfc;


class DbTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    DbTest(bool inmemory=false, const char *name="DbTest")
    :   hamsterDB_fixture(name),
<<<<<<< HEAD
        m_db(0), m_dbp(0), m_env(0), m_inmemory(inmemory)
=======
        m_db(0), m_env(0), m_inmemory(inmemory)
>>>>>>> flash-bang-grenade
    {
        //if (name)
        //    return;
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(DbTest, checkStructurePackingTest);
        BFC_REGISTER_TEST(DbTest, headerTest);
        BFC_REGISTER_TEST(DbTest, structureTest);
        BFC_REGISTER_TEST(DbTest, envStructureTest);
        BFC_REGISTER_TEST(DbTest, defaultCompareTest);
        BFC_REGISTER_TEST(DbTest, defaultPrefixCompareTest);
        BFC_REGISTER_TEST(DbTest, allocPageTest);
        BFC_REGISTER_TEST(DbTest, fetchPageTest);
        BFC_REGISTER_TEST(DbTest, fetchPageTest1);
        BFC_REGISTER_TEST(DbTest, fetchPageTest2);
        BFC_REGISTER_TEST(DbTest, flushPageTest);
        BFC_REGISTER_TEST(DbTest, checkDbRedimTest);
    }

protected:
    ham_db_t *m_db;
    Database *m_dbp;
    ham_env_t *m_env;
    mem_allocator_t *m_alloc;
    ham_bool_t m_inmemory;

public:
    virtual void setup()
    {
        __super::setup();

<<<<<<< HEAD
=======
        ham_set_default_allocator_template(m_alloc = memtracker_new());
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0,
                ham_env_create(m_env, BFC_OPATH(".test"),
                        (m_inmemory ? HAM_IN_MEMORY_DB : 0), 0644));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(m_env, m_db, 13,
                        HAM_ENABLE_DUPLICATES, 0));
        m_dbp=(Database *)m_db;
    }
<<<<<<< HEAD
    
=======

>>>>>>> flash-bang-grenade
    virtual void teardown()
    {
        __super::teardown();

        ham_env_close(m_env, 0);
        ham_close(m_db, 0);
        ham_delete(m_db);
        ham_env_delete(m_env);
<<<<<<< HEAD
=======
        BFC_ASSERT_EQUAL(memtracker_get_leaks(ham_get_default_allocator_template()), 0U);
>>>>>>> flash-bang-grenade
    }

    void headerTest()
    {
<<<<<<< HEAD
        ((Environment *)m_env)->set_magic('1', '2', '3', '4');
        BFC_ASSERT_EQUAL(true,
                ((Environment *)m_env)->compare_magic('1', '2', '3', '4'));

        ((Environment *)m_env)->set_version(1, 2, 3, 4);
        BFC_ASSERT_EQUAL((ham_u8_t)1, ((Environment *)m_env)->get_version(0));
        BFC_ASSERT_EQUAL((ham_u8_t)2, ((Environment *)m_env)->get_version(1));
        BFC_ASSERT_EQUAL((ham_u8_t)3, ((Environment *)m_env)->get_version(2));
        BFC_ASSERT_EQUAL((ham_u8_t)4, ((Environment *)m_env)->get_version(3));

        ((Environment *)m_env)->set_serialno(0x1234);
        BFC_ASSERT_EQUAL(0x1234u, ((Environment *)m_env)->get_serialno());
=======
        env_set_magic(m_env, '1', '2', '3', '4');
        BFC_ASSERT_EQUAL(env_get_magic(env_get_header(m_env), 0), '1');
        BFC_ASSERT_EQUAL(env_get_magic(env_get_header(m_env), 1), '2');
        BFC_ASSERT_EQUAL(env_get_magic(env_get_header(m_env), 2), '3');
        BFC_ASSERT_EQUAL(env_get_magic(env_get_header(m_env), 3), '4');

        env_set_version(m_env, 1, 2, 3, 4);
        BFC_ASSERT_EQUAL(env_get_version(m_env, 0), 1);
        BFC_ASSERT_EQUAL(env_get_version(m_env, 1), 2);
        BFC_ASSERT_EQUAL(env_get_version(m_env, 2), 3);
        BFC_ASSERT_EQUAL(env_get_version(m_env, 3), 4);

        env_set_serialno(m_env, 0x1234);
        BFC_ASSERT_EQUAL(env_get_serialno(m_env), 0x1234U);

        ham_size_t ps=env_get_pagesize(m_env); // since 1.1.0 cooked != persistent/raw necessarily
        ham_size_t raw_ps=env_get_persistent_pagesize(m_env);
        env_set_persistent_pagesize(m_env, 1024*32);
        BFC_ASSERT_EQUAL(env_get_pagesize(m_env), ps);
        BFC_ASSERT_EQUAL(env_get_persistent_pagesize(m_env), 1024*32U);
        env_set_pagesize(m_env, ps);
        env_set_persistent_pagesize(m_env, raw_ps);

        ham_size_t maxdbs=env_get_max_databases(m_env);
        ham_size_t fs_maxdbs=env_get_persistent_max_databases(m_env);
        env_set_persistent_max_databases(m_env, 56789U);
        BFC_ASSERT_EQUAL(env_get_max_databases(m_env), maxdbs);
        BFC_ASSERT_EQUAL(env_get_persistent_max_databases(m_env), 56789U);
        BFC_ASSERT(env_get_max_databases(m_env) > 0);
        env_set_max_databases(m_env, maxdbs);
        env_set_persistent_max_databases(m_env, fs_maxdbs);

        env_set_txn(m_env, (ham_txn_t *)13);
        BFC_ASSERT_EQUAL(env_get_txn(m_env), (ham_txn_t *)13);

        db_set_extkey_cache(m_db, (extkey_cache_t *)14);
        BFC_ASSERT_EQUAL(db_get_extkey_cache(m_db), (extkey_cache_t *)14);
        env_set_txn(m_env, (ham_txn_t *)0);
        db_set_extkey_cache(m_db, 0);
>>>>>>> flash-bang-grenade
    }

    void structureTest()
    {
<<<<<<< HEAD
        BFC_ASSERT(((Environment *)m_env)->get_header_page()!=0);

        BFC_ASSERT_EQUAL(0, m_dbp->get_error());
        m_dbp->set_error(HAM_IO_ERROR);
        BFC_ASSERT_EQUAL(HAM_IO_ERROR, m_dbp->get_error());

        BFC_ASSERT_NOTNULL(m_dbp->get_backend());// already initialized
        ham_backend_t *oldbe=m_dbp->get_backend();
        m_dbp->set_backend((ham_backend_t *)15);
        BFC_ASSERT_EQUAL((ham_backend_t *)15, m_dbp->get_backend());
        m_dbp->set_backend(oldbe);

        BFC_ASSERT_NOTNULL(((Environment *)m_env)->get_cache());

        BFC_ASSERT(0!=m_dbp->get_prefix_compare_func());
        ham_prefix_compare_func_t oldfoo=m_dbp->get_prefix_compare_func();
        m_dbp->set_prefix_compare_func((ham_prefix_compare_func_t)18);
        BFC_ASSERT_EQUAL((ham_prefix_compare_func_t)18,
                m_dbp->get_prefix_compare_func());
        m_dbp->set_prefix_compare_func(oldfoo);

        ham_compare_func_t oldfoo2=m_dbp->get_compare_func();
        BFC_ASSERT(0!=m_dbp->get_compare_func());
        m_dbp->set_compare_func((ham_compare_func_t)19);
        BFC_ASSERT_EQUAL((ham_compare_func_t)19, m_dbp->get_compare_func());
        m_dbp->set_compare_func(oldfoo2);

        ((Environment *)m_env)->get_header_page()->set_dirty(false);
        BFC_ASSERT(!((Environment *)m_env)->is_dirty());
        ((Environment *)m_env)->set_dirty(true);
        BFC_ASSERT(((Environment *)m_env)->is_dirty());

        BFC_ASSERT(0!=m_dbp->get_rt_flags());

        BFC_ASSERT(m_dbp->get_env()!=0);

        BFC_ASSERT_EQUAL((void *)0, m_dbp->get_next());
        m_dbp->set_next((Database *)40);
        BFC_ASSERT_EQUAL((Database *)40, m_dbp->get_next());
        m_dbp->set_next((Database *)0);

        BFC_ASSERT_EQUAL(0u, m_dbp->get_record_allocsize());
        m_dbp->set_record_allocsize(21);
        BFC_ASSERT_EQUAL(21u, m_dbp->get_record_allocsize());
        m_dbp->set_record_allocsize(0);

        BFC_ASSERT_EQUAL((void *)0, m_dbp->get_record_allocdata());
        m_dbp->set_record_allocdata((void *)22);
        BFC_ASSERT_EQUAL((void *)22, m_dbp->get_record_allocdata());
        m_dbp->set_record_allocdata(0);

        BFC_ASSERT_EQUAL(1u, m_dbp->is_active());
        m_dbp->set_active(HAM_FALSE);
        BFC_ASSERT_EQUAL(0u, m_dbp->is_active());
        m_dbp->set_active(HAM_TRUE);
        BFC_ASSERT_EQUAL(1u, m_dbp->is_active());
=======
        BFC_ASSERT(env_get_header_page(m_env)!=0);

#if 0 // [i_a] db_set_error/db_get_error are gone.
        BFC_ASSERT_EQUAL(0, db_get_error(m_db));
        db_set_error(m_db, HAM_IO_ERROR);
        BFC_ASSERT_EQUAL(HAM_IO_ERROR, db_get_error(m_db));
#endif

        BFC_ASSERT_NOTNULL(db_get_backend(m_db));// already initialized
        ham_backend_t *oldbe=db_get_backend(m_db);
        db_set_backend(m_db, (ham_backend_t *)15);
        BFC_ASSERT_EQUAL((ham_backend_t *)15, db_get_backend(m_db));
        db_set_backend(m_db, oldbe);

        BFC_ASSERT_NOTNULL(env_get_cache(m_env));
        ham_cache_t * oldca = env_get_cache(m_env);
        env_set_cache(m_env, (ham_cache_t *)16);
        BFC_ASSERT(env_get_cache(m_env)==(ham_cache_t *)16);
        env_set_cache(m_env, oldca);

        BFC_ASSERT_NOTNULL(db_get_prefix_compare_func(m_db));
        ham_prefix_compare_func_t oldfoo=db_get_prefix_compare_func(m_db);
        db_set_prefix_compare_func(m_db, (ham_prefix_compare_func_t)18);
        BFC_ASSERT_EQUAL((ham_prefix_compare_func_t)18,
                db_get_prefix_compare_func(m_db));
        db_set_prefix_compare_func(m_db, oldfoo);

        ham_compare_func_t oldfoo2=db_get_compare_func(m_db);
        BFC_ASSERT_NOTNULL(db_get_compare_func(m_db));
        db_set_compare_func(m_db, (ham_compare_func_t)19);
        BFC_ASSERT_EQUAL((ham_compare_func_t)19, db_get_compare_func(m_db));
        db_set_compare_func(m_db, oldfoo2);

        BFC_ASSERT(env_is_dirty(m_env));
        page_set_undirty(env_get_header_page(m_env));
        BFC_ASSERT(!env_is_dirty(m_env));
        env_set_dirty(m_env);
        BFC_ASSERT(env_is_dirty(m_env));

        BFC_ASSERT(0!=db_get_rt_flags(m_db));

        BFC_ASSERT_NOTNULL(m_env);
        BFC_ASSERT_NOTNULL(db_get_env(m_db));

        BFC_ASSERT_EQUAL((void *)0, db_get_next(m_db));
        db_set_next(m_db, (ham_db_t *)40);
        BFC_ASSERT_EQUAL((ham_db_t *)40, db_get_next(m_db));
        db_set_next(m_db, (ham_db_t *)0);

        BFC_ASSERT_EQUAL(0u, db_get_record_allocsize(m_db));
        db_set_record_allocsize(m_db, 21);
        BFC_ASSERT_EQUAL(21u, db_get_record_allocsize(m_db));
        db_set_record_allocsize(m_db, 0);

        BFC_ASSERT_EQUAL((void *)0, db_get_record_allocdata(m_db));
        db_set_record_allocdata(m_db, (void *)22);
        BFC_ASSERT_EQUAL((void *)22, db_get_record_allocdata(m_db));
        db_set_record_allocdata(m_db, 0);

        BFC_ASSERT(db_is_active(m_db));
        db_set_active(m_db, HAM_FALSE);
        BFC_ASSERT(!db_is_active(m_db));
        db_set_active(m_db, HAM_TRUE);
        BFC_ASSERT(db_is_active(m_db));
>>>>>>> flash-bang-grenade
    }

    void envStructureTest()
    {
        ham_env_t *env;

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
<<<<<<< HEAD
        ((Environment *)env)->set_txn_id(0x12345ull);
        ((Environment *)env)->set_file_mode(0666);
        ((Environment *)env)->set_device((Device *)0x13);
        ((Environment *)env)->set_cache((Cache *)0x14);
        ((Environment *)env)->set_flags(0x18);

        BFC_ASSERT_EQUAL((Cache *)0x14, ((Environment *)env)->get_cache());
        /* TODO test other stuff! */

        BFC_ASSERT_EQUAL(0u, ((Environment *)env)->is_active());
        ((Environment *)env)->set_active(true);
        BFC_ASSERT_EQUAL(1u, ((Environment *)env)->is_active());
        ((Environment *)env)->set_active(false);
        BFC_ASSERT_EQUAL(0u, ((Environment *)env)->is_active());

        ((Environment *)env)->set_device((Device *)0x00);
        ((Environment *)env)->set_cache((Cache *)0x00);
        ((Environment *)env)->set_flags(0);
        ((Environment *)env)->set_databases(0);
        ((Environment *)env)->set_header_page(0);
=======
        env_set_txn_id(env, 0x12345ull);
        env_set_file_mode(env, 0666);
        env_set_device(env, (ham_device_t *)0x13);
        env_set_cache(env, (ham_cache_t *)0x14);
        env_set_txn(env, (ham_txn_t *)0x16);
        //env_set_extkey_cache(env, (extkey_cache_t *)0x17);
        env_set_rt_flags(env, 0x18);

        BFC_ASSERT_EQUAL((ham_cache_t *)0x14, env_get_cache(env));
        /* TODO test other stuff! */

        BFC_ASSERT(!env_is_active(env));
        env_set_active(env, HAM_TRUE);
        BFC_ASSERT(env_is_active(env));
        env_set_active(env, HAM_FALSE);
        BFC_ASSERT(!env_is_active(env));

        env_set_device(env, 0);
        env_set_cache(env, 0);
        env_set_txn(env, 0);
        //env_set_extkey_cache(env, 0);
        env_set_rt_flags(env, 0x18);
        env_set_header_page(env, 0);
        env_set_list(env, 0);
        env_set_header_page(env, (ham_page_t *)0);
>>>>>>> flash-bang-grenade
        ham_env_delete(env);
    }

    void defaultCompareTest()
    {
        BFC_ASSERT( 0==db_default_compare(0,
                        (ham_u8_t *)"abc", 3, (ham_u8_t *)"abc", 3));
        BFC_ASSERT(-1==db_default_compare(0,
                        (ham_u8_t *)"ab",  2, (ham_u8_t *)"abc", 3));
        BFC_ASSERT(-1==db_default_compare(0,
                        (ham_u8_t *)"abc", 3, (ham_u8_t *)"bcd", 3));
        BFC_ASSERT(+1==db_default_compare(0,
                        (ham_u8_t *)"abc", 3, (ham_u8_t *)0,     0));
        BFC_ASSERT(-1==db_default_compare(0,
                        (ham_u8_t *)0,     0, (ham_u8_t *)"abc", 3));
    }

    void defaultPrefixCompareTest()
    {
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)"abc", 3, 3,
                        (ham_u8_t *)"abc", 3, 3)==HAM_PREFIX_REQUEST_FULLKEY);
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)"ab",  2, 2,
                        (ham_u8_t *)"abc", 3, 3)==-1); // comparison code has become 'smarter' so can resolve this one without the need for further help
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)"ab",  2, 3,
                        (ham_u8_t *)"abc", 3, 3)==HAM_PREFIX_REQUEST_FULLKEY);
        BFC_ASSERT(-1==db_default_prefix_compare(0,
                        (ham_u8_t *)"abc", 3, 3,
                        (ham_u8_t *)"bcd", 3, 3));
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)"abc", 3, 3,
                        (ham_u8_t *)0,     0, 0)==+1); // comparison code has become 'smarter' so can resolve this one without the need for further help
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)0,     0, 0,
                        (ham_u8_t *)"abc", 3, 3)==-1); // comparison code has become 'smarter' so can resolve this one without the need for further help
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)"abc", 3, 3,
                        (ham_u8_t *)0,     0, 3)==HAM_PREFIX_REQUEST_FULLKEY);
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)0,     0, 3,
                        (ham_u8_t *)"abc", 3, 3)==HAM_PREFIX_REQUEST_FULLKEY);
        BFC_ASSERT(db_default_prefix_compare(0,
                        (ham_u8_t *)"abc", 3, 80239,
                        (ham_u8_t *)"abc", 3, 2)==HAM_PREFIX_REQUEST_FULLKEY);
    }

    void allocPageTest(void)
    {
<<<<<<< HEAD
        Page *page;
        BFC_ASSERT_EQUAL(0,
                db_alloc_page(&page, m_dbp, 0, PAGE_IGNORE_FREELIST));
        BFC_ASSERT_EQUAL(m_dbp, page->get_db());
        BFC_ASSERT_EQUAL(0, db_free_page(page, 0));
=======
        ham_page_t *page;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        ham_status_t st = db_alloc_page(&page, PAGE_IGNORE_FREELIST, &info);

        BFC_ASSERT_NOTNULL(page);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(page_get_owner(page), m_db);
        BFC_ASSERT_EQUAL(db_free_page(page, 0), HAM_SUCCESS);
>>>>>>> flash-bang-grenade
    }

    void fetchPageTest(void)
    {
<<<<<<< HEAD
        Page *p1, *p2;
        BFC_ASSERT_EQUAL(0,
                db_alloc_page(&p1, m_dbp, 0, PAGE_IGNORE_FREELIST));
        BFC_ASSERT_EQUAL(m_dbp, p1->get_db());
        BFC_ASSERT_EQUAL(0,
                db_fetch_page(&p2, m_dbp, p1->get_self(), 0));
        BFC_ASSERT_EQUAL(p2->get_self(), p1->get_self());
=======
        ham_page_t *p1, *p2;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0,
                db_alloc_page(&p1, PAGE_IGNORE_FREELIST, &info));
        BFC_ASSERT_EQUAL(m_db, page_get_owner(p1));
        BFC_ASSERT_EQUAL(0,
                db_fetch_page(&p2, m_env, page_get_self(p1), 0));
        BFC_ASSERT_EQUAL(page_get_self(p2), page_get_self(p1));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, db_free_page(p1, 0));
    }

    void fetchPageTest1(void)
    {
        ham_status_t st;
        ham_page_t *p1, *p2;
        ham_page_t *p3, *p4;
        ham_offset_t a1, a3;
        ham_cache_t *cache=cache_new(m_env, 1);
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT(cache!=0);
        env_set_cache(m_env, cache);

        /* new code will b0rk with an assert when there's no live cache: */
        st = db_alloc_page(&p1, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(p1);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1U);
        BFC_ASSERT_EQUAL(page_get_owner(p1), m_db);
        a1 = page_get_self(p1);

        st = db_alloc_page(&p3, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(p3);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        /* right now, p1 is invalid as it was ditched from the cache */
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 2U : 1U));
        BFC_ASSERT_EQUAL(page_get_owner(p3), m_db);
        a3 = page_get_self(p3);
        BFC_ASSERT_CMP(a3, >, a1);

        st = db_fetch_page(&p2, m_env, a1, 0);
        BFC_ASSERT_NOTNULL(p2);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 2U : 1U));
        BFC_ASSERT_EQUAL(page_get_self(p2), a1);
        BFC_ASSERT_EQUAL(db_free_page(p2, 0), HAM_SUCCESS);

        st = db_fetch_page(&p4, m_env, a3, 0);
        BFC_ASSERT_NOTNULL(p4);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1U);
        BFC_ASSERT_EQUAL(page_get_self(p4), a3);
        BFC_ASSERT_EQUAL(db_free_page(p4, 0), HAM_SUCCESS);
    }

    void fetchPageTest2(void)
    {
        ham_status_t st;
        ham_page_t *p1, *p2;
        ham_page_t *p3, *p4;
        ham_offset_t a1, a3;
        ham_cache_t *cache=cache_new(m_env, 2);
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT(cache!=0);
        env_set_cache(m_env, cache);

        /* new code will b0rk with an assert when there's no live cache: */
        st = db_alloc_page(&p1, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(p1);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1U);
        BFC_ASSERT_EQUAL(page_get_owner(p1), m_db);
        a1 = page_get_self(p1);

        st = db_alloc_page(&p3, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(p3);
        /* right now, p1 is invalid as it was ditched from the cache */
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 2U);
        BFC_ASSERT_EQUAL(page_get_owner(p3), m_db);
        a3 = page_get_self(p3);

        //if (!m_inmemory)
        {
            st = db_fetch_page(&p2, m_env, a1, 0);
            BFC_ASSERT_NOTNULL(p2);
            st = db_fetch_page(&p4, m_env, a3, 0);
            BFC_ASSERT_NOTNULL(p4);

            BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 2U);

            BFC_ASSERT_EQUAL(page_get_self(p2), a1);
            BFC_ASSERT_EQUAL(page_get_self(p4), a3);
            BFC_ASSERT_EQUAL(db_free_page(p2, 0), HAM_SUCCESS);
            BFC_ASSERT_EQUAL(db_free_page(p4, 0), HAM_SUCCESS);
        }
    }

    void flushPageTest(void)
    {
<<<<<<< HEAD
        Page *page;
=======
        ham_status_t st;
        ham_page_t *page;
>>>>>>> flash-bang-grenade
        ham_offset_t address;
        ham_u8_t *p;
        dev_alloc_request_info_ex_t info = {0};

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0,
                db_alloc_page(&page, m_dbp, 0, PAGE_IGNORE_FREELIST));

        BFC_ASSERT(page->get_db()==m_dbp);
        p=page->get_raw_payload();
        for (int i=0; i<16; i++)
            p[i]=(ham_u8_t)i;
        page->set_dirty(true);
        address=page->get_self();
        BFC_ASSERT_EQUAL(0, db_flush_page((Environment *)m_env, page));
        BFC_ASSERT_EQUAL(0, db_free_page(page, 0));

        BFC_ASSERT_EQUAL(0, db_fetch_page(&page, m_dbp, address, 0));
        BFC_ASSERT(page!=0);
        BFC_ASSERT_EQUAL(address, page->get_self());
        p=page->get_raw_payload();
        /* TODO see comment in db.c - db_free_page()
=======
        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT_NOTNULL(cache);
        env_set_cache(m_env, cache);

        st = db_alloc_page(&page, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(page);
        BFC_ASSERT_EQUAL(page_get_owner(page), m_db);
        p=page_get_raw_payload(page);
        for (int i=0; i<16; i++)
            p[i]=(ham_u8_t)i;
        page_set_dirty(page, m_env);
        address=page_get_self(page);
        BFC_ASSERT_EQUAL(db_flush_page(m_env, page, 0), HAM_SUCCESS);
        if (!m_inmemory)
        {
            /*
            calling this will blow away the page without it
            getting flushed to disc as db_flush_page() will
            merely have stored the page in the cache for
            DELAYED flushing...
            */
            BFC_ASSERT_EQUAL(db_free_page(page, 0), HAM_SUCCESS);

            st = db_fetch_page(&page, m_env, address, 0);
            BFC_ASSERT_NOTNULL(page);
            BFC_ASSERT_EQUAL(address, page_get_self(page));
            p=page_get_raw_payload(page);
            for (int i=0; i<16; i++)
                BFC_ASSERT_EQUAL_I(p[i], 0, i);
            BFC_ASSERT_EQUAL(db_free_page(page, 0), HAM_SUCCESS);

            /* try again; this time FORCE a flush: HAM_WRITE_THROUGH */
            st = db_alloc_page(&page, PAGE_IGNORE_FREELIST, &info);
            BFC_ASSERT_NOTNULL(page);
            BFC_ASSERT_EQUAL(page_get_owner(page), m_db);
            p=page_get_raw_payload(page);
            for (int i=0; i<16; i++)
                p[i]=(ham_u8_t)i;
            page_set_dirty(page, m_env);
            address=page_get_self(page);
            BFC_ASSERT_EQUAL(db_flush_page(m_env, page, HAM_WRITE_THROUGH), HAM_SUCCESS);
            BFC_ASSERT_EQUAL(db_free_page(page, 0), HAM_SUCCESS);
        }

        st = db_fetch_page(&page, m_env, address, 0);
        BFC_ASSERT_NOTNULL(page);
        BFC_ASSERT_EQUAL(page_get_self(page), address);
        p=page_get_raw_payload(page);
>>>>>>> flash-bang-grenade
        for (int i=0; i<16; i++)
            BFC_ASSERT_EQUAL_I(p[i], i, i);
        BFC_ASSERT_EQUAL(db_free_page(page, 0), HAM_SUCCESS);
    }

    void checkStructurePackingTest(void)
    {
        int i;

        // checks to make sure structure packing by the compiler is still okay
        // HAM_PACK_0 HAM_PACK_1 HAM_PACK_2 OFFSETOF
<<<<<<< HEAD
        BFC_ASSERT(compare_sizes(sizeof(ham_backend_t),
                OFFSETOF(ham_btree_t, _rootpage)));
        BFC_ASSERT(compare_sizes(sizeof(blob_t), 28));
        BFC_ASSERT(compare_sizes(sizeof(dupe_entry_t), 16));
        BFC_ASSERT(compare_sizes(sizeof(dupe_table_t),
                8 + sizeof(dupe_entry_t)));
        BFC_ASSERT(compare_sizes(sizeof(ham_btree_t) - OFFSETOF(ham_btree_t,
                _rootpage), 8 + sizeof(void *)*2 + 2));
        BFC_ASSERT(compare_sizes(sizeof(btree_node_t), 28+sizeof(btree_key_t)));
        BFC_ASSERT(compare_sizes(sizeof(btree_key_t), 12));
        BFC_ASSERT(compare_sizes(sizeof(env_header_t), 20));
        BFC_ASSERT(compare_sizes(sizeof(db_indexdata_t), 32));
        BFC_ASSERT(compare_sizes(DB_INDEX_SIZE, 32));
        BFC_ASSERT(compare_sizes(sizeof(freelist_payload_t),
                16 + 13 + sizeof(freelist_page_statistics_t)));
        freelist_payload_t f;
        BFC_ASSERT(compare_sizes(sizeof(f._s._s16), 5));
        BFC_ASSERT(compare_sizes(OFFSETOF(freelist_payload_t,
                _s._s16), 16));
        BFC_ASSERT(compare_sizes(OFFSETOF(freelist_payload_t,
                _s._s16._bitmap), 16 + 4));
        BFC_ASSERT(compare_sizes(sizeof(freelist_page_statistics_t),
              4*8+sizeof(freelist_slotsize_stats_t)*HAM_FREELIST_SLOT_SPREAD));
        BFC_ASSERT(compare_sizes(sizeof(freelist_slotsize_stats_t), 8*4));
        BFC_ASSERT(compare_sizes(HAM_FREELIST_SLOT_SPREAD, 16-5+1));
        BFC_ASSERT(compare_sizes(db_get_freelist_header_size16(), 16 + 4));
        BFC_ASSERT(compare_sizes(db_get_freelist_header_size32(),
                16 + 12 + sizeof(freelist_page_statistics_t)));
        BFC_ASSERT(compare_sizes(db_get_int_key_header_size(), 11));
        BFC_ASSERT(compare_sizes(sizeof(Log::Header), 16));
        BFC_ASSERT(compare_sizes(sizeof(Log::Entry), 32));
        BFC_ASSERT(compare_sizes(sizeof(page_data_t), 13));
        page_data_t p;
        BFC_ASSERT(compare_sizes(sizeof(p._s), 13));
        BFC_ASSERT(compare_sizes(Page::sizeof_persistent_header, 12));

        BFC_ASSERT(compare_sizes(OFFSETOF(btree_node_t, _entries), 28));
        Page page;
        Database db;
        ham_backend_t be = {0};

        page.set_self(1000);
        page.set_db(&db);
        db.set_backend(&be);
        be_set_keysize(&be, 666);
        for (i = 0; i < 5; i++) {
            BFC_ASSERT_I(compare_sizes(
                (ham_size_t)btree_node_get_key_offset(&page, i),
                (ham_size_t)1000+12+28+(i*(11+666))), i);
        }
        BFC_ASSERT(compare_sizes(Page::sizeof_persistent_header, 12));
        // make sure the 'header page' is at least as large as your usual
        // header page, then hack it...
=======
        BFC_ASSERT_EQUAL(sizeof(ham_backend_t), OFFSETOF(ham_btree_t, _rootpage));
        BFC_ASSERT_EQUAL(sizeof(blob_t), 28U);
        BFC_ASSERT_EQUAL(sizeof(dupe_entry_t), 16U);
        BFC_ASSERT_EQUAL(sizeof(dupe_table_t), 8 + sizeof(dupe_entry_t));
#if 0
        BFC_ASSERT_EQUAL(sizeof(ham_btree_t) - OFFSETOF(ham_btree_t, _rootpage), 8U + 2);
#endif
        BFC_ASSERT_EQUAL(sizeof(btree_node_t), 28 + sizeof(int_key_t));
        BFC_ASSERT_EQUAL(sizeof(int_key_t), 12U);
        BFC_ASSERT_EQUAL(sizeof(env_header_t), 20U);
        BFC_ASSERT_EQUAL(sizeof(db_indexdata_t), 32U);
        db_indexdata_t d;
        BFC_ASSERT_EQUAL(sizeof(d.b), 32U);
        BFC_ASSERT_EQUAL(DB_INDEX_SIZE, 32U);
        BFC_ASSERT_EQUAL(sizeof(freelist_payload_t), 16 + 12 + sizeof(ham_pers_freelist_page_statistics_t) + sizeof(ham_pers_multitype_t));
        freelist_payload_t f;
        BFC_ASSERT_EQUAL(sizeof(f._s._s16), 12U);
        BFC_ASSERT_EQUAL(sizeof(f._s._s16._bitmap), 12U - 4);
        BFC_ASSERT_EQUAL(OFFSETOF(freelist_payload_t, _s._s16), 16U);
        BFC_ASSERT_EQUAL(OFFSETOF(freelist_payload_t, _s._s16._bitmap), 16U + 4);
        BFC_ASSERT_EQUAL(sizeof(ham_pers_freelist_page_statistics_t), 4*8 + sizeof(ham_pers_freelist_slotsize_stats_t)*HAM_FREELIST_SLOT_SPREAD);
        BFC_ASSERT_EQUAL(sizeof(ham_pers_freelist_slotsize_stats_t), 8*4U);
        BFC_ASSERT_EQUAL(HAM_FREELIST_SLOT_SPREAD, 16-5+1);
        BFC_ASSERT_EQUAL(db_get_freelist_header_size16(), 16 + 4U);
        BFC_ASSERT_EQUAL(db_get_freelist_header_size32(), 16 + 12 + sizeof(ham_pers_freelist_page_statistics_t));
        BFC_ASSERT_EQUAL(db_get_int_key_header_size(), 11U);
        BFC_ASSERT_EQUAL(sizeof(log_header_t), 8U);
        BFC_ASSERT_EQUAL(sizeof(log_entry_t), 40U);
        BFC_ASSERT_EQUAL(sizeof(ham_perm_page_union_t), 20U);
        ham_perm_page_union_t p;
        BFC_ASSERT_EQUAL(sizeof(p._s), 20U);
        BFC_ASSERT_EQUAL(sizeof(p._s._payload), 8U);
        BFC_ASSERT_EQUAL(page_get_persistent_header_size(), 12U);

        BFC_ASSERT_EQUAL(OFFSETOF(btree_node_t, _entries), 28U);
        ham_page_t page = {{0}};
        ham_db_t db = {0};
        ham_env_t env = {0};
        union {
            ham_btree_t btree;
            ham_backend_t backend;
        } be = {{0}};

        db_set_env(&db, &env);
        page_set_self(&page, 1000);
        page_set_owner(&page, &db);
        db_set_backend(&db, &be.backend);
        be_set_keysize(&be.backend, 666);
        btree_set_maxkeys(&be.btree, 999);
        BFC_ASSERT_CMP(sizeof(be.btree), >, sizeof(be.backend));
        BFC_ASSERT_EQUAL(&be.btree._fun_uncouple_all_cursors, &be.backend._fun_uncouple_all_cursors);
        BFC_ASSERT_EQUAL(&be.btree._keysize, &be.backend._keysize);
        BFC_ASSERT_EQUAL(&be.btree._flags, &be.backend._flags);
        for (i = 0; i < 5; i++)
        {
            //btree_node_t *node = ham_page_get_btree_node(&page);
            //ham_size_t keywidth = be_get_keysize(&be.backend) + db_get_int_key_header_size();
            ham_bool_t has_fast_index = HAM_FALSE;
            common_btree_datums_t btdata =
            {
                &be.btree,
                m_db,
                m_env,
                env_get_device(m_env),
                NULL,
                NULL,
                NULL,
                0,
                be_get_keysize(&be.btree),
                btree_get_maxkeys(&be.btree),
                be_get_keysize(&be.btree) + db_get_int_key_header_size(),
                has_fast_index,
                0,
                OFFSETOF(btree_node_t, _entries),
                OFFSETOF(btree_node_t, _entries)
                + (has_fast_index
                ? btree_get_maxkeys(&be.btree) * sizeof(ham_u16_t)
                : 0),
                {0, 0, NULL, 0, NULL, -1, HAM_FALSE, HAM_FALSE, HAM_FALSE, HAM_FALSE},
                MK_HAM_FLOAT(0.5),
                MK_HAM_FLOAT(0.33) // i.e. 1/3
            };
            int_key_t *key = btree_in_node_get_key_ref(&btdata, &page, (ham_u16_t)i);

            BFC_ASSERT_EQUAL_I(
                (ham_size_t)(((ham_u8_t *)key) - (ham_u8_t *)(&page)),
                (ham_size_t)1000+12+28+(i*(11+666)), i);
        }
        BFC_ASSERT_EQUAL(page_get_persistent_header_size(), 12U);
        // make sure the 'header page' is at least as large as your usual header page,
        // then hack it...
>>>>>>> flash-bang-grenade
        struct
        {
            page_data_t drit;
            env_header_t drat;
        } hdrpage_pers = {{{0}}};
<<<<<<< HEAD
        Page hdrpage;
        hdrpage.set_pers((page_data_t *)&hdrpage_pers);
        Page *hp = &hdrpage;
        ham_u8_t *pl1 = hp->get_payload();
        BFC_ASSERT(pl1);
        BFC_ASSERT(compare_sizes(pl1 - (ham_u8_t *)hdrpage.get_pers(), 12));
        env_header_t *hdrptr = (env_header_t *)(hdrpage.get_payload());
        BFC_ASSERT(compare_sizes(((ham_u8_t *)hdrptr) - (ham_u8_t *)hdrpage.get_pers(), 12));
        BFC_ASSERT(compare_sizes(DB_INDEX_SIZE, 32));
        hdrpage.set_pers(0);
=======
        ham_page_t hdrpage = {{0}};
        hdrpage._pers = (ham_perm_page_union_t *)&hdrpage_pers;
        env_set_header_page(db_get_env(&db), &hdrpage);
        ham_page_t *hp = env_get_header_page(db_get_env(&db));
        BFC_ASSERT(hp == (ham_page_t *)&hdrpage);
        ham_u8_t *pl1 = (ham_u8_t *)page_get_payload(hp);
        BFC_ASSERT_NOTNULL(pl1);
        BFC_ASSERT_EQUAL(pl1 - (ham_u8_t *)hdrpage._pers, 12);
        env_header_t *hdrptr = env_get_header(db_get_env(&db));
        BFC_ASSERT_EQUAL(((ham_u8_t *)hdrptr) - (ham_u8_t *)hdrpage._pers, 12);
        env_set_max_databases(db_get_env(&db), 71);
        BFC_ASSERT_EQUAL(SIZEOF_FULL_HEADER(&env), 20 + 71 * DB_INDEX_SIZE);
        BFC_ASSERT_EQUAL(DB_INDEX_SIZE, 32U);
>>>>>>> flash-bang-grenade
    }

    void checkDbRedimTest(void)
    {
        ham_status_t st;
        ham_page_t *page;
        ham_page_t *page2;
        ham_offset_t address;
        ham_offset_t address2;
        ham_u8_t *p;
        int i;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        /* it is important to test with cache size = 1 to detect possible page usage faults */
        ham_cache_t *cache=cache_new(m_env, 1);
        BFC_ASSERT_NOTNULL(cache);
        env_set_cache(m_env, cache);

        BFC_ASSERT_EQUAL(0, env_reserve_space(512, &info));
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 0U : 1U));

        st=db_alloc_page(&page, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(page);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1U);
        BFC_ASSERT_EQUAL(page_get_owner(page), m_db);

        ham_size_t ps=env_get_pagesize(m_env) * 512; // since 1.1.0 cooked != persistent/raw necessarily
        address=page_get_self(page);
        if (!m_inmemory)
            BFC_ASSERT_CMP(address, >, ps);

        p=page_get_raw_payload(page);
        for (i=0; i<16; i++)
            p[i]=(ham_u8_t)i;
        page_set_dirty(page, m_env);

        /* make sure original edit is caught by subsequent page fetches of same */
        st=db_fetch_page(&page2, m_env, address, DB_ONLY_FROM_CACHE);
        BFC_ASSERT_NOTNULL(page2);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1U);
        BFC_ASSERT_EQUAL(page, page2);

        /*
        blow out the 1-page cache at the same time as checking
        whether the redim/prealloc op above properly marked the entire range
        FREE: this page alloc should deliver space from WITHIN the
        predim'ed space range.
        */
        for (i = 0; i < 256; i++)
        {
            st=db_alloc_page(&page2, 0, 0);
            BFC_ASSERT_NOTNULL_I(page2, i);
            BFC_ASSERT_EQUAL_I(cache_get_cur_elements(cache), (m_inmemory ? 2U + i : (i >= 252 ? 2U : 1U)), i);
            BFC_ASSERT_EQUAL_I(page_get_owner(page2), m_db, i);
            address2=page_get_self(page2);
            if (!m_inmemory)
                BFC_ASSERT_CMP_I(address2, <, ps, i);
        }

        st=db_alloc_page(&page2, PAGE_IGNORE_FREELIST, &info);
        BFC_ASSERT_NOTNULL(page2);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 2U+256 : 1U));
        if (!m_inmemory)
        {
            st=db_fetch_page(&page2, m_env, address, DB_ONLY_FROM_CACHE);
            BFC_ASSERT_NULL(page2);
            BFC_ASSERT_EQUAL(st, HAM_INTERNAL_ERROR);
        }
        else
        {
            st=db_fetch_page(&page2, m_env, address, DB_ONLY_FROM_CACHE);
            BFC_ASSERT_NOTNULL(page2);
            BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        }
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 2U+256 : 1U));
        st=db_fetch_page(&page2, m_env, address, 0);
        BFC_ASSERT_NOTNULL(page2);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 2U+256 : 1U));
        BFC_ASSERT_EQUAL(page_get_owner(page2), m_db);
        p=page_get_raw_payload(page2);
        for (i=0; i<16; i++)
        {
            BFC_ASSERT_EQUAL_I(p[i], i, i);
        }
        BFC_ASSERT_EQUAL(db_free_page(page2, 0), 0);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), (m_inmemory ? 2U+256-1 : 0U));
    }
};

class DbInMemoryTest : public DbTest
{
public:
    DbInMemoryTest()
    :   DbTest(true, "DbInMemoryTest")
    {
    }
};

BFC_REGISTER_FIXTURE(DbTest);
BFC_REGISTER_FIXTURE(DbInMemoryTest);
