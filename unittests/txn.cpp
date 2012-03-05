<<<<<<< HEAD
/**
 * Copyright (C) 2005-2012 Christoph Rupp (chris@crupp.de).
=======
/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
>>>>>>> flash-bang-grenade
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
#include "../src/error.h"
#include "../src/freelist.h"
#include "../src/mem.h"
#include "../src/os.h"
<<<<<<< HEAD
=======
#include "../src/page.h"
#include "../src/txn.h"
#include "memtracker.h"
>>>>>>> flash-bang-grenade

#include "bfc-testsuite.hpp"
#include "hamster_fixture.hpp"

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>


using namespace bfc;

class TxnTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    TxnTest()
    : hamsterDB_fixture("TxnTest")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(TxnTest, checkIfLogCreatedTest);
        BFC_REGISTER_TEST(TxnTest, beginCommitTest);
        BFC_REGISTER_TEST(TxnTest, multipleBeginCommitTest);
        BFC_REGISTER_TEST(TxnTest, beginAbortTest);
        BFC_REGISTER_TEST(TxnTest, txnStructureTest);
        BFC_REGISTER_TEST(TxnTest, txnTreeStructureTest);
        BFC_REGISTER_TEST(TxnTest, txnMultipleTreesTest);
        BFC_REGISTER_TEST(TxnTest, txnNodeStructureTest);
        BFC_REGISTER_TEST(TxnTest, txnNodeCreatedOnceTest);
        BFC_REGISTER_TEST(TxnTest, txnMultipleNodesTest);
        BFC_REGISTER_TEST(TxnTest, txnOpStructureTest);
        BFC_REGISTER_TEST(TxnTest, txnMultipleOpsTest);
        BFC_REGISTER_TEST(TxnTest, txnInsertConflict1Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertConflict2Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertConflict3Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertConflict4Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertConflict5Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFind1Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFind2Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFind3Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFind4Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFind5Test);
        //BFC_REGISTER_TEST(TxnTest, txnPartialInsertFindTest);
        BFC_REGISTER_TEST(TxnTest, txnInsertFindErase1Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFindErase2Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFindErase3Test);
        BFC_REGISTER_TEST(TxnTest, txnInsertFindErase4Test);
    }

protected:
    ham_db_t *m_db;
    Database *m_dbp;
    ham_env_t *m_env;
<<<<<<< HEAD
=======
    mem_allocator_t *m_alloc;
>>>>>>> flash-bang-grenade

public:
    virtual void setup()
    {
        __super::setup();

<<<<<<< HEAD
=======
        ham_set_default_allocator_template(m_alloc = memtracker_new());

>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));
        BFC_ASSERT_EQUAL(0, ham_env_new(&m_env));
<<<<<<< HEAD

        BFC_ASSERT_EQUAL(0,
                ham_env_create(m_env, BFC_OPATH(".test"),
                    HAM_ENABLE_DUPLICATES
                        |HAM_ENABLE_RECOVERY
                        |HAM_ENABLE_TRANSACTIONS, 0664));
=======
        //env_set_allocator(m_env, (mem_allocator_t *)m_alloc);

        BFC_ASSERT_EQUAL(0,
                ham_env_create(m_env, BFC_OPATH(".test"),
                    HAM_ENABLE_RECOVERY|HAM_ENABLE_TRANSACTIONS, 0664));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(m_env, m_db, 13, 0, 0));
        m_dbp=(Database *)m_db;
    }
<<<<<<< HEAD
    
=======

>>>>>>> flash-bang-grenade
    virtual void teardown()
    {
        __super::teardown();

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(m_env, 0));
        ham_delete(m_db);
        ham_env_delete(m_env);
<<<<<<< HEAD
=======
        BFC_ASSERT_EQUAL((unsigned long)0, memtracker_get_leaks(ham_get_default_allocator_template()));
>>>>>>> flash-bang-grenade
    }

    void checkIfLogCreatedTest(void)
    {
<<<<<<< HEAD
        BFC_ASSERT(((Environment *)m_env)->get_log()!=0);
        BFC_ASSERT(m_dbp->get_rt_flags()&HAM_ENABLE_RECOVERY);
=======
        BFC_ASSERT_NOTNULL(env_get_log(m_env));
        BFC_ASSERT_NOTNULL(db_get_rt_flags(m_db)&HAM_ENABLE_RECOVERY);
>>>>>>> flash-bang-grenade
    }

    void beginCommitTest(void)
    {
        ham_txn_t *txn;

<<<<<<< HEAD
        BFC_ASSERT(ham_txn_begin(&txn, m_env, 0, 0, 0)==HAM_SUCCESS);
        BFC_ASSERT(ham_txn_commit(txn, 0)==HAM_SUCCESS);
=======
        BFC_ASSERT_EQUAL(ham_txn_begin(&txn, m_db, 0), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(ham_txn_commit(txn, 0), HAM_SUCCESS);
>>>>>>> flash-bang-grenade
    }

    void multipleBeginCommitTest(void)
    {
        ham_txn_t *txn1, *txn2, *txn3;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn3, m_env, 0, 0, 0));

        BFC_ASSERT_EQUAL((ham_txn_t *)0, txn_get_older(txn1));
        BFC_ASSERT_EQUAL(txn2, txn_get_newer(txn1));

        BFC_ASSERT_EQUAL(txn1, txn_get_older(txn2));
        BFC_ASSERT_EQUAL(txn3, txn_get_newer(txn2));

        BFC_ASSERT_EQUAL(txn2, txn_get_older(txn3));
        BFC_ASSERT_EQUAL((ham_txn_t *)0, txn_get_newer(txn3));

        /* have to commit the txns in the same order as they were created,
         * otherwise env_flush_committed_txns() will not flush the oldest
         * transaction */
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));

        BFC_ASSERT_EQUAL((ham_txn_t *)0, txn_get_older(txn2));
        BFC_ASSERT_EQUAL(txn3, txn_get_newer(txn2));
        BFC_ASSERT_EQUAL(txn2, txn_get_older(txn3));
        BFC_ASSERT_EQUAL((ham_txn_t *)0, txn_get_newer(txn3));

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));

        BFC_ASSERT_EQUAL((ham_txn_t *)0, txn_get_older(txn3));
        BFC_ASSERT_EQUAL((ham_txn_t *)0, txn_get_newer(txn3));

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn3, 0));
    }

    void beginAbortTest(void)
    {
        ham_txn_t *txn;

<<<<<<< HEAD
        BFC_ASSERT(ham_txn_begin(&txn, m_env, 0, 0, 0)==HAM_SUCCESS);
        BFC_ASSERT(ham_txn_abort(txn, 0)==HAM_SUCCESS);
=======
        BFC_ASSERT_EQUAL(ham_txn_begin(&txn, m_db, 0), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(ham_txn_abort(txn, 0), HAM_SUCCESS);
>>>>>>> flash-bang-grenade
    }

    void txnStructureTest(void)
    {
        ham_txn_t *txn;

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(m_env, txn_get_env(txn));
=======
        BFC_ASSERT_EQUAL(ham_txn_begin(&txn, m_db, 0), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(txn_get_env(txn), m_env);
        BFC_ASSERT_NULL(txn_get_pagelist(txn));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL((ham_u64_t)1, txn_get_id(txn));

        txn_set_flags(txn, 0x99);
        BFC_ASSERT_EQUAL((ham_u32_t)0x99, txn_get_flags(txn));

<<<<<<< HEAD
=======
        txn_set_pagelist(txn, (ham_page_t *)0x13);
        BFC_ASSERT_EQUAL(txn_get_pagelist(txn), (ham_page_t *)0x13);
        txn_set_pagelist(txn, 0);

>>>>>>> flash-bang-grenade
        txn_set_log_desc(txn, 4);
        BFC_ASSERT_EQUAL(4, txn_get_log_desc(txn));
        txn_set_log_desc(txn, 0);

<<<<<<< HEAD
        txn_set_oldest_op(txn, (txn_op_t *)2);
        BFC_ASSERT_EQUAL((txn_op_t *)2, txn_get_oldest_op(txn));
        txn_set_oldest_op(txn, (txn_op_t *)0);

        txn_set_newest_op(txn, (txn_op_t *)2);
        BFC_ASSERT_EQUAL((txn_op_t *)2, txn_get_newest_op(txn));
        txn_set_newest_op(txn, (txn_op_t *)0);

        txn_set_newer(txn, (ham_txn_t *)1);
        BFC_ASSERT_EQUAL((ham_txn_t *)1, txn_get_newer(txn));
        txn_set_newer(txn, (ham_txn_t *)0);

        txn_set_older(txn, (ham_txn_t *)3);
        BFC_ASSERT_EQUAL((ham_txn_t *)3, txn_get_older(txn));
        txn_set_older(txn, (ham_txn_t *)0);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======
        BFC_ASSERT_NULL(txn_get_pagelist(txn));
        BFC_ASSERT_EQUAL(ham_txn_commit(txn, 0), HAM_SUCCESS);
>>>>>>> flash-bang-grenade
    }

    void txnTreeStructureTest(void)
    {
        ham_txn_t *txn;
        txn_optree_t *tree;

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        tree=m_dbp->get_optree();
        BFC_ASSERT(tree!=0);

        txn_optree_set_db(tree, (Database *)1);
        BFC_ASSERT_EQUAL((Database *)1, txn_optree_get_db(tree));
        txn_optree_set_db(tree, m_dbp);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======
        page=page_new(m_env);
        BFC_ASSERT_NOTNULL(page);
        page_set_self(page, 0x12345);
        page_set_pers(page, &pers);

        BFC_ASSERT_EQUAL(ham_txn_begin(&txn, m_db, 0), HAM_SUCCESS);
        BFC_ASSERT_NULL(txn_get_page(txn, 0x12345));
        BFC_ASSERT_EQUAL(txn_add_page(txn, page, 0), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(txn_add_page(txn, page, 1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(txn_get_page(txn, 0x12345), page);

        BFC_ASSERT_EQUAL(ham_txn_commit(txn, 0), HAM_SUCCESS);

        page_set_pers(page, 0);
        // page_delete(page); - will be deleted in ham_close()
>>>>>>> flash-bang-grenade
    }

    void txnTreeCreatedOnceTest(void)
    {
        ham_txn_t *txn;
        txn_optree_t *tree, *tree2;

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        tree=m_dbp->get_optree();
        BFC_ASSERT(tree!=0);
        tree2=m_dbp->get_optree();
        BFC_ASSERT_EQUAL(tree, tree2);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
    }

    void txnMultipleTreesTest(void)
    {
        ham_db_t *db2, *db3;
        ham_txn_t *txn;
        txn_optree_t *tree1, *tree2, *tree3;

        BFC_ASSERT_EQUAL(0, ham_new(&db2));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db2, 14, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_new(&db3));
        BFC_ASSERT_EQUAL(0, ham_env_create_db(m_env, db3, 15, 0, 0));

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        tree1=m_dbp->get_optree();
        tree2=((Database *)db2)->get_optree();
        tree3=((Database *)db3)->get_optree();
        BFC_ASSERT(tree1!=0);
        BFC_ASSERT(tree2!=0);
        BFC_ASSERT(tree3!=0);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db2, 0));
        ham_delete(db2);
        BFC_ASSERT_EQUAL(0, ham_close(db3, 0));
        ham_delete(db3);
=======
        page=page_new(m_env);
        BFC_ASSERT_NOTNULL(page);
        page_set_self(page, 0x12345);

        BFC_ASSERT_EQUAL(ham_txn_begin(&txn, m_db, 0), HAM_SUCCESS);
        BFC_ASSERT_NULL(txn_get_page(txn, 0x12345));
        BFC_ASSERT_EQUAL(txn_add_page(txn, page, 0), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(txn_add_page(txn, page, 1), HAM_SUCCESS);
        BFC_ASSERT_EQUAL(txn_get_page(txn, 0x12345), page);
        BFC_ASSERT_EQUAL(0, txn_free_page(txn, page));
        BFC_ASSERT_NOTNULL(page_get_npers_flags(page) & PAGE_NPERS_DELETE_PENDING);

        BFC_ASSERT_EQUAL(ham_txn_abort(txn, 0), HAM_SUCCESS);

        // page_delete(page); - will be deleted in ham_close()
>>>>>>> flash-bang-grenade
    }

    void txnNodeStructureTest(void)
    {
        ham_txn_t *txn;
        txn_optree_t *tree;
        txn_opnode_t *node;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        tree=m_dbp->get_optree();
        node=txn_opnode_create(m_dbp, &key);
        BFC_ASSERT(node!=0);

        txn_opnode_set_db(tree, (Database *)1);
        BFC_ASSERT_EQUAL((Database *)1, txn_opnode_get_db(tree));
        txn_opnode_set_db(tree, m_dbp);

        ham_key_t *k=txn_opnode_get_key(node);
        BFC_ASSERT_EQUAL(k->size, key.size);
        BFC_ASSERT_EQUAL(0, memcmp(k->data, key.data, k->size));

<<<<<<< HEAD
        txn_opnode_set_oldest_op(node, (txn_op_t *)3);
        BFC_ASSERT_EQUAL((txn_op_t *)3, txn_opnode_get_oldest_op(node));
        txn_opnode_set_oldest_op(node, 0);
=======
        // page_delete(page); - will be deleted in ham_close()
    }
>>>>>>> flash-bang-grenade

        txn_opnode_set_newest_op(node, (txn_op_t *)4);
        BFC_ASSERT_EQUAL((txn_op_t *)4, txn_opnode_get_newest_op(node));
        txn_opnode_set_newest_op(node, 0);

<<<<<<< HEAD
        /* need at least one txn_op_t structure in this node, otherwise
         * memory won't be cleaned up correctly */
        (void)txn_opnode_append(txn, node, 0, TXN_OP_INSERT_DUP, 55, &rec);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_txn_begin(&txn2, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_txn_begin(&txn2, m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
>>>>>>> flash-bang-grenade
    }

    void txnNodeCreatedOnceTest(void)
    {
        ham_txn_t *txn;
        txn_opnode_t *node, *node2;
        ham_key_t key1, key2;
        memset(&key1, 0, sizeof(key1));
        key1.data=(void *)"hello";
        key1.size=5;
        memset(&key2, 0, sizeof(key2));
        key2.data=(void *)"world";
        key2.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        node=txn_opnode_create(m_dbp, &key1);
        BFC_ASSERT(node!=0);
        node2=txn_opnode_get(m_dbp, &key1, 0);
        BFC_ASSERT_EQUAL(node, node2);
        node2=txn_opnode_get(m_dbp, &key2, 0);
        BFC_ASSERT_EQUAL((txn_opnode_t *)NULL, node2);
        node2=txn_opnode_create(m_dbp, &key2);
        BFC_ASSERT(node!=node2);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_insert(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0,
                ham_insert(m_db, 0, &key, &rec, 0));
>>>>>>> flash-bang-grenade
    }

    void txnMultipleNodesTest(void)
    {
        ham_txn_t *txn;
        txn_opnode_t *node1, *node2, *node3;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"1111";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        node1=txn_opnode_create(m_dbp, &key);
        BFC_ASSERT(node1!=0);
        key.data=(void *)"2222";
        node2=txn_opnode_create(m_dbp, &key);
        BFC_ASSERT(node2!=0);
        key.data=(void *)"3333";
        node3=txn_opnode_create(m_dbp, &key);
        BFC_ASSERT(node3!=0);

        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_find(m_db, 0, &key, &rec, 0));
>>>>>>> flash-bang-grenade
    }

    void txnOpStructureTest(void)
    {
        ham_txn_t *txn;
        txn_opnode_t *node;
        txn_op_t *op, next;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
<<<<<<< HEAD
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t record;
        memset(&record, 0, sizeof(record));
        record.data=(void *)"world";
        record.size=5;
        memset(&next, 0, sizeof(next));

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        node=txn_opnode_create(m_dbp, &key);
        op=txn_opnode_append(txn, node, 0, TXN_OP_INSERT_DUP, 55, &record);
        BFC_ASSERT(op!=0);

        BFC_ASSERT_EQUAL(TXN_OP_INSERT_DUP, txn_op_get_flags(op));
        txn_op_set_flags(op, 13);
        BFC_ASSERT_EQUAL(13u, txn_op_get_flags(op));
        txn_op_set_flags(op, TXN_OP_INSERT_DUP);

        BFC_ASSERT_EQUAL(55ull, txn_op_get_lsn(op));
        txn_op_set_lsn(op, 23);
        BFC_ASSERT_EQUAL(23ull, txn_op_get_lsn(op));
        txn_op_set_lsn(op, 55);

        BFC_ASSERT_EQUAL((txn_op_t *)0, txn_op_get_next_in_node(op));
        txn_op_set_next_in_node(op, &next);
        BFC_ASSERT_EQUAL(&next, txn_op_get_next_in_node(op));
        txn_op_set_next_in_node(op, 0);

        BFC_ASSERT_EQUAL((txn_op_t *)0, txn_op_get_next_in_txn(op));
        txn_op_set_next_in_txn(op, &next);
        BFC_ASSERT_EQUAL(&next, txn_op_get_next_in_txn(op));
        txn_op_set_next_in_txn(op, 0);

        BFC_ASSERT_EQUAL((txn_cursor_t *)0, txn_op_get_cursors(op));
        txn_op_set_cursors(op, (txn_cursor_t *)0x43);
        BFC_ASSERT_EQUAL((txn_cursor_t *)0x43, txn_op_get_cursors(op));
        txn_op_set_cursors(op, (txn_cursor_t *)0x0);

        txn_free_ops(txn);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_erase(m_db, 0, &key, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_erase(m_db, 0, &key, 0));
>>>>>>> flash-bang-grenade
    }

    void txnMultipleOpsTest(void)
    {
        ham_txn_t *txn;
        txn_opnode_t *node;
        txn_op_t *op1, *op2, *op3;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        node=txn_opnode_create(m_dbp, &key);
        op1=txn_opnode_append(txn, node, 0, TXN_OP_INSERT_DUP, 55, &rec);
        BFC_ASSERT(op1!=0);
        op2=txn_opnode_append(txn, node, 0, TXN_OP_ERASE, 55, &rec);
        BFC_ASSERT(op2!=0);
        op3=txn_opnode_append(txn, node, 0, TXN_OP_NOP, 55, &rec);
        BFC_ASSERT(op3!=0);

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_check_integrity(m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0,
                ham_check_integrity(m_db, 0));
>>>>>>> flash-bang-grenade
    }

    void txnInsertConflict1Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

<<<<<<< HEAD
        /* begin(T1); begin(T2); insert(T1, a); insert(T2, a) -> conflict */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_TXN_CONFLICT,
                    ham_insert(m_db, txn2, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_get_key_count(m_db, 0, 0, &o));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0,
                ham_get_key_count(m_db, 0, 0, &o));
>>>>>>> flash-bang-grenade
    }

    void txnInsertConflict2Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

        /* begin(T1); begin(T2); insert(T1, a); insert(T2, a) -> duplicate */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(HAM_DUPLICATE_KEY,
                    ham_insert(m_db, txn2, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

    void txnInsertConflict3Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

        /* begin(T1); begin(T2); insert(T1, a); commit(T1);
         * insert(T2, a, OW) -> ok */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0,
                    ham_insert(m_db, txn2, &key, &rec, HAM_OVERWRITE));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

<<<<<<< HEAD
    void txnInsertConflict4Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

        /* begin(T1); begin(T2); insert(T1, a); commit(T1);
         * insert(T2, a, DUP) -> ok */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0,
                    ham_insert(m_db, txn2, &key, &rec, HAM_DUPLICATE));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_env_erase_db(m_env, 123, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
>>>>>>> flash-bang-grenade
    }

    void txnInsertConflict5Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

        /* begin(T1); begin(T2); insert(T1, a); abort(T1);
         * insert(T2, a) */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn2, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

    void txnInsertFind1Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));

        /* begin(T1); begin(T2); insert(T1, a); commit(T1); find(T2, a) -> ok */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn2, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));

        BFC_ASSERT_EQUAL(rec.size, rec2.size);
        BFC_ASSERT_EQUAL(0, memcmp(rec.data, rec2.data, rec2.size));
    }

    void txnInsertFind2Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));

        /* begin(T1); begin(T2); insert(T1, a); insert(T2, a) -> conflict */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_TXN_CONFLICT,
                    ham_find(m_db, txn2, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

    void txnInsertFind3Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));

        /* begin(T1); begin(T2); insert(T1, a); commit(T1);
         * commit(T2); find(temp, a) -> ok */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec2, 0));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(rec.size, rec2.size);
        BFC_ASSERT_EQUAL(0, memcmp(rec.data, rec2.data, rec2.size));
    }
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_find(c, &key, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_move(c, 0, 0, HAM_CURSOR_FIRST));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_insert(c, &key, &rec, 0));
>>>>>>> flash-bang-grenade

    void txnInsertFind4Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));

<<<<<<< HEAD
        /* begin(T1); begin(T2); insert(T1, a); abort(T1);
         * find(T2, a) -> fail */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn1, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_find(m_db, txn2, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

    void txnInsertFind5Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        ham_key_t key2;
        memset(&key2, 0, sizeof(key2));
        key2.data=(void *)"world";
        key2.size=5;

        /* begin(T1); begin(T2); insert(T1, a); commit(T1);
         * find(T2, c) -> fail */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn1, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_find(m_db, txn2, &key2, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

#if 0
    void txnPartialInsertFindTest(void)
    {
        ham_txn_t *txn;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"worldworld";
        rec.size=9;
        rec.partial_offset=1;
        rec.partial_size=2;

        /* insert partial record */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, HAM_PARTIAL));
=======
        BFC_ASSERT_EQUAL(0,
                ham_cursor_find(c, &key, 0));

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_overwrite(c, &rec, 0));
        ham_size_t count=0;
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_get_duplicate_count(c, &count, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_clone(c, &clone));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED,
                ham_cursor_erase(c, 0));
        BFC_ASSERT_EQUAL(0, ham_cursor_close(c));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));

        /* and read it back */
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));
        rec2.partial_offset=1;
        rec2.partial_size=2;
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec2, HAM_PARTIAL));

TODO weiter hier - compare record; must be "\0or\0\0\0\0\0\0\0" (ists
aber nicht)
    }
#endif

    void txnInsertFindErase1Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));

        /* begin(T1); begin(T2); insert(T1, a); commit(T1); erase(T2, a);
         * find(T2, a) -> fail */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_erase(m_db, txn2, &key, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_find(m_db, txn2, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_erase(m_db, txn2, &key, 0));
    }

    void txnInsertFindErase2Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));

        /* begin(T1); begin(T2); insert(T1, a); commit(T1); commit(T2);
         * erase(T3, a) -> ok; find(T2, a) -> fail */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(0, ham_erase(m_db, txn2, &key, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_find(m_db, txn2, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_erase(m_db, txn2, &key, 0));
    }

    void txnInsertFindErase3Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;

        /* begin(T1); begin(T2); insert(T1, a); abort(T1); erase(T2, a) -> fail;
         * commit(T2); */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn1, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND, ham_erase(m_db, txn2, &key, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

    void txnInsertFindErase4Test(void)
    {
        ham_txn_t *txn1, *txn2;
        ham_key_t key;
        memset(&key, 0, sizeof(key));
        key.data=(void *)"hello";
        key.size=5;
        ham_record_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.data=(void *)"world";
        rec.size=5;
        ham_record_t rec2;
        memset(&rec2, 0, sizeof(rec2));

        /* begin(T1); begin(T2); insert(T1, a); erase(T1, a); -> ok;
         * commit(T2); */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn1, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn2, m_env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn1, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_erase(m_db, txn1, &key, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_erase(m_db, txn1, &key, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn1, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                    ham_erase(m_db, txn2, &key, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn2, 0));
    }

};

class HighLevelTxnTest : public hamsterDB_fixture
{
    define_super(hamsterDB_fixture);

public:
    HighLevelTxnTest()
    : hamsterDB_fixture("HighLevelTxnTest")
    {
        testrunner::get_instance()->register_fixture(this);
        BFC_REGISTER_TEST(HighLevelTxnTest, noPersistentDatabaseFlagTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, noPersistentEnvironmentFlagTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, cursorStillOpenTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, txnStillOpenTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, clonedCursorStillOpenTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, autoAbortDatabaseTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, autoCommitDatabaseTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, autoAbortEnvironmentTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, autoAbortEnvironment2Test);
        BFC_REGISTER_TEST(HighLevelTxnTest, autoCommitEnvironmentTest);
<<<<<<< HEAD
=======
        BFC_REGISTER_TEST(HighLevelTxnTest, environmentTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, rollbackBigBlobTest);
        // TODO : huge blobs are not reused if a txn is aborted @@@
        BFC_REGISTER_TEST(HighLevelTxnTest, rollbackHugeBlobTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, rollbackNormalBlobTest);
>>>>>>> flash-bang-grenade
        BFC_REGISTER_TEST(HighLevelTxnTest, insertFindCommitTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, insertFindEraseTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, insertFindEraseTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, getKeyCountTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, getKeyCountDupesTest);
        BFC_REGISTER_TEST(HighLevelTxnTest, getKeyCountOverwriteTest);
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
<<<<<<< HEAD
=======
        __super::setup();

        ham_set_default_allocator_template(m_alloc = memtracker_new());
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_new(&m_db));

        m_env = 0;
    }
<<<<<<< HEAD
    
    virtual void teardown()
    {
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        ham_delete(m_db);
=======

    virtual void teardown()
    {
        __super::teardown();

        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        ham_delete(m_db);
        BFC_ASSERT(!memtracker_get_leaks(ham_get_default_allocator_template()));
>>>>>>> flash-bang-grenade
    }

    void noPersistentDatabaseFlagTest(void)
    {
        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
        m_env=ham_get_env(m_db);

        BFC_ASSERT(HAM_ENABLE_TRANSACTIONS&((Database *)m_db)->get_rt_flags());
        BFC_ASSERT(HAM_ENABLE_RECOVERY&((Database *)m_db)->get_rt_flags());
        BFC_ASSERT(DB_ENV_IS_PRIVATE&((Database *)m_db)->get_rt_flags());
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));

        BFC_ASSERT_EQUAL(0, ham_open(m_db, BFC_OPATH(".test"), 0));
        BFC_ASSERT(!(HAM_ENABLE_TRANSACTIONS&((Database *)m_db)->get_rt_flags()));
        BFC_ASSERT(!(HAM_ENABLE_RECOVERY&((Database *)m_db)->get_rt_flags()));
        BFC_ASSERT(DB_ENV_IS_PRIVATE&((Database *)m_db)->get_rt_flags());
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void noPersistentEnvironmentFlagTest(void)
    {
        ham_env_t *env;

        ham_env_new(&env);
<<<<<<< HEAD
=======

>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0,
                ham_env_create(env, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT(HAM_ENABLE_TRANSACTIONS&((Environment *)env)->get_flags());
        BFC_ASSERT(HAM_ENABLE_RECOVERY&((Environment *)env)->get_flags());
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));

        BFC_ASSERT_EQUAL(0, ham_env_open(env, BFC_OPATH(".test"), 0));
        BFC_ASSERT(!(HAM_ENABLE_TRANSACTIONS&((Environment *)env)->get_flags()));
        BFC_ASSERT(!(HAM_ENABLE_RECOVERY&((Environment *)env)->get_flags()));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));

        ham_env_delete(env);
    }

    void cursorStillOpenTest(void)
    {
        ham_txn_t *txn;
        ham_cursor_t *cursor;

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                        HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_cursor_create(m_db, txn, 0, &cursor));
        BFC_ASSERT_EQUAL(HAM_CURSOR_STILL_OPEN, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(HAM_CURSOR_STILL_OPEN, ham_txn_abort(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_cursor_close(cursor));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void txnStillOpenTest(void)
    {
        ham_env_t *env;

        ham_env_new(&env);
        BFC_ASSERT_EQUAL(0,
                ham_env_create(env, BFC_OPATH(".test"),
                        HAM_ENABLE_TRANSACTIONS, 0644));

        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        BFC_ASSERT_EQUAL(0, ham_env_create_db(env, m_db, 1, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_TXN_STILL_OPEN, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));
        ham_env_delete(env);
    }

    void clonedCursorStillOpenTest(void)
    {
        ham_txn_t *txn;
        ham_cursor_t *cursor, *clone;

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                        HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_cursor_create(m_db, txn, 0, &cursor));
        BFC_ASSERT_EQUAL(0, ham_cursor_clone(cursor, &clone));
        BFC_ASSERT_EQUAL(0, ham_cursor_close(cursor));
        BFC_ASSERT_EQUAL(HAM_CURSOR_STILL_OPEN, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(HAM_CURSOR_STILL_OPEN, ham_txn_abort(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_cursor_close(clone));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void autoAbortDatabaseTest(void)
    {
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));

        BFC_ASSERT_EQUAL(0,
                ham_open(m_db, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                        ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void autoCommitDatabaseTest(void)
    {
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                        HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, HAM_TXN_AUTO_COMMIT));

        BFC_ASSERT_EQUAL(0,
                ham_open(m_db, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0,
                ham_find(m_db, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void autoAbortEnvironmentTest(void)
    {
        ham_env_t *env;
        ham_db_t *db1, *db2;
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        ham_env_new(&env);
        ham_new(&db1);
        ham_new(&db2);

        BFC_ASSERT_EQUAL(0,
                ham_env_create(env, BFC_OPATH(".test"),
<<<<<<< HEAD
                        HAM_ENABLE_TRANSACTIONS, 0644));
=======
                    HAM_ENABLE_TRANSACTIONS, 0644));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db2, 2, 0, 0));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));

        BFC_ASSERT_EQUAL(0,
                ham_env_open(env, BFC_OPATH(".test"),
                        HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db, 1, 0, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_find(db, 0, &key, &rec, 0));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db2, 0));

        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db2, 2, 0, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_find(db1, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_find(db2, 0, &key, &rec, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));

        ham_env_delete(env);
        ham_delete(db1);
        ham_delete(db2);
    }

    void autoAbortEnvironment2Test(void)
    {
        ham_env_t *env;
        ham_db_t *db1, *db2;
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        ham_env_new(&env);
        ham_new(&db1);
        ham_new(&db2);

        BFC_ASSERT_EQUAL(0,
                ham_env_create(env, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db2, 2, 0, 0));

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0)); // close the ENV instead of the DB #2, which is now closed implicitly. The close should still imply TXN ABORT

        BFC_ASSERT_EQUAL(0,
                ham_env_open(env, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db2, 2, 0, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_find(db1, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_KEY_NOT_FOUND,
                ham_find(db2, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));
        ham_delete(db1);
        ham_delete(db2);
        ham_env_delete(env);
    }

    void autoCommitEnvironmentTest(void)
    {
        ham_env_t *env;
        ham_db_t *db1, *db2;
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        ham_env_new(&env);
        ham_new(&db1);
        ham_new(&db2);

        BFC_ASSERT_EQUAL(0,
                ham_env_create(env, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db2, 2, 0, 0));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, env, 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, HAM_TXN_AUTO_COMMIT));
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, HAM_TXN_AUTO_COMMIT)); // now close the ENV (and DB2 implicitly) with auto-TXN COMMIT: this time around, the inserted data should persist...
>>>>>>> flash-bang-grenade

        BFC_ASSERT_EQUAL(0,
                ham_env_open(env, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS));
        BFC_ASSERT_EQUAL(0,
<<<<<<< HEAD
                ham_env_open_db(env, db, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_find(db, 0, &key, &rec, 0));
=======
                ham_env_open_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db2, 2, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_find(db1, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0,
                ham_find(db2, 0, &key, &rec, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));
        ham_delete(db1);
        ham_delete(db2);
        ham_env_delete(env);
    }

    void environmentTest(void)
    {
        ham_env_t *env;
        ham_db_t *db1, *db2;
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));

        ham_env_new(&env);
        ham_new(&db1);
        ham_new(&db2);

        BFC_ASSERT_EQUAL(0,
                ham_env_create(env, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_create_db(env, db2, 2, 0, 0));

        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db1, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_close(db1, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(db2, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0)); // now perform a TXN COMMIT explicitly: subsequent close operations should not have any effect any longer: the inserted data must persist...
        BFC_ASSERT_EQUAL(0, ham_close(db2, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));

        BFC_ASSERT_EQUAL(0,
                ham_env_open(env, BFC_OPATH(".test"), HAM_ENABLE_TRANSACTIONS));

        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db1, 1, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_env_open_db(env, db2, 2, 0, 0));
        BFC_ASSERT_EQUAL(0,
                ham_find(db1, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0,
                ham_find(db2, 0, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_env_close(env, 0));

        ham_delete(db1);
        ham_delete(db2);
        ham_env_delete(env);
    }

    void insertFindCommitTest(void)
    {
        ham_status_t st;
        ham_txn_t *txn;
        ham_key_t key;
<<<<<<< HEAD
        ham_record_t rec, rec2;
        ham_u8_t buffer[64];
=======
        ham_record_t rec;
        ham_u8_t buffer[1024*8];
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        //info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;
        info.record = &rec;
        info.key = &key;

>>>>>>> flash-bang-grenade
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));
        ::memset(&rec2, 0, sizeof(rec));
        rec.data=&buffer[0];
        rec.size=sizeof(buffer);

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
<<<<<<< HEAD
                    HAM_ENABLE_TRANSACTIONS, 0644));
        m_env=ham_get_env(m_db);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
=======
                            HAM_ENABLE_TRANSACTIONS, 0644));
        info.env = db_get_env(m_db);
        m_env=db_get_env(m_db);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(HAM_TXN_CONFLICT, ham_find(m_db, 0, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, 0, &key, &rec2, 0));

<<<<<<< HEAD
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void insertFindEraseTest(void)
=======
        ham_offset_t o;
        st = freel_alloc_area(&o, sizeof(buffer), &info);
        BFC_ASSERT_NOTNULL(o);
        BFC_ASSERT_EQUAL(0, st);
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    void rollbackHugeBlobTest(void)
>>>>>>> flash-bang-grenade
    {
        ham_status_t st;
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
<<<<<<< HEAD
        ham_u8_t buffer[64];
=======
        ham_size_t ps=os_get_pagesize();
        ham_u8_t *buffer=(ham_u8_t *)malloc(ps*2);
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        //info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;
        info.record = &rec;
        info.key = &key;

>>>>>>> flash-bang-grenade
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));
        rec.data=&buffer[0];
        rec.size=sizeof(buffer);

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
<<<<<<< HEAD
        m_env=ham_get_env(m_db);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_env, 0, 0, 0));
=======
        info.env = db_get_env(m_db);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_TXN_CONFLICT, ham_erase(m_db, 0, &key, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, ham_erase(m_db, 0, &key, 0));

<<<<<<< HEAD
=======
        ham_offset_t o;
        st = freel_alloc_area(&o, ps*2, &info);
        BFC_ASSERT_NOTNULL(o);
        BFC_ASSERT_EQUAL(0, st);
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
    }

    ham_status_t insert(ham_txn_t *txn, const char *keydata,
                    const char *recorddata, int flags)
    {
        ham_key_t key;
        ham_record_t rec;
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));
        key.data=(void *)keydata;
        key.size=strlen(keydata)+1;
        rec.data=(void *)recorddata;
        rec.size=strlen(recorddata)+1;

        return (ham_insert(m_db, txn, &key, &rec, flags));
    }

    ham_status_t find(ham_txn_t *txn, const char *keydata,
                    const char *recorddata)
    {
        ham_status_t st;
<<<<<<< HEAD
        ham_key_t key;
        ham_record_t rec;
=======
        ham_txn_t *txn;
        ham_key_t key;
        ham_record_t rec;
        ham_u8_t buffer[64];
        dev_alloc_request_info_ex_t info = {0};

        info.db = m_db;
        info.env = m_env;
        info.entire_page = HAM_FALSE;
        info.space_type = PAGE_TYPE_UNKNOWN;
        info.record = &rec;
        info.key = &key;

>>>>>>> flash-bang-grenade
        ::memset(&key, 0, sizeof(key));
        ::memset(&rec, 0, sizeof(rec));
        key.data=(void *)keydata;
        key.size=strlen(keydata)+1;

        st=ham_find(m_db, txn, &key, &rec, 0);
        if (st)
            return (st);
        BFC_ASSERT_EQUAL(0, strcmp(recorddata, (char *)rec.data));
        BFC_ASSERT_EQUAL(rec.size, strlen(recorddata)+1);
        return (0);
    }

    void getKeyCountTest(void)
    {
        ham_txn_t *txn;
        ham_u64_t count;

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
                    HAM_ENABLE_TRANSACTIONS, 0644));
<<<<<<< HEAD
        /* without txn */
        BFC_ASSERT_EQUAL(0, insert(0, "key1", "rec1", 0));
        BFC_ASSERT_EQUAL(0, find(0, "key1", "rec1"));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, 0, 0, &count));
        BFC_ASSERT_EQUAL(1ull, count);

        /* in an active txn */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(1ull, count);
        BFC_ASSERT_EQUAL(0, insert(txn, "key2", "rec2", 0));
        BFC_ASSERT_EQUAL(HAM_TXN_CONFLICT, find(0, "key2", "rec2"));
        BFC_ASSERT_EQUAL(0, find(txn, "key2", "rec2"));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);
        BFC_ASSERT_EQUAL(0, insert(txn, "key2", "rec2", HAM_OVERWRITE));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));
        BFC_ASSERT_EQUAL(0, find(0, "key2", "rec2"));

        /* after commit */
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, 0, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);

        /* in temp. txn */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, insert(txn, "key3", "rec1", 0));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(3ull, count);
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn, 0));

        /* after abort */
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, 0, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);
=======
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_txn_abort(txn, 0));

        ham_offset_t o;
        st = freel_alloc_area(&o, sizeof(buffer), &info);
        BFC_ASSERT_NOTNULL(o);
        BFC_ASSERT_EQUAL(0, st);
        BFC_ASSERT_EQUAL(0, ham_close(m_db, 0));
>>>>>>> flash-bang-grenade
    }

    void getKeyCountDupesTest(void)
    {
        ham_txn_t *txn;
        ham_u64_t count;

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
<<<<<<< HEAD
                    HAM_ENABLE_TRANSACTIONS|HAM_ENABLE_DUPLICATES, 0644));
        /* without txn */
        BFC_ASSERT_EQUAL(0, insert(0, "key1", "rec1", 0));
        BFC_ASSERT_EQUAL(0, insert(0, "key2", "rec1", 0));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, 0, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);

        /* in an active txn */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);
        BFC_ASSERT_EQUAL(0, insert(txn, "key3", "rec3", 0));
        BFC_ASSERT_EQUAL(0, insert(txn, "key3", "rec4", HAM_DUPLICATE));
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(4ull, count);
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, HAM_SKIP_DUPLICATES, &count));
        BFC_ASSERT_EQUAL(3ull, count);
=======
                    HAM_ENABLE_TRANSACTIONS, 0644));
        m_env=db_get_env(m_db);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec2, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED, ham_find(m_db, 0, &key, &rec2, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));

        /* after commit */
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(4ull, count);
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, HAM_SKIP_DUPLICATES, &count));
        BFC_ASSERT_EQUAL(3ull, count);
    }

    void getKeyCountOverwriteTest(void)
    {
        ham_txn_t *txn;
        ham_u64_t count;

        BFC_ASSERT_EQUAL(0,
                ham_create(m_db, BFC_OPATH(".test"),
<<<<<<< HEAD
                    HAM_ENABLE_TRANSACTIONS|HAM_ENABLE_DUPLICATES, 0644));
        /* without txn */
        BFC_ASSERT_EQUAL(0, insert(0, "key1", "rec1", 0));
        BFC_ASSERT_EQUAL(0, insert(0, "key2", "rec1", 0));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, 0, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);

        /* in an active txn */
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, ham_get_env(m_db), 0, 0, 0));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);
        BFC_ASSERT_EQUAL(0, insert(txn, "key2", "rec4", HAM_OVERWRITE));
        BFC_ASSERT_EQUAL(0, ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(2ull, count);
        BFC_ASSERT_EQUAL(0, insert(txn, "key3", "rec3", 0));
        BFC_ASSERT_EQUAL(0, insert(txn, "key3", "rec4", HAM_OVERWRITE));
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(3ull, count);
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, HAM_SKIP_DUPLICATES, &count));
        BFC_ASSERT_EQUAL(3ull, count);
=======
                    HAM_ENABLE_TRANSACTIONS, 0644));
        m_env=db_get_env(m_db);
        BFC_ASSERT_EQUAL(0, ham_txn_begin(&txn, m_db, 0));
        BFC_ASSERT_EQUAL(0, ham_insert(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(0, ham_find(m_db, txn, &key, &rec, 0));
        BFC_ASSERT_EQUAL(HAM_LIMITS_REACHED, ham_erase(m_db, 0, &key, 0));
>>>>>>> flash-bang-grenade
        BFC_ASSERT_EQUAL(0, ham_txn_commit(txn, 0));

        /* after commit */
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, 0, &count));
        BFC_ASSERT_EQUAL(3ull, count);
        BFC_ASSERT_EQUAL(0,
                    ham_get_key_count(m_db, txn, HAM_SKIP_DUPLICATES, &count));
        BFC_ASSERT_EQUAL(3ull, count);
    }

};

BFC_REGISTER_FIXTURE(TxnTest);
BFC_REGISTER_FIXTURE(HighLevelTxnTest);

