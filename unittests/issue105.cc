/*
 * Copyright (C) 2005-2016 Christoph Rupp (chris@crupp.de).
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See the file COPYING for License information.
 */

#include <iostream>
#include <cstdlib>
#include <ups/upscaledb.hpp>

int main(int argc, char* argv[])
{
    ups_env_t* env;
    ups_env_create(&env, "test.db", UPS_ENABLE_TRANSACTIONS, 0664, 0);
    //ups_env_create(&env, "test.db", 0, 0664, 0);
    if ( argc > 1 )
    {
        ups_set_committed_flush_threshold(std::atoi(argv[1]));
    }
    ups_parameter_t params[] = {
    {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
    {0, }
    };

    ups_db_t* db;
    ups_env_create_db(env, &db, 1, 0, &params[0]);

    const int initial_item_count = 50;

    for (int i = 0; i < initial_item_count; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_db_insert(db, 0, &key, &record, 0);
    }

    const int k = initial_item_count / 2;
    for(int i = 0; i < k; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));

        ups_db_erase(db, 0, &key, 0);
    }

    uint64_t count = 0;
    ups_db_count(db,0, 0, &count);

    size_t error_count = 0;
    if(count != initial_item_count - k){
        std::cerr << "Item count after delete: " << count << std::endl;
        ++error_count;
    }

    for(int i = 0; i < k; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_cursor_t* cursor;
        ups_cursor_create(&cursor, db, 0, 0);
        ups_status_t st = ups_cursor_find(cursor, &key, &record, UPS_FIND_GEQ_MATCH);
        //ups_status_t st = ups_db_find(db, 0, &key, &record, UPS_FIND_GEQ_MATCH);

        if(st == UPS_SUCCESS ) {
          const int found_key_data = *reinterpret_cast<int*>(key.data);
          if ( found_key_data != k ) {
            std::cerr << "When searching with " << i
                      << " found wrong item: " << found_key_data << std::endl;
            ++error_count;
          }
        }

        ups_cursor_close(cursor);
    }

    ups_db_close(db, 0);

    return error_count;
}
