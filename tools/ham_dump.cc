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
#include <ctype.h>
#include <assert.h>
#include <locale.h>

#include "../3rdparty/json/JSON_parser.h"


#include <ham/hamsterdb.h>
#include <ham/hamsterdb_int.h> /* HAM_DEFAULT_DATABASE_NAME */


#include "getopts.h"

#define ARG_DUMP            0 /* implicit command */
#define ARG_HELP            1
#define ARG_DBNAME          2
#define ARG_KEY_FORMAT      3
#define ARG_REC_FORMAT      4
#define ARG_KEY_MAX_SIZE    5
#define ARG_REC_MAX_SIZE    6
#define ARG_EXPORT          7
#define ARG_IMPORT          8
#define ARG_CLONE           9
#define ARG_CLONE_SETTINGS 10

#define FMT_NUMERIC         1
#define FMT_STRING          2
#define FMT_ENCODED_STRING  3
#define FMT_BINARY          4

#define DBNAME_ANY          0xFFFFU

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
        "only dump/clone this database",
        GETOPTS_NEED_ARGUMENT },
    {
        ARG_KEY_FORMAT,
        "key",
        "key-format",
        "format of the key\n\t\t('string', 'encoded-string', 'binary' (default), 'numeric')",
        GETOPTS_NEED_ARGUMENT },
    {
        ARG_KEY_MAX_SIZE,
        "maxkey",
        "max-key-size",
        "limit dumped key length to N bytes",
        GETOPTS_NEED_ARGUMENT },
    {
        ARG_REC_FORMAT,
        "rec",
        "record-format",
        "format of the record\n\t\t('string', 'encoded-string', 'binary' (default), 'numeric')",
        GETOPTS_NEED_ARGUMENT },
    {
        ARG_REC_MAX_SIZE,
        "maxrec",
        "max-rec-size",
        "limit dumped record length to N bytes",
        GETOPTS_NEED_ARGUMENT },
    {
        ARG_EXPORT,
        "ex",
        "export",
        "export database in HamsterDB JSON export format",
        0 },
    {
        ARG_IMPORT,
        "im",
        "import",
        "import database in HamsterDB JSON export format",
        0 },
    {
        ARG_CLONE,
        "cl",
        "clone",
        "clone database in HamsterDB native format",
        0 },
    {
        ARG_CLONE_SETTINGS,
        "clcfg",
        "clone-settings",
        "specify additional clone options in a JSON formatted configuration file",
        0 },
    { 0, 0, 0, 0, 0 } /* terminating element */
};


typedef struct dbclone_db_cfg_t
{
    ham_s32_t dbname;               /* -1: default for all DBs */

    ham_u32_t db_create_flags;
    ham_parameter_t *db_create_params;

    ham_u32_t key_insert_flags;
    ham_compare_func_t *key_cmp_fn;
    ham_duplicate_compare_func_t *dupe_cmp_fn;
} dbclone_db_cfg_t;

typedef struct dbclone_env_cfg_t
{
    dbclone_db_cfg_t *db_cfg;
    ham_size_t db_cfg_count;
} dbclone_env_cfg_t;


typedef struct dbdata_t
{
    char *sourcefilename;
    char *targetfilename;
    int keyfmt;
    int recfmt;
    int keysize;
    int recsize;
    ham_u16_t dbname;
    int cmd;

    ham_u16_t *names;
    ham_size_t names_count;

    ham_env_t *env;
    ham_db_t *db;

    ham_u32_t maj;
    ham_u32_t min;
    ham_u32_t rev;
    const char *licensee;
    const char *product;

    dbclone_env_cfg_t *dbclone_cfg;

} dbdata_t;


static void
error(const char *foo, ham_status_t st)
{
    printf("%s() returned error %d: %s\n", foo, st, ham_strerror(st));
    exit(EXIT_FAILURE);
}



static void
semi_url_encode(char *dst, const char *src, size_t srclen)
{
    const unsigned char *s = (const unsigned char *)src;

    for (; srclen; srclen--)
    {
        unsigned char c = *s++;

        if (c >= ' ' && c < 127)
        {
            switch (c)
            {
            case '%':
                break;

            default:
                *dst++ = c;
                continue;
            }
        }
        sprintf(dst, "%%%02X", (unsigned int)c);
        dst += 3;
    }
    *dst = 0;
}


static void
dump_item(ham_key_t *key, ham_record_t *rec, int key_fmt, int max_keysize,
                int rec_fmt, int max_recsize)
{
    int i;
    int ok = 0;
    char *zterm = NULL;

    printf("key: ");

    if (max_keysize <= 0)
        max_keysize = key->size;

    if (!key->data || !key->size)
    {
        printf("(null)");  /* hmmmm... how about a database which stores keys + zero-length records? */
    }
    else
    {
        switch (key_fmt)
        {
        case FMT_STRING:
            if (((char *)key->data)[key->size-1] != 0)
            {
                size_t len = (key->size > max_keysize ? max_keysize : key->size);
                zterm = malloc(len+1);
                if (!zterm)
                {
                    error("dump_item", HAM_OUT_OF_MEMORY);
                }
                memcpy(zterm, key->data, len);
                zterm[len] = 0;
            }
            if (key->size > max_keysize)
            {
                ((char *)key->data)[max_keysize] = 0;
            }
            printf("%s", zterm ? zterm : (const char *)key->data);
            break;

        case FMT_ENCODED_STRING:
            /*
            data is semi-'URL encoded' to ensure all characters map into ASCII space.

            Note that there is no such thing as a 'C string' in here: 'strings'
            which appear NUL-terminated in the DB get their terminator dumped
            _with_ them; this differentiates them from the 'strings' which were
            NOT NUL-terminated in the database!
            */
            zterm = malloc(key->size * 3 + 1);
            if (!zterm)
            {
                error("dump_item", HAM_OUT_OF_MEMORY);
            }
            semi_url_encode(zterm, key->data, (key->size > max_keysize ? max_keysize : key->size));
            printf("%s", zterm);
            break;
        case FMT_NUMERIC:
            switch (key->size)
            {
            case 1:
                printf("%u", (unsigned) *(unsigned char *)key->data);
                ok=1;
                break;
            case 2:
                printf("%u", (unsigned) *(unsigned short *)key->data);
                ok=1;
                break;
            case 4:
                printf("%u", *(unsigned int *)key->data);
                ok=1;
                break;
            case 8:
                printf("%llu", *(unsigned long long *)key->data);
                ok=1;
                break;
            default:
                /* fall through */
                break;
            }
            if (ok)
                break;
            printf("(illegal numeric size: %u)\n", (unsigned)key->size);
            break;

        case FMT_BINARY:
            if (key->size<max_keysize)
                max_keysize=key->size;
            for (i=0; i<max_keysize; i++)
                printf("%02x ", ((unsigned char *)key->data)[i]);
            break;
        }
    }

    if (zterm)
    {
        free(zterm);
        zterm=0;
    }

    printf(" => ");

    ok=0;

    if (max_recsize <= 0)
        max_recsize=rec->size;

    if (!rec->data || !rec->size)
    {
        printf("(null)");
    }
    else
    {
        switch (rec_fmt)
        {
        case FMT_STRING:
            if (((char *)rec->data)[rec->size-1]!=0)
            {
                zterm=malloc(rec->size+1);
                if (!zterm) {
                    error("dump_item", HAM_OUT_OF_MEMORY);
                }
                memcpy(zterm, rec->data, rec->size);
                zterm[key->size]=0;
            }
            if (rec->size>(unsigned)max_recsize)
                ((char *)rec->data)[max_recsize]=0;
            printf("%s", zterm ? zterm : (const char *)rec->data);
            break;
        case FMT_NUMERIC:
            switch (rec->size)
            {
            case 1:
                printf("%u", (unsigned) *(unsigned char *)rec->data);
                ok=1;
                break;
            case 2:
                printf("%u", (unsigned) *(unsigned short *)rec->data);
                ok=1;
                break;
            case 4:
                printf("%u", *(unsigned int *)rec->data);
                ok=1;
                break;
            case 8:
                printf("%llu", *(unsigned long long *)rec->data);
                ok=1;
                break;
            default:
                /* fall through */
                break;
            }
            if (ok)
                break;
            printf("(illegal numeric size: %u)\n", (unsigned)rec->size);
            break;
        case FMT_BINARY:
            if (rec->size < (unsigned)max_recsize)
                max_recsize = rec->size;
            for (i = 0; i < max_recsize; i++)
                printf("%02x ", ((unsigned char *)rec->data)[i]);
            break;
        }
    }

    printf("\n");

    if (zterm)
    {
        free(zterm);
        zterm=0;
    }
}

static ham_status_t
dump_database(dbdata_t *cfg)
{
    ham_cursor_t *cursor;
    ham_status_t st;
    ham_key_t key;
    ham_record_t rec;

    memset(&key, 0, sizeof(key));
    memset(&rec, 0, sizeof(rec));

    printf("database %d (0x%x)\n", (int)cfg->dbname, (int)cfg->dbname);

    st=ham_cursor_create(cfg->db, 0, 0, &cursor);
    if (st!=HAM_SUCCESS)
        error("ham_cursor_create", st);

    for(;;)
    {
        st = ham_cursor_move(cursor, &key, &rec, HAM_CURSOR_NEXT);
        if (st != HAM_SUCCESS)
        {
            /* reached end of the database? */
            if (st == HAM_KEY_NOT_FOUND)
                break;
            else
                error("ham_cursor_next", st);
        }

        dump_item(&key, &rec, cfg->keyfmt, cfg->keysize, cfg->recfmt, cfg->recsize);
    }

    ham_cursor_close(cursor);
    printf("\n");

    return 0;
}



static ham_status_t
dump_env_info_header(dbdata_t *cfg)
{
    return 0;
}


static ham_status_t
dump_env_info_footer(dbdata_t *cfg)
{
    return 0;
}


static ham_status_t
import_database(dbdata_t *cfg)
{
    return 0;
}







static int print(void* ctx, int type, const JSON_value* value);

int main2(int argc, char* argv[])
{
    int count = 0;
    FILE* input;

    JSON_config config;

    struct JSON_parser_struct* jc = NULL;

    init_JSON_config(&config);

    config.depth                  = 20;
    config.callback               = &print;
    config.allow_comments         = 1;
    config.handle_floats_manually = 0;

    /* Important! Set locale before parser is created.*/
    if (argc >= 2) {
        if (!setlocale(LC_ALL, argv[1])) {
            fprintf(stderr, "Failed to set locale to '%s'\n", argv[1]);
        }
    } else {
        fprintf(stderr, "No locale provided, C locale is used\n");
    }

    jc = new_JSON_parser(&config);

    input = stdin;
    for (; input ; ++count) {
        int next_char = fgetc(input);
        if (next_char <= 0) {
            break;
        }
        if (!JSON_parser_char(jc, next_char)) {
            delete_JSON_parser(jc);
            fprintf(stderr, "JSON_parser_char: syntax error, byte %d\n", count);
            return 1;
        }
    }
    if (!JSON_parser_done(jc)) {
        delete_JSON_parser(jc);
        fprintf(stderr, "JSON_parser_end: syntax error\n");
        return 1;
    }

    return 0;
}

static size_t s_Level = 0;

static const char* s_pIndention = "  ";

static int s_IsKey = 0;

static void print_indention()
{
    size_t i;

    for (i = 0; i < s_Level; ++i) {
        printf(s_pIndention);
    }
}


static int print(void* ctx, int type, const JSON_value* value)
{
    switch(type) {
    case JSON_T_ARRAY_BEGIN:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("[\n");
        ++s_Level;
        break;
    case JSON_T_ARRAY_END:
        assert(!s_IsKey);
        if (s_Level > 0) --s_Level;
        print_indention();
        printf("]\n");
        break;
    case JSON_T_OBJECT_BEGIN:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("{\n");
        ++s_Level;
        break;
    case JSON_T_OBJECT_END:
        assert(!s_IsKey);
        if (s_Level > 0) --s_Level;
        print_indention();
        printf("}\n");
        break;
    case JSON_T_INTEGER:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("integer: "JSON_PARSER_INTEGER_SPRINTF_TOKEN"\n", value->vu.integer_value);
        break;
    case JSON_T_FLOAT:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("float: " JSON_PARSER_FLOAT_SPRINTF_TOKEN "\n", value->vu.float_value); /* We wanted stringified floats */
        break;
    case JSON_T_NULL:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("null\n");
        break;
    case JSON_T_TRUE:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("true\n");
        break;
    case JSON_T_FALSE:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("false\n");
        break;
    case JSON_T_KEY:
        s_IsKey = 1;
        print_indention();
        printf("key = '%s', value = ", value->vu.str.value);
        break;
    case JSON_T_STRING:
        if (!s_IsKey) print_indention();
        s_IsKey = 0;
        printf("string: '%s'\n", value->vu.str.value);
        break;
    default:
        assert(0);
        break;
    }

    return 1;
}






static int add_argv_elem(int *argc, char ***argv, const char *elem)
{
    if (!argv[0])
    {
        argv[0] = malloc(sizeof(argv[0]) * 2);
        if (!argv[0]) return -1;
        argc[0] = 1;
    }
    else
    {
        argv[0] = realloc(argv[0], sizeof(argv[0]) * (argc[0] + 2));
        if (!argv[0]) return -1;
        argc[0]++;
    }
    argv[0][argc[0] - 1] = strdup(elem);
    if (!argv[0][argc[0] - 1]) return -1;
    argv[0][argc[0]] = NULL;
    return 0;
}


/*
 * process '@filename' commandline arguments (response files)
 */
static int handle_response_files(int *argc, char ***argv)
{
    int new_argc = 0;
    char **new_argv = NULL;
    int i;
    FILE *f;
    char str[1024];
    char *elem;
    char *endp;

    for (i = 0; i < argc[0]; i++)
    {
        if (i == 0)
        {
            if (add_argv_elem(&new_argc, &new_argv, argv[0][0]))
                return -1;
            continue;
        }
        if (argv[0][i][0] == '@')
        {
            f = fopen(argv[0][i]+1, "r");
            if (!f)
            {
                fprintf(stderr, "Cannot open responsefile %s\n", argv[0][i]+1);
                return -1;
            }
            while (!feof(f))
            {
                str[0] = 0;
                fgets(str, sizeof(str), f);
                for (elem = str; *elem; )
                {
                    /* skip leading whitespace */
                    while (*elem && (isspace(*elem) || strchr(" \t\r\n", *elem)))
                        elem++;
                    switch (*elem)
                    {
                    case 0:
                        break;

                    case '\'':
                    case '\"':
                        /* quoted argument... */
                        {
                            char c = *elem++;

                            endp = elem;
                            while (*endp && *endp != c)
                                endp++;
                            if (*endp != c)
                            {
                                fprintf(stderr, "unterminated string in response file\n");
                                return -1;
                            }
                            *endp++ = 0;
                            if (add_argv_elem(&new_argc, &new_argv, elem))
                                return -1;
                            elem = endp;
                        }
                        continue;

                    default:
                        endp = elem;
                        while (*endp && !isspace(*endp) && !strchr(" \t\r\n", *endp))
                            endp++;
                        if (*endp)
                            *endp++ = 0;
                        if (add_argv_elem(&new_argc, &new_argv, elem))
                            return -1;
                        elem = endp;
                        continue;
                    }
                }
            }
            fclose(f);
        }
        else
        {
            /* regular argument... */
            if (add_argv_elem(&new_argc, &new_argv, argv[0][i]))
                return -1;
        }
    }

    *argc = new_argc;
    *argv = new_argv;

    for (i = 0; i < argc[0]; i++)
    {
        printf("argv[%d] = \"%s\"\n", i, argv[0][i]);
    }
    return 0;
}

static void shift_argv(int *new_argc, char ***new_argv, int shift)
{
    int c = *new_argc;
    int i;

    for (i = 1; i <= shift; i++)
    {
        free(new_argv[0][i]);
        new_argv[0][i] = NULL;
    }

    if (c <= shift + 1)
    {
        /* may be too large shift? or at least it's maximum shift: clip all args */
        *new_argc = (c ? 1 : 0);
    }
    else
    {
        memmove(&new_argv[0][1], &new_argv[0][1 + shift], (c - shift - 1) * sizeof(new_argv[0][0]));
        *new_argc -= shift;
    }
}

static void free_argv(int new_argc, char **new_argv)
{
    int i;

    for (i = 0; i < new_argc; i++)
    {
        if (new_argv[i])
        {
            free(new_argv[i]);
        }
    }
    free(new_argv);
}





int
main(int argc, char **argv)
{
    dbdata_t cfg = {0};

    unsigned opt;
    char *param;
    char *endptr = 0;

    ham_size_t i;
    ham_status_t st;

    int new_argc = argc;
    char **new_argv = argv;

    cfg.sourcefilename = 0;
    cfg.targetfilename = 0;
    cfg.keyfmt = FMT_BINARY;
    cfg.recfmt = FMT_BINARY;
    cfg.keysize = -1;
    cfg.recsize = -1;
    cfg.dbname = DBNAME_ANY;
    cfg.cmd = ARG_DUMP;

    cfg.names_count = 0;

    ham_get_license(&cfg.licensee, &cfg.product);
    ham_get_version(&cfg.maj, &cfg.min, &cfg.rev);

    if (handle_response_files(&new_argc, &new_argv))
    {
        fprintf(stderr, "error preparing arguments.\n");
        return EXIT_FAILURE;
    }

    getopts_init(new_argc, new_argv, "ham_dump");

    while ((opt = getopts(&opts[0], &param)))
    {
        switch (opt)
        {
        case ARG_EXPORT:
        case ARG_IMPORT:
        case ARG_CLONE:
            cfg.cmd = opt;
            break;

        case ARG_DBNAME:
            if (!param)
            {
                printf("Parameter `dbname' is missing.\n");
                return (-1);
            }
            cfg.dbname = (short)strtoul(param, &endptr, 0);
            if (endptr && *endptr)
            {
                printf("Invalid parameter `dbname'; numerical value "
                       "expected.\n");
                return (-1);
            }
            break;

        case ARG_KEY_FORMAT:
            if (param)
            {
                if (!strcmp(param, "numeric"))
                    cfg.keyfmt = FMT_NUMERIC;
                else if (!strcmp(param, "string"))
                    cfg.keyfmt = FMT_STRING;
                else if (!strcmp(param, "encoded-string"))
                    cfg.keyfmt = FMT_ENCODED_STRING;
                else if (!strcmp(param, "binary"))
                    cfg.keyfmt = FMT_BINARY;
                else
                {
                    printf("Invalid parameter `key-format'.\n");
                    return (-1);
                }
            }
            break;

        case ARG_REC_FORMAT:
            if (param)
            {
                if (!strcmp(param, "numeric"))
                    cfg.recfmt = FMT_NUMERIC;
                else if (!strcmp(param, "string"))
                    cfg.recfmt = FMT_STRING;
                else if (!strcmp(param, "encoded-string"))
                    cfg.recfmt = FMT_ENCODED_STRING;
                else if (!strcmp(param, "binary"))
                    cfg.recfmt = FMT_BINARY;
                else
                {
                    printf("Invalid parameter `record-format'.\n");
                    return (-1);
                }
            }
            break;

        case ARG_KEY_MAX_SIZE:
            cfg.keysize = (short)strtoul(param, &endptr, 0);
            if (endptr && *endptr)
            {
                printf("Invalid parameter `max-key-size'; numerical value "
                       "expected.\n");
                return (-1);
            }
            break;

        case ARG_REC_MAX_SIZE:
            cfg.recsize = (short)strtoul(param, &endptr, 0);
            if (endptr && *endptr)
            {
                printf("Invalid parameter `max-rec-size'; numerical value "
                       "expected.\n");
                return (-1);
            }
            break;

        case GETOPTS_PARAMETER:
            if (cfg.sourcefilename && cfg.targetfilename)
            {
                printf("Multiple files specified. Please specify "
                       "only one filename.\n");
                return (-1);
            }
            if (!param || !*param)
            {
                printf("Invalid parameter `filename': cannot be empty.\n");
                return (-1);
            }
            if (cfg.sourcefilename)
            {
                cfg.targetfilename = strdup(param);
            }
            else
            {
                cfg.sourcefilename = strdup(param);
            }
            break;

        case ARG_HELP:
            printf("hamsterdb %d.%d.%d - Copyright (C) 2005-2009 "
                   "Christoph Rupp (chris@crupp.de).\n\n",
                   cfg.maj, cfg.min, cfg.rev);

            if (cfg.licensee[0] == '\0')
            {
                printf(
                   "This program is free software; you can redistribute "
                   "it and/or modify it\nunder the terms of the GNU "
                   "General Public License as published by the Free\n"
                   "Software Foundation; either version 2 of the License,\n"
                   "or (at your option) any later version.\n\n"
                   "See file COPYING.GPL2 and COPYING.GPL3 for License "
                   "information.\n\n");
            }
            else
            {
                printf("Commercial version; licensed for %s (%s)\n\n",
                        cfg.licensee, cfg.product);
            }
            printf("usage: ham_dump [-db DBNAME] [-key FMT] [-maxkey N] "
                   "[-rec FMT] [-maxrec N] sourcefile targetfile\n");
            printf("usage: ham_dump -h\n");

            getopts_usage(opts);

            return EXIT_SUCCESS;

        default:
            printf("Invalid or unknown parameter `%s'. "
                   "Enter `ham_dump --help' for usage.", param);
            return (-1);
        }
    }

    if (!cfg.sourcefilename)
    {
        printf("Source filename is missing. Enter `ham_dump --help' for usage.\n");
        return (-1);
    }
    if (!cfg.targetfilename && cfg.cmd == ARG_CLONE)
    {
        printf("Target filename is missing: we do not know where you want to clone to. Enter `ham_dump --help' for usage.\n");
        return (-1);
    }

    /*
    set up defaults, which have not been overwritten by the user yet
    */
    switch (cfg.cmd)
    {
    default:
        if (cfg.keysize < 0)
            cfg.keysize = 16;
        if (cfg.recsize < 0)
            cfg.recsize = 16;
        break;

    case ARG_IMPORT:
    case ARG_EXPORT:
    case ARG_CLONE:
        if (cfg.keysize < 0)
            cfg.keysize = 0; /* unlimited dump length by default */
        if (cfg.recsize < 0)
            cfg.recsize = 0;
        break;
    }

    switch (cfg.cmd)
    {
    default:
        /*
         * open the environment
         */
        st = ham_env_new(&cfg.env);
        if (st)
            error("ham_env_new", st);
        st = ham_env_open(cfg.env, cfg.sourcefilename, HAM_READ_ONLY);
        if (st == HAM_FILE_NOT_FOUND)
        {
            printf("File `%s' not found or unable to open it\n", cfg.sourcefilename);
            return (-1);
        }
        else if (st != HAM_SUCCESS)
        {
            error("ham_env_open", st);
        }

        /*
         * get a list of all databases
         */
        cfg.names_count = HAM_DEFAULT_DATABASE_NAME + 2;
        cfg.names = calloc(cfg.names_count, sizeof(cfg.names[0]));
        if (!cfg.names)
        {
            error("calloc", HAM_OUT_OF_MEMORY);
        }
        st = ham_env_get_database_names(cfg.env, cfg.names, &cfg.names_count);
        if (st)
            error("ham_env_get_database_names", st);

        st = dump_env_info_header(&cfg);
        if (st)
            error("dump_env_info_header", st);

        /*
         * did the user specify a database name? if yes, show only this database
         */
        if (cfg.dbname != DBNAME_ANY)
        {
            st = ham_new(&cfg.db);
            if (st)
                error("ham_new", st);

            st = ham_env_open_db(cfg.env, cfg.db, cfg.dbname, 0, 0);
            if (st == HAM_DATABASE_NOT_FOUND)
            {
                printf("Database %u (0x%x) not found\n", cfg.dbname, cfg.dbname);
                return (-1);
            }
            else if (st)
            {
                error("ham_env_open_db", st);
            }

            st = dump_database(&cfg);
            if (st)
            {
                error("dump_database", st);
            }

            st = ham_close(cfg.db, 0);
            if (st)
                error("ham_close", st);
            ham_delete(cfg.db);
        }
        else
        {
            /*
             * otherwise: for each database: print information about the database
             */
            for (i = 0; i < cfg.names_count; i++)
            {
                st = ham_new(&cfg.db);
                if (st)
                    error("ham_new", st);

                st = ham_env_open_db(cfg.env, cfg.db, cfg.names[i], 0, 0);
                if (st)
                    error("ham_env_open_db", st);

                cfg.dbname = cfg.names[i];
                st = dump_database(&cfg);
                if (st)
                {
                    error("dump_database", st);
                }

                st = ham_close(cfg.db, 0);
                if (st)
                    error("ham_close", st);
                ham_delete(cfg.db);
            }
        }

        st = dump_env_info_footer(&cfg);
        if (st)
            error("dump_env_info_footer", st);

        /*
         * clean up
         */
        st = ham_env_close(cfg.env, 0);
        if (st)
            error("ham_env_close", st);

        ham_env_delete(cfg.env);
        break;

    case ARG_IMPORT:
        /*
         * create the environment
         */
        st = ham_env_new(&cfg.env);
        if (st)
            error("ham_env_new", st);
        st = ham_env_create(cfg.env, cfg.sourcefilename, HAM_USE_BTREE, 0644);
        if (st)
            error("ham_env_create", st);

        /*
         * for each database in the env: fetch the database name and restore/import the data.
         *
         * Also detect the (max) keysize and (max) recsize used for this db:
         */
        for (;;)
        {
            st = ham_new(&cfg.db);
            if (st)
                error("ham_new", st);

            cfg.dbname = 1;

            st = ham_env_create_db(cfg.env, cfg.db, cfg.dbname, 0, 0);
            if (st)
                error("ham_env_create_db", st);

            st = import_database(&cfg);
            if (st)
                error("import_database", st);

            st = ham_close(cfg.db, 0);
            if (st)
                error("ham_close", st);
            ham_delete(cfg.db);
            break;
        }

        /*
         * clean up
         */
        st = ham_env_close(cfg.env, 0);
        if (st)
            error("ham_env_close", st);

        ham_env_delete(cfg.env);
        break;

    case ARG_CLONE:
        /*
         * open the source environment
         */
        st = ham_env_new(&cfg.env);
        if (st)
            error("ham_env_new", st);
        st = ham_env_open(cfg.env, cfg.sourcefilename, HAM_READ_ONLY);
        if (st == HAM_FILE_NOT_FOUND)
        {
            printf("File `%s' not found or unable to open it\n", cfg.sourcefilename);
            return (-1);
        }
        else if (st != HAM_SUCCESS)
        {
            error("ham_env_open", st);
        }

        /*
         * get a list of all databases
         */
        cfg.names_count = HAM_DEFAULT_DATABASE_NAME + 2;
        cfg.names = calloc(cfg.names_count, sizeof(cfg.names[0]));
        if (!cfg.names)
        {
            error("calloc", HAM_OUT_OF_MEMORY);
        }
        st = ham_env_get_database_names(cfg.env, cfg.names, &cfg.names_count);
        if (st)
            error("ham_env_get_database_names", st);

        st = dump_env_info_header(&cfg);
        if (st)
            error("dump_env_info_header", st);

        /*
         * did the user specify a database name? if yes, show only this database
         */
        if (cfg.dbname != DBNAME_ANY)
        {
            st = ham_new(&cfg.db);
            if (st)
                error("ham_new", st);

            st = ham_env_open_db(cfg.env, cfg.db, cfg.dbname, 0, 0);
            if (st == HAM_DATABASE_NOT_FOUND)
            {
                printf("Database %u (0x%x) not found\n", cfg.dbname, cfg.dbname);
                return (-1);
            }
            else if (st)
            {
                error("ham_env_open_db", st);
            }

            st = dump_database(&cfg);
            if (st)
            {
                error("dump_database", st);
            }

            st = ham_close(cfg.db, 0);
            if (st)
                error("ham_close", st);
            ham_delete(cfg.db);
        }
        else
        {
            /*
             * otherwise: for each database: print information about the database
             */
            for (i = 0; i < cfg.names_count; i++)
            {
                st = ham_new(&cfg.db);
                if (st)
                    error("ham_new", st);

                st = ham_env_open_db(cfg.env, cfg.db, cfg.names[i], 0, 0);
                if (st)
                    error("ham_env_open_db", st);

                cfg.dbname = cfg.names[i];
                st = dump_database(&cfg);
                if (st)
                {
                    error("dump_database", st);
                }

                st = ham_close(cfg.db, 0);
                if (st)
                    error("ham_close", st);
                ham_delete(cfg.db);
            }
        }

        st = dump_env_info_footer(&cfg);
        if (st)
            error("dump_env_info_footer", st);

        /*
         * clean up
         */
        st = ham_env_close(cfg.env, 0);
        if (st)
            error("ham_env_close", st);

        ham_env_delete(cfg.env);
        break;
    }

    free_argv(new_argc, new_argv);

    return EXIT_SUCCESS;
}
