/**
 * Copyright (C) 2005-2008 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

#include <assert.h>
#include <boost/thread/thread.hpp>
#include "../src/config.h"

#include "../src/internal_fwd_decl.h"
#include "../src/db.h"

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

using namespace bfc;

class ThreadingTest : public hamsterDB_fixture
{
public:
    ThreadingTest()
    :   hamsterDB_fixture("ThreadingTest")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(ThreadingTest, tlsTest);
    }

public:
    static boost::thread_specific_ptr<boost::thread::id> ms_tls;

    void tlsTest(void);
};

static void tlsThread(void) {
    if (!ThreadingTest::ms_tls.get())
        ThreadingTest::ms_tls.reset(new boost::thread::id());
    *ThreadingTest::ms_tls.get()=boost::this_thread::get_id();
#if defined(WIN32) || defined(_WIN32)
	Sleep(1000);
#else
	sleep(1);
#endif
	assert(*ThreadingTest::ms_tls.get()==boost::this_thread::get_id());
}

boost::thread_specific_ptr<boost::thread::id> ThreadingTest::ms_tls;

void ThreadingTest::tlsTest(void) {
    Thread thread0(tlsThread);
    Thread thread1(tlsThread);
    Thread thread2(tlsThread);
    Thread thread3(tlsThread);
    Thread thread4(tlsThread);
    Thread thread5(tlsThread);

    thread0.join();
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
}

BFC_REGISTER_FIXTURE(ThreadingTest);
