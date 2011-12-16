﻿/**
 * Copyright (C) 2005-2011 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * See file COPYING.GPL2 and COPYING.GPL3 for License information.
 *
 */

using Hamster;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace Unittests
{
    /// <summary>
    ///This is a test class for TransactionTest and is intended
    ///to contain all TransactionTest Unit Tests
    ///</summary>
    [TestClass()]
    [DeploymentItem("..\\win32\\out\\dll_debug\\hamsterdb-2.0.0.rc4.dll")]
    public class TransactionTest
    {
        private Database db;

        [TestInitialize()]
        public void MyTestInitialize()
        {
            db = new Database();
            db.Create("test.db", HamConst.HAM_ENABLE_TRANSACTIONS);
        }
        
        [TestCleanup()]
        public void MyTestCleanup()
        {
            db.Close(HamConst.HAM_AUTO_CLEANUP);
        }

        [DeploymentItem("HamsterDb-dotnet.dll")]

        [TestMethod()]
        public void AbortTest()
        {
            Transaction t = db.Begin();
            t.Abort();
        }

        [TestMethod()]
        public void CommitTest()
        {
            Transaction t = db.Begin();
            t.Commit();
        }

        [TestMethod()]
        public void FailLimitsReachedTest()
        {
            Transaction t1 = db.Begin();
            try
            {
                Transaction t2 = db.Begin();
            }
            catch (DatabaseException e)
            {
                Assert.AreEqual(HamConst.HAM_LIMITS_REACHED, e.ErrorCode);
            }
            t1.Commit();
            Transaction t3 = db.Begin();
            t3.Commit();
        }
        /*
        [TestMethod()]
        public void InsertFindCommitTest() {
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Transaction t=db.Begin();
            db.Insert(t, k, r);
            db.Find(t, k);
            try {
                db.Find(k);
            }
            catch (DatabaseException e) {
                Assert.AreEqual(HamConst.HAM_KEY_NOT_FOUND, e.ErrorCode);
            }
            t.Commit();
            db.Find(k);
        }

        [TestMethod()]
        public void InsertFindAbortTest() {
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Transaction t=db.Begin();
            db.Insert(t, k, r);
            db.Find(t, k);
            t.Abort();
            try {
                db.Find(k);
            }
            catch (DatabaseException e) {
                Assert.AreEqual(HamConst.HAM_KEY_NOT_FOUND, e.ErrorCode);
            }
        }

        [TestMethod()]
        public void EraseFindCommitTest() {
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Transaction t=db.Begin();
            db.Insert(t, k, r);
            db.Find(t, k);
            try {
                db.Erase(k);
            }
            catch (DatabaseException e) {
                Assert.AreEqual(HamConst.HAM_KEY_NOT_FOUND, e.ErrorCode);
            }
            t.Commit();
            db.Erase(k);
        }


        [TestMethod()]
        public void CursorTest() {
            Transaction t=db.Begin();
            Cursor c = new Cursor(db, t);
            byte[] k = new byte[5];
            byte[] r = new byte[5];
            c.Insert(k, r);
            db.Find(t, k);
            c.Close();
            t.Commit();
            db.Find(k);
        }

        [TestMethod()]
        public void GetKeyCountTest() {
            Transaction t=db.Begin();

            byte[] k = new byte[5];
            byte[] r = new byte[5];
            Assert.AreEqual(0, db.GetKeyCount());
            db.Insert(t, k, r);
            Assert.AreEqual(1, db.GetKeyCount(t));
            Assert.AreEqual(0, db.GetKeyCount());
            k[0] = 1;
            db.Insert(k, r);
            Assert.AreEqual(2, db.GetKeyCount(t));
            Assert.AreEqual(0, db.GetKeyCount());
            t.Commit();
            Assert.AreEqual(2, db.GetKeyCount());
        }
     */

    }
}
