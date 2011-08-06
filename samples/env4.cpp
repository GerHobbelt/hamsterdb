/*
 * Copyright (C) 2009-2011 Ger Hobbelt (ger@hobbelt.com), Christoph Rupp (chris@crupp.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

/**
 * @file env4.hpp
 * @author Ger Hobbelt, ger@hobbelt.com
 * @version 1.1.12
 *
 * This sample shows the Hamster used as a store for time series (stock trades, physical measurements, ...)
 * which are stored ordered by time. A run either import a series of data points (or simulate such a import
 * through generation of random data) and will subsequently perform a (nonsense) Monte Carlo model test
 * simulation on the data and then produce a (gnuplot) report output.
 *
 * This extended example showcases several great features of the Hamster:
 *
 * - As the input data time series suffers from sample jitter, it's sample times are not EXACT, which
 *   makes it harder for regular databases to retrieve individual data points.
 *
 * - The Hamster includes 'near match' FIND (SQL: SELECT) modes which are used to provide a
 *   fast time grid sampled retrieval of data points. These search modes (LT/GT/LEQ/GEQ) are
 *   inspired on the VMS RMS $FIND search modes and are not available as such in many other database engine
 *   offerings, either Open Source or commercial. The Hamster search modes are, for example, more powerful
 *   than BerkeleyDB's DB_SET_RANGE get() flag, which is the closest approximation of this which BDB has currently
 *   on offer.
 *
 * - The timeseries_compressor class showcases a way to compress physical or other times series data
 *   in a way which keeps the database storage size small while permitting an (almost) infinite number of
 *   data samples over long periods of time to be collected and stored. This class' compression idea is
 *   in effect somewhat similar to the OSIsoft PI database compression techniques but have been designed
 *   and developed completely independently.
 *
 * - This sample serves as a testbed for the statistics-driven run-time data processing optimizations (hinting)
 *   available in the Hamster since v1.1.0.
 */

#define HAM_CPP_WRAPPER_THROWS_NO_EXCEPTIONS 1

#include "hamsterdb_ex.hpp"

#include <iostream>
#include <stdlib.h> /* for exit() */

#define ASSERT(e)        env4_assert(bool(e), #e)

static __inline void env4_assert(bool e, const char *txt)
{
    if (!e)
    {
        std::cerr << "ENV4 ASSERTION FAILED: " << txt << std::endl;
        throw ham_ex::error(+1, txt);
    }
}



// the random generator which drives the Monte Carlo simulator and Data Feed Simulator
#include "randomgen.hpp"
// The simplified Monte Carlo test rig
#include "montecarlo.hpp"
// The Data Feed Simulator
#include "datafeedsim.hpp"
// The (overly simplistic) model engine applied to the collected data
#include "calcmodel.hpp"
// The test report generator
#include "mcreporter.hpp"





#if 0

int
run_demo(void)
{
    int i;
    ham::env env;           /* hamsterdb environment */
    ham::db db[MAX_DBS];    /* hamsterdb database objects */
    ham::cursor cursor[MAX_DBS]; /* a cursor for each database */
    ham::key key, cust_key, ord_key, c2o_key;
    ham::record record, cust_record, ord_record, c2o_record;

    customer_t customers[MAX_CUSTOMERS]={
        { 1, "Alan Antonov Corp." },
        { 2, "Barry Broke Inc." },
        { 3, "Carl Caesar Lat." },
        { 4, "Doris Dove Brd." }
    };

    order_t orders[MAX_ORDERS]={
        { 1, 1, "Joe" },
        { 2, 1, "Tom" },
        { 3, 3, "Joe" },
        { 4, 4, "Tom" },
        { 5, 3, "Ben" },
        { 6, 3, "Ben" },
        { 7, 4, "Chris" },
        { 8, 1, "Ben" }
    };

    /*
     * create a new database file for the environment
     */
    env.create("test.db");

    /*
     * then create the two databases in this environment; each database
     * has a name - the first is our "customer" database, the second
     * is for the "orders"; the third manages our 1:n relation and
     * therefore needs to enable duplicate keys
     */
    db[DBIDX_CUSTOMER]=env.create_db(DBNAME_CUSTOMER);
    db[DBIDX_ORDER]   =env.create_db(DBNAME_ORDER);
    db[DBIDX_C2O]     =env.create_db(DBNAME_C2O, HAM_ENABLE_DUPLICATES);

    /*
     * create a cursor for each database
     */
    for (i=0; i<MAX_DBS; i++) {
        cursor[i].create(&db[i]);
    }

    /*
     * insert the customers in the customer table
     *
     * INSERT INTO customers VALUES (1, "Alan Antonov Corp.");
     * INSERT INTO customers VALUES (2, "Barry Broke Inc.");
     * etc
     */
    for (i=0; i<MAX_CUSTOMERS; i++) {
        key.set_size(sizeof(int));
        key.set_data(&customers[i].id);

        record.set_size(sizeof(customer_t));
        record.set_data(&customers[i]);

        db[0].insert(&key, &record);
    }

    /*
     * and now the orders in the second database; contrary to env1,
     * we only store the assignee, not the whole structure
     *
     * INSERT INTO orders VALUES (1, "Joe");
     * INSERT INTO orders VALUES (2, "Tom");
     */
    for (i=0; i<MAX_ORDERS; i++) {
        key.set_size(sizeof(int));
        key.set_data(&orders[i].id);

        record.set_size(sizeof(orders[i].assignee));
        record.set_data(orders[i].assignee);

        db[1].insert(&key, &record);
    }

    /*
     * and now the 1:n relationships; the flag HAM_DUPLICATE creates
     * a duplicate key, if the key already exists
     *
     * INSERT INTO c2o VALUES (1, 1);
     * INSERT INTO c2o VALUES (2, 1);
     * etc
     */
    for (i=0; i<MAX_ORDERS; i++) {
        key.set_size(sizeof(int));
        key.set_data(&orders[i].customer_id);

        record.set_size(sizeof(int));
        record.set_data(&orders[i].id);

        db[2].insert(&key, &record, HAM_DUPLICATE);
    }

    /*
     * now start the query - we want to dump each customer with his
     * orders
     *
     * loop over the customer; for each customer, loop over the 1:n table
     * and pick those orders with the customer id. then load the order
     * and print it
     *
     * the outer loop is similar to
     * SELECT * FROM customers WHERE 1;
     */
    while (1) {
        customer_t *customer;

        try {
            cursor[0].move_next(&cust_key, &cust_record);
        }
        catch (ham::error &e) {
            /* reached end of the database? */
            if (e.get_errno()==HAM_KEY_NOT_FOUND)
                break;
            else {
                std::cerr << "cursor.move_next() failed: " << e.get_string()
                          << std::endl;
                return (-1);
            }
        }

        customer=(customer_t *)cust_record.get_data();

        /* print the customer id and name */
        std::cout << "customer " << customer->id << " ('"
                  << customer->name << "')" << std::endl;

        /*
         * loop over the 1:n table
         *
         * before we start the loop, we move the cursor to the
         * first duplicate key
         *
         * SELECT * FROM customers, orders, c2o
         *   WHERE c2o.customer_id=customers.id AND
         *      c2o.order_id=orders.id;
         */
        c2o_key.set_data(&customer->id);
        c2o_key.set_size(sizeof(int));

        try {
            cursor[2].find(&c2o_key);
        }
        catch (ham::error &e) {
            if (e.get_errno()==HAM_KEY_NOT_FOUND)
                continue;
            else {
                std::cerr << "cursor.find() failed: " << e.get_string()
                          << std::endl;
                return (-1);
            }
        }

        /* get the record of this database entry */
        cursor[2].move(0, &c2o_record);

        do {
            int order_id;

            order_id=*(int *)c2o_record.get_data();
            ord_key.set_data(&order_id);
            ord_key.set_size(sizeof(int));

            /*
             * load the order
             * SELECT * FROM orders WHERE id = order_id;
             */
            ord_record=db[1].find(&ord_key);

            std::cout << "  order: " << order_id << " (assigned to "
                      << (char *)ord_record.get_data() << ")" << std::endl;

            /*
             * the flag HAM_ONLY_DUPLICATES restricts the cursor
             * movement to the duplicate list.
             */
            try {
                cursor[2].move(&c2o_key, &c2o_record,
                            HAM_CURSOR_NEXT|HAM_ONLY_DUPLICATES);
            }
            catch (ham::error &e) {
                /* reached end of the database? */
                if (e.get_errno()==HAM_KEY_NOT_FOUND)
                    break;
                else {
                    std::cerr << "cursor.move() failed: " << e.get_string()
                            << std::endl;
                    return (-1);
                }
            }

        } while(1);
    }

    /*
     * we're done! no need to cleanup, the destructors will prevent memory
     * leaks
     */

    std::cout << "success!" << std::endl;
    return (0);
}

#endif


int
main(int argc, char **argv)
{
    try
    {
        env store;
        store.create("stockdata");

        Monte_Carlo_simulator::model_argument_cfg args[3];
        args[0] = Monte_Carlo_simulator::model_argument_cfg(0, 100);
        args[1] = Monte_Carlo_simulator::model_argument_cfg(1, 2);
        args[2] = Monte_Carlo_simulator::model_argument_cfg(2, 3);

        Monte_Carlo_simulator::argument_cfg tstart(0, 0);
        Monte_Carlo_simulator::argument_cfg tend(1000000, 0);
        Monte_Carlo_simulator::configuration mc_cfg(100, 60, tstart, tend, 3, args);

        StockTradeModel mut(store);

        Monte_Carlo_simulator rig(777, mut, mc_cfg);

        rig.execute();
        rig.munch_collected_data();
        rig.generate_report();
    }
    catch (ham::error &e)
    {
        std::cerr << argv[0]
                  << " failed with unexpected error "
                  << e.get_errno() << " ('"
                  << e.get_string() << "')" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

