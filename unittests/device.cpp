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
#include "../src/mem.h"
#include "../src/db.h"
#include "../src/env.h"
<<<<<<< HEAD
=======
#include "../src/device.h"
#include "memtracker.h"
>>>>>>> flash-bang-grenade
#include "os.hpp"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <cstring>


using namespace bfc;

class DeviceTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    DeviceTest(bool inmemory=false, const char *name="DeviceTest")
    : hamsterDB_fixture(name),
<<<<<<< HEAD
        m_db(0), m_inmemory(inmemory), m_dev(0)
=======
        m_db(NULL), m_env(NULL), m_inmemory(inmemory), m_dev(NULL)
>>>>>>> flash-bang-grenade
    {
        //if (name)
        //    return;
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(DeviceTest, newDeleteTest);
        BFC_REGISTER_TEST(DeviceTest, createCloseTest);
        BFC_REGISTER_TEST(DeviceTest, openCloseTest);
        BFC_REGISTER_TEST(DeviceTest, pagesizeTest);
        BFC_REGISTER_TEST(DeviceTest, allocTest);
        BFC_REGISTER_TEST(DeviceTest, allocFreeTest);
        BFC_REGISTER_TEST(DeviceTest, flushTest);
        BFC_REGISTER_TEST(DeviceTest, mmapUnmapTest);
        BFC_REGISTER_TEST(DeviceTest, readWriteTest);
        BFC_REGISTER_TEST(DeviceTest, readWritePageTest);
    }

protected:
    ham_db_t *m_db;
    ham_env_t *m_env;
    mem_allocator_t *m_alloc;
    ham_bool_t m_inmemory;
<<<<<<< HEAD
    Device *m_dev;
=======
    ham_device_t *m_dev;
>>>>>>> flash-bang-grenade

public:
    virtual void setup()
    {
        __super::setup();

        (void)os::unlink(BFC_OPATH(".test"));

<<<<<<< HEAD
=======
        ham_set_default_allocator_template(m_alloc = memtracker_new());
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                        m_inmemory ? HAM_IN_MEMORY_DB : 0, 0644));
<<<<<<< HEAD
        m_env=ham_get_env(m_db);
        m_dev=((Environment *)m_env)->get_device();
    }
    
=======
        m_env = db_get_env(m_db);
        m_dev=env_get_device(m_env);
    }

>>>>>>> flash-bang-grenade
    virtual void teardown()
    {
        __super::teardown();

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_delete(m_db));
<<<<<<< HEAD
=======
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));
>>>>>>> flash-bang-grenade
    }

    void newDeleteTest()
    {
    }

    void createCloseTest()
    {
        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        if (!m_inmemory) {
            BFC_ASSERT_EQUAL(0, m_dev->close());
            BFC_ASSERT_EQUAL(0, m_dev->is_open());
            BFC_ASSERT_EQUAL(0, m_dev->open(BFC_OPATH(".test"), 0));
            BFC_ASSERT_EQUAL(1, m_dev->is_open());
        }
    }

    void openCloseTest()
    {
        if (!m_inmemory) {
            BFC_ASSERT_EQUAL(1, m_dev->is_open());
            BFC_ASSERT_EQUAL(0, m_dev->close());
            BFC_ASSERT_EQUAL(0, m_dev->is_open());
            BFC_ASSERT_EQUAL(0, m_dev->open(BFC_OPATH(".test"), 0));
            BFC_ASSERT_EQUAL(1, m_dev->is_open());
            BFC_ASSERT_EQUAL(0, m_dev->close());
            BFC_ASSERT_EQUAL(0, m_dev->is_open());
            BFC_ASSERT_EQUAL(0, m_dev->open(BFC_OPATH(".test"), 0));
            BFC_ASSERT_EQUAL(1, m_dev->is_open());
        }
    }

    void pagesizeTest()
    {
        ham_size_t cps;
        ham_size_t ps=m_dev->get_pagesize();
        BFC_ASSERT(ps!=0);
        BFC_ASSERT(ps%1024==0);
        cps=m_dev->get_pagesize();
        BFC_ASSERT(cps!=0);
        BFC_ASSERT(cps % DB_CHUNKSIZE == 0);
        BFC_ASSERT_EQUAL(cps, ps);
    }

    void allocTest()
    {
        int i;
        ham_offset_t address;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        for (i=0; i<10; i++) {
<<<<<<< HEAD
            BFC_ASSERT_EQUAL(0, m_dev->alloc(1024, &address));
            BFC_ASSERT_EQUAL((((Environment *)m_env)->get_pagesize()*2)+1024*i, address);
=======
            BFC_ASSERT_EQUAL(0, m_dev->alloc(m_dev, 1024, &address, &info));
            BFC_ASSERT_EQUAL((env_get_pagesize(m_env)*2)+1024*i, address);
>>>>>>> flash-bang-grenade
        }
    }

    void allocFreeTest()
    {
<<<<<<< HEAD
        Page page((Environment *)m_env);
        page.set_db((Database *)m_db);

        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        BFC_ASSERT_EQUAL(0, m_dev->alloc_page(&page));
        BFC_ASSERT(page.get_pers()!=0);
        BFC_ASSERT_EQUAL(0, m_dev->free_page(&page));
=======
        ham_page_t page;
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;

        memset(&page, 0, sizeof(page));
        page_set_owner(&page, m_db);

        BFC_ASSERT_EQUAL(1, m_dev->is_open(m_dev));
        BFC_ASSERT_EQUAL(0, m_dev->alloc_page(m_dev, &page,
                    env_get_pagesize(m_env), &info));
        BFC_ASSERT(page_get_pers(&page)!=0);
        BFC_ASSERT_EQUAL(0, m_dev->free_page(m_dev, &page));
>>>>>>> flash-bang-grenade
    }

    void flushTest()
    {
        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        BFC_ASSERT_EQUAL(0, m_dev->flush());
        BFC_ASSERT_EQUAL(1, m_dev->is_open());
    }

    void mmapUnmapTest()
    {
        int i;
        Page pages[10];
        ham_size_t ps=m_dev->get_pagesize();
        ham_u8_t *temp=(ham_u8_t *)malloc(ps);

        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        BFC_ASSERT_EQUAL(0, m_dev->truncate(ps*10));
        for (i=0; i<10; i++) {
<<<<<<< HEAD
            memset(&pages[i], 0, sizeof(Page));
            pages[i].set_db((Database *)m_db);
            pages[i].set_self(i*ps);
            BFC_ASSERT_EQUAL(0, m_dev->read_page(&pages[i]));
=======
            memset(&pages[i], 0, sizeof(ham_page_t));
            page_set_owner(&pages[i], m_db);
            page_set_self(&pages[i], i*ps);
            BFC_ASSERT_EQUAL(0, m_dev->read_page(m_dev, &pages[i],
                        env_get_pagesize(m_env)));
>>>>>>> flash-bang-grenade
        }
        for (i=0; i<10; i++)
            memset(pages[i].get_pers(), i, ps);
        for (i=0; i<10; i++) {
            BFC_ASSERT_EQUAL(0, m_dev->write_page(&pages[i]));
        }
        for (i=0; i<10; i++) {
            ham_u8_t *buffer;
            memset(temp, i, ps);
            BFC_ASSERT_EQUAL(0, m_dev->free_page(&pages[i]));

<<<<<<< HEAD
            BFC_ASSERT_EQUAL(0, m_dev->read_page(&pages[i]));
            buffer=(ham_u8_t *)pages[i].get_pers();
=======
            BFC_ASSERT_EQUAL(0, m_dev->read_page(m_dev, &pages[i],
                        env_get_pagesize(m_env)));
            buffer=(ham_u8_t *)page_get_pers(&pages[i]);
>>>>>>> flash-bang-grenade
            BFC_ASSERT_EQUAL(0, memcmp(buffer, temp, ps));
        }
        for (i=0; i<10; i++) {
            BFC_ASSERT_EQUAL(0, m_dev->free_page(&pages[i]));
        }
        free(temp);
    }

    void readWriteTest()
    {
        int i;
        ham_u8_t *buffer[10]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        ham_size_t ps=m_dev->get_pagesize();
        ham_u8_t *temp=(ham_u8_t *)malloc(ps);

        m_dev->set_flags(HAM_DISABLE_MMAP);

        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        BFC_ASSERT_EQUAL(0, m_dev->truncate(ps*10));
        for (i=0; i<10; i++) {
            buffer[i]=(ham_u8_t *)malloc(ps);
            BFC_ASSERT_EQUAL(0,
                    m_dev->read(i*ps, buffer[i], ps));
        }
        for (i=0; i<10; i++)
            memset(buffer[i], i, ps);
        for (i=0; i<10; i++) {
            BFC_ASSERT_EQUAL(0,
<<<<<<< HEAD
                    m_dev->write(i*ps, buffer[i], ps));
        }
        for (i=0; i<10; i++) {
            BFC_ASSERT_EQUAL(0,
                    m_dev->read(i*ps, buffer[i], ps));
=======
                    m_dev->write(m_dev, i*ps, buffer[i], ps));
        }
        for (i=0; i<10; i++) {
            BFC_ASSERT_EQUAL(0,
                    m_dev->read(m_dev, i*ps, buffer[i], ps));
>>>>>>> flash-bang-grenade
            memset(temp, i, ps);
            BFC_ASSERT_EQUAL(0, memcmp(buffer[i], temp, ps));
            free(buffer[i]);
        }
        free(temp);
    }

    void readWritePageTest()
    {
        int i;
        Page *pages[2];
        ham_size_t ps=m_dev->get_pagesize();

        m_dev->set_flags(HAM_DISABLE_MMAP);

        BFC_ASSERT_EQUAL(1, m_dev->is_open());
        BFC_ASSERT_EQUAL(0, m_dev->truncate(ps*2));
        for (i=0; i<2; i++) {
<<<<<<< HEAD
            BFC_ASSERT((pages[i]=new Page((Environment *)m_env)));
            pages[i]->set_self(ps*i);
            BFC_ASSERT_EQUAL(0, m_dev->read_page(pages[i]));
        }
        for (i=0; i<2; i++) {
            BFC_ASSERT(pages[i]->get_flags()&Page::NPERS_MALLOC);
            memset(pages[i]->get_pers(), i+1, ps);
            BFC_ASSERT_EQUAL(0, m_dev->write_page(pages[i]));
            BFC_ASSERT_EQUAL(0, pages[i]->free());
            delete pages[i];
=======
            BFC_ASSERT((pages[i]=page_new(m_env)));
            page_set_self(pages[i], ps*i);
            BFC_ASSERT_EQUAL(0, m_dev->read_page(m_dev, pages[i], 0));
        }
        for (i=0; i<2; i++) {
            BFC_ASSERT(page_get_npers_flags(pages[i])&PAGE_NPERS_MALLOC);
            memset(page_get_pers(pages[i]), i+1, ps);
            BFC_ASSERT_EQUAL(0,
                    m_dev->write_page(m_dev, pages[i]));
            BFC_ASSERT_EQUAL(0, page_free(pages[i]));
            page_delete(pages[i]);
>>>>>>> flash-bang-grenade
        }

        for (i=0; i<2; i++) {
            char temp[1024];
            memset(temp, i+1, sizeof(temp));
<<<<<<< HEAD
            BFC_ASSERT((pages[i]=new Page((Environment *)m_env)));
            pages[i]->set_self(ps*i);
            BFC_ASSERT_EQUAL(0, m_dev->read_page(pages[i]));
            BFC_ASSERT_EQUAL(0,
                    memcmp(pages[i]->get_pers(), temp, sizeof(temp)));
            BFC_ASSERT_EQUAL(0, pages[i]->free());
            delete pages[i];
=======
            BFC_ASSERT((pages[i]=page_new(m_env)));
            page_set_self(pages[i], ps*i);
            BFC_ASSERT_EQUAL(0, m_dev->read_page(m_dev, pages[i], ps));
            BFC_ASSERT_EQUAL(0,
                    memcmp(page_get_pers(pages[i]), temp, sizeof(temp)));
            BFC_ASSERT_EQUAL(0, page_free(pages[i]));
            page_delete(pages[i]);
>>>>>>> flash-bang-grenade
        }
    }

};

class InMemoryDeviceTest : public DeviceTest
{
public:
    InMemoryDeviceTest()
    :   DeviceTest(true, "InMemoryDeviceTest")
    {
        clear_tests(); // don't inherit tests
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(InMemoryDeviceTest, newDeleteTest);
        BFC_REGISTER_TEST(InMemoryDeviceTest, createCloseTest);
        BFC_REGISTER_TEST(InMemoryDeviceTest, openCloseTest);
        BFC_REGISTER_TEST(InMemoryDeviceTest, pagesizeTest);
        //BFC_REGISTER_TEST(InMemoryDeviceTest, allocTest);
        BFC_REGISTER_TEST(InMemoryDeviceTest, allocFreeTest);
        BFC_REGISTER_TEST(InMemoryDeviceTest, flushTest);
        //BFC_REGISTER_TEST(InMemoryDeviceTest, mmapUnmapTest);
        //BFC_REGISTER_TEST(InMemoryDeviceTest, readWriteTest);
        //BFC_REGISTER_TEST(InMemoryDeviceTest, readWritePageTest);
    }

};

BFC_REGISTER_FIXTURE(DeviceTest);
BFC_REGISTER_FIXTURE(InMemoryDeviceTest);

