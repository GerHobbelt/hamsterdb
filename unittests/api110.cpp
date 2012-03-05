/*
 * Copyright (C) 2005-2012 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

#include <ham/hamsterdb.h>

#include <stdexcept>
#include <cstring>
#include <time.h>
<<<<<<< HEAD

#include <ham/hamsterdb.h>
=======
#include "../src/config.h"
#include "../src/mem.h"
>>>>>>> flash-bang-grenade
#include "../src/db.h"
#include "../src/env.h"
#include "../src/device.h"
#include "../src/os.h"
#include "../src/version.h"
#include "../src/serial.h"
#include "../src/btree.h"
<<<<<<< HEAD
#include "../src/btree_stats.h"
#include "../src/os.h"
=======
#include "../src/statistics.h"
#include "memtracker.h"
>>>>>>> flash-bang-grenade
#include "os.hpp"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

using namespace bfc;



/* indexing: 4096   8192   16384   32768   65536 --> 0   3   1   2   4 */
// ham_size_t sollwert_max_dbs[5] = { 4096, 16384, 32768, 8192, 65536 };
static const ham_size_t sollwert_max_dbs[5] = { 109, 493, 1005, 237, 2029 };
static const ham_size_t sollwert_keycount[5] = { 126, 510, 1022, 254, 2046 };


class APIv110Test : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    APIv110Test()
    :   hamsterDB_fixture("APIv110Test"), m_db(NULL)
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(APIv110Test, DbReUseTest);
        BFC_REGISTER_TEST(APIv110Test, transactionTest);
        BFC_REGISTER_TEST(APIv110Test, v10xDBformatDetectTest);
        BFC_REGISTER_TEST(APIv110Test, getDefaultEnvParamsTest);
        BFC_REGISTER_TEST(APIv110Test, getInitializedEnvParamsTest);
        BFC_REGISTER_TEST(APIv110Test, getInitializedReadonlyEnvParamsTest);
        BFC_REGISTER_TEST(APIv110Test, getDefaultDbParamsTest);
        BFC_REGISTER_TEST(APIv110Test, getInitializedDbParamsTest);
        BFC_REGISTER_TEST(APIv110Test, getInitializedReadonlyDbParamsTest);
<<<<<<< HEAD
        BFC_REGISTER_TEST(APIv110Test, negativeApproxMatchingTest);
=======
        BFC_REGISTER_TEST(APIv110Test, v10xDBformatUseTest);
        BFC_REGISTER_TEST(APIv110Test, v110DBformatDetectTest);
        BFC_REGISTER_TEST(APIv110Test, v110DBformatUseTest);
        BFC_REGISTER_TEST(APIv110Test, getEnvParamsTest);
        BFC_REGISTER_TEST(APIv110Test, getDbParamsTest);
        BFC_REGISTER_TEST(APIv110Test, statisticsGatheringTest);
        BFC_REGISTER_TEST(APIv110Test, findHinterTest);
        BFC_REGISTER_TEST(APIv110Test, insertHinterTest);
        BFC_REGISTER_TEST(APIv110Test, eraseHinterTest);
        BFC_REGISTER_TEST(APIv110Test, statsAfterAbortedTransactionTest);
        BFC_REGISTER_TEST(APIv110Test, differentDAMperDBTest);
        BFC_REGISTER_TEST(APIv110Test, pageFilterWithHeaderTest);
        BFC_REGISTER_TEST(APIv110Test, pageFilterWithFirstExtraHeaderTest);
        BFC_REGISTER_TEST(APIv110Test, stackedPageFiltersTest);
        BFC_REGISTER_TEST(APIv110Test, lowerBoundDatabaseSizeTest);
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

        os::unlink(BFC_OPATH(".test"));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));

        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0, ham_create(m_db, 0, HAM_IN_MEMORY_DB, 0));
    }

=======
        ham_set_default_allocator_template(m_alloc = memtracker_new());
        BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0, ham_create(m_db, 0, HAM_IN_MEMORY_DB, 0));
    }

>>>>>>> flash-bang-grenade
    virtual void teardown()
    {
        __super::teardown();

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_close(m_db, HAM_AUTO_CLEANUP));
        ham_delete(m_db);
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));
        ham_env_delete(m_env);
=======
        if (m_db)
        {
            BFC_ASSERT_EQUAL(0, ham_close(m_db, HAM_AUTO_CLEANUP));
            ham_delete(m_db);
        }
        if (m_env)
        {
            ham_env_close(m_env, HAM_AUTO_CLEANUP);
            ham_env_delete(m_env);
        }
        BFC_ASSERT_EQUAL(memtracker_get_leaks(ham_get_default_allocator_template()), 0U);
>>>>>>> flash-bang-grenade
    }

    void transactionTest(void)
    {
        ham_txn_t *txn;
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_txn_begin(&txn, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_txn_abort(txn, 0));

        // reopen the database, check the transaction flag vs. actual
        // use of transactions
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        ham_delete(m_db);
<<<<<<< HEAD
        m_db=0;

        BFC_ASSERT(ham_new(&m_db)==HAM_SUCCESS);
=======
        m_db = 0;
        BFC_ASSERT_EQUAL(memtracker_get_leaks(ham_get_default_allocator_template()), 0U);

        BFC_ASSERT_EQUAL(ham_new(&m_db), HAM_SUCCESS);
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(HAM_SUCCESS,
                ham_create(m_db, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn, 0));

        // can we cope with dual ham_close(), BTW? if not, we b0rk in teardown() (yup, we b0rked in teardown()...)
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(HAM_NOT_INITIALIZED, ham_close(m_db, 0));
        ham_delete(m_db);
        m_db=0;
    };

    void v10xDBformatDetectTest(void)
    {
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        os::unlink(BFC_OPATH(".test"));

        BFC_ASSERT_EQUAL(true,
            os::copy(BFC_IPATH("data/dupe-endian-test-open-database-be.hdb"),
                BFC_OPATH(".test")));
        BFC_ASSERT_EQUAL(0, ham_open(m_db, BFC_OPATH(".test"), 0));

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(true, os::unlink(BFC_OPATH(".test"), false));

        BFC_ASSERT_EQUAL(true,
            os::copy(BFC_IPATH("data/dupe-endian-test-open-database-le.hdb"),
                BFC_OPATH(".test")));
        BFC_ASSERT_EQUAL(0, ham_open(m_db, BFC_OPATH(".test"), 0));

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(true, os::unlink(BFC_OPATH(".test"), false));

        /* now the same, environment-based */
        BFC_ASSERT_EQUAL(true,
            os::copy(BFC_IPATH("data/dupe-endian-test-open-database-be.hdb"),
                BFC_OPATH(".test")));
        BFC_ASSERT_EQUAL(0, ham_env_open(m_env, BFC_OPATH(".test"), 0));

        ham_db_t *m_db2;
        BFC_ASSERT_EQUAL(0, ham_new(&m_db2));
        // first and only DB in there seems to be db # 0xF000,
        // which is an illegal number
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(m_env, m_db2, HAM_FIRST_DATABASE_NAME /* 1 */,
                    0, 0));

        BFC_ASSERT_EQUAL(0, ham_close(m_db2, 0));
        ham_delete(m_db2);
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, 0));
        BFC_ASSERT_EQUAL(true, os::unlink(BFC_OPATH(".test"), false));

        BFC_ASSERT_EQUAL(true,
            os::copy(BFC_IPATH("data/dupe-endian-test-open-database-le.hdb"),
                BFC_OPATH(".test")));
        BFC_ASSERT_EQUAL(0, ham_env_open(m_env, BFC_OPATH(".test"), 0));

        ham_delete(m_db);
        m_db=0;
    }

    void v10xDBformatUseTest(void)
    {
    }

    void v110DBformatDetectTest(void)
    {
    }

    void v110DBformatUseTest(void)
    {
    }

    ham_offset_t get_param_value(ham_parameter_t *param, ham_u16_t name)
    {
        for (; param->name; param++)
        {
            if (param->name == name)
            {
                return param->value.n;
            }
        }
        return (ham_offset_t)-1;
    }

    void getDefaultEnvParamsTest(void)
    {
        ham_size_t sollwert_max_dbs[5] = { 109, 493, 1005, 237, 2029 };

        ham_statistics_t stats = {0};
        ham_parameter_t params[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_STATISTICS, 0},
            {0,0}
        };
        params[6].value.stats_ref = &stats;

        ham_size_t sollwert_pagesize = os_get_pagesize();
        ham_size_t sollwert_cachesize = HAM_DEFAULT_CACHESIZE;

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_get_parameters(NULL, params));

        BFC_ASSERT_EQUAL(1u, stats.dont_collect_global_stats);
        BFC_ASSERT_EQUAL(1u, stats.dont_collect_db_stats);
        BFC_ASSERT_EQUAL(1u, stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_cachesize,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_pagesize,
                get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(sollwert_max_dbs[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0u,
                get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u,
                get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));
    }

    void getInitializedEnvParamsTest(void)
    {
        ham_env_t *env;
        ham_statistics_t stats = {0};
        ham_parameter_t params[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_STATISTICS, 0 /* &stats */ },
            {0,0}
        };
        params[6].value.stats_ref = &stats;
        ham_parameter_t set_params[] =
        {
            {HAM_PARAM_CACHESIZE, 1024*32},
            {HAM_PARAM_PAGESIZE, 1024*64},
            {HAM_PARAM_MAX_ENV_DATABASES, 32},
            {0,0}
        };

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_ex(env, BFC_OPATH(".test"), HAM_DISABLE_MMAP,
<<<<<<< HEAD
                                0664, &set_params[0]));

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));

        BFC_ASSERT_EQUAL(1024*32u,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(1024*64u,
                get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL((ham_offset_t)32,
=======
                                0664, set_params));

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));

        // cachesize is resported in BYTES, but is also made sure to be a whole number of pages:
        BFC_ASSERT_NOTEQUAL(1024*32u,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(1024*64u,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(1024*64u,
                get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(32u,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(HAM_DISABLE_MMAP,
                get_param_value(params, HAM_PARAM_GET_FLAGS));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)0664,
=======
        BFC_ASSERT_EQUAL(0664u,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0, strcmp(BFC_OPATH(".test"),
                (char *)get_param_value(params, HAM_PARAM_GET_FILENAME)));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));

        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));
        ham_env_delete(env);
    }

    void getInitializedReadonlyEnvParamsTest(void)
    {
        ham_env_t *env;
        ham_statistics_t stats = {0};
        ham_parameter_t params[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {0,0}
        };
        ham_parameter_t set_params[] =
        {
            {HAM_PARAM_CACHESIZE, 1024*32},
            {HAM_PARAM_PAGESIZE, 1024*64},
            {HAM_PARAM_MAX_ENV_DATABASES, 32},
            {0,0}
        };

        BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_ex(env, BFC_OPATH(".test"), HAM_DISABLE_MMAP,
                                0664, &set_params[0]));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_ex(env, BFC_OPATH(".test"), HAM_READ_ONLY, 0));

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));

        BFC_ASSERT_EQUAL((ham_offset_t)HAM_DEFAULT_CACHESIZE,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(1024*64u,
                get_param_value(params, HAM_PARAM_PAGESIZE));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)32,
=======
        BFC_ASSERT_EQUAL(32u,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(HAM_READ_ONLY,
                get_param_value(params, HAM_PARAM_GET_FLAGS));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)0644,
=======
        BFC_ASSERT_EQUAL(0644u,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0, strcmp(BFC_OPATH(".test"),
                (char *)get_param_value(params, HAM_PARAM_GET_FILENAME)));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));

        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));
        ham_env_delete(env);
    }

    void getDefaultDbParamsTest(void)
    {
        const ham_size_t pagesizes_inputs[] = { 0, 4096, 8192, 16384, 32768, 65536 };
        const ham_size_t pagesizes_sollwerte[] = { os_get_pagesize(), 4096, 8192, 16384, 32768, 65536 };
        int i;

        for (i = 0; i < sizeof(pagesizes_inputs) / sizeof(pagesizes_inputs[0]); i++)
        {
            ham_statistics_t stats = {0};
            ham_parameter_t params[] =
            {
                {HAM_PARAM_CACHESIZE, 0},
                {HAM_PARAM_KEYSIZE, 0},
                {HAM_PARAM_PAGESIZE, 0},
                {HAM_PARAM_MAX_ENV_DATABASES, 0},
                {HAM_PARAM_DBNAME, 0},
                {HAM_PARAM_GET_FLAGS, 0},
                {HAM_PARAM_GET_FILEMODE, 0},
                {HAM_PARAM_GET_FILENAME, 0},
                {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
                {HAM_PARAM_GET_DATA_ACCESS_MODE, 0},
                {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
                {0,0}
            };
            params[2].value.n = pagesizes_inputs[i];

            ham_size_t sollwert_pagesize = pagesizes_sollwerte[i];
            ham_size_t sollwert_cachesize = HAM_DEFAULT_CACHESIZE;
            ham_size_t sollwert_keysize = 21;

            BFC_ASSERT_EQUAL_I(0, ham_get_parameters(NULL, params), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(1u, stats.dont_collect_global_stats, sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(1u, stats.dont_collect_db_stats, sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(1u, stats.dont_collect_freelist_stats, sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(sollwert_cachesize,
                    get_param_value(params, HAM_PARAM_CACHESIZE), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(sollwert_keysize,
                    get_param_value(params, HAM_PARAM_KEYSIZE), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(sollwert_pagesize,
                    get_param_value(params, HAM_PARAM_PAGESIZE), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(0u,
                    get_param_value(params, HAM_PARAM_GET_DATA_ACCESS_MODE), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(sollwert_max_dbs[pg2i(sollwert_pagesize)] /* not: DB_MAX_INDICES */,
                    get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(sollwert_keycount[pg2i(sollwert_pagesize)],
                    get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(0u,
                    get_param_value(params, HAM_PARAM_DBNAME), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(sollwert_pagesize % os_get_granularity() != 0
                                ? HAM_DISABLE_MMAP
                                : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(0u,
                    get_param_value(params, HAM_PARAM_GET_FILEMODE), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I(0u,
                    get_param_value(params, HAM_PARAM_GET_FILENAME), sollwert_pagesize);
            BFC_ASSERT_EQUAL_I((ham_offset_t)&stats,
                    get_param_value(params, HAM_PARAM_GET_STATISTICS), sollwert_pagesize);
        }
    }

    void getInitializedDbParamsTest(void)
    {
        ham_db_t *db;
        ham_statistics_t stats = {0};
        ham_parameter_t params[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_DATA_ACCESS_MODE, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {0,0}
        };

        ham_parameter_t set_params[] =
        {
            {HAM_PARAM_CACHESIZE, 1024*32},
            {HAM_PARAM_KEYSIZE, 16},
            {HAM_PARAM_PAGESIZE, 1024},
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_SEQUENTIAL_INSERT},
            {0,0}
        };

        ham_new(&db);
        BFC_ASSERT_EQUAL(0,
                ham_create_ex(db, ".test.db",
                        HAM_CACHE_STRICT, 0644, &set_params[0]));

        BFC_ASSERT_EQUAL(0, ham_get_parameters(db, params));
        BFC_ASSERT_EQUAL(1024*32u,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(16u,
                get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(1024u,
                get_param_value(params, HAM_PARAM_PAGESIZE));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)HAM_DAM_SEQUENTIAL_INSERT,
=======
        BFC_ASSERT_EQUAL(HAM_DAM_SEQUENTIAL_INSERT,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_GET_DATA_ACCESS_MODE));
        BFC_ASSERT_EQUAL(13u,
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(36u,
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)HAM_DEFAULT_DATABASE_NAME,
                get_param_value(params, HAM_PARAM_DBNAME));
        BFC_ASSERT_EQUAL((ham_offset_t)DB_ENV_IS_PRIVATE|HAM_CACHE_STRICT|HAM_DISABLE_MMAP,
                get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL((ham_offset_t)0644,
=======
        BFC_ASSERT_EQUAL(HAM_FIRST_DATABASE_NAME /* HAM_DEFAULT_DATABASE_NAME */,
                get_param_value(params, HAM_PARAM_DBNAME));
        BFC_ASSERT_EQUAL(DB_ENV_IS_PRIVATE|HAM_CACHE_STRICT|HAM_DISABLE_MMAP,
                get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0644u,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0, strcmp(".test.db",
                (char *)get_param_value(params, HAM_PARAM_GET_FILENAME)));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));

        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }

    void getInitializedReadonlyDbParamsTest(void)
    {
        ham_db_t *db;
        ham_statistics_t stats = {0};
        ham_parameter_t params[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_DATA_ACCESS_MODE, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {0,0}
        };

        ham_parameter_t set_params[] =
        {
            {HAM_PARAM_CACHESIZE, 1024*32},
            {HAM_PARAM_KEYSIZE, 16},
            {HAM_PARAM_PAGESIZE, 1024},
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_RANDOM_WRITE},
            {0,0}
        };

        ham_new(&db);
        BFC_ASSERT_EQUAL(0,
                ham_create_ex(db, ".test.db",
                        HAM_CACHE_STRICT, 0644, &set_params[0]));
        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        BFC_ASSERT_EQUAL(0,
                ham_open_ex(db, ".test.db",
                        HAM_READ_ONLY, 0));

        BFC_ASSERT_EQUAL(0, ham_get_parameters(db, params));
        BFC_ASSERT_EQUAL((ham_offset_t)HAM_DEFAULT_CACHESIZE,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(16u,
                get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(1024u,
                get_param_value(params, HAM_PARAM_PAGESIZE));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)HAM_DAM_RANDOM_WRITE,
=======
        BFC_ASSERT_EQUAL(HAM_DAM_RANDOM_WRITE,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_GET_DATA_ACCESS_MODE));
        BFC_ASSERT_EQUAL(13u,
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(36u,
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));
<<<<<<< HEAD
        BFC_ASSERT_EQUAL((ham_offset_t)HAM_DEFAULT_DATABASE_NAME,
                get_param_value(params, HAM_PARAM_DBNAME));
        BFC_ASSERT_EQUAL((ham_offset_t)DB_ENV_IS_PRIVATE|HAM_READ_ONLY|HAM_DISABLE_MMAP,
                get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL((ham_offset_t)0644,
=======
        BFC_ASSERT_EQUAL(HAM_FIRST_DATABASE_NAME /* HAM_DEFAULT_DATABASE_NAME */,
                get_param_value(params, HAM_PARAM_DBNAME));
        BFC_ASSERT_EQUAL(DB_ENV_IS_PRIVATE|HAM_READ_ONLY|HAM_DISABLE_MMAP,
                get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0644u,
>>>>>>> flash-bang-grenade
                get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0, strcmp(".test.db",
                (char *)get_param_value(params, HAM_PARAM_GET_FILENAME)));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));

        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }

<<<<<<< HEAD
    void negativeApproxMatchingTest(void)
    {
        ham_db_t *db;
        ham_key_t key={0};
        ham_cursor_t *cursor;

        ham_new(&db);
        BFC_ASSERT_EQUAL(0,
                ham_create(db, ".test.db",
                        HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0, ham_cursor_create(db, 0, 0, &cursor));

        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                    ham_cursor_find(cursor, &key, HAM_FIND_GEQ_MATCH));

        BFC_ASSERT_EQUAL(0, ham_cursor_close(cursor));
        BFC_ASSERT_EQUAL(0, ham_close(db, 0));
        ham_delete(db);
    }
=======
    void getEnvParamsTest(void)
    {
        ham_env_t *env;
        ham_statistics_t stats = {0};
        ham_parameter_t params0[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            //{HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {HAM_PARAM_DATA_ACCESS_MODE, 0},
            {0,0}
        };
        ham_parameter_t params1[] =
        {
            {HAM_PARAM_CACHESIZE, (13*4+1)*1024},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 4*1024},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            //{HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_ENFORCE_PRE110_FORMAT | HAM_DAM_SEQUENTIAL_INSERT},
            {0,0}
        };
        ham_parameter_t params2[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 64*1024},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            //{HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_ENFORCE_PRE110_FORMAT | HAM_DAM_SEQUENTIAL_INSERT},
            {0,0}
        };
        ham_parameter_t params3[] =
        {
            {HAM_PARAM_CACHESIZE, 7},
            {HAM_PARAM_KEYSIZE, 17},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 31},
            //{HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 111},
            {HAM_PARAM_GET_FILEMODE, 0644},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {HAM_PARAM_DATA_ACCESS_MODE, 0},
            {0,0}
        };
        ham_parameter_t params4[] =
        {
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_ENFORCE_PRE110_FORMAT},
            {0,0}
        };
        ham_parameter_t params5[] =
        {
            {HAM_PARAM_DATA_ACCESS_MODE, HAM_DAM_RANDOM_WRITE},
            {0,0}
        };

        ham_size_t sollwert_pagesize = os_get_pagesize();
        ham_size_t sollwert_cachesize = HAM_DEFAULT_CACHESIZE;
        ham_size_t sollwert_keysize = 21;
        ham_parameter_t *params = params0;

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(NULL, params));

        BFC_ASSERT(stats.dont_collect_global_stats);
        BFC_ASSERT(stats.dont_collect_db_stats);
        BFC_ASSERT(stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_cachesize,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_keysize,
                get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(sollwert_pagesize,
                get_param_value(params, HAM_PARAM_PAGESIZE));

        // the exact values are verified in the getDefaultDbParamsTest test, so we wing it here!
        BFC_ASSERT_EQUAL(sollwert_max_dbs[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(sollwert_keycount[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));

        BFC_ASSERT_EQUAL((ham_offset_t)-1,
                get_param_value(params, HAM_PARAM_DBNAME)); // illegal search
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));

        params = params1;
        sollwert_pagesize = 4*1024;
        sollwert_keysize = 21;
        sollwert_cachesize = (13+1)*sollwert_pagesize; // conf code rounds cachesize UP

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(NULL, params));

        BFC_ASSERT(stats.dont_collect_global_stats);
        BFC_ASSERT(stats.dont_collect_db_stats);
        BFC_ASSERT(stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_pagesize, get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(sollwert_cachesize, get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_keysize, get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(sollwert_max_dbs[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL((ham_offset_t)-1, get_param_value(params, HAM_PARAM_DBNAME)); // illegal search
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL(sollwert_keycount[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats, get_param_value(params, HAM_PARAM_GET_STATISTICS));
        BFC_ASSERT_EQUAL(HAM_DAM_ENFORCE_PRE110_FORMAT | HAM_DAM_SEQUENTIAL_INSERT, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));

        params = params2;
        sollwert_pagesize = 64*1024;
        sollwert_cachesize = HAM_DEFAULT_CACHESIZE;
        sollwert_keysize = 21;

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(NULL, params));

        BFC_ASSERT(stats.dont_collect_global_stats);
        BFC_ASSERT(stats.dont_collect_db_stats);
        BFC_ASSERT(stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_cachesize, get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_keysize, get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(sollwert_pagesize, get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(sollwert_max_dbs[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL((ham_offset_t)-1, get_param_value(params, HAM_PARAM_DBNAME)); // illegal search
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL(sollwert_keycount[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats, get_param_value(params, HAM_PARAM_GET_STATISTICS));
        BFC_ASSERT_EQUAL(HAM_DAM_ENFORCE_PRE110_FORMAT | HAM_DAM_SEQUENTIAL_INSERT, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));

        ham_env_new(&env);

        sollwert_pagesize = os_get_pagesize();
        sollwert_cachesize = 7 * sollwert_pagesize;
        sollwert_keysize = 17;
        env_set_data_access_mode(env, HAM_DAM_SEQUENTIAL_INSERT); // copy this setting into the DAM
        params = params3;
        static const ham_size_t sollwert_keycount_m2[5] = { 144, 510 * 144 / 126, 1022 * 144 / 126, 254 * 144 / 126, 2046 * 144 / 126 };

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));
        BFC_ASSERT(stats.dont_collect_global_stats);
        BFC_ASSERT(stats.dont_collect_db_stats);
        BFC_ASSERT(stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_pagesize, get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(sollwert_cachesize, get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_keysize, get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(31u, get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL((ham_offset_t)-1, get_param_value(params, HAM_PARAM_DBNAME)); // illegal search
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL(sollwert_keycount_m2[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats, get_param_value(params, HAM_PARAM_GET_STATISTICS));
        BFC_ASSERT_EQUAL(HAM_DAM_SEQUENTIAL_INSERT, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));
        BFC_ASSERT_EQUAL(HAM_DAM_SEQUENTIAL_INSERT, env_get_data_access_mode(env));

        env_set_data_access_mode(env, HAM_DAM_SEQUENTIAL_INSERT); // MIX this setting into the DAM
        params = params4;

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));
        BFC_ASSERT_EQUAL((ham_offset_t)-1, get_param_value(params, HAM_PARAM_GET_FLAGS)); // illegal search
        BFC_ASSERT_EQUAL(HAM_DAM_SEQUENTIAL_INSERT | HAM_DAM_ENFORCE_PRE110_FORMAT, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));
        BFC_ASSERT_EQUAL(HAM_DAM_SEQUENTIAL_INSERT | HAM_DAM_ENFORCE_PRE110_FORMAT, env_get_data_access_mode(env)); // PRE110 always percolates down to ENV/DB as well

        env_set_data_access_mode(env, HAM_DAM_SEQUENTIAL_INSERT); // DENY this setting from the DAM: user params win over this one
        env_set_legacy(env, 0);
        params = params5;

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));
        BFC_ASSERT_EQUAL(HAM_DAM_RANDOM_WRITE, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));
        BFC_ASSERT_EQUAL(HAM_DAM_RANDOM_WRITE, env_get_data_access_mode(env)); // ENV/DB get overwritten with the new DAM when one was specified on get_param() input. !!!SIDE EFFECT!!!

        env_set_data_access_mode(env, HAM_DAM_SEQUENTIAL_INSERT); // DENY this setting from the DAM: user params win over this one
        env_set_legacy(env, 1);
        params = params5;

        BFC_ASSERT_EQUAL(0, ham_env_get_parameters(env, params));
        BFC_ASSERT_EQUAL(HAM_DAM_RANDOM_WRITE | HAM_DAM_ENFORCE_PRE110_FORMAT, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));
        BFC_ASSERT_EQUAL(HAM_DAM_RANDOM_WRITE | HAM_DAM_ENFORCE_PRE110_FORMAT, env_get_data_access_mode(env)); // ENV/DB get overwritten with the new DAM when one was specified on get_param() input. !!!SIDE EFFECT!!!

        ham_env_delete(env);
    }

        void getDbParamsTest(void)
    {
        ham_db_t *db;
        ham_env_t *env;

        ham_size_t sollwert_max_dbs[5] = { 109, 493, 1005, 237, 2029 };
        ham_size_t sollwert_keycount[5] = { 126, 510, 1022, 254, 2046 };

        ham_statistics_t stats = {0};
        ham_parameter_t params0[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_DAM, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {HAM_PARAM_DATA_ACCESS_MODE, 0},
            {0,0}
        };
        ham_parameter_t params1[] =
        {
            {HAM_PARAM_CACHESIZE, 0},
            {HAM_PARAM_KEYSIZE, 0},
            {HAM_PARAM_PAGESIZE, 0},
            {HAM_PARAM_MAX_ENV_DATABASES, 0},
            {HAM_PARAM_DBNAME, 0},
            {HAM_PARAM_GET_FLAGS, 0},
            {HAM_PARAM_GET_FILEMODE, 0},
            {HAM_PARAM_GET_FILENAME, 0},
            {HAM_PARAM_GET_KEYS_PER_PAGE, 0},
            {HAM_PARAM_GET_DAM, 0},
            {HAM_PARAM_GET_STATISTICS, (ham_offset_t)&stats},
            {HAM_PARAM_DATA_ACCESS_MODE, 0},
            {0,0}
        };

        ham_size_t sollwert_pagesize = os_get_pagesize();
        ham_size_t sollwert_cachesize = HAM_DEFAULT_CACHESIZE;
        ham_size_t sollwert_keysize = 21;
        ham_parameter_t *params = params0;

        BFC_ASSERT_EQUAL(0, ham_get_parameters(NULL, params));
        BFC_ASSERT(stats.dont_collect_global_stats);
        BFC_ASSERT(stats.dont_collect_db_stats);
        BFC_ASSERT(stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_cachesize,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_keysize,
                get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(sollwert_pagesize,
                get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(0u,
                get_param_value(params, HAM_PARAM_GET_DAM));

        // the exact values are verified in the getDefaultDbParamsTest test, so we wing it here!
        BFC_ASSERT_EQUAL(sollwert_max_dbs[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(sollwert_keycount[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));

        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_DBNAME));
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));

        ham_new(&db);

        // db which is not (yet) linked to an ENV triggers an error:
        params = params1;
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_get_parameters(db, params));

        ham_env_new(&env);
        db_set_env(db, env);

        sollwert_pagesize = os_get_pagesize();
        sollwert_cachesize = HAM_DEFAULT_CACHESIZE;
        sollwert_keysize = 21;
        params = params1;

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_get_parameters(db, params));
        BFC_ASSERT(stats.dont_collect_global_stats);
        BFC_ASSERT(stats.dont_collect_db_stats);
        BFC_ASSERT(stats.dont_collect_freelist_stats);
        BFC_ASSERT_EQUAL(sollwert_cachesize,
                get_param_value(params, HAM_PARAM_CACHESIZE));
        BFC_ASSERT_EQUAL(sollwert_keysize,
                get_param_value(params, HAM_PARAM_KEYSIZE));
        BFC_ASSERT_EQUAL(sollwert_pagesize,
                get_param_value(params, HAM_PARAM_PAGESIZE));
        BFC_ASSERT_EQUAL(0u,
                get_param_value(params, HAM_PARAM_GET_DAM));

        BFC_ASSERT_EQUAL(1u, // a DB will always report a max_databases count of 1; only ENV will ever be able to report them all.
                get_param_value(params, HAM_PARAM_MAX_ENV_DATABASES));
        BFC_ASSERT_EQUAL(sollwert_keycount[pg2i(sollwert_pagesize)],
                get_param_value(params, HAM_PARAM_GET_KEYS_PER_PAGE));
        BFC_ASSERT_EQUAL(sollwert_pagesize % os_get_granularity() != 0
                            ? HAM_DISABLE_MMAP
                            : 0u, get_param_value(params, HAM_PARAM_GET_FLAGS));
        BFC_ASSERT_EQUAL(HAM_FIRST_DATABASE_NAME,
                get_param_value(params, HAM_PARAM_DBNAME));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILEMODE));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_GET_FILENAME));
        BFC_ASSERT_EQUAL((ham_offset_t)&stats,
                get_param_value(params, HAM_PARAM_GET_STATISTICS));
        BFC_ASSERT_EQUAL(0u, get_param_value(params, HAM_PARAM_DATA_ACCESS_MODE));

        ham_delete(db);
    }

    void statisticsGatheringTest(void)
    {
        /*
        Collect some statistics; then produce those stats through a get_param call
        */
    }

    void findHinterTest(void)
    {
        /*
        IDEA:

        the effectiveness of the stats + hinter can be easily measured by hooking the
        comparison routine and augmenting that one with a counter: the number of key
        comparisons with and without hinting should be different (some hints may
        take a few more comparisons than usual).

        The second place to hook is the device layer: push a event monitoring device layer
        on top and listen in on the page read/write operations: count those too to
        determine hinter activity.
        */
    }

    void insertHinterTest(void)
    {
    }

    void eraseHinterTest(void)
    {
    }

    void statsAfterAbortedTransactionTest(void)
    {
    }

    void differentDAMperDBTest(void)
    {
    }

    void pageFilterWithHeaderTest(void)
    {
    }

    void pageFilterWithFirstExtraHeaderTest(void)
    {
    }

    void stackedPageFiltersTest(void)
    {
    }

    void lowerBoundDatabaseSizeTest(void)
    {
        ham_parameter_t params[] =
        {
            {HAM_PARAM_INITIAL_DB_SIZE, 1/*024*/},
            {0,0}
        };
        ham_size_t sollwert_pagesize = os_get_pagesize();
        ham_offset_t filesize;

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));
        os::unlink(BFC_OPATH(".test"));

        BFC_ASSERT_CMP(m_env, !=, db_get_env(m_db));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_create_ex(m_db, BFC_OPATH(".test"), 0, 0644, params));

        BFC_ASSERT_CMP(m_env, !=, db_get_env(m_db));
        BFC_ASSERT_NULL(env_get_device(m_env));
        BFC_ASSERT_NOTNULL(env_get_device(db_get_env(m_db)));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, env_get_device(db_get_env(m_db))->get_filesize(env_get_device(db_get_env(m_db)), &filesize));
        BFC_ASSERT_CMP(filesize, >=, sollwert_pagesize * params[0].value.n);
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));
        os::unlink(BFC_OPATH(".test"));

        /* now the same, environment-based */
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_create_ex(m_env, BFC_OPATH(".test"), 0, 0644, params));
        // the next API call will fail as it's re-using the m_db in a non-private ENV setting this time around:
        BFC_ASSERT_CMP(m_env, !=, db_get_env(m_db));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_env_create_db(m_env, m_db, 1, 0, params));
        // to remedy that, kill the m_db and create a fresh one:
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_delete(m_db));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_new(&m_db));
        // now the same API should succeed:
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_create_db(m_env, m_db, 1, 0, params));
        //BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_create_db(m_env, m_db, 1, 0, NULL));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, env_get_device(m_env)->get_filesize(env_get_device(m_env), &filesize));
        BFC_ASSERT_CMP(filesize, >=, sollwert_pagesize * params[0].value.n);

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_env_open_db(m_env, m_db, 1, 0, params));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_open_db(m_env, m_db, 1, 0, NULL));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_close(m_env, 0));

        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_env_open_ex(m_env, BFC_OPATH(".test"), 0, params));

        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_open_ex(m_db, BFC_OPATH(".test"), 0, params));

        ham_delete(m_db);
        m_db=0;
        ham_env_delete(m_env);
        m_env=0;
    }

    void DbReUseTest(void)
    {
        ham_env_t *env = db_get_env(m_db);

        BFC_ASSERT_NOTEQUAL(env, m_env);

        // due to the way m_db is set up, we have certain expectations regarding these flags and their effects following the ham_close call:
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT(db_is_active(m_db));
        BFC_ASSERT(env_is_active(env));
        BFC_ASSERT_NOTEQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));

        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT_NOTEQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_delete(m_db));
        m_db=0;

        os::unlink(BFC_OPATH(".test"));

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));


        // create a file DB sans explicit ENV: private+internal
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_create_ex(m_db, BFC_OPATH(".test"), 0, 0644, NULL));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        env = db_get_env(m_db);
        BFC_ASSERT_NOTEQUAL(env, m_env);
        BFC_ASSERT(env_is_active(env));
        BFC_ASSERT_NOTEQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));

        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT_NOTEQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));


        //re-use the m_db: the bugger should, once again, come with the same env and be private+internal
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_open_ex(m_db, BFC_OPATH(".test"), 0, NULL));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(env, db_get_env(m_db));
        BFC_ASSERT_NOTEQUAL(env, m_env);
        BFC_ASSERT(env_is_active(env));
        BFC_ASSERT_NOTEQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));

        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT_NOTEQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_delete(m_db));
        m_db=0;

        BFC_ASSERT_EQUAL(true, os::unlink(BFC_OPATH(".test"), false));

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));


        /*
        now do the same with an explicit (i.e. NON-internal) ENV:

        to accomplish this we re-use the m_db after first using in
        an explicit ENV+DB setting so that the ENV remains
        linked to the DB:
        */
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_create_ex(m_env, BFC_OPATH(".test"), 0, 0644, NULL));
        BFC_ASSERT(env_is_active(m_env));
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_create_db(m_env, m_db, 1, 0, NULL));
        BFC_ASSERT(db_is_active(m_db));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(m_env, db_get_env(m_db));
        BFC_ASSERT(env_is_active(m_env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        // now close the DB: the ENV should remain active, at least until it is closed as well!
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(m_env, db_get_env(m_db));
        BFC_ASSERT(env_is_active(m_env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_open_db(m_env, m_db, 1, 0, NULL));
        BFC_ASSERT(db_is_active(m_db));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(m_env, db_get_env(m_db));
        BFC_ASSERT(env_is_active(m_env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_close(m_db, 0));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(m_env, db_get_env(m_db));
        BFC_ASSERT(env_is_active(m_env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_close(m_env, 0));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(m_env, db_get_env(m_db));
        BFC_ASSERT(!env_is_active(m_env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        // now go and re-use m_db in a sans-ENV API mode:
        // (BTW: this may sound rediculous, but quite a few other unittests act this way implicitly due to their setup + unittest code mix!)

        // create a file DB sans explicit ENV: private but NOT internal
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_create_ex(m_db, BFC_OPATH(".test"), 0, 0644, NULL));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        env = db_get_env(m_db);
        BFC_ASSERT_EQUAL(env, m_env);
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_NOT_INITIALIZED, ham_close(m_db, 0));

        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));


        //re-use the m_db: the bugger should, once again, come with the same env and be private+!internal
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_open_ex(m_db, BFC_OPATH(".test"), 0, NULL));
        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT_EQUAL(env, db_get_env(m_db));
        BFC_ASSERT_EQUAL(env, m_env);
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_NOT_INITIALIZED, ham_close(m_db, 0));

        BFC_ASSERT_NOTNULL(db_get_env(m_db));
        BFC_ASSERT(!db_is_active(m_db));
        BFC_ASSERT(!env_is_active(env));
        BFC_ASSERT_EQUAL(0u, DB_ENV_IS_PRIVATE & db_get_rt_flags(m_db));

        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_delete(m_db));
        m_db=0;

        BFC_ASSERT_EQUAL(true, os::unlink(BFC_OPATH(".test"), false));

        BFC_ASSERT_NULL(m_db);
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER, ham_delete(m_db));
        m_db=0;
        BFC_ASSERT_EQUAL(HAM_SUCCESS, ham_env_delete(m_env));
        m_env=0;
    }
>>>>>>> flash-bang-grenade
};


BFC_REGISTER_FIXTURE(APIv110Test);

