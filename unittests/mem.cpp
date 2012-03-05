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

#include "../src/mem.h"
#include "memtracker.h"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>


using namespace bfc;

#undef free
#undef realloc

class MemoryTest : public hamsterDB_fixture
{
public:
    MemoryTest()
    :   hamsterDB_fixture("MemoryTest")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(MemoryTest, simpleTest);
        BFC_REGISTER_TEST(MemoryTest, trackingTest);
        BFC_REGISTER_TEST(MemoryTest, trackingTest2);
        BFC_REGISTER_TEST(MemoryTest, freeNullTest);
        BFC_REGISTER_TEST(MemoryTest, reallocTest);
    }

public:
    void simpleTest() {
        void *p;
        mem_allocator_t *alloc=ham_default_allocator_new();
        p=alloc->alloc(alloc, __FILE__, __LINE__, 128);
        BFC_ASSERT(p);
        /* yuck! MSVC defines free() as a macro, therefore the next line
         * would fail to compile */
#undef free
        alloc->free(alloc, __FILE__, __LINE__, p);
        alloc->destroy(&alloc);
    }

    /*
     * TODO ham_mem_free_null is missing
     */

    void trackingTest() {
        void *p;
        mem_allocator_t *alloc=memtracker_new();
        p=alloc->alloc(alloc, __FILE__, __LINE__, 128);
        BFC_ASSERT(p);
        alloc->free(alloc, __FILE__, __LINE__, p);
        BFC_ASSERT(!memtracker_get_leaks(alloc));
        alloc->destroy(&alloc);
    }

    void trackingTest2() {
        void *p[3];
        mem_allocator_t *alloc=memtracker_new();
        p[0]=alloc->alloc(alloc, __FILE__, __LINE__, 10);
        BFC_ASSERT(p[0]);
        p[1]=alloc->alloc(alloc, __FILE__, __LINE__, 12);
        BFC_ASSERT(p[1]);
        p[2]=alloc->alloc(alloc, __FILE__, __LINE__, 14);
        BFC_ASSERT(p[2]);
        alloc->free(alloc, __FILE__, __LINE__, p[0]);
        BFC_ASSERT(memtracker_get_leaks(alloc)==26);
        alloc->free(alloc, __FILE__, __LINE__, p[1]);
        BFC_ASSERT(memtracker_get_leaks(alloc)==14);
        alloc->free(alloc, __FILE__, __LINE__, p[2]);
        BFC_ASSERT(memtracker_get_leaks(alloc)==0);
        alloc->destroy(&alloc);
    }

    void freeNullTest() {
        void *p=0;
        mem_allocator_t *alloc=memtracker_new();
        try {
            alloc->free(alloc, __FILE__, __LINE__, p);
        }
        catch (std::logic_error e) {
            BFC_ASSERT(memtracker_get_leaks(alloc)==0);
            alloc->destroy(&alloc);
            return;
        }

        BFC_ASSERT(!"should not be here");
    }

    void reallocTest(void) {
        void *p=0;
        mem_allocator_t *alloc=memtracker_new();

        p=allocator_realloc(alloc, p, 15);
        BFC_ASSERT(p!=0);
        allocator_free(alloc, p);
        BFC_ASSERT_EQUAL(0lu, memtracker_get_leaks(alloc));

        p=allocator_realloc(alloc, 0, 15);
        BFC_ASSERT(p!=0);
        p=allocator_realloc(alloc, p, 30);
        BFC_ASSERT(p!=0);
        allocator_free(alloc, p);
        BFC_ASSERT_EQUAL(0lu, memtracker_get_leaks(alloc));
    }

};

BFC_REGISTER_FIXTURE(MemoryTest);
