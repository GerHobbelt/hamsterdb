/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "../3rdparty/json/JSON_parser.h"

#include <ham/hamsterdb.h>
#include <ham/hamsterdb_int.h>

#include "getopts.h"



#define ARG_HELP            1
#define ARG_DBNAME          2
#define ARG_FULL            3
#define ARG_OUTPUT_JSON     4

/*
 * command line parameters
 */
static option_t opts[]={
    {
        ARG_HELP,               // symbolic name of this option
        "h",                    // short option
        "help",                 // long option
        "this help screen",     // help string
        0 },                    // no flags
    {
        ARG_DBNAME,
        "db",
        "dbname",
        "only print info about this database",
        GETOPTS_NEED_ARGUMENT },
    {
        ARG_FULL,
        "f",
        "full",
        "print full information",
        0 },
    {
        ARG_OUTPUT_JSON,
        "out",
        "output-json",
        "write the info to the specified file in JSON format suitable for 'ham_dump --clone-settings'",
        GETOPTS_NEED_ARGUMENT },
	{
		0,
		NULL, NULL,
		"\n"
		"%s prints information about the specified HamsterDB database, such as\n"
		"  a list of the database tables stored in the given file, their identifiers,\n"
		"  configured key size and configuration flags.",
		GETOPTS_IS_ONLY_INFO },
    { 0, 0, 0, 0, 0 } /* terminating element */
};

static void
error(const char *foo, ham_status_t st)
{
    printf("%s() returned error %d: %s\n", foo, st, ham_strerror(st));
    exit(-1);
}

static void
print_environment(ham_env_t *env)
{
	ham_env_info_t env_info = {0};
    ham_parameter_t params[] =
    {
        {HAM_PARAM_GET_ENVIRONMENT_INFO, 0 /* &env_info */ },
        {0,0}
    };
    ham_status_t st;

	params[0].value.env_info_ref = &env_info;

	st = ham_env_get_parameters(env, params);
    if (st != HAM_SUCCESS)
        error("ham_env_get_parameters", st);

    printf("environment\n");
    printf("    pagesize:                      %u\n", env_info.pagesize);
    printf("    version:                       %u.%u.%u.%u\n",
            env_info.version_info[0],
            env_info.version_info[1],
            env_info.version_info[2],
            env_info.version_info[3]);
    printf("    serialno:                      %u\n", env_info.serial_no);
	printf("    max databases:                 %u\n", env_info.max_db_count);
	printf("    number of databases defined:   %u\n", env_info.db_count);
}

static void
print_database(ham_db_t *db, ham_u16_t dbname, int full)
{
	ham_db_info_t db_info = {0};
    ham_parameter_t params[] =
    {
        {HAM_PARAM_GET_DATABASE_INFO, 0 /* &db_info */ },
        {0,0}
    };
    ham_cursor_t *cursor;
    ham_status_t st;
    ham_key_t key;
    ham_record_t rec;
    unsigned num_items=0, ext_keys=0, min_key_size=0xffffffff,
             max_key_size=0, min_rec_size=0xffffffff, max_rec_size=0,
            total_key_size=0, total_rec_size=0;
	char strbuf[256];

	params[0].value.db_info_ref = &db_info;

    memset(&key, 0, sizeof(key));
    memset(&rec, 0, sizeof(rec));

	st = ham_get_parameters(db, params);
    if (st != HAM_SUCCESS)
        error("ham_get_parameters", st);

	assert(db_info.dbname == dbname);

    printf("\n");
    printf("    database %d (0x%x)\n", (int)dbname, (int)dbname);
	printf("        max key size:           %u\n", db_info.key_size);
	printf("        max keys per page:      %u\n", db_info.max_key_count_per_page);
	printf("        flags:                  0x%04x (%s)\n",
			db_info.create_flags,
			ham_create_flags2str(strbuf, 256, db_info.create_flags));

    if (!full)
        return;

    st=ham_cursor_create(db, 0, 0, &cursor);
    if (st!=HAM_SUCCESS)
        error("ham_cursor_create", st);

    for (;;) {
        st=ham_cursor_move(cursor, &key, &rec, HAM_CURSOR_NEXT);
        if (st!=HAM_SUCCESS) {
            /* reached end of the database? */
            if (st==HAM_KEY_NOT_FOUND)
                break;
            else
                error("ham_cursor_next", st);
        }

        num_items++;

        if (key.size<min_key_size)
            min_key_size=key.size;
        if (key.size>max_key_size)
            max_key_size=key.size;

        if (rec.size<min_rec_size)
            min_rec_size=rec.size;
        if (rec.size>max_rec_size)
            max_rec_size=rec.size;

		if (key.size > db_info.key_size)
            ext_keys++;

        total_key_size+=key.size;
        total_rec_size+=rec.size;
    }

    ham_cursor_close(cursor);

    printf("        number of items:        %u\n", num_items);
    if (num_items==0)
        return;
    printf("        average key size:       %u\n", total_key_size/num_items);
    printf("        minimum key size:       %u\n", min_key_size);
    printf("        maximum key size:       %u\n", max_key_size);
    printf("        number of extended keys:%u\n", ext_keys);
    printf("        total keys (bytes):     %u\n", total_key_size);
    printf("        average record size:    %u\n", total_rec_size/num_items);
    printf("        minimum record size:    %u\n", min_rec_size);
    printf("        maximum record size:    %u\n", min_rec_size);
    printf("        total records (bytes):  %u\n", total_rec_size);
}

int
main(int argc, char **argv)
{
    unsigned opt;
    char *param, *filename=0, *endptr=0;
    char *output_file = NULL;
    unsigned short dbname=0xffff;
    int full=0;

    ham_u16_t names[1024];
    ham_size_t i, names_count=1024;
    ham_status_t st;
    ham_env_t *env;
    ham_db_t *db;

    ham_u32_t maj, min, rev;
    const char *licensee, *product;
    ham_get_license(&licensee, &product);
    ham_get_version(&maj, &min, &rev);

    getopts_init(argc, argv, "ham_info");

    while ((opt=getopts(&opts[0], &param))) {
        switch (opt) {
            case ARG_DBNAME:
                if (!param) {
                    printf("Parameter `dbname' is missing.\n");
                    return (-1);
                }
                dbname=(short)strtoul(param, &endptr, 0);
                if (endptr && *endptr) {
                    printf("Invalid parameter `dbname'; numerical value "
                           "expected.\n");
                    return (-1);
                }
                break;
            case ARG_FULL:
                full=1;
                break;
			case ARG_OUTPUT_JSON:
                if (!param) {
                    printf("Parameter `output-file' is missing.\n");
                    return (-1);
                }
                output_file = strdup(param);
                break;

            case GETOPTS_PARAMETER:
                if (filename) {
                    printf("Multiple files specified. Please specify "
                           "only one filename.\n");
                    return (-1);
                }
                filename=param;
                break;
            case ARG_HELP:
                printf("hamsterdb %d.%d.%d - Copyright (C) 2005-2011 "
                       "Christoph Rupp (chris@crupp.de).\n\n",
                       maj, min, rev);

                if (licensee[0]=='\0')
                    printf(
                       "This program is free software; you can redistribute "
                       "it and/or modify it\nunder the terms of the GNU "
                       "General Public License as published by the Free\n"
                       "Software Foundation; either version 2 of the License,\n"
                       "or (at your option) any later version.\n\n"
                       "See file COPYING.GPL2 and COPYING.GPL3 for License "
                       "information.\n\n");
                else
                    printf("Commercial version; licensed for %s (%s)\n\n",
                            licensee, product);

				getopts_usage(opts);
                return (0);
            default:
                printf("Invalid or unknown parameter `%s'. "
                       "Enter `ham_info --help' for usage.", param);
                return (-1);
        }
    }

    if (!filename) {
        printf("Filename is missing. Enter `ham_info --help' for usage.\n");
        return (-1);
    }

    /*
     * open the environment
     */
    st=ham_env_new(&env);
    if (st!=HAM_SUCCESS)
        error("ham_env_new", st);
    st=ham_env_open(env, filename, HAM_READ_ONLY);
    if (st==HAM_FILE_NOT_FOUND) {
        printf("File `%s' not found or unable to open it\n", filename);
        return (-1);
    }
    else if (st!=HAM_SUCCESS)
        error("ham_env_open", st);

    /*
     * print information about the environment
     */
    print_environment(env);

    /*
     * get a list of all databases
     */
    st=ham_env_get_database_names(env, names, &names_count);
    if (st!=HAM_SUCCESS)
        error("ham_env_get_database_names", st);

    /*
     * did the user specify a database name? if yes, show only this database
     */
    if (dbname!=0xffff) {
        st=ham_new(&db);
        if (st)
            error("ham_new", st);

        st=ham_env_open_db(env, db, dbname, 0, 0);
        if (st==HAM_DATABASE_NOT_FOUND) {
            printf("Database %u (0x%x) not found\n", dbname, dbname);
            return (-1);
        }
        else if (st)
            error("ham_env_open_db", st);

        print_database(db, dbname, full);

        st=ham_close(db, 0);
        if (st)
            error("ham_close", st);
        ham_delete(db);
    }
    else {
        /*
         * otherwise: for each database: print information about the database
         */
        for (i=0; i<names_count; i++) {
            st=ham_new(&db);
            if (st)
                error("ham_new", st);

            st=ham_env_open_db(env, db, names[i], 0, 0);
            if (st)
                error("ham_env_open_db", st);

            print_database(db, names[i], full);

            st=ham_close(db, 0);
            if (st)
                error("ham_close", st);
            ham_delete(db);
        }
    }
    /*
     * clean up
     */
    st=ham_env_close(env, 0);
    if (st!=HAM_SUCCESS)
        error("ham_env_close", st);

    ham_env_delete(env);

    return (0);
}
