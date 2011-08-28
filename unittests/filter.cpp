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

#include <ham/hamsterdb_int.h>
#include "memtracker.h"
#include "../src/db.h"
#include "../src/env.h"
#include "../src/mem.h"
#include "os.hpp"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <cstring>


using namespace bfc;

typedef struct simple_filter_t
{
    int written;
    int read;
    int closed;
} simple_filter_t;

/*
 * XOR is a commutative operation:
 *
 * A ^ B ^ C == A ^ C ^ B
 *
 * so, in order to detect filter chain defects, we MUST construct a filter
 * set F1, F2 which is NOT commutative -- nope, two XOR filters isn't
 * cutting it -- and given that all simple invertible math ops are commutative,
 * at least as far as we're concerned, i.e. '-' (subtraction) is also
 * commutative for our purposes:
 *
 * M - F1 - F2 == M - F2 - F1
 *
 * what we need for the test is:
 *
 * M . F1 : F2  !=  M : F2 . F1
 *
 * where . and : are operators to be determined.
 *
 * The easiest is a combo of XOR and ADD (with wrap-around in the 2^8 domain):
 *
 * M XOR F1 ADD F2 != M ADD F2 XOR F1
 *
 * (ADD can be inverted by subtraction with the same wrap-around condition).
 */

#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

static ham_status_t
my_xor_pre_cb(ham_file_filter_t *filter,
              ham_env_t *env,
              ham_file_filter_bufinfo_t *data)
{
    ham_u8_t ch=*(ham_u8_t *)filter->userdata;
    for (ham_size_t i=0; i<data->sourcewidth; i++)
        data->source[i]^=ch;
    return (0);
}

static ham_status_t
my_xor_post_cb(ham_file_filter_t *filter,
               ham_env_t *env,
               ham_file_filter_bufinfo_t *data)
{
    ham_u8_t ch=*(ham_u8_t *)filter->userdata;
    for (ham_size_t i=0; i<data->sourcewidth; i++)
        data->source[i]^=ch;
    return (0);
}


static ham_status_t
my_add_pre_cb(ham_file_filter_t *filter,
              ham_env_t *env,
              ham_file_filter_bufinfo_t *data)
{
    ham_u8_t ch=*(ham_u8_t *)filter->userdata;
    for (ham_size_t i=0; i<data->sourcewidth; i++)
        data->source[i]+=ch;
    return (0);
}

static ham_status_t
my_add_post_cb(ham_file_filter_t *filter,
               ham_env_t *env,
               ham_file_filter_bufinfo_t *data)
{
    ham_u8_t ch=*(ham_u8_t *)filter->userdata;
    for (ham_size_t i=0; i<data->sourcewidth; i++)
        data->source[i]-=ch;
    return (0);
}

static ham_status_t
my_file_pre_cb(ham_file_filter_t *filter,
               ham_env_t *env,
               ham_file_filter_bufinfo_t *data)
{
    simple_filter_t *sf=(simple_filter_t *)filter->userdata;
    sf->written++;
    return (0);
}

static ham_status_t
my_file_post_cb(ham_file_filter_t *filter,
                ham_env_t *env,
                ham_file_filter_bufinfo_t *data)
{
    simple_filter_t *sf=(simple_filter_t *)filter->userdata;
    sf->read++;
    return (0);
}

static ham_status_t
my_file_close_cb(ham_file_filter_t *filter, ham_env_t *)
{
    simple_filter_t *sf=(simple_filter_t *)filter->userdata;
    sf->closed++;
    return 0;
}

#endif

static ham_status_t
my_record_pre_cb(ham_db_t *db,
                 ham_record_filter_t *filter, ham_record_t *record, ham_rec_filter_info_io_t *info)
{
    simple_filter_t *sf=(simple_filter_t *)filter->userdata;
    sf->written++;
    return (0);
}

static ham_status_t
my_record_post_cb(ham_db_t *db,
                  ham_record_filter_t *filter, ham_record_t *record, ham_rec_filter_info_io_t *info)
{
    simple_filter_t *sf=(simple_filter_t *)filter->userdata;
    sf->read++;
    return (0);
}

static void
my_record_close_cb(ham_db_t *db,
                   ham_record_filter_t *filter, ham_rec_filter_info_io_t *info)
{
    simple_filter_t *sf=(simple_filter_t *)filter->userdata;
    sf->closed++;
}

class FilterTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    FilterTest()
        : hamsterDB_fixture("FilterTest")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(FilterTest, addRemoveFileTest);
        BFC_REGISTER_TEST(FilterTest, addRemoveRecordTest);
        BFC_REGISTER_TEST(FilterTest, simpleFileFilterTest);
        BFC_REGISTER_TEST(FilterTest, cascadedFileFilterTest);
        BFC_REGISTER_TEST(FilterTest, simpleRecordFilterTest);
        BFC_REGISTER_TEST(FilterTest, aesFilterTest);
        BFC_REGISTER_TEST(FilterTest, aesFilterInMemoryTest);
        BFC_REGISTER_TEST(FilterTest, aesTwiceFilterTest);
        BFC_REGISTER_TEST(FilterTest, negativeAesFilterTest);
        BFC_REGISTER_TEST(FilterTest, zlibFilterTest);
        BFC_REGISTER_TEST(FilterTest, zlibFilterEmptyRecordTest);
        BFC_REGISTER_TEST(FilterTest, zlibEnvFilterTest);
    }

protected:
    ham_db_t *m_db;
    ham_u32_t m_flags;
    ham_env_t *m_env;
    mem_allocator_t *m_alloc;
    /*
    filters MUST 'live' for the entire lifetime of the ENV they are related to.
    (now that teardown can call their close method, this is even more important
    to adhere to)

    WARNING: filter 'user data' must have the same lifetime (or longer)!
    */
#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */
    ham_file_filter_t filter1, filter2, filter3;
#endif
    simple_filter_t sf;
#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */
    ham_file_filter_t filter;
#endif
    char ch1, ch2;
    ham_record_filter_t rec_filter1, rec_filter2, rec_filter3;


public:
    virtual void setup()
    {
        __super::setup();

        m_flags=0;

        os::unlink(BFC_OPATH(".test"));
        ham_set_default_allocator_template(m_alloc = memtracker_new());
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
    }

    virtual void teardown()
    {
        __super::teardown();

        ham_delete(m_db);
        if (m_env)
        {
            BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));
            BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        }
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));
    }

    void addRemoveFileTest()
    {
#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

        //ham_file_filter_t filter1, filter2, filter3;
        memset(&filter1, 0, sizeof(filter1));
        memset(&filter2, 0, sizeof(filter2));
        memset(&filter3, 0, sizeof(filter3));

        /*
        prevent continuous errors in Win32/64 when one test fails, due to Win32 inability to (re)open a file
        which still has a mmap page open.

        Hence env must be destroyed in teardown. And for convenience, it's created in setup now.
        */
        //BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));

        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_env_add_file_filter(0, &filter1));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_env_add_file_filter(m_env, 0));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_env_remove_file_filter(0, &filter1));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_env_remove_file_filter(m_env, 0));

        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter1));
        BFC_ASSERT(filter1._next==0);
        // filters have a cyclic 'prev' chain; see the tech documentation
        BFC_ASSERT(filter1._prev==&filter1);
        BFC_ASSERT_EQUAL(&filter1, env_get_file_filter(m_env));

        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter2));
        BFC_ASSERT(filter1._next==&filter2);
        BFC_ASSERT(filter2._prev==&filter1);
        // filters have a cyclic 'prev' chain; see the tech documentation
        BFC_ASSERT(filter1._prev==&filter2);
        BFC_ASSERT(filter2._next==0);
        BFC_ASSERT_EQUAL(&filter1, env_get_file_filter(m_env));

        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter3));
        BFC_ASSERT(filter1._next==&filter2);
        BFC_ASSERT(filter2._prev==&filter1);
        BFC_ASSERT(filter2._next==&filter3);
        BFC_ASSERT(filter3._prev==&filter2);
        // filters have a cyclic 'prev' chain; see the tech documentation
        BFC_ASSERT(filter1._prev==&filter3);
        BFC_ASSERT(filter3._next==0);
        BFC_ASSERT_EQUAL(&filter1, env_get_file_filter(m_env));

        BFC_ASSERT_EQUAL(0, ham_env_remove_file_filter(m_env, &filter2));
        BFC_ASSERT(filter1._next==&filter3);
        BFC_ASSERT(filter3._prev==&filter1);
        // filters have a cyclic 'prev' chain; see the tech documentation
        BFC_ASSERT(filter1._prev==&filter3);
        BFC_ASSERT(filter3._next==0);
        BFC_ASSERT_EQUAL(&filter1, env_get_file_filter(m_env));

        BFC_ASSERT_EQUAL(0, ham_env_remove_file_filter(m_env, &filter3));
        // filters have a cyclic 'prev' chain; see the tech documentation
        BFC_ASSERT(filter1._prev==&filter1);
        BFC_ASSERT(filter1._next==0);
        BFC_ASSERT_EQUAL(&filter1, env_get_file_filter(m_env));

        BFC_ASSERT_EQUAL(0, ham_env_remove_file_filter(m_env, &filter1));
        BFC_ASSERT(0==env_get_file_filter(m_env));

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, 0));
        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        m_env = 0;
#endif
    }

    void addRemoveRecordTest()
    {
        //ham_record_filter_t rec_filter1, rec_filter2, rec_filter3;
        memset(&rec_filter1, 0, sizeof(rec_filter1));
        memset(&rec_filter2, 0, sizeof(rec_filter2));
        memset(&rec_filter3, 0, sizeof(rec_filter3));

        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_add_record_filter(0, &rec_filter1));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_add_record_filter(m_db, 0));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_remove_record_filter(0, &rec_filter1));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_remove_record_filter(m_db, 0));

        BFC_ASSERT_EQUAL(0, ham_add_record_filter(m_db, &rec_filter1));
        BFC_ASSERT(rec_filter1._next==0);
        BFC_ASSERT(rec_filter1._prev==0);
        BFC_ASSERT_EQUAL(&rec_filter1, db_get_record_filter(m_db));

        BFC_ASSERT_EQUAL(0, ham_add_record_filter(m_db, &rec_filter2));
        BFC_ASSERT(rec_filter1._next==&rec_filter2);
        BFC_ASSERT(rec_filter2._prev==&rec_filter1);
        BFC_ASSERT(rec_filter1._prev==0);
        BFC_ASSERT(rec_filter2._next==0);
        BFC_ASSERT_EQUAL(&rec_filter1, db_get_record_filter(m_db));

        BFC_ASSERT_EQUAL(0, ham_add_record_filter(m_db, &rec_filter3));
        BFC_ASSERT(rec_filter1._next==&rec_filter2);
        BFC_ASSERT(rec_filter2._prev==&rec_filter1);
        BFC_ASSERT(rec_filter2._next==&rec_filter3);
        BFC_ASSERT(rec_filter3._prev==&rec_filter2);
        BFC_ASSERT(rec_filter1._prev==0);
        BFC_ASSERT(rec_filter3._next==0);
        BFC_ASSERT_EQUAL(&rec_filter1, db_get_record_filter(m_db));

        BFC_ASSERT_EQUAL(0, ham_remove_record_filter(m_db, &rec_filter2));
        BFC_ASSERT(rec_filter1._next==&rec_filter3);
        BFC_ASSERT(rec_filter3._prev==&rec_filter1);
        BFC_ASSERT(rec_filter1._prev==0);
        BFC_ASSERT(rec_filter3._next==0);
        BFC_ASSERT_EQUAL(&rec_filter1, db_get_record_filter(m_db));

        BFC_ASSERT_EQUAL(0, ham_remove_record_filter(m_db, &rec_filter3));
        BFC_ASSERT(rec_filter1._prev==0);
        BFC_ASSERT(rec_filter1._next==0);
        BFC_ASSERT_EQUAL(&rec_filter1, db_get_record_filter(m_db));

        BFC_ASSERT_EQUAL(0, ham_remove_record_filter(m_db, &rec_filter1));
        BFC_ASSERT(0==db_get_record_filter(m_db));

        BFC_ASSERT_EQUAL(0, ham_create(m_db, BFC_OPATH(".test"), m_flags, 0664));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void simpleFileFilterTest()
    {
#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

        ham_db_t *db;

        //simple_filter_t sf;
        //ham_file_filter_t filter;
        memset(&filter, 0, sizeof(filter));
        memset(&sf, 0, sizeof(sf));
        filter.userdata=(void *)&sf;
        filter.write_cb=my_file_pre_cb;
        filter.read_cb=my_file_post_cb;
        filter.close_cb=my_file_close_cb;

        //BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(m_env, db, 333, 0, 0));

        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        BFC_ASSERT_EQUAL(0, ham_insert(db, 0, &key, &rec, 0));

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));

        BFC_ASSERT_EQUAL(1, sf.written);
        BFC_ASSERT_EQUAL(1, sf.read);
        BFC_ASSERT_EQUAL(1, sf.closed);

        memset(&sf, 0, sizeof(sf));
        BFC_ASSERT_EQUAL(0, ham_env_open(m_env, BFC_OPATH(".test"), 0));
        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter));
        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(0, sf.written);
        BFC_ASSERT_EQUAL(0, sf.read);
        BFC_ASSERT_EQUAL(0, sf.closed);

        BFC_ASSERT_EQUAL(0, ham_find(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, sf.written);
        BFC_ASSERT_EQUAL(1, sf.read);
        BFC_ASSERT_EQUAL(0, sf.closed);

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));
        BFC_ASSERT_EQUAL(0, sf.written);
        BFC_ASSERT_EQUAL(1, sf.read);
        BFC_ASSERT_EQUAL(1, sf.closed);

        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db));
        m_env = 0;
#endif
    }

    void cascadedFileFilterTest()
    {
#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

        ham_db_t *db;

        ch1=0x13;
        ch2=0x15;
        //ham_file_filter_t filter1, filter2;
        memset(&filter1, 0, sizeof(filter1));
        filter1.userdata=(void *)&ch1;
        filter1.write_cb=my_xor_pre_cb;
        filter1.read_cb=my_xor_post_cb;
        memset(&filter2, 0, sizeof(filter2));
        filter2.userdata=(void *)&ch2;
        filter2.write_cb=my_add_pre_cb; // make sure filters break when swapped in exec order
        filter2.read_cb=my_add_post_cb;

        //BFC_ASSERT_EQUAL(0, ham_env_new(&env));
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter1));
        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter2));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db, 333, 0, 0));

        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        BFC_ASSERT_EQUAL(0, ham_insert(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));

        BFC_ASSERT_EQUAL(0, ham_env_open(m_env, BFC_OPATH(".test"), 0));
        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter1));
        BFC_ASSERT_EQUAL(0, ham_env_add_file_filter(m_env, &filter2));
        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db, 333, 0, 0));

        BFC_ASSERT_EQUAL(0, ham_find(db, 0, &key, &rec, 0));

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));

        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db));
        m_env = 0;
#endif
    }

    void simpleRecordFilterTest()
    {
        //simple_filter_t sf;
        //ham_record_filter_t filter;
        memset(&rec_filter1, 0, sizeof(rec_filter1));
        memset(&sf, 0, sizeof(sf));
        rec_filter1.userdata=(void *)&sf;
        rec_filter1.before_write_cb=my_record_pre_cb;
        rec_filter1.after_read_cb=my_record_post_cb;
        rec_filter1.close_cb=my_record_close_cb;

        BFC_ASSERT_EQUAL(0,
                    ham_add_record_filter(m_db, &rec_filter1));

        BFC_ASSERT_EQUAL(0,
                    ham_create(m_db, BFC_OPATH(".test"), m_flags, 0664));

        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"123";
        rec.size=3;
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, 0, &key, &rec, 0));

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));

        BFC_ASSERT_EQUAL(1, sf.written);
        BFC_ASSERT_EQUAL(0, sf.read);
        BFC_ASSERT_EQUAL(1, sf.closed);

        memset(&sf, 0, sizeof(sf));
        BFC_ASSERT_EQUAL(0, ham_add_record_filter(m_db, &rec_filter1));
        BFC_ASSERT_EQUAL(0, ham_open(m_db, BFC_OPATH(".test"), 0));
        BFC_ASSERT_EQUAL(0, sf.written);
        BFC_ASSERT_EQUAL(0, sf.read);
        BFC_ASSERT_EQUAL(0, sf.closed);

        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, sf.written);
        BFC_ASSERT_EQUAL(1, sf.read);
        BFC_ASSERT_EQUAL(0, sf.closed);
        BFC_ASSERT(!strcmp((const char *)rec.data, "123"));

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, sf.written);
        BFC_ASSERT_EQUAL(1, sf.read);
        BFC_ASSERT_EQUAL(1, sf.closed);
    }

    void aesFilterTest()
    {
#ifndef HAM_DISABLE_ENCRYPTION
        ham_db_t *db;

        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        ham_u8_t aeskey[16] ={0x13};
        ham_u8_t aeskey2[16]={0x14};

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_enable_encryption(m_env, aeskey, 0));

        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db, 0));

        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));

        BFC_ASSERT_EQUAL(0, ham_env_open(m_env, BFC_OPATH(".test"), 0));
        BFC_ASSERT_EQUAL(HAM_ACCESS_DENIED,
                ham_env_enable_encryption(m_env, aeskey2, 0));
        BFC_ASSERT_EQUAL(0, ham_env_enable_encryption(m_env, aeskey, 0));
        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));

        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db));
        m_env = 0;
#endif
    }

    void aesFilterInMemoryTest()
    {
#ifndef HAM_DISABLE_ENCRYPTION
        ham_db_t *db;

        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        ham_u8_t aeskey[16] ={0x13};

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0,
                ham_env_create(m_env, BFC_OPATH(".test"),
                        HAM_IN_MEMORY_DB, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_enable_encryption(m_env, aeskey, 0));

        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));

        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db));
#endif
    }

    void aesTwiceFilterTest()
    {
#ifndef HAM_DISABLE_ENCRYPTION
        ham_db_t *db;

        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        ham_u8_t aeskey1[16]={0x13};
        ham_u8_t aeskey2[16]={0x14};

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_enable_encryption(m_env, aeskey1, 0));
        BFC_ASSERT_EQUAL(HAM_ALREADY_INITIALIZED,
                ham_env_enable_encryption(m_env, aeskey2, 0));

        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db, 0));

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));
        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db));
#endif
    }

    void negativeAesFilterTest(void)
    {
#ifndef HAM_DISABLE_ENCRYPTION
        //ham_env_t *env=(ham_env_t *)1;
        ham_db_t *db;
        ham_u8_t aeskey[16]={0x13};

        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                        ham_env_enable_encryption(0, aeskey, 0));

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&db));
        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db, 333, 0, 0));
        BFC_ASSERT_EQUAL(HAM_DATABASE_ALREADY_OPEN,
                        ham_env_enable_encryption(m_env, aeskey, 0));

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));
        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db));
#endif
    }

    void zlibFilterTest()
    {
#ifndef HAM_DISABLE_COMPRESSION
        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"hello world 12345 12345 12345 12345 12345";
        rec.size=(ham_size_t)strlen((char *)rec.data);

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"), m_flags, 0664));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_enable_compression(0, 0, 0));
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_enable_compression(m_db, 9999, 0));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(m_db, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));

        BFC_ASSERT_EQUAL(0, ham_open(m_db, BFC_OPATH(".test"), 0));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(m_db, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec, 0));
        rec.flags=HAM_RECORD_USER_ALLOC;
        BFC_ASSERT_EQUAL(HAM_INV_PARAMETER,
                ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
#endif
    }

    void zlibFilterEmptyRecordTest(void)
    {
#ifndef HAM_DISABLE_COMPRESSION
        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"), m_flags, 0664));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(m_db, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));

        BFC_ASSERT_EQUAL(0, ham_open(m_db, BFC_OPATH(".test"), 0));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(m_db, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
#endif
    }

    void zlibEnvFilterTest()
    {
#ifndef HAM_DISABLE_COMPRESSION
        ham_db_t *db[3];
        ham_key_t key;
        ham_record_t rec;
        memset(&key, 0, sizeof(key));
        memset(&rec, 0, sizeof(rec));

        rec.data=(void *)"123";
        rec.size=(ham_size_t)strlen("123");

        //BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
        BFC_ASSERT_EQUAL(0, ham_new(&db[0]));
        BFC_ASSERT_EQUAL(0, ham_new(&db[1]));
        BFC_ASSERT_EQUAL(0, ham_new(&db[2]));

        BFC_ASSERT_EQUAL(0, ham_env_create(m_env, BFC_OPATH(".test"), 0, 0664));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db[0], 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db[1], 334, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db[2], 335, 0, 0));

        BFC_ASSERT_EQUAL(0, ham_enable_compression(db[0], 3, 0));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(db[1], 8, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db[0], 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db[1], 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db[2], 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db[0], 0));
        BFC_ASSERT_EQUAL(0, ham_close(db[1], 0));
        BFC_ASSERT_EQUAL(0, ham_close(db[2], 0));

        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db[0], 333, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db[1], 334, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_env_open_db(m_env, db[2], 335, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(db[0], 3, 0));
        BFC_ASSERT_EQUAL(0, ham_enable_compression(db[1], 8, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db[0], 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db[1], 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db[2], 0, &key, &rec, 0));

        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, HAM_AUTO_CLEANUP));
        BFC_ASSERT_EQUAL(0, ham_env_delete(m_env));
        BFC_ASSERT_EQUAL(0, ham_delete(db[0]));
        BFC_ASSERT_EQUAL(0, ham_delete(db[1]));
        BFC_ASSERT_EQUAL(0, ham_delete(db[2]));
#endif
    }

};

// deactivated because the unittests test a functionality that was not
// implemented
//BFC_REGISTER_FIXTURE(FilterTest);

