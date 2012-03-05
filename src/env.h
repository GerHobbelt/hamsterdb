/*
 * Copyright (C) 2005-2012 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

/**
<<<<<<< HEAD
 * @brief The Environment structure definitions, getters/setters and functions
=======
* @cond ham_internals
*/

/**
 * @brief internal macros and headers
>>>>>>> flash-bang-grenade
 *
 */

#ifndef HAM_ENV_H__
#define HAM_ENV_H__

#include "internal_fwd_decl.h"
#include <string>

#include <ham/hamsterdb_stats.h>

#include "endianswap.h"
#include "error.h"
#include "page.h"
<<<<<<< HEAD
#include "changeset.h"
=======


#ifdef __cplusplus
extern "C" {
#endif




#if 0 /* some MSVC versions don't like this in pedantic mode */
#define OFFSETOF(type, member) ((unsigned int) &((type *)0)->member)
#else
#define OFFSETOF(type, member) ((unsigned int) (((char *)&((type *)0)->member) - ((char *)0)))
#endif
>>>>>>> flash-bang-grenade

/**
 * This is the minimum chunk size; all chunks (pages and blobs) are aligned
 * to this size.
 *
<<<<<<< HEAD
 * WARNING: pages (and 'aligned' huge blobs) are aligned to
 * a DB_PAGESIZE_MIN_REQD_ALIGNMENT boundary, that is, any 'aligned=true'
 * freelist allocations will produce blocks which are aligned to a
 * 8*32 == 256 bytes boundary.
=======
 * WARNING: pages (and 'aligned' huge blobs) are aligned to a DB_PAGESIZE_MIN_REQD_ALIGNMENT
 *          boundary, that is, any 'aligned=true' freelist allocations will
 *          produce blocks which are aligned to a 8*32 == 256 bytes boundary.
>>>>>>> flash-bang-grenade
 */
#define DB_CHUNKSIZE        32

/**
<<<<<<< HEAD
 * The minimum required database page alignment: since the freelist scanner
 * works on a byte-boundary basis for aligned storage, all aligned storage
 * must/align to an 8-bits times 1 DB_CHUNKSIZE-per-bit boundary. Which for a
 * 32 bytes chunksize means your pagesize minimum required alignment/size
 * is 8*32 = 256 bytes.
 */
=======
The minimum required database page alignment: since the freelist scanner works
on a byte-boundary basis for aligned storage, all aligned storage must/align
to an 8-bits times 1 DB_CHUNKSIZE-per-bit boundary. Which for a 32 bytes chunksize
means your pagesize minimum required alignment/size is 8*32 = 256 bytes.
*/
>>>>>>> flash-bang-grenade
#define DB_PAGESIZE_MIN_REQD_ALIGNMENT      (8 * DB_CHUNKSIZE)

#include <ham/packstart.h>

/**
<<<<<<< HEAD
 * the persistent file header
 */
=======
* the persistent database header
*/
>>>>>>> flash-bang-grenade
typedef HAM_PACK_0 struct HAM_PACK_1
{
    /** magic cookie - always "ham\0" */
    ham_u8_t  _magic[4];
<<<<<<< HEAD

    /** version information - major, minor, rev, reserved */
    ham_u8_t  _version[4];

    /** serial number */
    ham_u32_t _serialno;

    /** size of the page */
    ham_u32_t _pagesize;

    /**
     * maximum number of databases for this environment
     *
     * NOTE: formerly, the _max_databases was 32 bits, but since
     * nobody would use more than 64K tables/indexes, we have the
     * MSW free for repurposing; as we store data in Little Endian
     * order, that would be the second WORD.
     *
     * For reasons of backwards compatibility, the default value
     * there would be zero (0).
     */
    ham_u16_t _max_databases;

    /** reserved */
    ham_u16_t _reserved1;

    /*
     * following here:
     *
     * 1. the private data of the index backend(s)
     *      -> see env_get_indexdata()
     *
=======

    /** version information - major, minor, rev, reserved */
    ham_u8_t  _version[4];

    /** serial number */
    ham_u32_t _serialno;

    /** size of the page */
    ham_u32_t _pagesize;

    /**
     * maximum number of databases for this environment
     *
     * NOTE: formerly, the _max_databases was 32 bits, but since
     * nobody would use more than 64K tables/indexes, we have the
     * MSW free for repurposing; as we store data in Little Endian
     * order, that would be the second WORD.
     *
     * For reasons of backwards compatibility, the default value
     * there would be zero (0).
     */
    ham_u16_t _max_databases;

    /** reserved */
    ham_u16_t _reserved3;

    /*
     * following here:
     *
     * 1. the private data of the index backend(s)
     *      -> see env_get_indexdata()
     *
>>>>>>> flash-bang-grenade
     * 2. the freelist data
     *      -> see env_get_freelist()
     */

} HAM_PACK_2 env_header_t;

#include <ham/packstop.h>


#define envheader_get_version(hdr, i)  ((hdr))->_version[i]


/**
 * A helper structure; ham_env_t is declared in ham/hamsterdb.h as an
 * opaque C structure, but internally we use a C++ class. The ham_env_t
 * struct satisfies the C compiler, and internally we just cast the pointers.
 */
<<<<<<< HEAD
struct ham_env_t {
    int dummy;
};

struct db_indexdata_t;
typedef struct db_indexdata_t db_indexdata_t;

#define SIZEOF_FULL_HEADER(env)                                             \
    (sizeof(env_header_t)+                                                  \
     (env)->get_max_databases()*sizeof(db_indexdata_t))

/**
 * the Environment structure
 */
class Environment
{
  public:
    /** default constructor initializes all members */
    Environment();

    /** destructor */
    ~Environment();
=======
struct ham_env_t
{
    /** the current transaction ID */
    ham_txn_id_t _txn_id;

    /** the filename of the environment file */
    const char *_filename;

    /** the 'mode' parameter of ham_env_create_ex */
    ham_u32_t _file_mode;

    /** the user-provided context data */
    void *_context;

    /** the device (either a file or an in-memory-db) */
    ham_device_t *_device;

    /** the cache */
    ham_cache_t *_cache;

    /** the memory allocator */
    mem_allocator_t *_alloc;

    /** the file header page */
    ham_page_t *_hdrpage;

    /** the active txn */
    ham_txn_t *_txn;

    /** the log object */
    ham_log_t *_log;

#if 0 /* [i_a] a bottleneck on env's which have multiple dbs open at the same time; keep this a per-db item */
    /* the cache for extended keys */
    extkey_cache_t *_extkey_cache;
#endif

    /** the Environment flags - a combination of the persistent flags
     * and runtime flags */
    ham_u32_t _rt_flags;

    /** a linked list of all open databases */
    ham_db_t *_next;

    /** the pagesize which was specified when the env was created */
    ham_size_t _pagesize;

    /** the cachesize which was specified when the env was created/opened */
    ham_size_t _cachesize;

#ifdef HAM_ENABLE_REMOTE
    /** libcurl remote handle */
    void *_curl;
#endif

    /** the max. number of databases which was specified when the env
     * was created */
    ham_u16_t  _max_databases;
    ham_u16_t  _data_access_mode;

    /**
     * non-zero after this item has been opened/created.
     * Indicates whether this environment is 'active', i.e. between
     * a create/open and matching close API call.
     */
    unsigned _is_active: 1;

    /**
     * non-zero if this Environment is pre-1.1.0 format
     */
    unsigned _is_legacy: 1;

    /**
     * non-zero when @ref ham_env_close noticed the (possibly implicit) Environment being flagged with
     * @ref HAM_ENV_CLOSE_WAITS_FOR_DB_CLOSE hence requiring postponing that
     * operation until the last ham_close has been called explicitly.
     *
     * This bit is introduced to help the C++ and other HamsterDB interface wrappers
     * produce deterministic (and above all non-fatal) results when they cannot
     * or will not guarantee the order in which @ref ham_env_close, @ref ham_env_delete,
     * @ref ham_close and @ref ham_delete, @ref ham_txn_abort, @ref ham_txn_commit and
     * @ref ham_cursor_close will occur.
     */
    unsigned _close_is_pending: 1;

    /**
     * non-zero when @ref ham_env_delete noticed the (possibly implicit) Environment being flagged with
     * @ref HAM_ENV_CLOSE_WAITS_FOR_DB_CLOSE hence requiring postponing that
     * operation until the last ham_close has been called explicitly.
     *
     * This bit is introduced to help the C++ and other HamsterDB interface wrappers
     * produce deterministic (and above all non-fatal) results when they cannot
     * or will not guarantee the order in which @ref ham_env_close, @ref ham_env_delete,
     * @ref ham_close and @ref ham_delete, @ref ham_txn_abort, @ref ham_txn_commit and
     * @ref ham_cursor_close will occur.
     */
    unsigned _delete_is_pending: 1;

    /** linked list of all file-level filters */
    ham_file_filter_t *_file_filters;

    /**
     * some freelist algorithm specific run-time data
     *
     * This is done as a union as it will reduce code complexity
     * significantly in the common freelist processing areas.
     */
    ham_runtime_statistics_globdata_t _perf_data;
>>>>>>> flash-bang-grenade

    /*
     * following here: function pointers which implement access to
     * local or remote databases. they are initialized in ham_env_create_ex
     * and ham_env_open_ex after the Environment handle was initialized and
     * an allocator was created.
     *
     * @sa env_initialize_local
     * @sa env_initialize_remote
     */

    /**
     * initialize and create a new Environment
     */
    ham_status_t (*_fun_create)(Environment *env, const char *filename,
            ham_u32_t flags, ham_u32_t mode,
            const ham_parameter_t *param);

    /**
     * initialize and open a new Environment
     */
    ham_status_t (*_fun_open)(Environment *env, const char *filename,
            ham_u32_t flags, const ham_parameter_t *param);

    /**
     * rename a database in the Environment
     */
<<<<<<< HEAD
    ham_status_t (*_fun_rename_db)(Environment *env, ham_u16_t oldname,
=======
    ham_status_t (*_fun_rename_db)(ham_env_t *env, ham_u16_t oldname,
>>>>>>> flash-bang-grenade
            ham_u16_t newname, ham_u32_t flags);

    /**
     * erase a database from the Environment
     */
<<<<<<< HEAD
    ham_status_t (*_fun_erase_db)(Environment *env, ham_u16_t name,
=======
    ham_status_t (*_fun_erase_db)(ham_env_t *env, ham_u16_t name,
>>>>>>> flash-bang-grenade
            ham_u32_t flags);

    /**
     * get all database names
     */
<<<<<<< HEAD
    ham_status_t (*_fun_get_database_names)(Environment *env,
=======
    ham_status_t (*_fun_get_database_names)(ham_env_t *env,
>>>>>>> flash-bang-grenade
            ham_u16_t *names, ham_size_t *count);

    /**
     * get environment parameters
     */
    ham_status_t (*_fun_get_parameters)(Environment *env, ham_parameter_t *param);

    /**
     * flush the environment
     */
    ham_status_t (*_fun_flush)(Environment *env, ham_u32_t flags);

    /**
     * create a database in the environment
     */
<<<<<<< HEAD
    ham_status_t (*_fun_create_db)(Environment *env, Database *db,
=======
    ham_status_t (*_fun_create_db)(ham_env_t *env, ham_db_t *db,
>>>>>>> flash-bang-grenade
                ham_u16_t dbname, ham_u32_t flags,
                const ham_parameter_t *param);

    /**
     * open a database in the environment
     */
<<<<<<< HEAD
    ham_status_t (*_fun_open_db)(Environment *env, Database *db,
=======
    ham_status_t (*_fun_open_db)(ham_env_t *env, ham_db_t *db,
>>>>>>> flash-bang-grenade
                ham_u16_t dbname, ham_u32_t flags,
                const ham_parameter_t *param);

    /**
     * create a transaction in this environment
     */
<<<<<<< HEAD
    ham_status_t (*_fun_txn_begin)(Environment *env, ham_txn_t **txn,
                const char *name, ham_u32_t flags);
=======
    ham_status_t (*_fun_txn_begin)(ham_env_t *env, ham_db_t *db,
                ham_txn_t **txn, ham_u32_t flags);
>>>>>>> flash-bang-grenade

    /**
     * aborts a transaction
     */
<<<<<<< HEAD
    ham_status_t (*_fun_txn_abort)(Environment *env, ham_txn_t *txn,
=======
    ham_status_t (*_fun_txn_abort)(ham_env_t *env, ham_txn_t *txn,
>>>>>>> flash-bang-grenade
                ham_u32_t flags);

    /**
     * commits a transaction
     */
<<<<<<< HEAD
    ham_status_t (*_fun_txn_commit)(Environment *env, ham_txn_t *txn,
=======
    ham_status_t (*_fun_txn_commit)(ham_env_t *env, ham_txn_t *txn,
>>>>>>> flash-bang-grenade
                ham_u32_t flags);

    /**
     * close the Environment
     */
<<<<<<< HEAD
    ham_status_t (*_fun_close)(Environment *env, ham_u32_t flags);

    /**
     * destroy the environment object, free all memory
     */
    ham_status_t (*destroy)(Environment *self);

    /** get the filename */
    const std::string &get_filename() {
        return (m_filename);
    }

    /** set the filename */
    void set_filename(std::string filename) {
        m_filename=filename;
    }

    /** get the unix file mode */
    ham_u32_t get_file_mode() {
        return (m_file_mode);
    }

    /** set the unix file mode */
    void set_file_mode(ham_u32_t mode) {
        m_file_mode=mode;
    }

    /** get the user-provided context pointer */
    void *get_context_data() {
        return (m_context);
    }

    /** set the user-provided context pointer */
    void set_context_data(void *ctxt) {
        m_context=ctxt;
    }

    /** get the current transaction ID */
    ham_u64_t get_txn_id() {
        return (m_txn_id);
    }

    /** set the current transaction ID */
    void set_txn_id(ham_u64_t id) {
        m_txn_id=id;
    }

    /** get the device */
    Device *get_device() {
        return (m_device);
    }

    /** set the device */
    void set_device(Device *device) {
        m_device=device;
    }

    /** get the cache pointer */
    Cache *get_cache() {
        return (m_cache);
    }

    /** set the cache pointer */
    void set_cache(Cache *cache) {
        m_cache=cache;
    }

    /** get the allocator */
    Allocator *get_allocator() {
        return (m_alloc);
    }

    /** set the allocator */
    void set_allocator(Allocator *alloc) {
        m_alloc=alloc;
    }

    /** get the header page */
    Page *get_header_page() {
        return (m_hdrpage);
    }

    /** set the header page */
    void set_header_page(Page *page) {
        m_hdrpage=page;
    }

    /** get a pointer to the header data */
    env_header_t *get_header() {
        return ((env_header_t *)(get_header_page()->get_payload()));
    }

    /** get the oldest transaction */
    ham_txn_t *get_oldest_txn() {
        return (m_oldest_txn);
    }

    /** set the oldest transaction */
    void set_oldest_txn(ham_txn_t *txn) {
        m_oldest_txn=txn;
    }

    /** get the newest transaction */
    ham_txn_t *get_newest_txn() {
        return (m_newest_txn);
    }

    /** set the newest transaction */
    void set_newest_txn(ham_txn_t *txn) {
        m_newest_txn=txn;
    }

    /** get the log object */
    Log *get_log() {
        return (m_log);
    }

    /** set the log object */
    void set_log(Log *log) {
        m_log=log;
    }

    /** get the journal */
    Journal *get_journal() {
        return (m_journal);
    }

    /** set the journal */
    void set_journal(Journal *j) {
        m_journal=j;
    }

    /** get the flags */
    ham_u32_t get_flags() {
        return (m_flags);
    }

    /** set the flags */
    void set_flags(ham_u32_t flags) {
        m_flags=flags;
    }

    /** get the linked list of all open databases */
    Database *get_databases() {
        return (m_databases);
    }

    /** set the linked list of all open databases */
    void set_databases(Database *db) {
        m_databases=db;
    }

    /** get the current changeset */
    Changeset &get_changeset() {
        return (m_changeset);
    }

    /** get the pagesize as specified in ham_env_create_ex */
    ham_size_t get_pagesize() {
        return (m_pagesize);
    }

    /** get the size of the usable persistent payload of a page */
    ham_size_t get_usable_pagesize() {
        return (get_pagesize()-Page::sizeof_persistent_header);
    }

    /** set the pagesize as specified in ham_env_create_ex */
    void set_pagesize(ham_size_t ps) {
        m_pagesize=ps;
    }

    /** get the cachesize as specified in ham_env_create_ex/ham_env_open_ex */
    ham_u64_t get_cachesize() {
        return (m_cachesize);
    }

    /** set the cachesize as specified in ham_env_create_ex/ham_env_open_ex */
    void set_cachesize(ham_u64_t cs) {
        m_cachesize=cs;
    }
=======
    ham_status_t (*_fun_close)(ham_env_t *env, ham_u32_t flags);


    /**
     * destroy the environment object, free all memory
     */
    ham_status_t (*destroy)(ham_env_t **self_reference);
};


/**
 * get the current transaction ID
 */
#define env_get_txn_id(env)              (env)->_txn_id

/**
 * set the current transaction ID
 */
#define env_set_txn_id(env, id)          (env)->_txn_id=(id)

/**
 * get the filename
 */
#define env_get_filename(env)            (env)->_filename

/**
 * set the filename
 */
#define env_set_filename(env, f)         (env)->_filename=(f)

/**
 * get the unix file mode
 */
#define env_get_file_mode(env)           (env)->_file_mode

/**
 * set the unix file mode
 */
#define env_set_file_mode(env, m)        (env)->_file_mode=(m)

 /**
 * get the user-provided context pointer
 */
#define env_get_context_data(env)        (env)->_context

 /**
 * set the user-provided context pointer
 */
#define env_set_context_data(env, ctxt)  (env)->_context=(ctxt)

/**
 * get the device
 */
#define env_get_device(env)              (env)->_device

/**
 * set the device
 */
#define env_set_device(env, d)           (env)->_device=(d)

/**
 * get the allocator
 */
#define env_get_allocator(env)           (env)->_alloc

/**
 * set the allocator
 */
#define env_set_allocator(env, a)        (env)->_alloc=(a)

/**
 * get the cache pointer
 */
#define env_get_cache(env)               (env)->_cache

/**
 * set the cache pointer
 */
#define env_set_cache(env, c)            (env)->_cache=(c)

/**
 * get the curl handle
 */
#define env_get_curl(env)                (env)->_curl

/**
 * set the curl handle
 */
#define env_set_curl(env, c)             (env)->_curl=(c)

/**
 * get the header page
 */
#define env_get_header_page(env)         (env)->_hdrpage

/**
 * get a pointer to the header data
 */
#if 01
#define env_get_header(env)                                                 \
    ((env_header_t *)(page_get_payload(env_get_header_page(env))))
#else /* appease GCC 4.4.x 'pointer aliasing' errors in -O3 */
env_header_t *
env_get_header(ham_env_t *env);
#endif

/**
 * set the header page
 */
#define env_set_header_page(env, h)      (env)->_hdrpage=(h)

/**
 * set the dirty-flag - this is the same as db_set_dirty()
 */
#define env_set_dirty(env)              page_set_dirty(env_get_header_page(env), env)

/**
* get the dirty-flag
*/
#define env_is_dirty(env)                page_is_dirty(env_get_header_page(env))


/**
 * Get a reference to the array of database-specific private data;
 * interpretation of the data is up to the backends.
 *
 * @return a pointer to the persisted @ref db_indexdata_t data array.
 *
 * @note Use @ref env_get_indexdata_ptr instead when you want a reference to the
 *       @ref db_indexdata_t-based private data for a particular database in
 *       the environment.
 */
#define env_get_indexdata_arrptr(env)                                         \
    ((db_indexdata_t *)(((ham_u8_t *)page_get_payload(                        \
        env_get_header_page(env))) + sizeof(env_header_t)))

/**
 * Get the private data of the specified database stored at index @a i;
 * interpretation of the data is up to the backend.
 *
 * @return a pointer to the persisted @ref db_indexdata_t data for the
 * given database.
 *
 * @note Use @ref db_get_indexdata_offset to retrieve the @a i index value
 * for your database.
 */
#define env_get_indexdata_ptr(env, i)      (env_get_indexdata_arrptr(env) + (i))

/**
 * get the currently active transaction
 */
#define env_get_txn(env)                 (env)->_txn

/**
 * set the currently active transaction
 */
#define env_set_txn(env, txn)            (env)->_txn=(txn)

/**
 * get the log object
 */
#define env_get_log(env)                 (env)->_log

/**
 * set the log object
 */
#define env_set_log(env, log)            (env)->_log=(log)

/**
 * get the runtime-flags
 */
#define env_get_rt_flags(env)            (env)->_rt_flags

/**
 * modify the runtime-flags
 */
#define env_modify_rt_flags(env, or_mask, inv_and_mask)                     \
        env_set_rt_flags(env,                                               \
            (env_get_rt_flags(env) & ~(inv_and_mask)) | (or_mask))

/**
 * set the runtime-flags
 */
#define env_set_rt_flags(env, f)         (env)->_rt_flags=(f)

/**
 * get the linked list of all open databases
 */
#define env_get_list(env)                (env)->_next

/**
 * set the linked list of all open databases
 */
#define env_set_list(env, db)            (env)->_next=(db)

/**
 * get the pagesize as specified in ham_env_create_ex
 */
#define env_get_pagesize(env)            (env)->_pagesize
>>>>>>> flash-bang-grenade

#if HAM_ENABLE_REMOTE
    /** get the curl handle */
    void *get_curl() {
        return (m_curl);
    }

    /** set the curl handle */
    void set_curl(void *curl) {
        m_curl=curl;
    }
#endif

    /**
     * get the maximum number of databases for this file (cached, not read
     * from header page)
     */
    ham_u16_t get_max_databases_cached() {
        return (m_max_databases_cached);
    }

    /**
     * set the maximum number of databases for this file (cached, not written
     * to header page)
     */
    void set_max_databases_cached(ham_u16_t md) {
        m_max_databases_cached=md;
    }

    /** set the 'active' flag of this Environment */
    void set_active(bool a) {
        m_is_active=a;
    }

    /** check whether this environment has been opened/created.  */
    bool is_active() {
        return (m_is_active);
    }

    /** set the 'legacy' flag of the environment */
    void set_legacy(bool l) {
        m_is_legacy=l;
    }

    /** check whether this environment is a legacy file (pre 1.1.0) */
    bool is_legacy() {
        return (m_is_legacy);
    }

    /** returns true if this Environment is private to a Database
     * (was implicitly created in ham_create/ham_open) */
    bool is_private();

    /** set the dirty-flag - this is the same as db_set_dirty() */
    void set_dirty(bool dirty) {
        get_header_page()->set_dirty(dirty);
    }

    /** get the dirty-flag */
    bool is_dirty() {
        return (get_header_page()->is_dirty());
    }

<<<<<<< HEAD
    /**
     * Get the private data of the specified database stored at index @a i;
     * interpretation of the data is up to the backend.
     */
    db_indexdata_t *get_indexdata_ptr(int i);

    /** get the linked list of all file-level filters */
    ham_file_filter_t *get_file_filter() {
        return (m_file_filters);
    }

    /** set the linked list of all file-level filters */
    void set_file_filter(ham_file_filter_t *f) {
        m_file_filters=f;
    }

    /** get the maximum number of databases for this file */
    ham_u16_t get_max_databases() {
        env_header_t *hdr=(env_header_t*)
                    (get_header_page()->get_payload());
        return (ham_db2h16(hdr->_max_databases));
    }

    /** set the maximum number of databases for this file */
    void set_max_databases(ham_u16_t md) {
        get_header()->_max_databases=md;
    }

    /** get the page size from the header page */
    // TODO can be private?
    ham_size_t get_persistent_pagesize() {
        return (ham_db2h32(get_header()->_pagesize));
    }

    /** set the page size in the header page */
    // TODO can be private?
    void set_persistent_pagesize(ham_size_t ps) {
        get_header()->_pagesize=ham_h2db32(ps);
    }

    /** get a reference to the DB FILE (global) statistics */
    ham_runtime_statistics_globdata_t *get_global_perf_data() {
        return (&m_perf_data);
    }

    /** set the 'magic' field of a file header */
    void set_magic(ham_u8_t m1, ham_u8_t m2, ham_u8_t m3, ham_u8_t m4) {
        get_header()->_magic[0]=m1;
        get_header()->_magic[1]=m2;
        get_header()->_magic[2]=m3;
        get_header()->_magic[3]=m4;
    }

    /** returns true if the magic matches */
    bool compare_magic(ham_u8_t m1, ham_u8_t m2, ham_u8_t m3, ham_u8_t m4) {
        if (get_header()->_magic[0]!=m1)
            return (false);
        if (get_header()->_magic[1]!=m2)
            return (false);
        if (get_header()->_magic[2]!=m3)
            return (false);
        if (get_header()->_magic[3]!=m4)
            return (false);
        return (true);
    }

    /** get byte @a i of the 'version'-header */
    ham_u8_t get_version(ham_size_t idx) {
        env_header_t *hdr=(env_header_t *)
                    (get_header_page()->get_payload());
        return (envheader_get_version(hdr, idx));
    }

    /** set the version of a file header */
    void set_version(ham_u8_t a, ham_u8_t b, ham_u8_t c, ham_u8_t d) {
        get_header()->_version[0]=a;
        get_header()->_version[1]=b;
        get_header()->_version[2]=c;
        get_header()->_version[3]=d;
    }

    /** get the serial number */
    ham_u32_t get_serialno() {
        env_header_t *hdr=(env_header_t*)
                    (get_header_page()->get_payload());
        return (ham_db2h32(hdr->_serialno));
    }

    /** set the serial number */
    void set_serialno(ham_u32_t n) {
        env_header_t *hdr=(env_header_t*)
                    (get_header_page()->get_payload());
        hdr->_serialno=ham_h2db32(n);
    }

    /** get the freelist object of the database */
    freelist_payload_t *get_freelist();

    /** set the logfile directory */
    void set_log_directory(const std::string &dir) {
        m_log_directory=dir;
    }

    /** get the logfile directory */
    const std::string &get_log_directory() {
        return (m_log_directory);
    }

    /** get the mutex */
    Mutex &get_mutex() {
        return (m_mutex);
    }

  private:
    /** a mutex for this Environment */
    Mutex m_mutex;

    /** the filename of the environment file */
    std::string m_filename;
=======
/**
 * get the keysize
 */
#define env_get_keysize(env)             (env)->_keysize

/**
 * get the maximum number of databases for this file
 */
#define env_get_max_databases(env)       (env)->_max_databases

/**
 * get the expected data access mode for this file
 */
#define env_get_data_access_mode(env)    (env)->_data_access_mode
>>>>>>> flash-bang-grenade

    /** the 'mode' parameter of ham_env_create_ex */
    ham_u32_t m_file_mode;

<<<<<<< HEAD
    /** the current transaction ID */
    ham_u64_t m_txn_id;

    /** the user-provided context data */
    void *m_context;

    /** the device (either a file or an in-memory-db) */
    Device *m_device;

    /** the cache */
    Cache *m_cache;

    /** the memory allocator */
    Allocator *m_alloc;

    /** the file header page */
    Page *m_hdrpage;
=======
/**
 * set the maximum number of databases for this file
 */
#define env_set_max_databases(env, md)   (env)->_max_databases=(md)

/**
 * set the expected data access mode for this file
 */
#define env_set_data_access_mode(env, md) (env)->_data_access_mode=(md)

/**
Mix a set of flag bits into the data access mode, according to the rule

<pre>
DAM(new) = (DAM & and_mask) | or_mask
</pre>

This is a quick way to set or unset particular DAM bits.

@sa ham_data_access_modes
*/
#define env_set_data_access_mode_masked(env, or_mask, and_mask)             \
    (env)->_data_access_mode=(((env)->_data_access_mode & (and_mask))       \
                              | (or_mask))



#define env_get_persistent_max_databases(env)                               \
    ham_db2h16(env_get_header(env)->_max_databases)

#define env_set_persistent_max_databases(env, s)                            \
    env_get_header(env)->_max_databases = ham_h2db16(s)

/**
 * get the page size
 */
#define env_get_persistent_pagesize(env)                                    \
    (ham_db2h32(env_get_header(env)->_pagesize))

/**
 * set the page size
 */
#define env_set_persistent_pagesize(env, ps)                                \
    env_get_header(env)->_pagesize=ham_h2db32(ps)

/**
 * set the 'magic' field of a file header
 */
#define env_set_magic(env, a,b,c,d)                                         \
    do {                                                                    \
      env_header_t *_env = env_get_header(env);                             \
      _env->_magic[0]=(a);                                                  \
      _env->_magic[1]=(b);                                                  \
      _env->_magic[2]=(c);                                                  \
      _env->_magic[3]=(d);                                                  \
    } while (0)
>>>>>>> flash-bang-grenade

    /** the head of the transaction list (the oldest transaction) */
    ham_txn_t *m_oldest_txn;

<<<<<<< HEAD
    /** the tail of the transaction list (the youngest/newest transaction) */
    ham_txn_t *m_newest_txn;
=======
/**
 * set the version of a file header
 */
#define env_set_version(env,a,b,c,d)                                        \
    do {                                                                    \
      env_header_t *_env = env_get_header(env);                             \
      _env->_version[0]=(a);                                                \
      _env->_version[1]=(b);                                                \
      _env->_version[2]=(c);                                                \
      _env->_version[3]=(d);                                                \
    } while (0)
>>>>>>> flash-bang-grenade

    /** the physical log */
    Log *m_log;

<<<<<<< HEAD
    /** the logical journal */
    Journal *m_journal;

    /** the Environment flags - a combination of the persistent flags
     * and runtime flags */
    ham_u32_t m_flags;

    /** a linked list of all open databases */
    Database *m_databases;
=======
/**
 * get byte @a i of the 'version'-header
 */
#define env_get_version(env, i)   (envheader_get_version(env_get_header(env), i))

/**
 * get the serial number
 */
#define env_get_serialno(env)       (ham_db2h32(env_get_header(env)->_serialno))

/**
 * set the serial number
 */
#define env_set_serialno(env, n)    env_get_header(env)->_serialno=ham_h2db32(n)




#define SIZEOF_FULL_HEADER(env)                                             \
    (sizeof(env_header_t) +                                                 \
     env_get_max_databases(env) * sizeof(db_indexdata_t))

/**
 * set the 'active' flag of the environment: a non-zero value
 * for @a s sets the @a env to 'active', zero(0) sets the @a env
 * to 'inactive' (closed)
 */
#define env_set_active(env,s)       (env)->_is_active=!!(s)
>>>>>>> flash-bang-grenade

    /** the changeset - a list of all pages that were modified during
     * one database operation */
    Changeset m_changeset;

    /** the pagesize which was specified when the env was created */
    ham_size_t m_pagesize;

    /** the cachesize which was specified when the env was created/opened */
    ham_u64_t m_cachesize;

    /** the max. number of databases which was specified when the env
     * was created */
    ham_u16_t m_max_databases_cached;

    /** true after this object is already in use */
    bool m_is_active;

    /** true if this Environment is pre-1.1.0 format */
    bool m_is_legacy;

#if HAM_ENABLE_REMOTE
    /** libcurl remote handle */
    void *m_curl;
#endif

<<<<<<< HEAD
    /** linked list of all file-level filters */
    ham_file_filter_t *m_file_filters;

    /** some freelist algorithm specific run-time data */
    ham_runtime_statistics_globdata_t m_perf_data;
=======
/**
 * get the freelist object of the database
 */
#define env_get_freelist(env)                                               \
    ((freelist_payload_t *)                                                 \
     (((ham_u8_t *)page_get_payload(env_get_header_page(env))) +            \
      SIZEOF_FULL_HEADER(env)))



/**
 * get the size of the usable persistent payload of a page
 */
#define env_get_usable_pagesize(env)                                        \
    (env_get_pagesize(env) - page_get_persistent_header_size())


>>>>>>> flash-bang-grenade

    /** the directory of the log file and journal files */
    std::string m_log_directory;
};

<<<<<<< HEAD
/**
 * fetch a page.
 *
 * This is like db_fetch_page, but only for those cases when there's
 * no Database handle
 */
extern ham_status_t
env_fetch_page(Page **page_ref, Environment *env,
        ham_offset_t address, ham_u32_t flags);

/**
 * allocate a page.
 *
 * This is like db_alloc_page, but only for those cases when there's
 * no Database handle
 */
extern ham_status_t
env_alloc_page(Page **page_ref, Environment *env,
                ham_u32_t type, ham_u32_t flags);
=======

>>>>>>> flash-bang-grenade

/**
 * create a env_backend_t structure for accessing local files
 */
extern ham_status_t
env_initialize_local(Environment *env);

/**
 * create a env_backend_t structure for accessing remote server
 */
extern ham_status_t
env_initialize_remote(Environment *env);






/**
 * Ensure that the environment occupies a minimum number of pages.
 *
 * This is useful with various storage devices to prevent / reduce
 * fragmentation.
 *
 * @param env the environment reference.
<<<<<<< HEAD
 * @param minimum_page_count The desired minimum number of storage pages
        * available to the environment/database.
=======
 *
 * @param minimum_page_count The desired minimum number of storage pages
 *        available to the environment/database.
>>>>>>> flash-bang-grenade
 *
 * process:
 *
 * <ol>
 * <li> detect how many pages we already have in the environment
 *
 * <li> calculate how many pages we should have
 *
 * <li> when this is more than what we've got so far, tell
 *      the device driver to allocate the remainder and mark
 *      them all as 'free'.
 * </ol>
 *
 * @remark Caveat:
 *    The required size may be so large that it does not
 *    fit in the current freelist, so one or more of
 *    the allocated 'free' pages will be used for the
 *    extended freelist.
 */
extern ham_status_t
<<<<<<< HEAD
env_reserve_space(Environment *env, ham_offset_t minimum_page_count);
=======
env_reserve_space(ham_offset_t minimum_page_count,
                  dev_alloc_request_info_ex_t *extra_dev_alloc_info);

/**
 Purge the page cache, when needed.

 This decision is influenced by the @a purge_depth parameter, which
 is the target of the number of cache pages which should be purged from the cache
 during this round of purging.

 This function does <em>not</em> guarantee that this target will be
 met, but it will try its best to arrive at it and it may surpass
 the specified target at its discretion.

 When the target equals zero, no purging will be performed.


 @param purge_depth the purge target, i.e. the requested number of pages
        to purge from the cache. When zero, no purging will be performed.
*/
extern ham_status_t
env_purge_cache(ham_env_t *env, ham_size_t purge_depth);
>>>>>>> flash-bang-grenade

/**
 * add a new transaction to this Environment
 */
extern void
env_append_txn(Environment *env, ham_txn_t *txn);

/**
 * remove a transaction to this Environment
 */
extern void
env_remove_txn(Environment *env, ham_txn_t *txn);

/*
 * flush all committed Transactions to disk
 */
extern ham_status_t
env_flush_committed_txns(Environment *env);

/*
 * increments the lsn and returns the incremended value; if the lsn
 * overflows, HAM_LIMITS_REACHED is returned
 *
 * only works if a journal is created! Otherwise assert(0)
 */
extern ham_status_t
env_get_incremented_lsn(Environment *env, ham_u64_t *lsn);

/*
 * purge the cache if the limits are exceeded
 */
extern ham_status_t
env_purge_cache(Environment *env);


#endif /* HAM_ENV_H__ */

/**
* @endcond
*/

