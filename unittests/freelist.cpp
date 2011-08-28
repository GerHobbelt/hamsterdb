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
#include "../src/device.h"
#include "../src/env.h"
#include "../src/freelist.h"
#include "../src/mem.h"
#include "../src/page.h"
#include "../src/txn.h"
#include "memtracker.h"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>


using namespace bfc;

class FreelistBaseTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    FreelistBaseTest(const char *name, unsigned pagesize=4096)
    :   hamsterDB_fixture(name), m_db(NULL), m_env(NULL), m_alloc(NULL), m_pagesize(0)
    {
        m_pagesize=pagesize;
    }

protected:
    ham_db_t *m_db;
    ham_env_t *m_env;
    mem_allocator_t *m_alloc;
    ham_u32_t m_pagesize;

public:

    virtual ham_status_t open(ham_u32_t flags)
    {
        return (ham_open_ex(m_db, BFC_OPATH(".test"), flags, 0));
    }

    virtual ham_parameter_t *get_create_parameters(void)
    {
        static ham_parameter_t p[]={
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_DEFAULT /* 0 */},
            {0, 0}};
        p[0].value.n = m_pagesize;
        return &p[0];
    }

    virtual void setup()
    {
        __super::setup();

        ham_set_default_allocator_template(m_alloc = memtracker_new());
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_new(&m_db));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_create_ex(m_db, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS, 0644,
                        get_create_parameters()));
        m_env = db_get_env(m_db);
    }

    virtual void teardown()
    {
        __super::teardown();

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_delete(m_db));
        m_db=0;
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));
    }

    void structureTest(void)
    {
        freelist_payload_t *f;

        f=env_get_freelist(m_env);

        BFC_ASSERT_EQUAL(0, freel_get_allocated_bits16(f));
        freel_set_allocated_bits16(f, 13);

        BFC_ASSERT_EQUAL(0, freel_get_max_bits16(f));
        freel_set_max_bits16(f, 0x1234);

        BFC_ASSERT_EQUAL(0u, freel_get_overflow(f));
        freel_set_overflow(f, 0x12345678ull);

        freel_set_start_address(f, 0x7878787878787878ull);
        BFC_ASSERT_EQUAL(freel_get_start_address(f), 0x7878787878787878ull);
        BFC_ASSERT_EQUAL(freel_get_allocated_bits16(f), 13u);
        BFC_ASSERT_EQUAL(freel_get_max_bits16(f), 0x1234u);
        BFC_ASSERT_EQUAL(freel_get_overflow(f), 0x12345678ull);

        env_set_dirty(m_env);

        // reopen the database, check if the values were stored correctly
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        ham_delete(m_db);
        m_db=0;
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_new(&m_db));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, open(0));
        f=env_get_freelist(m_env);

        BFC_ASSERT_EQUAL(freel_get_start_address(f), 0x7878787878787878ull);
        BFC_ASSERT_EQUAL(freel_get_allocated_bits16(f), 13u);
        BFC_ASSERT_EQUAL(freel_get_max_bits16(f), 0x1234u);
        BFC_ASSERT_EQUAL(freel_get_overflow(f), 0x12345678ull);
    }

    void markAllocPageTest(void)
    {
        ham_status_t st;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_txn_t *txn;
        ham_offset_t addr;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        for (int i=0; i<10; i++) {
            BFC_ASSERT_EQUAL(0,
                    freel_mark_free(m_env, ps+i*DB_CHUNKSIZE, DB_CHUNKSIZE,
                        HAM_FALSE));
        }

        for (int i=0; i<10; i++) {
            st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
            BFC_ASSERT_EQUAL((ham_offset_t)(ps+i*DB_CHUNKSIZE), addr);
            BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        }

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)0, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT(env_is_dirty(m_env));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocAlignedTest(void)
    {
        ham_status_t st;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_txn_t *txn;
        ham_offset_t addr;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(0,
                    freel_mark_free(m_env, ps, ps, HAM_FALSE));
        st = freel_alloc_page(&addr, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocHighOffsetTest(void)
    {
        ham_status_t st;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_txn_t *txn;
        ham_offset_t addr;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        for (int i=60; i<70; i++) {
            BFC_ASSERT_EQUAL_I(0, freel_mark_free(m_env,
                        ps+i*DB_CHUNKSIZE, DB_CHUNKSIZE, HAM_FALSE), i);
        }

        for (int i=60; i<70; i++) {
            st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
            BFC_ASSERT_EQUAL_I(ps+i*DB_CHUNKSIZE, addr, i);
            BFC_ASSERT_EQUAL_I(st, HAM_SUCCESS, i);
        }

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT(env_is_dirty(m_env));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocRangeTest(void)
    {
        ham_status_t st;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_offset_t offset=ps;
        ham_txn_t *txn;
        ham_offset_t addr;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        for (int i=60; i<70; i++) {
            BFC_ASSERT_EQUAL_I(0, freel_mark_free(m_env, offset,
                        (i+1)*DB_CHUNKSIZE, HAM_FALSE), i);
            offset+=(i+1)*DB_CHUNKSIZE;
        }

        offset=ps;
        for (int i=60; i<70; i++) {
            st = freel_alloc_area(&addr, (i+1)*DB_CHUNKSIZE, &info);
            BFC_ASSERT_EQUAL_I(offset, addr, i);
            BFC_ASSERT_EQUAL_I(st, HAM_SUCCESS, i);
            offset+=(i+1)*DB_CHUNKSIZE;
        }

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT(env_is_dirty(m_env));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocOverflowTest(void)
    {
        ham_status_t st;
        ham_offset_t o=env_get_usable_pagesize(m_env)*8*DB_CHUNKSIZE;
        ham_txn_t *txn;
        ham_offset_t addr;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(0,
                freel_mark_free(m_env, o, DB_CHUNKSIZE, HAM_FALSE));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, open(HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(o, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);

        BFC_ASSERT_EQUAL(0,
                freel_mark_free(m_env, o*2, DB_CHUNKSIZE, HAM_FALSE));

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, open(HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(o*2, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocOverflow2Test(void)
    {
        ham_status_t st;
        ham_offset_t addr;
        ham_offset_t o=env_get_usable_pagesize(m_env)*8*DB_CHUNKSIZE;
        ham_txn_t *txn;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(0,
                freel_mark_free(m_env, 3*o, DB_CHUNKSIZE, HAM_FALSE));
        /*
         * The hinters must be disabled for this test to succeed; at least
         * they need to be instructed to kick in late.
         */
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db) &
                        ~(HAM_DAM_SEQUENTIAL_INSERT
                         | HAM_DAM_RANDOM_WRITE
                         | HAM_DAM_FAST_INSERT));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(3*o, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT(env_is_dirty(m_env));

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, open(HAM_ENABLE_TRANSACTIONS));
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db) &
                        ~(HAM_DAM_SEQUENTIAL_INSERT
                         | HAM_DAM_RANDOM_WRITE
                         | HAM_DAM_FAST_INSERT));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);

        BFC_ASSERT_EQUAL(0,
                freel_mark_free(m_env, 10*o, DB_CHUNKSIZE, HAM_FALSE));
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(10*o, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, open(HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocOverflow4Test(void)
    {
        ham_status_t st;
        ham_offset_t addr;
        ham_offset_t o=HAM_MAX_S32 - 1024;
        ham_txn_t *txn;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(0,
                freel_mark_free(m_env, o, DB_CHUNKSIZE*3, HAM_FALSE));
        /*
         * The hinters must be disabled for this test to succeed; at least
         * they need to be instructed to kick in late.
         */
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db) &
                        ~(HAM_DAM_SEQUENTIAL_INSERT
                         | HAM_DAM_RANDOM_WRITE
                         | HAM_DAM_FAST_INSERT));
        /*
         * and since we'll be having about 33027 freelist entries in the list,
         * the hinters will make a ruckus anyhow; the only way to get a hit
         * on the alloc is either through luck (which would take multiple
         * rounds as the hinters will drive the free space search using
         * SRNG technology, but it _is_ deterministic, so we could test for
         * that; however, I'm lazy so I'll just set a special 'impossible mode'
         * to disable the hinters entirely.
         */
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db)
                        | HAM_DAM_RANDOM_WRITE
                        | HAM_DAM_SEQUENTIAL_INSERT);

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(o, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT(env_is_dirty(m_env));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, open(HAM_ENABLE_TRANSACTIONS));

        /* set DAM - see above */
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db) &
                        ~(HAM_DAM_SEQUENTIAL_INSERT
                         | HAM_DAM_RANDOM_WRITE
                         | HAM_DAM_FAST_INSERT));
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db)
                        | HAM_DAM_RANDOM_WRITE
                        | HAM_DAM_SEQUENTIAL_INSERT);

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)o+DB_CHUNKSIZE, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)o+DB_CHUNKSIZE*2, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);

        BFC_ASSERT_EQUAL(0,
                freel_mark_free(m_env, o, DB_CHUNKSIZE*2, HAM_FALSE));
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(o, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, open(HAM_ENABLE_TRANSACTIONS));
        /* set DAM - see above */
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db) &
                        ~(HAM_DAM_SEQUENTIAL_INSERT
                         | HAM_DAM_RANDOM_WRITE
                         | HAM_DAM_FAST_INSERT));
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db)
                        | HAM_DAM_RANDOM_WRITE
                        | HAM_DAM_SEQUENTIAL_INSERT);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(o+DB_CHUNKSIZE, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)0, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocOverflow3Test(void)
    {
        ham_txn_t *txn;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        // this code snippet crashed in an acceptance test
        BFC_ASSERT_EQUAL(0, freel_mark_free(m_env, 2036736,
                    env_get_pagesize(m_env)-1024, HAM_FALSE));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocAlignTest(void)
    {
        ham_status_t st;
        ham_offset_t addr;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_txn_t *txn;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, freel_mark_free(m_env, ps, ps, HAM_FALSE));
        st = freel_alloc_page(&addr, &info);
        BFC_ASSERT_EQUAL(ps, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL(0u, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocAlignMultipleTest(void)
    {
        ham_status_t st;
        ham_offset_t addr;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_txn_t *txn;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(0, freel_mark_free(m_env, ps, ps*2, HAM_FALSE));
        st = freel_alloc_page(&addr, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps*1, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_page(&addr, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps*2, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)0, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void markAllocMultipleFromAddrTest(void)
    {
        ham_status_t st;
        ham_offset_t addr;
        ham_size_t ps=env_get_pagesize(m_env);
        ham_txn_t *txn;
		dev_alloc_request_info_ex_t info = {0};

		info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));

        BFC_ASSERT_EQUAL(0, freel_mark_free(m_env, ps, ps*4, HAM_FALSE));
        /*
        check whether lower_bound_address based scans deliver like
        they should PLUS make sure those lower_bound_address searches do not
        screw up the freelist hinter/stats.
        */
        st = freel_alloc_area_ex(&addr, ps/2, 0, ps*2, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps*2, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area_ex(&addr, ps/2, 0, 0, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area_ex(&addr, ps/2, 0, ps*2, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps*2 + ps/2, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area_ex(&addr, ps/2, 0, 0, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps + ps/2, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area_ex(&addr, ps, 0, 0, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)ps*3, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        st = freel_alloc_area(&addr, DB_CHUNKSIZE, &info);
        BFC_ASSERT_EQUAL((ham_offset_t)0, addr);
        BFC_ASSERT_EQUAL(st, HAM_SUCCESS);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void checkStructurePackingTest(void)
    {
        // checks to make sure structure packing by the compiler is still okay
        BFC_ASSERT_EQUAL(sizeof(freelist_payload_t),
                16 + 13 + sizeof(ham_pers_freelist_page_statistics_t));
        freelist_payload_t f;
        BFC_ASSERT_EQUAL(sizeof(f._s._s16), 5u);
        BFC_ASSERT_EQUAL(OFFSETOF(freelist_payload_t, _s._s16), 16u);
        BFC_ASSERT_EQUAL(OFFSETOF(freelist_payload_t, _s._s16._bitmap),
                16 + 4u);
        BFC_ASSERT_EQUAL(db_get_freelist_header_size16(), 16 + 4u);
        BFC_ASSERT_EQUAL(db_get_freelist_header_size32(),
                16 + 12 + sizeof(ham_pers_freelist_page_statistics_t));
        BFC_ASSERT_EQUAL(sizeof(ham_pers_freelist_page_statistics_t),
                8*4 + sizeof(ham_pers_freelist_page_statistics_t)
                        *HAM_FREELIST_SLOT_SPREAD);
        BFC_ASSERT_EQUAL(sizeof(ham_pers_freelist_page_statistics_t), 8*4u);
        BFC_ASSERT_EQUAL(HAM_FREELIST_SLOT_SPREAD, 16-5+1);
    }

};

class FreelistV1Test : public FreelistBaseTest
{
    define_super(FreelistBaseTest);

public:
    FreelistV1Test()
    :   FreelistBaseTest("FreelistV1Test")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(FreelistV1Test, checkStructurePackingTest);
        BFC_REGISTER_TEST(FreelistV1Test, structureTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocAlignedTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocPageTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocHighOffsetTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocRangeTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocOverflowTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocOverflow2Test);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocOverflow3Test);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocOverflow4Test);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocAlignTest);
        BFC_REGISTER_TEST(FreelistV1Test, markAllocAlignMultipleTest);
    }

    virtual ham_parameter_t *get_create_parameters(void)
    {
        static ham_parameter_t p[]={
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_ENFORCE_PRE110_FORMAT},
            {0, 0}};
        p[0].value.n = m_pagesize;
        return &p[0];
    }

    virtual void setup()
    {
        __super::setup();

#if 0
        /*
        Doing this is extremely naughty; you can't just go and set PRE110
        as is your fancy. ;-)
        The only proper way to set PRE110 for a db is through the PARAMs,
        as persisted header and actual db record should agree on this at
        all times; we assert for that within the hamster.

        Of course you don't persist those DAM flags, but I do and this
        was a bit of a surprise attack today. Made me clear the cobwebs
        off my mind quite thoroughly, so, in a way, this was a good thing.
        */
        db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db)|HAM_DAM_ENFORCE_PRE110_FORMAT);
#endif
    }

    virtual ham_status_t open(ham_u32_t flags)
    {
        /*
        since DAM is persisted, this will automagically pick up the PRE110
        flag.
        */
        ham_status_t st=ham_open_ex(m_db, BFC_OPATH(".test"), flags, 0);
#if 0
        if (st==0)
            db_set_data_access_mode(m_db,
                db_get_data_access_mode(m_db)|HAM_DAM_ENFORCE_PRE110_FORMAT);
#else
        if (st == 0)
        {
            BFC_ASSERT(db_is_mgt_mode_set(db_get_data_access_mode(m_db), HAM_DAM_ENFORCE_PRE110_FORMAT));
        }
#endif
        return (st);
    }
};

class FreelistV2Test : public FreelistBaseTest
{
    define_super(FreelistBaseTest);

public:
    FreelistV2Test()
    :   FreelistBaseTest("FreelistV2Test")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(FreelistV2Test, checkStructurePackingTest);
        BFC_REGISTER_TEST(FreelistV2Test, structureTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocAlignedTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocPageTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocHighOffsetTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocRangeTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocOverflowTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocOverflow2Test);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocOverflow3Test);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocOverflow4Test);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocAlignTest);
        BFC_REGISTER_TEST(FreelistV2Test, markAllocAlignMultipleTest);
    }
};

class FreelistV2Pagesize3072Test : public FreelistBaseTest
{
    define_super(FreelistBaseTest);

public:
    FreelistV2Pagesize3072Test()
    :   FreelistBaseTest("FreelistV2Pagesize3072Test", 3072)
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, checkStructurePackingTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, structureTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocAlignedTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocPageTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocHighOffsetTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocRangeTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocOverflowTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocOverflow2Test);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocOverflow3Test);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocOverflow4Test);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocAlignTest);
        BFC_REGISTER_TEST(FreelistV2Pagesize3072Test, markAllocAlignMultipleTest);
    }
};

BFC_REGISTER_FIXTURE(FreelistV1Test);
BFC_REGISTER_FIXTURE(FreelistV2Test);
BFC_REGISTER_FIXTURE(FreelistV2Pagesize3072Test);

