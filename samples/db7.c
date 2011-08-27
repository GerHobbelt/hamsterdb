/*
 * Copyright (C) 2005-2011 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

/**
 * This sample demonstrates the use of approximate matches; every line is
 * split into words, and each word is inserted with its line number + word number.
 * Then a cursor is used to print all words in a sorted order starting
 * at the given (or next higher) line number, with the lines in which the word occurred.
 * Approximate matching is required here as we always look for word #0, which never exists (each line starts at word #1).
 */

#include <stdio.h>
#include <string.h>
#include <ham/hamsterdb.h>
#include <ham/hamsterdb_int.h>


typedef struct approx_key
{
    unsigned int lineno;
    unsigned int wordno;
    char fluff[(8000*8)/5];
} approx_key;



void show_params(ham_env_t *env, ham_db_t *db, const ham_parameter_t *db_params_out)
{
    int i;

    puts("\n\n-----------\nParameters:\n\n");
    for (i = 0; db_params_out[i].name; i++)
    {
        const char *desc;

        switch (db_params_out[i].name)
        {
        case HAM_PARAM_CACHESIZE:
            desc = "HAM_PARAM_CACHESIZE: sets the cache size (%u bytes)";
            break;

        case HAM_PARAM_PAGESIZE:
            desc = "HAM_PARAM_PAGESIZE: sets the page size (%u bytes)";
            break;

        case HAM_PARAM_KEYSIZE:
            desc = "HAM_PARAM_KEYSIZE: sets the key size (%u bytes)";
            break;

        case HAM_PARAM_MAX_ENV_DATABASES:
            desc = "HAM_PARAM_MAX_ENV_DATABASES: sets the number of maximum Databases (%u)";
            break;

        case HAM_PARAM_DBNAME:
            desc = "HAM_PARAM_DBNAME: db ID in database environment (%u)";
            break;

        case HAM_PARAM_GET_FLAGS:
            desc = "HAM_PARAM_GET_FLAGS: %08x";
            break;

        case HAM_PARAM_GET_FILEMODE:
            desc = "HAM_PARAM_GET_FILEMODE: %04o";
            break;

        case HAM_PARAM_GET_FILENAME:
            printf("HAM_PARAM_GET_FILENAME: %s\n", (const char *)db_params_out[i].value.p);
            continue;

        case HAM_PARAM_GET_KEYS_PER_PAGE:
            desc = "HAM_PARAM_GET_KEYS_PER_PAGE: MAX_KEYSIZE: maximum allowed keysize (unless you change your pagesize) = %u bytes";
            break;

        default:
            continue;
        }
        printf(desc, (unsigned int)db_params_out[i].value.n);
        puts("");
    }
}


int
main(int argc, char **argv)
{
    ham_status_t st;      /* status variable */
    ham_db_t *db;         /* hamsterdb database object */
    ham_cursor_t *cursor; /* a database cursor */
    ham_size_t maxkeys = 0;
    const ham_parameter_t db_params_in[] =
    {
        { HAM_PARAM_PAGESIZE, 64*1024 },
        { HAM_PARAM_KEYSIZE, sizeof(approx_key) },
        {0,0},
    };
    ham_parameter_t db_params_out[] =
    {
        { HAM_PARAM_PAGESIZE, 0 },
        { HAM_PARAM_KEYSIZE, 0 },
        { HAM_PARAM_CACHESIZE, 0 },
        { HAM_PARAM_MAX_ENV_DATABASES, 0 },
        { HAM_PARAM_DBNAME, 0 },
        { HAM_PARAM_GET_FLAGS, 0 },
        { HAM_PARAM_GET_FILEMODE, 0 },
        { HAM_PARAM_GET_FILENAME, 0 },
        { HAM_PARAM_GET_KEYS_PER_PAGE, 0 },
        {0,0},
    };
    char line[1024*4];    /* a buffer for reading lines */
    unsigned lineno=0;    /* the current line number */
    const char *lines[] =
    {
        "a1 a2 a3 a4\n",
        "b1 b2\n",
        "c1\n",
        "d1\n",
        "e1 e2 e3 e4\n"
    };


    printf("This sample uses hamsterdb and approximate keys to list all words "
            "in the\noriginal order, together with their line number.\n");
    printf("Reading from stdin... (stop by entering a dot on a line)\n");

    /*
     * first step: create a new hamsterdb object
     */
    st=ham_new(&db);
    if (st!=HAM_SUCCESS) {
        printf("ham_new() failed with error %d\n", st);
        return (-1);
    }

    {
        ham_env_t *env;

        ham_env_new(&env);
        st=ham_env_get_parameters(env, db_params_out);
        if (st!=HAM_SUCCESS) {
            printf("ham_get_env_params() failed with error %d\n", st);
            return (-1);
        }
        show_params(env, NULL, db_params_out);
    }


    st=ham_get_parameters(db, db_params_out);
    if (st!=HAM_SUCCESS) {
        printf("ham_get_db_params(NULL) failed with error %d\n", st);
        return (-1);
    }
    show_params(NULL, db, db_params_out);

    /*
     * second step: create a new database with support for duplicate keys
     *
     * we could create an in-memory-database to speed up the sorting.
     */
    st=ham_create_ex(db, "test.db", HAM_USE_BTREE, 0664, db_params_in);
    if (st!=HAM_SUCCESS) {
        printf("ham_create() failed with error %d\n", st);
        return (-1);
    }

    st=ham_get_parameters(db, db_params_out);
    if (st!=HAM_SUCCESS) {
        printf("ham_get_db_params(DB) failed with error %d\n", st);
        return (-1);
    }
    show_params(NULL, db, db_params_out);

    st=ham_calc_maxkeys_per_page(db, &maxkeys, sizeof(approx_key));
    printf("ham_calc_maxkeys_per_page(keysize=%u) reported a keycount of %u, while producing error %d (%s)\n",
        (unsigned int)sizeof(approx_key), (unsigned int)maxkeys, st, ham_strerror(st));

    st=ham_calc_maxkeys_per_page(db, &maxkeys, 0);
    printf("ham_calc_maxkeys_per_page(keysize=%u) reported a keycount of %u, while producing error %d (%s)\n",
        0, (unsigned int)maxkeys, st, ham_strerror(st));


    /*
     * now we read each line from stdin and split it in words; then each
     * word is inserted into the database
     */
#if 01
    for(;lineno < sizeof(lines)/sizeof(lines[0]);) {
        char *start=line;
        char *p;
        unsigned wordno = 0;

        strcpy(line, lines[lineno]);
#else
    while (fgets(line, sizeof(line), stdin)) {
        char *start=line;
        char *p;
        unsigned wordno = 0;
#endif

        if (strcmp(line, ".\r\n") == 0 || strcmp(line, ".\n") == 0)
            break;

        lineno++;

        /*
         * strtok is not the best function because it's not threadsafe
         * and not flexible, but it's good enough for this example.
         */
        while ((p=strtok(start, " \t\r\n"))) {
            ham_key_t key = {0};
            ham_record_t record = {0};
            approx_key k = {0};

            k.lineno = lineno;
            k.wordno = ++wordno;

            key.data = &k;
            key.size = sizeof(k);

            record.data=p;
            record.size=(ham_size_t)strlen(p)+1; /* also store the terminating
                                                 0-byte */

            /* note: the second parameter of ham_insert() is reserved; set it
             * to NULL */
            st=ham_insert(db, 0, &key, &record, 0);
            if (st!=HAM_SUCCESS) {
                printf("ham_insert() failed with error %d\n", st);
                return (-1);
            }
            printf(".");

            start=0;
        }
    }

    /*
     * create a cursor
     */
    st=ham_cursor_create(db, 0, 0, &cursor);
    if (st!=HAM_SUCCESS) {
        printf("ham_cursor_create() failed with error %d\n", st);
        return (-1);
    }
    else
    {
        ham_key_t key = {0};
        ham_record_t record = {0};
        approx_key k = {0};
        unsigned int l;

        /* approximate key: there is no wordno==0 in there! */
        for (l = 0; l <= lineno + 1; l++)
        {
            printf("\nSTART @ lineno == %u\n", l);

            k.lineno = l;
            k.wordno = 0;

            key.data = &k;
            key.size = sizeof(k);

            /*
             * iterate over all items and print them
             */
            st=ham_cursor_find_ex(cursor, &key, &record, HAM_FIND_GEQ_MATCH);
#if 0
            st=ham_cursor_move(cursor, &key, &record, 0);
#endif
            /*
               ham_cursor_find positions the cursor, but you'll need cursor_move(FIRST)
               to grab the record data, which is equivalent to first-time cursor_move(NEXT) by
               definition:
             */
            for(;;) {
                if (st!=HAM_SUCCESS) {
                    /* reached end of the database? */
                    if (st==HAM_KEY_NOT_FOUND)
                        break;
                    else {
                        printf("ham_cursor_next() failed with error %d (%s)\n", st, ham_strerror(st));
                        break;
                    }
                }
                k = *(approx_key *)key.data;

                /*
                 * print the word and the line number
                 */
                printf("%s: appeared in line %u @ %u\n", (const char *)record.data,
                        k.lineno, k.wordno);

                st=ham_cursor_move(cursor, &key, &record, HAM_CURSOR_NEXT);
            }


            /*
               And now also check the cursor-less find() operation:
             */
            printf("\nSTART for CURSOR-LESS FIND(GT) @ lineno == %u\n", l);

            k.lineno = l;
            k.wordno = 0;

            key.data = &k;
            key.size = sizeof(k);

            /*
             * iterate over all items and print them
             */
            st=ham_find(db, NULL, &key, &record, HAM_FIND_GEQ_MATCH);
            if (st!=HAM_SUCCESS)
            {
                printf("ham_find(GT) failed with error %d (%s)\n", st, ham_strerror(st));
            }
            else
            {
                k = *(approx_key *)key.data;

                /*
                 * print the word and the line number
                 */
                printf("%s: appeared in line %u @ %u\n", (const char *)record.data,
                        k.lineno, k.wordno);
            }

            printf("\nSTART for CURSOR-LESS FIND(LT) @ lineno == %u\n", l);

            k.lineno = l;
            k.wordno = 0;

            key.data = &k;
            key.size = sizeof(k);

            /*
             * iterate over all items and print them
             */
            st=ham_find(db, NULL, &key, &record, HAM_FIND_LEQ_MATCH);
            if (st!=HAM_SUCCESS)
            {
                printf("ham_find(LT) failed with error %d (%s)\n", st, ham_strerror(st));
            }
            else
            {
                k = *(approx_key *)key.data;

                /*
                 * print the word and the line number
                 */
                printf("%s: appeared in line %u @ %u\n", (const char *)record.data,
                        k.lineno, k.wordno);
            }

            printf("\nSTART for CURSOR-LESS FIND(GT+LT) @ lineno == %u\n", l);

            k.lineno = l;
            k.wordno = 0;

            key.data = &k;
            key.size = sizeof(k);

            /*
             * iterate over all items and print them
             */
            st=ham_find(db, NULL, &key, &record, HAM_FIND_NEAR_MATCH);
            if (st!=HAM_SUCCESS)
            {
                printf("ham_find(GT+LT) failed with error %d (%s)\n", st, ham_strerror(st));
            }
            else
            {
                k = *(approx_key *)key.data;

                /*
                 * print the word and the line number
                 */
                printf("%s: appeared in line %u @ %u\n", (const char *)record.data,
                        k.lineno, k.wordno);
            }
        }
    }

    /*
     * then close the database handle; the flag
     * HAM_AUTO_CLEANUP will automatically close all cursors, and we
     * do not need to call ham_cursor_close
     */
    st=ham_close(db, HAM_AUTO_CLEANUP);
    if (st!=HAM_SUCCESS) {
        printf("ham_close() failed with error %d\n", st);
        return (-1);
    }

    /*
     * delete the database object to avoid memory leaks
     */
    ham_delete(db);

    /*
     * success!
     */
    return (0);
}

