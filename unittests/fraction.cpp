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

#include <stdexcept>
#include <cstring>
#include <float.h>
#include <math.h>
#include <ham/hamsterdb.h>
#include "../src/fraction.h"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

using namespace bfc;

class FractionTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    FractionTest(const char *name="FractionTest")
    :   hamsterDB_fixture(name)
    {
        //if (name)
        //    return;
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(FractionTest, basicTest);
    }

public:

    void basicTest(void)
    {
        ham_fraction_t ret;
        double vut;

        vut = 0.1;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 0.99999997;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = (0x40000000 - 1.0) / (0x40000000 + 1.0);
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 1.0 / 3.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 1.0 / (0x40000000 - 1.0);
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 320.0 / 240.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 6.0 / 7.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 320.0 / 241.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 720.0 / 577.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 2971.0 / 3511.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 3041.0 / 7639.0;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 1.0 / sqrt(2.0);
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
        vut = 3.1415926535897932384626433832795 /* M_PI */;
        to_fract(&ret, vut);
        BFC_ASSERT(fabs(vut - fract2dbl(&ret)) < 1E-9);
    }
};


BFC_REGISTER_FIXTURE(FractionTest);
