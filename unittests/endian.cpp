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

#include "../src/endianswap.h"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

using namespace bfc;

class EndianTest : public hamsterDB_fixture
{
public:
    EndianTest()
    :   hamsterDB_fixture("EndianTest")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(EndianTest, byteswap16);
        BFC_REGISTER_TEST(EndianTest, byteswap32);
        BFC_REGISTER_TEST(EndianTest, byteswap64);
        BFC_REGISTER_TEST(EndianTest, byteswapTwice16);
        BFC_REGISTER_TEST(EndianTest, byteswapTwice32);
        BFC_REGISTER_TEST(EndianTest, byteswapTwice64);
    }

public:
    void byteswap16() {
        BFC_ASSERT_EQUAL(0x3412u,
                             _ham_byteswap16(0x1234u));
        BFC_ASSERT_EQUAL(0xafbcu,
                             _ham_byteswap16(0xbcafu));
        BFC_ASSERT_EQUAL(0x0000u,
                             _ham_byteswap16(0x0000u));
        BFC_ASSERT_EQUAL(0xffffu,
                             _ham_byteswap16(0xffffu));
    }

    void byteswap32() {
        BFC_ASSERT_EQUAL((unsigned int)0x78563412u,
                             _ham_byteswap32(0x12345678u));
        BFC_ASSERT_EQUAL((unsigned int)0xafbc1324u,
                             _ham_byteswap32(0x2413bcafu));
        BFC_ASSERT_EQUAL((unsigned int)0x00000000u,
                             _ham_byteswap32(0x00000000u));
        BFC_ASSERT_EQUAL((unsigned int)0xffffffffu,
                             _ham_byteswap32(0xffffffffu));
    }

    void byteswap64() {
#if defined(HAM_64BIT)   /* [GerH] non-64bit-int platforms have been modified to use 32bit ints only and do not use nor require these macros */
        BFC_ASSERT_EQUAL((unsigned long long)0x3210cba987654321ull,
                             _ham_byteswap64(0x21436587a9cb1032ull));
        BFC_ASSERT_EQUAL((unsigned long long)0xafbc132423abcf09ull,
                             _ham_byteswap64(0x09cfab232413bcafull));
        BFC_ASSERT_EQUAL((unsigned long long)0x0000000000000000ull,
                             _ham_byteswap64(0x0000000000000000ull));
        BFC_ASSERT_EQUAL((unsigned long long)0xffffffffffffffffull,
                             _ham_byteswap64(0xffffffffffffffffull));
#endif
    }

    void byteswapTwice16() {
        unsigned short swapped;
        unsigned short orig;
        unsigned short d[]={0x1234u, 0xafbcu, 0u, 0xffffu};
        for (int i=0; i<4; i++) {
            orig=d[i];
            swapped=_ham_byteswap16(orig);
            BFC_ASSERT_EQUAL(orig,
                        (unsigned short)_ham_byteswap16(swapped));
        }
    }

    void byteswapTwice32() {
        unsigned int swapped;
        unsigned int orig;
        unsigned int d[]={0x12345678u, 0xafbc2389u, 0u, 0xffffffffu};
        for (int i=0; i<4; i++) {
            orig=d[i];
            swapped=_ham_byteswap32(orig);
            BFC_ASSERT_EQUAL(orig,
                        (unsigned int)_ham_byteswap32(swapped));
        }
    }

    void byteswapTwice64() {
#if defined(HAM_64BIT)
        unsigned long long swapped;
        unsigned long long orig;
        unsigned long long d[]={0x12345678abcd0123ull,
                0xafbc238919475868ull, 0ull, 0xffffffffffffffffull};
        for (int i=0; i<4; i++) {
            orig=d[i];
            swapped=_ham_byteswap64(orig);
            BFC_ASSERT_EQUAL(orig,
                        (unsigned long long)_ham_byteswap64(swapped));
        }
#endif
    }

};

BFC_REGISTER_FIXTURE(EndianTest);
