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
#include "../src/device.h"
#include "memtracker.h"
#include "os.hpp"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <cstring>


using namespace bfc;

class DeviceGraphTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    DeviceGraphTest(const char *name="DeviceGraphTest")
    : hamsterDB_fixture(name)
    {
        //if (name)
        //    return;
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(DeviceGraphTest, ConstructDefaultGraphTest);
    }

protected:

public:
    virtual void setup()
    {
        __super::setup();

        (void)os::unlink(BFC_OPATH(".test"));
        ham_set_default_allocator_template(memtracker_new());
    }

    virtual void teardown()
    {
        __super::teardown();
    }

    void ConstructDefaultGraphTest()
    {
    }

};

BFC_REGISTER_FIXTURE(DeviceGraphTest);

