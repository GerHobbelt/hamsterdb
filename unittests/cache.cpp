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
#include "../src/mem.h"
#include "../src/page.h"
#include "../src/cache.h"
#include "../src/error.h"
#include "../src/device.h"
#include "../src/os.h"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <vector>


using namespace bfc;

#ifndef cache_too_big
#define cache_too_big(c)                                                      \
    (cache_get_cur_elements(c) > cache_get_max_elements(c))
#endif

class CacheTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    CacheTest()
    : hamsterDB_fixture("CacheTest"), m_db(NULL), m_env(NULL), m_alloc(NULL)
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(CacheTest, newDeleteTest);
        BFC_REGISTER_TEST(CacheTest, putGetTest);
        BFC_REGISTER_TEST(CacheTest, putGetRemoveGetTest);
        BFC_REGISTER_TEST(CacheTest, putGetReplaceTest);
        BFC_REGISTER_TEST(CacheTest, multiplePutTest);
        BFC_REGISTER_TEST(CacheTest, negativeGetTest);
<<<<<<< HEAD
=======
//        BFC_REGISTER_TEST(CacheTest, garbageTest);
        BFC_REGISTER_TEST(CacheTest, unusedTest);
>>>>>>> flash-bang-grenade
        BFC_REGISTER_TEST(CacheTest, overflowTest);
        BFC_REGISTER_TEST(CacheTest, strictTest);
        BFC_REGISTER_TEST(CacheTest, setSizeEnvCreateTest);
        BFC_REGISTER_TEST(CacheTest, setSizeEnvOpenTest);
        BFC_REGISTER_TEST(CacheTest, setSizeDbCreateTest);
        BFC_REGISTER_TEST(CacheTest, setSizeDbOpenTest);
<<<<<<< HEAD
        BFC_REGISTER_TEST(CacheTest, bigSizeTest);
=======
        BFC_REGISTER_TEST(CacheTest, setSizeEnvCreateLegacyTest);
        BFC_REGISTER_TEST(CacheTest, setSizeEnvOpenLegacyTest);
        BFC_REGISTER_TEST(CacheTest, setSizeDbCreateLegacyTest);
        BFC_REGISTER_TEST(CacheTest, setSizeDbOpenLegacyTest);
>>>>>>> flash-bang-grenade
    }

protected:
    ham_db_t *m_db;
    ham_env_t *m_env;
<<<<<<< HEAD
=======
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
        BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0,
                ham_env_create(m_env, BFC_OPATH(".test"),
                        HAM_ENABLE_TRANSACTIONS
                        | HAM_ENABLE_RECOVERY, 0644));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(m_env, m_db, 13,
                        HAM_ENABLE_DUPLICATES, 0));
    }
<<<<<<< HEAD
    
=======

>>>>>>> flash-bang-grenade
    virtual void teardown() {
        __super::teardown();

        ham_env_close(m_env, 0);
        ham_close(m_db, 0);
        ham_delete(m_db);
        ham_env_delete(m_env);
<<<<<<< HEAD
=======
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));
>>>>>>> flash-bang-grenade
    }

    void newDeleteTest(void)
    {
<<<<<<< HEAD
        Cache *cache=new Cache((Environment *)m_env, 15);
        BFC_ASSERT(cache!=0);
        delete cache;
    }

=======
        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT_NOTNULL(cache);
        cache_delete(m_env, cache);
    }

    void structureTest(void)
    {
        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT(cache!=0);
        BFC_ASSERT_EQUAL(cache_get_max_elements(cache), 1u); // element are not bytes!
        //BFC_ASSERT_EQUAL(cache_get_env(cache), m_env);
        cache_set_cur_elements(cache, 12);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 12u);
        cache_set_bucketsize(cache, 11);
        BFC_ASSERT_EQUAL(cache_get_bucketsize(cache), 11u);
        cache_delete(m_env, cache);

        cache=cache_new(m_env, 15 * os_get_pagesize());
        BFC_ASSERT(cache!=0);
        BFC_ASSERT_EQUAL(cache_get_max_elements(cache), 15u);
        //BFC_ASSERT_EQUAL(cache_get_env(cache), m_env);
        cache_set_cur_elements(cache, 12);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 12u);
        cache_set_bucketsize(cache, 11);
        BFC_ASSERT_EQUAL(cache_get_bucketsize(cache), 11u);
        BFC_ASSERT_NULL(cache_get_totallist(cache));
        BFC_ASSERT_NULL(cache_get_unused_page(cache, HAM_FALSE));
        BFC_ASSERT_NULL(cache_get_page(cache, 0x123ull, 0));
        BFC_ASSERT(!cache_too_big(cache));
        cache_delete(m_env, cache);
    }

>>>>>>> flash-bang-grenade
    void putGetTest(void)
    {
        Page *page;
        page_data_t pers;
        memset(&pers, 0, sizeof(pers));
<<<<<<< HEAD
        Cache *cache=new Cache((Environment *)m_env, 15);
        BFC_ASSERT(cache!=0);
        page=new Page((Environment *)m_env);
        page->set_self(0x123ull);
        page->set_pers(&pers);
        page->set_flags(Page::NPERS_NO_HEADER);
        cache->put_page(page);
        cache->get_page(0x123ull, 0);
        delete cache;
        page->set_pers(0);
        delete page;
=======
        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT_NOTNULL(cache);
        page=page_new(m_env);
        page_set_self(page, 0x123ull);
        page_set_pers(page, &pers);
        page_set_npers_flags(page, PAGE_NPERS_NO_HEADER);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_page(cache, 0x123ull, 0), page);
        cache_delete(m_env, cache);
        page_set_pers(page, 0);
        page_delete(page);
>>>>>>> flash-bang-grenade
    }

    void putGetRemoveGetTest(void)
    {
        Page *page;
        page_data_t pers;
        memset(&pers, 0, sizeof(pers));
<<<<<<< HEAD
        Cache *cache=new Cache((Environment *)m_env, 15);
        BFC_ASSERT(cache!=0);
        page=new Page((Environment *)m_env);
        page->set_flags(Page::NPERS_NO_HEADER);
        page->set_self(0x123ull);
        page->set_pers(&pers);
        cache->put_page(page);
        BFC_ASSERT(cache->get_cur_elements()==1);
        BFC_ASSERT(cache->get_page(0x123ull, 0)==page);
        BFC_ASSERT(cache->get_cur_elements()==0);
        cache->remove_page(page);
        BFC_ASSERT(cache->get_cur_elements()==0);
        BFC_ASSERT(cache->get_page(0x123ull, 0)==0);
        delete cache;
        page->set_pers(0);
        delete page;
=======
        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT_NOTNULL(cache);
        page=page_new(m_env);
        page_set_npers_flags(page, PAGE_NPERS_NO_HEADER);
        page_set_self(page, 0x123ull);
        page_set_pers(page, &pers);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1u);
        BFC_ASSERT_EQUAL(cache_get_page(cache, 0x123ull, 0), page); // page flag 'CACHE_NOREMOVE' is not set, so get will POP!
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 0u);
        BFC_ASSERT_EQUAL(cache_remove_page(cache, page), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 0u);          // and before 1.1.0, this count would be -1 instead of 0
        BFC_ASSERT_NULL(cache_get_page(cache, 0x123ull, 0));
        cache_delete(m_env, cache);
        page_set_pers(page, 0);
        page_delete(page);
>>>>>>> flash-bang-grenade
    }

    void putGetReplaceTest(void)
    {
        Page *page1, *page2;
        page_data_t pers1, pers2;
        memset(&pers1, 0, sizeof(pers1));
        memset(&pers2, 0, sizeof(pers2));
<<<<<<< HEAD
        Cache *cache=new Cache((Environment *)m_env, 15);
        BFC_ASSERT(cache!=0);
        page1=new Page((Environment *)m_env);
        page1->set_flags(Page::NPERS_NO_HEADER);
        page1->set_self(0x123ull);
        page1->set_pers(&pers1);
        page2=new Page((Environment *)m_env);
        page2->set_flags(Page::NPERS_NO_HEADER);
        page2->set_self(0x456ull);
        page2->set_pers(&pers2);
        cache->put_page(page1);
        BFC_ASSERT(cache->get_cur_elements()==1);
        cache->remove_page(page1);
        BFC_ASSERT(cache->get_cur_elements()==0);
        cache->put_page(page2);
        BFC_ASSERT(cache->get_cur_elements()==1);
        BFC_ASSERT(cache->get_page(0x123ull, 0)==0);
        BFC_ASSERT(cache->get_cur_elements()==1);
        BFC_ASSERT(cache->get_page(0x456ull, 0)==page2);
        BFC_ASSERT(cache->get_cur_elements()==0);
        delete cache;
        page1->set_pers(0);
        delete page1;
        page2->set_pers(0);
        delete page2;
=======
        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT_NOTNULL(cache);
        page1=page_new(m_env);
        page_set_npers_flags(page1, PAGE_NPERS_NO_HEADER);
        page_set_self(page1, 0x123ull);
        page_set_pers(page1, &pers1);
        page2=page_new(m_env);
        page_set_npers_flags(page2, PAGE_NPERS_NO_HEADER);
        page_set_self(page2, 0x456ull);
        page_set_pers(page2, &pers2);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1u);
        BFC_ASSERT_EQUAL(cache_remove_page(cache, page1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 0u);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page2), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1u);
        BFC_ASSERT_NULL(cache_get_page(cache, 0x123ull, 0));
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 1u); // failed to grab, no POPping therefor ;-)
        BFC_ASSERT_EQUAL(cache_get_page(cache, 0x456ull, 0), page2);
        BFC_ASSERT_EQUAL(cache_get_cur_elements(cache), 0u); // POP from cache as NODELETE wasn't set.
        cache_delete(m_env, cache);
        page_set_pers(page1, 0);
        page_delete(page1);
        page_set_pers(page2, 0);
        page_delete(page2);
>>>>>>> flash-bang-grenade
    }

    void multiplePutTest(void)
    {
<<<<<<< HEAD
        Page *page[20];
        page_data_t pers[20];
        Cache *cache=new Cache((Environment *)m_env, 15);

        for (int i=0; i<20; i++) {
            page[i]=new Page((Environment *)m_env);
            memset(&pers[i], 0, sizeof(pers[i]));
            page[i]->set_flags(Page::NPERS_NO_HEADER);
            page[i]->set_self((i+1)*1024);
            page[i]->set_pers(&pers[i]);
            cache->put_page(page[i]);
        }
        for (int i=0; i<20; i++) {
            BFC_ASSERT(cache->get_page((i+1)*1024, 0)==page[i]);
        }
        for (int i=0; i<20; i++) {
            cache->remove_page(page[i]);
        }
        for (int i=0; i<20; i++) {
            BFC_ASSERT(cache->get_page((i+1)*1024, 0)==0);
            page[i]->set_pers(0);
            delete page[i];
        }
        delete cache;
=======
        const int MAX=20;
        ham_page_t *page[MAX];
        ham_perm_page_union_t pers[MAX];
        ham_cache_t *cache=cache_new(m_env, 15);

        for (int i=0; i<MAX; i++) {
            page[i]=page_new(m_env);
            memset(&pers[i], 0, sizeof(pers[i]));
            page_set_npers_flags(page[i], PAGE_NPERS_NO_HEADER);
            page_set_self(page[i], i*1024);
            page_set_pers(page[i], &pers[i]);
            BFC_ASSERT_EQUAL_I(cache_put_page(cache, page[i]), HAM_SUCCESS, i);
        }
        for (int i=0; i<MAX; i++) {
            BFC_ASSERT_EQUAL_I(cache_get_page(cache, i*1024, 0), page[i], i);
        }
        for (int i=0; i<MAX; i++) {
            BFC_ASSERT_EQUAL_I(cache_remove_page(cache, page[i]), 0, i);
        }
        for (int i=0; i<MAX; i++) {
            BFC_ASSERT_NULL_I(cache_get_page(cache, i*1024, 0), i);
            page_set_pers(page[i], 0);
            page_delete(page[i]);
        }
        cache_delete(m_env, cache);
>>>>>>> flash-bang-grenade
    }

    void negativeGetTest(void)
    {
        Cache *cache=new Cache((Environment *)m_env, 15);
        for (int i=0; i<20; i++) {
<<<<<<< HEAD
            BFC_ASSERT(cache->get_page(i*1024*13, 0)==0);
        }
        delete cache;
=======
            BFC_ASSERT_NULL_I(cache_get_page(cache, i*1024*13, 0), i);
        }
        cache_delete(m_env, cache);
    }

#if 0
    void garbageTest(void)
    {
        ham_page_t *page;
        ham_cache_t *cache=cache_new(m_db, 15);
        BFC_ASSERT_NOTNULL(cache);
        page=page_new(m_db);
        page_set_npers_flags(page, PAGE_NPERS_NO_HEADER);
        page_set_self(page, 0x123ull);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_page(cache, 0x123ull, 0), page);
        BFC_ASSERT_EQUAL(cache_move_to_garbage(cache, page), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_page(cache, 0x123ull, 0), 0);
        BFC_ASSERT_EQUAL(cache_get_unused_page(cache), page);
        BFC_ASSERT_EQUAL(cache_get_unused_page(cache), 0);
        cache_delete(m_db, cache);
        page_delete(page);
    }
#endif

    void unusedTest(void)
    {
        ham_page_t *page1, *page2;
        ham_perm_page_union_t pers1, pers2;
        memset(&pers1, 0, sizeof(pers1));
        memset(&pers2, 0, sizeof(pers2));
        ham_cache_t *cache=cache_new(m_env, 15);
        BFC_ASSERT_NOTNULL(cache);
        page1=page_new(m_env);
        page_set_npers_flags(page1, PAGE_NPERS_NO_HEADER);
        page_set_self(page1, 0x123ull);
        page_set_pers(page1, &pers1);
        page_add_ref(page1);
        page2=page_new(m_env);
        page_set_npers_flags(page2, PAGE_NPERS_NO_HEADER);
        page_set_self(page2, 0x456ull);
        page_set_pers(page2, &pers2);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_put_page(cache, page2), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(cache_get_unused_page(cache, HAM_FALSE), page2);
        BFC_ASSERT_NULL(cache_get_unused_page(cache, HAM_TRUE));
        BFC_ASSERT_NULL(cache_get_unused_page(cache, HAM_FALSE));
        BFC_ASSERT_EQUAL(cache_get_page(cache, 0x123ull, 0), page1);
        BFC_ASSERT_NULL(cache_get_page(cache, 0x456ull, 0));
        //cache_delete(m_env, cache);  -- crashes the code for good reasons! you do NOT delete the cache halfway through!
        page_release_ref(page1);
        page_add_ref(page2);
        BFC_ASSERT_NULL(cache_get_page(cache, 0x123ull, CACHE_NOREMOVE));
        BFC_ASSERT_EQUAL(cache_put_page(cache, page1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(page1, cache_get_page(cache, 0x123ull, CACHE_NOREMOVE));
        BFC_ASSERT_NULL(cache_get_page(cache, 0x456ull, 0));
        BFC_ASSERT_EQUAL(cache_put_page(cache, page2), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(page2, cache_get_page(cache, 0x456ull, CACHE_NOREMOVE));
        BFC_ASSERT_EQUAL(page1, cache_get_unused_page(cache, HAM_TRUE));
        BFC_ASSERT_EQUAL(cache_put_page(cache, page1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(page1, cache_get_unused_page(cache, HAM_FALSE));
        BFC_ASSERT_NULL(cache_get_page(cache, 0x123ull, 0));
        page_release_ref(page2);
        cache_delete(m_env, cache);
        page_set_pers(page1, 0);
        page_delete(page1);
        page_set_pers(page2, 0);
        page_delete(page2);
>>>>>>> flash-bang-grenade
    }

    void overflowTest(void)
    {
        Cache *cache=new Cache((Environment *)m_env, 15*os_get_pagesize());
        page_data_t pers;
        memset(&pers, 0, sizeof(pers));
        std::vector<Page *> v;

        for (unsigned int i=0; i<15; i++) {
            Page *p=new Page((Environment *)m_env);
            p->set_flags(Page::NPERS_NO_HEADER);
            p->set_self((i+1)*1024);
            p->set_pers(&pers);
            v.push_back(p);
<<<<<<< HEAD
            cache->put_page(p);
            BFC_ASSERT(!cache->is_too_big());
=======
            BFC_ASSERT_EQUAL(cache_put_page(cache, p), 0);
            BFC_ASSERT(!cache_too_big(cache));
>>>>>>> flash-bang-grenade
        }

        for (unsigned int i=0; i<5; i++) {
            Page *p=new Page((Environment *)m_env);
            p->set_flags(Page::NPERS_NO_HEADER);
            p->set_self((i+1)*1024);
            p->set_pers(&pers);
            v.push_back(p);
<<<<<<< HEAD
            cache->put_page(p);
            BFC_ASSERT(cache->is_too_big());
=======
            BFC_ASSERT_EQUAL(cache_put_page(cache, p), 0);
            BFC_ASSERT(cache_too_big(cache)); // now it's too big
>>>>>>> flash-bang-grenade
        }

        for (unsigned int i=0; i<5; i++) {
            Page *p;
            BFC_ASSERT(cache->is_too_big());
            p=v.back();
            v.pop_back();
<<<<<<< HEAD
            cache->remove_page(p);
            p->set_pers(0);
            delete p;
=======
            BFC_ASSERT_EQUAL(cache_remove_page(cache, p), HAM_SUCCESS);
            page_set_pers(p, 0);
            page_delete(p);
>>>>>>> flash-bang-grenade
        }

        for (unsigned int i=0; i<15; i++) {
            Page *p;
            p=v.back();
            v.pop_back();
<<<<<<< HEAD
            cache->remove_page(p);
            BFC_ASSERT(!cache->is_too_big());
            p->set_pers(0);
            delete p;
        }

        BFC_ASSERT(!cache->is_too_big());
        delete cache;
=======
            BFC_ASSERT_EQUAL(cache_remove_page(cache, p), HAM_SUCCESS);
            BFC_ASSERT(!cache_too_big(cache));
            page_set_pers(p, 0);
            page_delete(p);
        }

        BFC_ASSERT(!cache_too_big(cache));
        cache_delete(m_env, cache);
>>>>>>> flash-bang-grenade
    }

    void strictTest(void)
    {
        dev_alloc_request_info_ex_t info = {0};

        ham_env_close(m_env, 0);
        ham_close(m_db, 0);

        ham_parameter_t param[]={
            {HAM_PARAM_PAGESIZE,  1024*128},
            {0, 0}};

        Page *p[1024];
        ham_db_t *db;
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0,
                ham_create_ex(db, ".test", HAM_CACHE_STRICT, 0644, &param[0]));
        ham_env_t *env=ham_get_env(db);
        Cache *cache=((Environment *)env)->get_cache();

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(cache->get_capacity(), 1024*1024*2u);

        unsigned int max_pages=HAM_DEFAULT_CACHESIZE/(1024*128);
        unsigned int i;
        for (i=0; i<max_pages; i++)
            BFC_ASSERT_EQUAL(0, db_alloc_page(&p[i], (Database *)db, 0, 0));

        BFC_ASSERT_EQUAL(HAM_CACHE_FULL, db_alloc_page(&p[i], (Database *)db, 0, 0));
        BFC_ASSERT_EQUAL(0, env_purge_cache((Environment *)ham_get_env(db)));
        BFC_ASSERT_EQUAL(0, db_alloc_page(&p[i], (Database *)db, 0, 0));
=======
        //BFC_ASSERT_EQUAL(cache_get_capacity(cache), 1024*1024*2u);
        BFC_ASSERT_EQUAL(cache_get_max_elements(cache), (1024*1024*2u)/(1024*128));

        info.db = db;
        info.env = env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        unsigned int max_pages=HAM_DEFAULT_CACHESIZE/(1024*128);
        unsigned int i;
        for (i=0; i < max_pages; i++) {
            BFC_ASSERT_EQUAL(HAM_SUCCESS, db_alloc_page(&p[i], 0, &info));
            page_add_ref(p[i]);
        }

        BFC_ASSERT_EQUAL(HAM_CACHE_FULL, db_alloc_page(&p[i], 0, &info));

        for (i=0; i<max_pages; i++) {
            page_release_ref(p[i]);
        }

        BFC_ASSERT_EQUAL(0, db_alloc_page(&p[i], 0, &info));
>>>>>>> flash-bang-grenade

        ham_close(db, 0);
        ham_delete(db);
    }

    void setSizeEnvCreateTest(void)
    {
        ham_env_t *env;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100*1024},
            {HAM_PARAM_PAGESIZE,  1024},
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_ex(env, ".test.db", 0, 0644, &param[0]));
        Cache *cache=((Environment *)env)->get_cache();

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(100*1024u, cache->get_capacity());
=======
        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));
>>>>>>> flash-bang-grenade

        ham_env_close(env, 0);
        ham_env_delete(env);
    }

    void setSizeEnvOpenTest(void)
    {
        ham_env_t *env;
        ham_parameter_t param[]={
<<<<<<< HEAD
=======
            {HAM_PARAM_PAGESIZE, 1024},
            {HAM_PARAM_CACHESIZE, 100*1024},
            {0, 0}};
        ham_parameter_t param4open[]={
            //{HAM_PARAM_PAGESIZE, 1024},
>>>>>>> flash-bang-grenade
            {HAM_PARAM_CACHESIZE, 100*1024},
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_ex(env, ".test.db", 0, 0644, &param[0]));
        ham_env_close(env, 0);
        BFC_ASSERT_EQUAL(0,
<<<<<<< HEAD
                ham_env_open_ex(env, ".test.db", 0, &param[0]));
        Cache *cache=((Environment *)env)->get_cache();

        BFC_ASSERT_EQUAL(100*1024u, cache->get_capacity());
=======
                ham_env_open_ex(env, ".test.db", 0, &param4open[0]));
        ham_cache_t *cache=env_get_cache(env);

        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));
>>>>>>> flash-bang-grenade

        ham_env_close(env, 0);
        ham_env_delete(env);
    }

    void setSizeDbCreateTest(void)
    {
        ham_db_t *db;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100*1024},
            {HAM_PARAM_PAGESIZE,  1024},
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_create_ex(db, ".test.db", 0, 0644, &param[0]));
        ham_env_t *env=ham_get_env(db);
        Cache *cache=((Environment *)env)->get_cache();

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(100*1024u, cache->get_capacity());
=======
        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));
>>>>>>> flash-bang-grenade

        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }

    void setSizeDbOpenTest(void)
    {
        ham_db_t *db;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100*1024},
<<<<<<< HEAD
=======
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_create_ex(db, ".test.db", 0, 0644, &param[0]));
        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        BFC_ASSERT_EQUAL(0, ham_open_ex(db, ".test.db", 0, &param[0]));
        ham_env_t *env=db_get_env(db);
        ham_cache_t *cache=env_get_cache(env);

        BFC_ASSERT_EQUAL(100*1024u / os_get_pagesize(), cache_get_max_elements(cache));

        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }

    void setSizeEnvCreateLegacyTest(void)
    {
        ham_env_t *env;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100},  // for values less than 512
            {HAM_PARAM_PAGESIZE,  1024},
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_ex(env, ".test.db", 0, 0644, &param[0]));
        ham_cache_t *cache=env_get_cache(env);

        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));

        ham_env_close(env, 0);
        ham_env_delete(env);
    }

    void setSizeEnvOpenLegacyTest(void)
    {
        ham_env_t *env;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100},  // for values less than 512
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_ex(env, ".test.db", 0, 0644, &param[0]));
        ham_env_close(env, 0);
        BFC_ASSERT_EQUAL(0,
                ham_env_open_ex(env, ".test.db", 0, &param[0]));
        ham_cache_t *cache=env_get_cache(env);

        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));

        ham_env_close(env, 0);
        ham_env_delete(env);
    }

    void setSizeDbCreateLegacyTest(void)
    {
        ham_db_t *db;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100},  // for values less than 512
            {HAM_PARAM_PAGESIZE,  1024},
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_create_ex(db, ".test.db", 0, 0644, &param[0]));
        ham_env_t *env=db_get_env(db);
        ham_cache_t *cache=env_get_cache(env);

        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));

        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }

    void setSizeDbOpenLegacyTest(void)
    {
        ham_db_t *db;
        ham_parameter_t param[]={
            {HAM_PARAM_CACHESIZE, 100},  // for values less than 512
>>>>>>> flash-bang-grenade
            {0, 0}};

        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_create_ex(db, ".test.db", 0, 0644, &param[0]));
        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        BFC_ASSERT_EQUAL(0, ham_open_ex(db, ".test.db", 0, &param[0]));
        ham_env_t *env=ham_get_env(db);
        Cache *cache=((Environment *)env)->get_cache();

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(100*1024u, cache->get_capacity());
=======
        BFC_ASSERT_EQUAL(100u, cache_get_max_elements(cache));
>>>>>>> flash-bang-grenade

        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }

    void bigSizeTest(void)
    {
        ham_u64_t size=1024ull*1024ull*1024ull*16ull;
        Cache *cache=new Cache((Environment *)m_env, size);
        BFC_ASSERT(cache!=0);
        BFC_ASSERT_EQUAL(size, cache->get_capacity());
        delete cache;
    }
};

BFC_REGISTER_FIXTURE(CacheTest);

