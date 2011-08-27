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
 * @file hamsterdb.h
 * @brief Include file for hamsterdb Embedded Storage
 * @author Christoph Rupp, chris@crupp.de
 * @version 1.1.13
 *
 * @mainpage
 *
 * This manual documents the hamsterdb C API. hamsterdb is a key/value database
 * that is linked directly into your application, avoiding all the overhead
 * that is related to external databases and RDBMS systems.
 *
 * This header file declares all functions and macros that are needed to use
 * hamsterdb. The comments are formatted in Doxygen style and can be extracted
 * to automagically generate documentation. The documentation is also available
 * online here: <a href="http://hamsterdb.com/public/scripts/html_www">
    http://hamsterdb.com/public/scripts/html_www</a>.
 *
 * In addition, there's a tutorial book hosted on github:
 * <a href="http://github.com/cruppstahl/hamsterdb/wiki/Tutorial">
    http://github.com/cruppstahl/hamsterdb/wiki/Tutorial</a>.
 *
 * If you want to create or open Databases or Environments (a collection of
 * multiple Databases), the following functions will be interesting for you:
 * <table>
 * <tr><td>@ref ham_env_new</td><td>Allocates a new Environment handle</td></tr>
 * <tr><td>@ref ham_env_create_ex</td><td>Creates an Environment</td></tr>
 * <tr><td>@ref ham_env_open_ex</td><td>Opens an Environment</td></tr>
 * <tr><td>@ref ham_env_close</td><td>Closes an Environment</td></tr>
 * <tr><td>@ref ham_env_delete</td><td>Deletes the Environment handle</td></tr>
 * <tr><td>@ref ham_new</td><td>Allocates a new handle for a Database</td></tr>
 * <tr><td>@ref ham_env_create_db</td><td>Creates a Database in an
    Environment</td></tr>
 * <tr><td>@ref ham_env_open_db</td><td>Opens a Database from an
    Environment</td></tr>
 * <tr><td>@ref ham_close</td><td>Closes a Database</td></tr>
 * <tr><td>@ref ham_delete</td><td>Deletes the Database handle</td></tr>
 * </table>
 *
 * To insert, lookup or delete key/value pairs, the following functions are
 * used:
 * <table>
 * <tr><td>@ref ham_insert</td><td>Inserts a key/value pair into a
    Database</td></tr>
 * <tr><td>@ref ham_find</td><td>Lookup of a key/value pair in a
    Database</td></tr>
 * <tr><td>@ref ham_erase</td><td>Erases a key/value pair from a
    Database</td></tr>
 * </table>
 *
 * Alternatively, you can use Cursors to iterate over a Database:
 * <table>
 * <tr><td>@ref ham_cursor_create</td><td>Creates a new Cursor</td></tr>
 * <tr><td>@ref ham_cursor_find</td><td>Positions the Cursor on a key</td></tr>
 * <tr><td>@ref ham_cursor_insert</td><td>Inserts a new key/value pair with a
    Cursor</td></tr>
 * <tr><td>@ref ham_cursor_erase</td><td>Deletes the key/value pair that
    the Cursor points to</td></tr>
 * <tr><td>@ref ham_cursor_overwrite</td><td>Overwrites the value of the current    key</td></tr>
 * <tr><td>@ref ham_cursor_move</td><td>Moves the Cursor to the first, next,
    previous or last key in the Database</td></tr>
 * <tr><td>@ref ham_cursor_close</td><td>Closes the Cursor</td></tr>
 * </table>
 *
 * If you want to use Transactions, then the following functions are required:
 * <table>
 * <tr><td>@ref ham_txn_begin</td><td>Begins a new Transaction</td></tr>
 * <tr><td>@ref ham_txn_commit</td><td>Commits the current
    Transaction</td></tr>
 * <tr><td>@ref ham_txn_abort</td><td>Aborts the current Transaction</td></tr>
 * </table>
 *
 * hamsterdb supports remote Databases via http. The server can be embedded
 * into your application or run standalone (see tools/hamzilla for a Unix
 * daemon or Win32 service which hosts Databases). If you want to embed the
 * server then the following functions have to be used:
 * <table>
 * <tr><td>@ref ham_srv_init</td><td>Initializes the server</td></tr>
 * <tr><td>@ref ham_srv_add_env</td><td>Adds an Environment to the
    server. The Environment with all its Databases will then be available
    remotely.</td></tr>
 * <tr><td>@ref ham_srv_close</td><td>Closes the server and frees all allocated
    resources</td></tr>
 * </table>
 *
 * If you need help then you're always welcome to use the <a
    href="http://hamsterdb-support.1045726.n5.nabble.com/">forum</a>,
 * drop a message (chris at crupp dot de) or with the <a
    href="http://hamsterdb.com/index/contact">contact form</a>.
 *
 * Have fun!
 */

#ifndef HAM_HAMSTERDB_H__
#define HAM_HAMSTERDB_H__

#include <ham/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ham_parameter_t;
typedef struct ham_parameter_t ham_parameter_t;

/**
 * The hamsterdb Database structure
 *
 * This structure is allocated with @ref ham_new and deleted with
 * @ref ham_delete.
 */
struct ham_db_t;
typedef struct ham_db_t ham_db_t;

/**
 * The hamsterdb Environment structure
 *
 * This structure is allocated with @ref ham_env_new and deleted with
 * @ref ham_env_delete.
 */
struct ham_env_t;
typedef struct ham_env_t ham_env_t;

/**
 * A Database Cursor
 *
 * A Cursor is used for bi-directionally traversing the Database and
 * for inserting/deleting/searching Database items.
 *
 * This structure is allocated with @ref ham_cursor_create and deleted with
 * @ref ham_cursor_close.
 */
struct ham_cursor_t;
typedef struct ham_cursor_t ham_cursor_t;

/**
 * A generic record.
 *
 * A record represents data items in hamsterdb. Before using a record, it
 * is important to initialize all record fields with zeroes, i.e. with
 * the C library routines memset(3) or bzero(2).
 *
 * When hamsterdb returns a record structure, the pointer to the record
 * data is provided in @a data. This pointer is only temporary and will be
 * overwritten by subsequent hamsterdb API calls.
 *
 * To avoid this, the calling application can allocate the @a data pointer.
 * In this case, you have to set the flag @ref HAM_RECORD_USER_ALLOC. The
 * @a size parameter will then return the size of the record. It's the
 * responsibility of the caller to make sure that the @a data parameter is
 * large enough for the record.
 *
 * @sa HAM_MAX_RECORD_SIZE_T
 */
typedef struct
{
    /** Pointer to the record data */
    void *data;

    /** The size of the record data, in bytes. See also the discussion at
     * @ref HAM_PARTIAL : this value always records the size of the
     * <em>entire</em> record.
	 *
	 * @sa HAM_MAX_RECORD_SIZE_T */
    ham_size_t size;

    /** The record flags; see @ref HAM_RECORD_USER_ALLOC and @ref HAM_PARTIAL */
    ham_u32_t flags;

    /** For internal use */
    ham_u32_t _intflags;

    /** Offset for partial reading/writing; see @ref HAM_PARTIAL for an in-depth discussion. */
    ham_size_t partial_offset;

    /** Size for partial reading/writing; see @ref HAM_PARTIAL for an in-depth discussion. */
    ham_size_t partial_size;

    /**
	 * For internal use;
     *
	 * This element MUST be able to carry sizeof(@ref ham_pers_rid_t) bytes, as it is also
	 * used to keep a permanent copy of the key in case of TINY/SMALL keys.
	 */
	ham_rid_t _rid;

} ham_record_t;

/** Flag for @ref ham_record_t (only really useful in combination with
 * @ref ham_cursor_move, @ref ham_cursor_find, @ref ham_cursor_find_ex
 * and @ref ham_find)
 *
 * @note
 * When any of these hamsterdb API functions returns either @ref HAM_SUCCESS
 * or @ref HAM_RECORDSIZE_TOO_SMALL, the record size field will be set to the
 * actual record size. The latter is done to help the hamsterdb user to
 * subsequently allocate a suitably sized data space when [s]he wishes
 * to keep using the @ref HAM_RECORD_USER_ALLOC flag.
 *
 * @sa HAM_PARTIAL
 * @sa ham_record_t
 */
#define HAM_RECORD_USER_ALLOC   1

/* also a record flag: HAM_PARTIAL (= 0x80) */

/**
 * A generic key.
 *
 * A key represents key items in hamsterdb. Before using a key, it
 * is important to initialize all key fields with zeroes, i.e. with
 * the C library routines memset(3) or bzero(2).
 *
 * hamsterdb usually uses keys to insert, delete or search for items.
 * However, when using Database Cursors and the function @ref ham_cursor_move,
 * hamsterdb also returns keys. In this case, the pointer to the key
 * data is provided in @a data. This pointer is only temporary and will be
 * overwritten by subsequent calls to @ref ham_cursor_move.
 *
 * To avoid this, the calling application can allocate the @a data pointer.
 * In this case, you have to set the flag @ref HAM_KEY_USER_ALLOC. The
 * @a size parameter will then return the size of the key. It's the
 * responsibility of the caller to make sure that the @a data parameter is
 * large enough for the key.
 *
 * @sa HAM_MAX_KEY_SIZE_T
 */
typedef struct
{
    /** The data of the key */
    void *data;

    /** The size of the key, in bytes.
	 *
	 * @sa HAM_MAX_KEY_SIZE_T */
    ham_u16_t size;

	ham_u16_t _alignment_padding_dummy1;

    /** The key flags; see @ref HAM_KEY_USER_ALLOC */
    ham_u32_t flags;

    /** For internal use */
    ham_u32_t _flags;

} ham_key_t;

/** Flag for @ref ham_key_t (only really useful in combination with
 * @ref ham_cursor_move, @ref ham_cursor_find, @ref ham_cursor_find_ex
 * and @ref ham_find)
 *
 * @note
 * When any of these functions returns either @ref HAM_SUCCESS or
 * @ref HAM_KEYSIZE_TOO_SMALL, the key size field will be set to the actual
 * keysize. The latter is done to help the hamsterdb user to subsequently
 * allocate a suitably sized data space when [s]he wishes to keep using
 * the @ref HAM_KEY_USER_ALLOC flag.
 */
#define HAM_KEY_USER_ALLOC      1

/**
 Function prototype for functions which can be passed as argument (parameter value) for parameters such as
 @ref HAM_PARAM_CUSTOM_DEVICE
*/
typedef void *ham_parameter_function_t(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);

/**
 * A named parameter.
 *
 * These parameter structures are used for functions like @ref ham_open_ex,
 * @ref ham_create_ex, etc. to pass variable length parameter lists.
 *
 * The lists are always arrays of type ham_parameter_t, with a terminating
 * element of { 0, NULL}, e.g.
 *
 * <pre>
 *   ham_parameter_t parameters[]={
 *      { HAM_PARAM_CACHESIZE, 2*1024*1024 }, // set cache size to 2 mb
 *      { HAM_PARAM_PAGESIZE, 4096 }, // set pagesize to 4 kb
 *      { 0, NULL }
 *   };
 * </pre>
 */
struct ham_parameter_t {
    /** The name of the parameter; all HAM_PARAM_*-constants */
    ham_u32_t name;

    /** The value of the parameter. */
    union
    {
        ham_offset_t n;
        void *p;
        ham_parameter_function_t *fn;
    } value;

};


/**
 * @defgroup ham_data_access_modes hamsterdb Data Access Mode Codes
 * @{
 *
 * which can be passed in the @ref HAM_PARAM_DATA_ACCESS_MODE parameter
 * when creating a new Database (see @ref ham_create_ex and @ref ham_env_create_ex) or
 * opening an existing Database (see @ref ham_open_ex and @ref ham_env_open_ex).
 *
 * @remark The Data Access Mode describes the typical application behaviour
 * (i.e. if data is only inserted sequentially) and allows hamsterdb to
 * optimize its routines for this behaviour.
 *
 * @remark The Data Access Mode is persisted in the Database file on a
 * per Database basis. This means different Databases within the same
 * Environment can have different Data Access Modes. The only requirement
 * is that either all or none of the databases in a single @ref ham_env_t
 * environment have their @ref HAM_DAM_ENFORCE_PRE110_FORMAT flag set.
 *
 * @sa ham_create_ex
 * @sa ham_env_create_ex
 * @sa ham_open_ex
 * @sa ham_env_open_ex
 * @sa ham_hinting_flags
 */

/**
 * Assume 'default' behaviour: Databases created with versions newer
 * than 1.0.9 will assume @ref HAM_DAM_RANDOM_WRITE, unless
 * the Database is RECNO-based, in which case
 * @ref HAM_DAM_SEQUENTIAL_INSERT is assumed.
 */
#define HAM_DAM_DEFAULT                  0u

/**
 * Assume random access (a mixed bag of random insert and delete).
 *
 * This is the default setting for (non-RECNO) Databases created with versions
 * newer than 1.0.9.
 *
 * Note: RECNO-based Databases will start in the implicit
 * @ref HAM_DAM_SEQUENTIAL_INSERT mode instead.
*/
#define HAM_DAM_RANDOM_WRITE             0x0001u

/**
 * Assume sequential insert (and few or no delete) operations.
 *
 * This is the default setting for RECNO based Databases created with versions
 * newer than 1.0.9.
 */
#define HAM_DAM_SEQUENTIAL_INSERT        0x0002u

/**
 * Suggest a faster allocation scheme, which will leave a few
 * gaps in the store, i.e. this mode exchanges raw insert speed for
 * a slightly larger Database file.
 *
 * May be combined with the @ref HAM_DAM_SEQUENTIAL_INSERT flag to
 * get the most brutal 'insert' operation performance possible, in
 * exchange for a slightly faster growing file, as available space
 * due to previously deleted records is mostly ignored.
 */
#define HAM_DAM_FAST_INSERT              0x0004u

/**
 * Enforce the use of the backwards compatible Database format.
 * Can be mixed with all but the @ref HAM_DAM_DEFAULT flag.
 *
 * @note This bit flag will be set implicitly when opening an old (hamsterdb v1.0.x) Database file.
 */
#define HAM_DAM_ENFORCE_PRE110_FORMAT    0x8000u

/**
 * @}
 */








/**
 * @defgroup ham_status_codes hamsterdb Status Codes
 * @{
 */

/** Operation completed successfully */
#define HAM_SUCCESS                  (  0)
/* reserved for comparison function return */
/*                                   ( -1) */
/* reserved                          ( -2) */
/** Invalid key size */
#define HAM_INV_KEYSIZE              ( -3)
/** Invalid page size (must be a multiple of 512 (8 (number of bits in a byte) * 32 (@ref DB_CHUNKSIZE)) */
#define HAM_INV_PAGESIZE             ( -4)
/** Memory allocation failed - out of memory */
#define HAM_OUT_OF_MEMORY            ( -6)
/** Object not initialized */
#define HAM_NOT_INITIALIZED          ( -7)
/** Invalid function parameter */
#define HAM_INV_PARAMETER            ( -8)
/** Invalid file header */
#define HAM_INV_FILE_HEADER          ( -9)
/** Invalid file version */
#define HAM_INV_FILE_VERSION         (-10)
/** Key was not found */
#define HAM_KEY_NOT_FOUND            (-11)
/** Tried to insert a key which already exists */
#define HAM_DUPLICATE_KEY            (-12)
/** Internal Database integrity violated */
#define HAM_INTEGRITY_VIOLATED       (-13)
/** Internal hamsterdb error */
#define HAM_INTERNAL_ERROR           (-14)
/** Tried to modify the Database, but the file was opened as read-only */
#define HAM_DB_READ_ONLY             (-15)
/** Database record not found */
#define HAM_BLOB_NOT_FOUND           (-16)
/** Prefix comparison function needs more data */
#define HAM_PREFIX_REQUEST_FULLKEY   (-17)
/** Generic file I/O error */
#define HAM_IO_ERROR                 (-18)
/** Database cache is full */
#define HAM_CACHE_FULL               (-19)
/** Function is not yet implemented */
#define HAM_NOT_IMPLEMENTED          (-20)
/** File not found */
#define HAM_FILE_NOT_FOUND           (-21)
/** Operation would block */
#define HAM_WOULD_BLOCK              (-22)
/** Object was not initialized correctly */
#define HAM_NOT_READY                (-23)
/** Database/system limits reached */
#define HAM_LIMITS_REACHED           (-24)
/** AES encryption key is wrong */
#define HAM_ACCESS_DENIED            (-25)
/** Object was already initialized */
#define HAM_ALREADY_INITIALIZED      (-27)
/** Database needs recovery */
#define HAM_NEED_RECOVERY            (-28)
/** Cursor must be closed prior to Transaction abort/commit */
#define HAM_CURSOR_STILL_OPEN        (-29)
/** User-allocated record size too small

@sa HAM_RECORD_USER_ALLOC
@sa ham_record_t
*/
#define HAM_RECORDSIZE_TOO_SMALL     (-30)
/** User-allocated key size too small

@sa HAM_KEY_USER_ALLOC
@sa ham_key_t
*/
#define HAM_KEYSIZE_TOO_SMALL        (-31)


#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

/** A special error signal for page filters which require precise page-sized I/O
(such as page-oriented encryption and compression filters); this error may only
be returned by such a page filter's @ref after_read() callback method when it
has modified the @ref ham_file_filter_t.info.raw_page_size value too.
*/
#define HAM_INVALID_PAGEFILTER_PAGESIZE         (-36)
/** The page filters quarreled unnecessarily about the right physical/raw pagesize
and the maximum alloted number of iterations has been used up.
This definitely hints at one (or more) page filters having a design (page layout) bug.
*/
#define HAM_INVALID_PAGEFILTER_DESIGN           (-37)

#endif

/** Filter was not part of the record/page filter chain */
#define HAM_FILTER_NOT_FOUND_IN_CHAIN           (-33)
/** Cycle detected in device graph */
#define HAM_CYCLE_IN_DEVICE_GRAPH    (-34)
/** Key already exists */
#define HAM_KEY_ALREADY_EXISTS       (-32)
/** Backend already initialized */
#define HAM_BACKEND_ALREADY_INITIALIZED         (-35)
/** Cursor does not point to a valid item */
#define HAM_CURSOR_IS_NIL            (-100)
/** Database not found */
#define HAM_DATABASE_NOT_FOUND       (-200)
/** Database name already exists */
#define HAM_DATABASE_ALREADY_EXISTS  (-201)
/** Database already open, or: Database handle is already initialized */
#define HAM_DATABASE_ALREADY_OPEN    (-202)
/** Environment already open, or: Environment handle is already initialized */
#define HAM_ENVIRONMENT_ALREADY_OPEN (-203)
/** Invalid log file header */
#define HAM_LOG_INV_FILE_HEADER      (-300)
/** Remote I/O error/Network error */
#define HAM_NETWORK_ERROR            (-400)

/**
 * @}
 */







/**
 * @defgroup ham_static hamsterdb Static Functions
 * @{
 */

/**
 * A typedef for a custom error handler function
 *
 * This error handler can be used in combination with
 * @ref ham_set_errhandler().
 *
 * @param message The error message
 * @param level The error level:
 *       @li 0 @ref HAM_DEBUG_LEVEL_DEBUG a debug message
 *       @li 1 @ref HAM_DEBUG_LEVEL_NORMAL a normal error message
 *       @li 2 reserved
 *       @li 3 @ref HAM_DEBUG_LEVEL_FATAL a fatal error message
 * @param file The hamsterdb source file where the error was detected.
 * @param line The linenumber in the hamsterdb source file where the error
 *             was detected.
 * @param function The function in the hamsterdb source file where the error
 *             was detected, when the platform supports the appropriate
 *             macro to detect this at zero cost at run-time. Otherwise
 *             the function will be listed as the string "???".
 *
 * @sa error_levels
 * @sa ham_set_errhandler
 */
typedef void HAM_CALLCONV (*ham_errhandler_fun)
                    (int level, const char *file, int line, const char *function, const char *message);

/**
 * @defgroup error_levels hamsterdb Error Levels
 * @{
 */

/**
 * A debug message
 *
 * Internally, these messages are generated by @ref ham_trace() calls.
 */
#define HAM_DEBUG_LEVEL_DEBUG       0

/**
 * A normal error message
 *
 * Internally, these messages are generated by @ref ham_log() calls.
 */
#define HAM_DEBUG_LEVEL_NORMAL      1

/**
 * A fatal error message
 *
 * Internally, these messages are generated by @ref ham_assert() assertion
 * checks which are compiled in DEBUG mode or by @ref ham_verify() assertion
 * macros, which are always active in any build.
 */
#define HAM_DEBUG_LEVEL_FATAL       3

/**
 * @}
 */


/**
 * Sets the global error handler
 *
 * This handler will receive all debug messages that are emitted
 * by hamsterdb. You can install the default handler by setting @a f to 0.
 *
 * The default error handler prints all messages to stderr. To install a
 * different logging facility, you can provide your own error handler.
 *
 * Note that the callback function must have the same calling convention
 * as the hamsterdb library.
 *
 * @param f A pointer to the error handler function, or NULL to restore
 *          the default handler
 */
HAM_EXPORT void HAM_CALLCONV
ham_set_errhandler(ham_errhandler_fun f);

/**
 * Translates a hamsterdb status code to a descriptive error string
 *
 * @param status The hamsterdb status code
 *
 * @return A pointer to a descriptive error string
 */
HAM_EXPORT const char * HAM_CALLCONV
ham_strerror(ham_status_t status);

/**
 * Returns the version of the hamsterdb library
 *
 * @param major If not NULL, will return the major version number
 * @param minor If not NULL, will return the minor version number
 * @param revision If not NULL, will return the revision version number
 */
HAM_EXPORT void HAM_CALLCONV
ham_get_version(ham_u32_t *major, ham_u32_t *minor,
        ham_u32_t *revision);

/**
 * Returns the name of the licensee and the name of the licensed product
 *
 * @param licensee If not NULL, will point to the licensee name, or to
 *        an empty string "" for non-commercial versions
 * @param product If not NULL, will point to the product name
 */
HAM_EXPORT void HAM_CALLCONV
ham_get_license(const char **licensee, const char **product);

/**
 * @}
 */








/**
 * @defgroup ham_env hamsterdb Environment Functions
 * @{
 */

/**
 * Allocates a ham_env_t handle
 *
 * @param env Pointer to the pointer which is allocated
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_OUT_OF_MEMORY if memory allocation failed
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_new(ham_env_t **env);

/**
 * Frees a @ref ham_env_t handle
 *
 * Frees the @ref ham_env_t structure, but does not close the
 * Environment. Call this function <b>AFTER</b> you have closed the
 * Environment using @ref ham_env_close, or you will lose your data!
 *
 * @param env A valid Environment handle
 *
 * @return This function always returns @ref HAM_SUCCESS
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_delete(ham_env_t *env);

/**
 * Creates a Database Environment
 *
 * This function is a simplified version of @sa ham_env_create_ex.
 * It is recommended to use @ref ham_env_create_ex instead.
 *
 * @sa ham_env_create_ex
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_create(ham_env_t *env, const char *filename,
        ham_u32_t flags, ham_u32_t mode);

/**
 * Creates a Database Environment - extended version
 *
 * A Database Environment is a collection of Databases, which are all stored
 * in one physical file (or in-memory). Per default, up to 16 Databases can be
 * stored in one file, but this setting can be overwritten by specifying
 * the parameter @ref HAM_PARAM_MAX_ENV_DATABASES.
 *
 * Each Database in an Environment is identified by a positive 16bit value (except
 * 0 and values at or above @c 0xf000 (@ref HAM_DEFAULT_DATABASE_NAME)).
 * Databases in an Environment can be created
 * with @ref ham_env_create_db or opened with @ref ham_env_open_db.
 *
 * @param env A valid Environment handle, which was created with
 *          @ref ham_env_new
 * @param filename The filename of the Environment file. If the file already
 *          exists, it is overwritten. Can be NULL for an In-Memory
 *          Environment.
 * @param flags Optional flags for opening the Environment, combined with
 *          bitwise OR. Possible flags are:
 *      <ul>
 *       <li>@ref HAM_WRITE_THROUGH Flushes all file handles after
 *            committing or aborting a Transaction using fsync(), fdatasync()
 *            or FlushFileBuffers(). This flag has no effect
 *            if Transactions are disabled. Slows down performance but makes
 *            sure that all file handles and operating system caches are
 *            transferred to disk, thus providing a stronger durability in case
 *            the computer crashes.
 *       <li>@ref HAM_IN_MEMORY_DB Creates an In-Memory Environment. No
 *            file will be created, and the Database contents are lost after
 *            the Environment is closed. The @a filename parameter can
 *            be NULL. Do <b>NOT</b> use in combination with
 *            @ref HAM_CACHE_STRICT and do <b>NOT</b> specify @a cachesize
 *            other than 0.
 *       <li>@ref HAM_DISABLE_MMAP Do not use memory mapped files for I/O.
 *            By default, hamsterdb checks if it can use mmap,
 *            since mmap is faster than read/write. For performance
 *            reasons, this flag should not be used.
 *       <li>@ref HAM_CACHE_STRICT Do not allow the cache to grow larger
 *            than @a cachesize. If a Database operation needs to resize the
 *            cache, it will return @ref HAM_CACHE_FULL.
 *            If the flag is not set, the cache is allowed to allocate
 *            more pages than the maximum cache size, but only if it's
 *            necessary and only for a short time.
 *       <li>@ref HAM_CACHE_UNLIMITED Do not limit the cache. Nearly as
 *            fast as an In-Memory Database. Not allowed in combination
 *            with @ref HAM_CACHE_STRICT or a limited cache size.
 *       <li>@ref HAM_DISABLE_FREELIST_FLUSH This flag is deprecated.
 *       <li>@ref HAM_LOCK_EXCLUSIVE Place an exclusive lock on the
 *            file. Only one process may hold an exclusive lock for
 *            a given file at a given time.<br>
 *            Deprecated - this is now the default.
 *       <li>@ref HAM_ENABLE_RECOVERY Enables logging/recovery for this
 *            Database. Not allowed in combination with @ref HAM_IN_MEMORY_DB,
 *            @ref HAM_DISABLE_FREELIST_FLUSH and @ref HAM_WRITE_THROUGH.
 *       <li>@ref HAM_ENABLE_TRANSACTIONS Enables Transactions for this
 *            Database.<br>
 *            <b>Remark</b> Transactions were introduced in hamsterdb 1.0.4,
 *            but with certain limitations. Please read the README file
 *            for details.<br>
 *            This flag implies @ref HAM_ENABLE_RECOVERY.
 *      </ul>
 *
 * @param mode File access rights for the new file. This is the @a mode
 *          parameter for creat(2). Ignored on Microsoft Windows.
 * @param param An array of ham_parameter_t structures. The following
 *          parameters are available:
 *        <ul>
 *        <li>@ref HAM_PARAM_CACHESIZE The size of the Database cache,
 *            in bytes. The default size is defined in src/config.h
 *            as @ref HAM_DEFAULT_CACHESIZE - usually 2 MByte, i.e. 512 pages on
 *            UNIX where 4K pages are usual (or 32 pages on Win32/Win64 where
 *            64K pages are usual).
 *        <li>@ref HAM_PARAM_PAGESIZE The size of a file page, in
 *            bytes. It is recommended not to change the default size. The
 *            default size depends on hardware and operating system.
 *            Page sizes must be a multiple of 512 (8 (number of bits in a byte) * 32 (@ref DB_CHUNKSIZE)).
 *        <li>@ref HAM_PARAM_MAX_ENV_DATABASES The number of maximum
 *            Databases in this Environment; default value: 16.
 *        <li>@ref HAM_PARAM_DATA_ACCESS_MODE Gives a hint regarding data
 *            access patterns. The default setting optimizes hamsterdb
 *            for random read/write access (@ref HAM_DAM_RANDOM_WRITE).
 *            Use @ref HAM_DAM_SEQUENTIAL_INSERT for sequential inserts (this
 *            is automatically set for record number Databases). Use
 *            @ref HAM_DAM_DEFAULT if you want to open this Environment with
 *            hamsterdb versions <= 1.0.9.<br>
 *            Data Access Mode hints can be set for individual Databases, too
 *            (see also @ref ham_create_ex) but the setting configured here
 *            is applied globally to all Databases within this Environment as
 *            the default, unless overridden by an DAM (Data Access Mode) setting
 *            specified with @ref ham_env_create_db or @ref ham_env_open_db .<br>
 *            For more information about available DAM (Data Access Mode)
 *            flags, see @ref ham_data_access_modes.
 *        </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL or an
 *              invalid combination of flags or parameters was specified
 * @return @ref HAM_INV_PARAMETER if the value for
 *              @ref HAM_PARAM_MAX_ENV_DATABASES is too high (either decrease
 *              it or increase the page size)
 * @return @ref HAM_IO_ERROR if the file could not be opened or
 *              reading/writing failed
 * @return @ref HAM_INV_FILE_VERSION if the Environment version is not
 *              compatible with the library version
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_INV_PAGESIZE if @a pagesize is not a multiple of 512
 * @return @ref HAM_INV_KEYSIZE if @a keysize is too large (at least 4
 *              keys must fit in a page)
 * @return @ref HAM_WOULD_BLOCK if another process has locked the file
 * @return @ref HAM_ENVIRONMENT_ALREADY_OPEN if @a env is already in use
 *
 * @sa ham_create_ex
 * @sa ham_env_close
 * @sa ham_env_open_ex
 * @sa ham_data_access_modes
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_create_ex(ham_env_t *env, const char *filename,
        ham_u32_t flags, ham_u32_t mode, const ham_parameter_t *param);

/**
 * Opens an existing Database Environment
 *
 * This function is a simplified version of @sa ham_env_open_ex.
 * It is recommended to use @ref ham_env_open_ex instead.
 *
 * @sa ham_env_open_ex
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_open(ham_env_t *env, const char *filename, ham_u32_t flags);

/**
 * Opens an existing Database Environment - extended version
 *
 * This function opens an existing Database Environment.
 *
 * A Database Environment is a collection of Databases, which are all stored
 * in one physical file (or in-memory). Per default, up to 16 Databases can be
 * stored in one file (see @ref ham_env_create_ex on how to store even more
 * Databases).
 *
 * Each Database in an Environment is identified by a positive 16bit
 * value (except 0 and values at or above @c 0xf000 (@ref HAM_DEFAULT_DATABASE_NAME)).
 * Databases in an Environment can be created with @ref ham_env_create_db
 * or opened with @ref ham_env_open_db.
 *
 * Specify a URL instead of a filename (i.e.
 * "http://localhost:8080/customers.db") to access a remote hamsterdb Server.
 *
 * @param env A valid Environment handle
 * @param filename The filename of the Environment file, or URL of a hamsterdb
 *          Server
 * @param flags Optional flags for opening the Environment, combined with
 *          bitwise OR. Possible flags are:
 *      <ul>
 *       <li>@ref HAM_READ_ONLY Opens the file for reading only.
 *            Operations that need write access (i.e. @ref ham_insert) will
 *            return @ref HAM_DB_READ_ONLY
 *       <li>@ref HAM_WRITE_THROUGH Flushes all file handles after
 *            committing or aborting a Transaction using fsync(), fdatasync()
 *            or FlushFileBuffers(). This flag has no effect
 *            if Transactions are disabled. Slows down performance but makes
 *            sure that all file handles and operating system caches are
 *            transferred to disk, thus providing a stronger durability in case
 *            the computer crashes.
 *       <li>@ref HAM_DISABLE_MMAP Do not use memory mapped files for I/O.
 *            By default, hamsterdb checks if it can use mmap,
 *            since mmap is faster than read/write. For performance
 *            reasons, this flag should not be used.
 *       <li>@ref HAM_CACHE_STRICT Do not allow the cache to grow larger
 *            than @a cachesize. If a Database operation needs to resize the
 *            cache, it will return @ref HAM_CACHE_FULL.
 *            If the flag is not set, the cache is allowed to allocate
 *            more pages than the maximum cache size, but only if it's
 *            necessary and only for a short time.
 *       <li>@ref HAM_CACHE_UNLIMITED Do not limit the cache. Nearly as
 *            fast as an In-Memory Database. Not allowed in combination
 *            with @ref HAM_CACHE_STRICT or a limited cache size.
 *       <li>@ref HAM_DISABLE_FREELIST_FLUSH This flag is deprecated.
 *       <li>@ref HAM_LOCK_EXCLUSIVE Place an exclusive lock on the
 *            file. Only one process may hold an exclusive lock for
 *            a given file at a given time.<br>
 *            Deprecated - this is now the default.
 *       <li>@ref HAM_ENABLE_RECOVERY Enables logging/recovery for this
 *            Database. Will return @ref HAM_NEED_RECOVERY, if the Database
 *            is in an inconsistent state. Not allowed in combination
 *            with @ref HAM_IN_MEMORY_DB, @ref HAM_DISABLE_FREELIST_FLUSH
 *            and @ref HAM_WRITE_THROUGH.
 *       <li>@ref HAM_AUTO_RECOVERY Automatically recover the Database,
 *            if necessary. This flag implies @ref HAM_ENABLE_RECOVERY.
 *       <li>@ref HAM_ENABLE_TRANSACTIONS Enables Transactions for this
 *            Database.<br>
 *            <b>Remark</b> Transactions were introduced in hamsterdb 1.0.4,
 *            but with certain limitations (which will be removed in later
 *            version). Please read the README file and the Release Notes
 *            for details.<br>
 *            This flag implies @ref HAM_ENABLE_RECOVERY.
 *      </ul>
 * @param param An array of ham_parameter_t structures. The following
 *          parameters are available:
 *      <ul>
 *        <li>@ref HAM_PARAM_CACHESIZE The size of the Database cache,
 *            in bytes. The default size is defined in src/config.h
 *            as @ref HAM_DEFAULT_CACHESIZE - usually 2 MByte, i.e. 512 pages on
 *            UNIX where 4K pages are usual (or 32 pages on Win32/Win64 where 64K
 *            pages are usual).
 *        <li>@ref HAM_PARAM_DATA_ACCESS_MODE Gives a hint regarding data
 *            access patterns. The default setting optimizes hamsterdb
 *            for random read/write access (@ref HAM_DAM_RANDOM_WRITE).
 *            Use @ref HAM_DAM_SEQUENTIAL_INSERT for sequential inserts (this
 *            is automatically set for record number Databases). Use
 *            @ref HAM_DAM_DEFAULT if you want to open this Environment with
 *            hamsterdb versions <= 1.0.9.<br>
 *            Data Access Mode hints can be set for individual Databases, too
 *            (see also @ref ham_create_ex) but are applied globally to all
 *            Databases within a single Environment.<br>
 *            For more information about available DAM (Data Access Mode)
 *            flags, see @ref ham_data_access_modes.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success.
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL, an
 *              invalid combination of flags was specified
 * @return @ref HAM_FILE_NOT_FOUND if the file does not exist
 * @return @ref HAM_IO_ERROR if the file could not be opened or reading failed
 * @return @ref HAM_INV_FILE_VERSION if the Environment version is not
 *              compatible with the library version.
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_WOULD_BLOCK if another process has locked the file
 * @return @ref HAM_NEED_RECOVERY if the Database is in an inconsistent state
 * @return @ref HAM_LOG_INV_FILE_HEADER if the logfile is corrupt
 * @return @ref HAM_ENVIRONMENT_ALREADY_OPEN if @a env is already in use
 * @return @ref HAM_NETWORK_ERROR if a remote server is not reachable
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_open_ex(ham_env_t *env, const char *filename,
        ham_u32_t flags, const ham_parameter_t *param);

/**
 * Retrieve the current value for a given Environment setting
 *
 * Only those values requested by the parameter array will be stored. If
 * the @a env handle is NULL, the default parameters will be returned.
 *
 * The following parameters are supported:
 *      <ul>
 *        <li>@ref HAM_PARAM_CACHESIZE returns the cache size
 *        <li>@ref HAM_PARAM_PAGESIZE returns the page size
 *        <li>@ref HAM_PARAM_MAX_ENV_DATABASES returns the max. number of
 *              Databases of this Database's Environment
 *        <li>@ref HAM_PARAM_GET_FLAGS returns the flags which were used to
 *              open or create this Database
 *        <li>@ref HAM_PARAM_GET_FILEMODE returns the @a mode parameter which
 *              was specified when creating this Database
 *        <li>@ref HAM_PARAM_GET_FILENAME returns the filename (the @a value
 *              of this parameter is a const char * pointer casted to a
 *              ham_u64_t variable)
 *        <li>@ref HAM_PARAM_GET_STATISTICS returns a @ref ham_statistics_t
 *              structure with the current statistics
 *      </ul>
 *
 * @param env A valid Environment handle; may be NULL
 * @param param An array of ham_parameter_t structures
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL or
 *              @a param is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_get_parameters(ham_env_t *env, ham_parameter_t *param);

/**
 * Creates a new Database in a Database Environment
 *
 * An Environment can handle up to 16 Databases, unless higher values are
 * configured when the Environment is created (see @sa ham_env_create_ex).
 *
 * Each Database in an Environment is identified by a positive 16bit
 * value (except 0 and values at or above @c 0xf000 (@ref HAM_DEFAULT_DATABASE_NAME)).
 *
 * This function initializes the ham_db_t handle (the second parameter).
 * When the handle is no longer in use, it should be closed with
 * @ref ham_close. Alternatively, the Database handle is closed automatically
 * if @ref ham_env_close is called with the flag @ref HAM_AUTO_CLEANUP.
 *
 * @param env A valid Environment handle.
 * @param db A valid Database handle, which will point to the created
 *          Database. To close the handle, use @ref ham_close.
 * @param name The name of the Database. If a Database with this name
 *          already exists, the function will fail with
 *          @ref HAM_DATABASE_ALREADY_EXISTS. Database names from
 *          @c 0xf000 (@ref HAM_DEFAULT_DATABASE_NAME) to
 *          @c 0xffff and @c 0 are reserved.
 * @param flags Optional flags for creating the Database, combined with
 *          bitwise OR. Possible flags are:
 *      <ul>
 *       <li>@ref HAM_USE_BTREE Use a B+Tree for the index structure.
 *            Currently enabled by default, but future releases
 *            of hamsterdb will offer additional index structures,
 *            like hash tables.
 *       <li>@ref HAM_DISABLE_VAR_KEYLEN Do not allow the use of variable
 *            length keys. Inserting a key, which is larger than the
 *            B+Tree index key size, returns @ref HAM_INV_KEYSIZE.
 *       <li>@ref HAM_ENABLE_DUPLICATES Enable duplicate keys for this
 *            Database. By default, duplicate keys are disabled.
 *       <li>@ref HAM_SORT_DUPLICATES Sort duplicate keys for this
 *            Database. Only allowed in combination with
 *            @ref HAM_ENABLE_DUPLICATES. A compare function can be set with
 *            @ref ham_set_duplicate_compare_func. This flag is not persistent.
 *
 *            <em>WARNING</em>: However, this type of sorted duplicate record order
 *            database is often used as a secundary index into another table
 *            (a 'database' in hamster terminology) and forgetting to set this
 *            flag on database open would corrupt such an application level
 *            secundary index in hard to diagnose ways.
 *       <li>@ref HAM_RECORD_NUMBER Creates an "auto-increment" Database.
 *            Keys in Record Number Databases are automatically assigned an
 *            incrementing 64bit value. If key->data is not NULL
 *            (and key->flags is @ref HAM_KEY_USER_ALLOC and key->size is 8),
 *            the value of the current key is returned in @a key (a
 *            host-endian 64bit number of type @ref ham_u64_t). If key-data is NULL
 *            and key->size is 0, key->data is temporarily allocated by
 *            hamsterdb.
 *      </ul>
 *
 * @param params An array of ham_parameter_t structures. The following
 *        parameters are available:
 *      <ul>
 *        <li>@ref HAM_PARAM_KEYSIZE The size of the keys in the B+Tree
 *            index. The default size is 21 bytes.
 *        <li>@ref HAM_PARAM_DATA_ACCESS_MODE Gives a hint regarding data
 *            access patterns. The default setting optimizes hamsterdb
 *            for random read/write access (@ref HAM_DAM_RANDOM_WRITE).
 *            Use @ref HAM_DAM_SEQUENTIAL_INSERT for sequential inserts (this
 *            is automatically set for record number Databases). Use
 *            @ref HAM_DAM_DEFAULT if you want to open this Environment with
 *            hamsterdb versions <= 1.0.9.<br>
 *            Data Access Mode hints can be set for individual Databases, too
 *            (see also @ref ham_create_ex) but are applied globally to all
 *            Databases within a single Environment.
 *            For more information about available DAM (Data Access Mode)
 *            flags, see @ref ham_data_access_modes.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL or an
 *              invalid combination of flags was specified
 * @return @ref HAM_DATABASE_ALREADY_EXISTS if a Database with this @a name
 *              already exists in this Environment
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_LIMITS_REACHED if the maximum number of Databases per
 *              Environment was already created
 * @return @ref HAM_DATABASE_ALREADY_OPEN if @a db is already in use
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_create_db(ham_env_t *env, ham_db_t *db,
        ham_u16_t name, ham_u32_t flags, const ham_parameter_t *params);

/**
 * Opens a Database in a Database Environment
 *
 * Each Database in an Environment is identified by a positive 16bit
 * value (except 0 and values at or above @c 0xf000 (@ref HAM_DEFAULT_DATABASE_NAME)).
 *
 * This function initializes the ham_db_t handle (the second parameter).
 * When the handle is no longer in use, it should be closed with
 * @ref ham_close. Alternatively, the Database handle is closed automatically
 * if @ref ham_env_close is called with the flag @ref HAM_AUTO_CLEANUP.
 *
 * @param env A valid Environment handle
 * @param db A valid Database handle, which will point to the opened
 *          Database. To close the handle, use @see ham_close.
 * @param name The name of the Database. If a Database with this name
 *          does not exist, the function will fail with
 *          @ref HAM_DATABASE_NOT_FOUND.
 * @param flags Optional flags for opening the Database, combined with
 *        bitwise OR. Possible flags are:
 *     <ul>
 *       <li>@ref HAM_DISABLE_VAR_KEYLEN Do not allow the use of variable
 *            length keys. Inserting a key, which is larger than the
 *            B+Tree index key size, returns @ref HAM_INV_KEYSIZE.
 *       <li>@ref HAM_SORT_DUPLICATES Sort duplicate keys for this
 *            Database. Only allowed if the Database was created with the flag
 *            @ref HAM_ENABLE_DUPLICATES. A compare function can be set with
 *            @ref ham_set_duplicate_compare_func. This flag is not persistent.
 *     </ul>
 *
 * @param params An array of ham_parameter_t structures. The following
 *          parameters are available:
 *      <ul>
 *        <li>@ref HAM_PARAM_DATA_ACCESS_MODE Gives a hint regarding data
 *            access patterns. The default setting optimizes hamsterdb
 *            for random read/write access (@ref HAM_DAM_RANDOM_WRITE).
 *            Use @ref HAM_DAM_SEQUENTIAL_INSERT for sequential inserts (this
 *            is automatically set for record number Databases).
 *            Data Access Mode hints can be set for individual Databases, too
 *            (see also @ref ham_create_ex) but are applied globally to all
 *            Databases within a single Environment.
 *            For more information about available DAM (Data Access Mode)
 *            flags, see @ref ham_data_access_modes.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL or an
 *              invalid combination of flags was specified
 * @return @ref HAM_DATABASE_NOT_FOUND if a Database with this @a name
 *              does not exist in this Environment.
 * @return @ref HAM_DATABASE_ALREADY_OPEN if this Database was already
 *              opened
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_DATABASE_ALREADY_OPEN if @a db is already in use
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_open_db(ham_env_t *env, ham_db_t *db,
        ham_u16_t name, ham_u32_t flags, const ham_parameter_t *params);

/**
 * Renames a Database in an Environment.
 *
 * @param env A valid Environment handle.
 * @param oldname The old name of the existing Database. If a Database
 *          with this name does not exist, the function will fail with
 *          @ref HAM_DATABASE_NOT_FOUND.
 * @param newname The new name of this Database. If a Database
 *          with this name already exists, the function will fail with
 *          @ref HAM_DATABASE_ALREADY_EXISTS.
 * @param flags Optional flags for renaming the Database, combined with
 *        bitwise OR; unused, set to 0.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL or if
 *              the new Database name is reserved
 * @return @ref HAM_DATABASE_NOT_FOUND if a Database with this @a name
 *              does not exist in this Environment
 * @return @ref HAM_DATABASE_ALREADY_EXISTS if a Database with the new name
 *              already exists
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_NOT_READY if the Environment @a env was not initialized
 *              correctly (i.e. not yet opened or created)
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_rename_db(ham_env_t *env, ham_u16_t oldname,
                ham_u16_t newname, ham_u32_t flags);

/**
 * Deletes a Database from an Environment
 *
 * @param env A valid Environment handle
 * @param name The name of the Database to delete. If a Database
 *          with this name does not exist, the function will fail with
 *          @ref HAM_DATABASE_NOT_FOUND. If the Database was already opened,
 *          the function will fail with @ref HAM_DATABASE_ALREADY_OPEN.
 * @param flags Optional flags for deleting the Database; unused, set to 0.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a env pointer is NULL or if
 *              the new Database name is reserved
 * @return @ref HAM_DATABASE_NOT_FOUND if a Database with this @a name
 *              does not exist
 * @return @ref HAM_DATABASE_ALREADY_OPEN if a Database with this name is
 *              still open
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_erase_db(ham_env_t *env, ham_u16_t name, ham_u32_t flags);

/**
 * Enables AES encryption
 *
 * This function enables AES encryption for every Database in the Environment.
 * The AES key is cached in the Environment handle. The AES
 * encryption/decryption is only active when file chunks are written to
 * disk/read from disk; the cached pages in RAM are decrypted. Please read
 * the FAQ for security relevant notes.
 *
 * The encryption has no effect on In-Memory Environments, but the function
 * will return @ref HAM_SUCCESS.
 *
 * Log files and the header page of the Database are not encrypted.
 *
 * The encryption will be active till @ref ham_env_close is called. If the
 * Environment handle is reused after calling @ref ham_env_close, the
 * encryption is no longer active. @ref ham_env_enable_encryption should
 * be called immediately <b>after</b> @ref ham_env_create[_ex] or @ref ham_env_open[_ex],
 * and MUST be called <b>before</b> @ref ham_env_create_db or @ref ham_env_open_db.
 *
 * @param env A valid Environment handle
 * @param key A 128bit AES key
 * @param flags Optional flags for encrypting; unused, set to 0
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if one of the parameters is NULL
 * @return @ref HAM_DATABASE_ALREADY_OPEN if this function was called AFTER
 *              @ref ham_env_open_db or @ref ham_env_create_db
 * @return @ref HAM_NOT_IMPLEMENTED if hamsterdb was compiled without support
 *              for AES encryption
 * @return @ref HAM_ACCESS_DENIED if the key (= password) was wrong
 * @return @ref HAM_ALREADY_INITIALIZED if encryption is already enabled
 *              for this Environment
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_enable_encryption(ham_env_t *env, ham_u8_t key[16], ham_u32_t flags);

/**
 * Returns the names of all Databases in an Environment
 *
 * This function returns the names of all Databases and the number of
 * Databases in an Environment.
 *
 * The memory for @a names must be allocated by the user. @a count
 * must be the size of @a names when calling the function, and will be
 * the number of Databases when the function returns. The function returns
 * @ref HAM_LIMITS_REACHED if @a names is not big enough; in this case, the
 * caller should resize the array and call the function again.
 *
 * You may call
 * @ref ham_env_get_parameters with the @ref HAM_PARAM_MAX_ENV_DATABASES
 * argument to obtain the hamsterdb-suggested size of the @a names array.
 *
 * @param env A valid Environment handle
 * @param names Pointer to an array for the Database names
 * @param count Pointer to the size of the array; will be used to store the
 *          number of Databases when the function returns.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a env, @a names or @a count is NULL
 * @return @ref HAM_LIMITS_REACHED if @a names is not large enough to hold
 *          all Database names; @a count will contain the required minimum
 *          number of name slots.
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_get_database_names(ham_env_t *env, ham_u16_t *names, ham_size_t *count);

/**
 * Closes the Database Environment
 *
 * This function closes the Database Environment. It does not free the
 * memory resources allocated in the @a env handle - use @ref ham_env_delete
 * to free @a env.
 *
 * If the flag @ref HAM_AUTO_CLEANUP is specified, hamsterdb automatically
 * calls @ref ham_close with flag @ref HAM_AUTO_CLEANUP on all open Databases
 * (which closes all open Databases and their Cursors). This invalidates the
 * @ref ham_db_t and @ref ham_cursor_t handles!
 *
 * If the flag is not specified, the application must close all Database
 * handles with @ref ham_close to prevent memory leaks.
 *
 * This function also aborts all Transactions which were not yet committed,
 * and therefore renders all Transaction handles invalid. If the flag
 * @ref HAM_TXN_AUTO_COMMIT is specified, all Transactions will be committed.
 *
 * @if vanilla_hamster
 * This function removes all file-level filters installed
 * with @ref ham_env_add_file_filter (and hence also, implicitly,
 * the filter installed by @ref ham_env_enable_encryption).
 * @endif
 *
 * @param env A valid Environment handle
 * @param flags Optional flags for closing the handle. Possible flags are:
 *          <ul>
 *            <li>@ref HAM_AUTO_CLEANUP. Calls @ref ham_close with the flag
 *                @ref HAM_AUTO_CLEANUP on every open Database
 *            <li>@ref HAM_TXN_AUTO_COMMIT. Automatically commit all open
 *               Transactions
 *            <li>@ref HAM_TXN_AUTO_ABORT. Automatically abort all open
 *               Transactions; this is the default behaviour
 *          </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a env is NULL
 *
 * @sa HAM_AUTO_CLEANUP
 * @sa ham_close
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_close(ham_env_t *env, ham_u32_t flags);

/**
 * @}
 */








/**
 * @defgroup ham_txn hamsterdb Transaction Functions
 * @{
 */

/**
 * The hamsterdb Transaction structure
 *
 * This structure is allocated with @ref ham_txn_begin and deleted with
 * @ref ham_txn_commit or @ref ham_txn_abort.
 */
struct ham_txn_t;
typedef struct ham_txn_t ham_txn_t;

/**
 * Begins a new Transaction
 *
 * A Transaction is an atomic sequence of Database operations. With @ref
 * ham_txn_begin a new sequence is started. To write all operations of this
 * sequence to the Database use @ref ham_txn_commit. To abort and cancel
 * this sequence use @ref ham_txn_abort.
 *
 * In order to use Transactions, the Environment has to be created or
 * opened with the flag @ref HAM_ENABLE_TRANSACTIONS.
 *
 * Although for historical reasons @ref ham_txn_begin creates a Transaction
 * and attaches it to a Database (the second parameter is a @ref ham_db_t
 * handle), the Transaction is actually valid for the whole Environment.
 *
 * @param txn Pointer to a pointer of a Transaction structure
 * @param db A valid Database handle
 * @param flags Optional flags for beginning the Transaction, combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *       <li>@ref HAM_TXN_READ_ONLY This Transaction is read-only and
 *            will not modify the Database.
 *      </ul>
 *
 * @note Note that as of hamsterdb 1.0.4, it is not possible to create
 *       multiple Transactions in parallel. This limitation will be removed
 *       in further versions.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_OUT_OF_MEMORY if memory allocation failed
 * @return @ref HAM_LIMITS_REACHED if there's already an open Transaction
 *          (see above)
 */
HAM_EXPORT ham_status_t
ham_txn_begin(ham_txn_t **txn, ham_db_t *db, ham_u32_t flags);

/** Flag for @ref ham_txn_begin */
#define HAM_TXN_READ_ONLY                                       1

/**
 * Commits a Transaction
 *
 * This function applies the sequence of Database operations.
 *
 * Note that the function will fail with @ref HAM_CURSOR_STILL_OPEN if
 * a Cursor was attached to this Transaction (with @ref ham_cursor_create
 * or @ref ham_cursor_clone), and the Cursor was not closed.
 *
 * To improve the Durability, you can specify the flag @ref HAM_WRITE_THROUGH
 * when opening or creating the Environment. hamsterdb will then flush all
 * open file handles when committing or aborting a Transaction using fsync(),
 * fdatasync() or FlushFileBuffers(). This slows down performance but makes
 * sure that all file handles and operating system caches are immediately
 * transferred to disk, thus providing a stronger durability in case the
 * computer crashes.
 *
 * @param txn Pointer to a Transaction structure
 * @param flags Optional flags for committing the Transaction, combined with
 *        bitwise OR. Unused, set to 0.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_IO_ERROR if writing to the file failed
 * @return @ref HAM_CURSOR_STILL_OPEN if there are Cursors attached to this
 *          Transaction
 */
HAM_EXPORT ham_status_t
ham_txn_commit(ham_txn_t *txn, ham_u32_t flags);


/**
 * Aborts a Transaction
 *
 * This function aborts (= cancels) the sequence of Database operations.
 *
 * Note that the function will fail with @ref HAM_CURSOR_STILL_OPEN if
 * a Cursor was attached to this Transaction (with @ref ham_cursor_create
 * or @ref ham_cursor_clone), and the Cursor was not closed.
 *
 * To improve the Durability, you can specify the flag @ref HAM_WRITE_THROUGH
 * when opening or creating the Environment. hamsterdb will then flush all
 * open file handles when committing or aborting a Transaction using fsync(),
 * fdatasync() or FlushFileBuffers(). This slows down performance but makes
 * sure that all file handles and operating system caches are immediately
 * transferred to disk, thus providing a stronger durability in case the
 * computer crashes.
 *
 * @param txn Pointer to a Transaction structure
 * @param flags Optional flags for aborting the Transaction, combined with
 *        bitwise OR. Unused, set to 0.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_IO_ERROR if writing to the Database file or logfile failed
 * @return @ref HAM_CURSOR_STILL_OPEN if there are Cursors attached to this
 *          Transaction
 */
HAM_EXPORT ham_status_t
ham_txn_abort(ham_txn_t *txn, ham_u32_t flags);

/* note: ham_txn_abort flag 0x0001 is reserved for internal use:
 * DO_NOT_NUKE_PAGE_STATS */

/**
 * @}
 */










/**
 * @defgroup ham_database hamsterdb Database Functions
 *
One or more 'databases' (also known as 'indexes' in database parlance) are
stored within a hamsterdb 'environment' (which is somewhat comparable to,
say, an Oracle schema).

The environment is implicit (but nevertheless still extant!) when a sole
database is created using @ref ham_new, followed by @ref ham_create or
@ref ham_create_ex or when opened using @ref ham_new, followed by
@ref ham_open or @ref ham_open_ex. Otherwise, an @e explicit environment must
be created or opened before the databases (indexes) contained there-in can be
accessed: this would, for instance, require calling the hamsterdb APIs
@ref ham_env_new, @ref ham_env_create, @ref ham_new and
@ref ham_env_create_db when creating a new database in a fresh (empty)
environment, while @ref ham_env_new, @ref ham_env_open, @ref ham_new plus
@ref ham_env_open_db can be invoked to access an already existing database
in a given environment.

Of course, additional databases can be added to any environment using
@ref ham_env_create_db.

The @ref ham_env_new call is mandatory when using explicit environments as
this call sets up a new environment structure to be used by hamsterdb to
track the environment status. The same goes for the @ref ham_new call which
sets up a fresh database structure for the same purposes.

When done, databases and environments must be closed and cleaned up using
@ref ham_close (which works for both implicit and explicit environments
alike, which explains why there is no @c ham_env_close_db API),
@ref ham_env_close and @ref ham_delete, @ref ham_env_delete respectively.

Meanwhile (i.e. while the database is 'active'/open) any of the
@ref ham_find, @ref ham_insert, @ref ham_erase, @ref ham_cursor_find,
@ref ham_cursor_find_ex, @ref ham_cursor_insert, @ref ham_cursor_erase,
@ref ham_cursor_move and @ref ham_get_key_count APIs can be used to extract,
add or erase information from the database.

@note
The @ref ham_env_rename_db and @ref ham_env_erase_db APIs require that the
addressed database is not 'active' ~ referenced by any @ref ham_db_t instance
when these APIs are invoked.

 * @{
 */

/**
 * Allocates a ham_db_t handle
 *
 * @param db Pointer to the pointer which is allocated
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_OUT_OF_MEMORY if memory allocation failed
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_new(ham_db_t **db);

/**
 * Frees a ham_db_t handle
 *
 * Frees the memory and resources of a ham_db_t structure, but does not
 * close the Database. Call this function <b>AFTER</b> you have closed the
 * Database using @ref ham_close, or you will lose your data!
 *
 * @param db A valid Database handle
 *
 * @return This function always returns @ref HAM_SUCCESS
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_delete(ham_db_t *db);

/**
 * Creates a Database
 *
 * This function is a simplified version of @sa ham_create_ex.
 * It is recommended to use @ref ham_create_ex instead.
 *
 * @sa ham_create_ex
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_create(ham_db_t *db, const char *filename,
        ham_u32_t flags, ham_u32_t mode);

/**
 * Creates a Database - extended version
 *
 * This function is a shortcut for a sequence of @ref ham_env_create_ex
 * followed by @ref ham_env_create_db.
 *
 * It creates an (internal, hidden) Environment and in this Environment it
 * creates a Database with a reserved identifier - HAM_DEFAULT_DATABASE_NAME.
 *
 * As a consequence, it is no problem to create a Database with
 * @ref ham_create_ex, and later open it with @ref ham_env_open_ex.
 *
 * The internal Environment handle can be retrieved with @ref ham_get_env.
 *
 * @param db A valid Database handle
 * @param filename The filename of the Database file. If the file already
 *          exists, it will be overwritten. Can be NULL if you create an
 *          In-Memory Database
 * @param flags Optional flags for opening the Database, combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *       <li>@ref HAM_WRITE_THROUGH Flushes all file handles after
 *            committing or aborting a Transaction using fsync(), fdatasync()
 *            or FlushFileBuffers(). This flag has no effect
 *            if Transactions are disabled. Slows down performance but makes
 *            sure that all file handles and operating system caches are
 *            transferred to disk, thus providing a stronger durability in case
 *            the computer crashes.
 *       <li>@ref HAM_USE_BTREE Use a B+Tree for the index structure.
 *            Currently enabled by default, but future releases
 *            of hamsterdb will offer additional index structures,
 *            like hash tables.
 *       <li>@ref HAM_DISABLE_VAR_KEYLEN Do not allow the use of variable
 *            length keys. Inserting a key, which is larger than the
 *            B+Tree index key size, returns @ref HAM_INV_KEYSIZE.
 *       <li>@ref HAM_IN_MEMORY_DB Creates an In-Memory Database. No file
 *            will be created, and the Database contents are lost after
 *            the Database is closed. The @a filename parameter can
 *            be NULL. Do <b>NOT</b> use in combination with
 *            @ref HAM_CACHE_STRICT and do <b>NOT</b> specify @a cachesize
 *            other than 0.
 *       <li>@ref HAM_RECORD_NUMBER Creates an "auto-increment" Database.
 *            Keys in Record Number Databases are automatically assigned an
 *            incrementing 64bit value. If key->data is not NULL
 *            (and key->flags is @ref HAM_KEY_USER_ALLOC and key->size is 8),
 *            the value of the current key is returned in @a key (a
 *            host-endian 64bit number of type ham_u64_t). If key-data is NULL
 *            and key->size is 0, key->data is temporarily allocated by
 *            hamsterdb.
 *       <li>@ref HAM_ENABLE_DUPLICATES Enable duplicate keys for this
 *            Database. By default, duplicate keys are disabled.
 *       <li>@ref HAM_SORT_DUPLICATES Sort duplicate keys for this
 *            Database. Only allowed in combination with
 *            @ref HAM_ENABLE_DUPLICATES. A compare function can be set with
 *            @ref ham_set_duplicate_compare_func. This flag is not persistent.
 *       <li>@ref HAM_DISABLE_MMAP Do not use memory mapped files for I/O.
 *            By default, hamsterdb checks if it can use mmap,
 *            since mmap is faster than read/write. For performance
 *            reasons, this flag should not be used.
 *       <li>@ref HAM_CACHE_STRICT Do not allow the cache to grow larger
 *            than @a cachesize. If a Database operation needs to resize the
 *            cache, it will return @ref HAM_CACHE_FULL.
 *            If the flag is not set, the cache is allowed to allocate
 *            more pages than the maximum cache size, but only if it's
 *            necessary and only for a short time.
 *       <li>@ref HAM_CACHE_UNLIMITED Do not limit the cache. Nearly as
 *            fast as an In-Memory Database. Not allowed in combination
 *            with @ref HAM_CACHE_STRICT or a limited cache size.
 *       <li>@ref HAM_DISABLE_FREELIST_FLUSH This flag is deprecated.
 *       <li>@ref HAM_LOCK_EXCLUSIVE Place an exclusive lock on the
 *            file. Only one process may hold an exclusive lock for
 *            a given file at a given time.<br>
 *            Deprecated - this is now the default.
 *       <li>@ref HAM_ENABLE_RECOVERY Enables logging/recovery for this
 *            Database. Not allowed in combination with @ref HAM_IN_MEMORY_DB,
 *            @ref HAM_DISABLE_FREELIST_FLUSH and @ref HAM_WRITE_THROUGH.
 *       <li>@ref HAM_ENABLE_TRANSACTIONS Enables Transactions for this
 *            Database.<br>
 *            <b>Remark</b> Transactions were introduced in hamsterdb 1.0.4,
 *            but with certain limitations (which will be removed in later
 *            version). Please read the README file and the Release Notes
 *            for details.<br>
 *            This flag implies @ref HAM_ENABLE_RECOVERY.
 *      </ul>
 *
 * @param mode File access rights for the new file. This is the @a mode
 *        parameter for creat(2). Ignored on Microsoft Windows.
 * @param param An array of ham_parameter_t structures. The following
 *        parameters are available:
 *      <ul>
 *        <li>@ref HAM_PARAM_CACHESIZE The size of the Database cache,
 *            in bytes. The default size is defined in src/config.h
 *            as @ref HAM_DEFAULT_CACHESIZE - usually 2 MByte, i.e. 512 pages on
 *            UNIX where 4K pages are usual (or 32 pages on Win32/Win64 where 64K
 *            pages are usual).
 *        <li>@ref HAM_PARAM_PAGESIZE The size of a file page, in
 *            bytes. It is recommended not to change the default size. The
 *            default size depends on hardware and operating system.
 *            Page sizes must be a multiple of 512 (8 (number of bits in a byte) * 32 (@ref DB_CHUNKSIZE)).
 *        <li>@ref HAM_PARAM_KEYSIZE The size of the keys in the B+Tree
 *            index. The default size is 21 bytes.
 *        <li>@ref HAM_PARAM_DATA_ACCESS_MODE Gives a hint regarding data
 *            access patterns. The default setting optimizes hamsterdb
 *            for random read/write access (@ref HAM_DAM_RANDOM_WRITE).
 *            Use @ref HAM_DAM_SEQUENTIAL_INSERT for sequential inserts (this
 *            is automatically set for record number Databases). Use
 *            @ref HAM_DAM_DEFAULT if you want to open this Database with
 *            hamsterdb versions <= 1.0.9.
 *            For more information about available DAM (Data Access Mode)
 *            flags, see @ref ham_data_access_modes.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a db pointer is NULL or an
 *              invalid combination of flags was specified
 * @return @ref HAM_IO_ERROR if the file could not be opened or
 *              reading/writing failed
 * @return @ref HAM_INV_FILE_VERSION if the Database version is not
 *              compatible with the library version
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_INV_PAGESIZE if @a pagesize is not a multiple of 512
 * @return @ref HAM_INV_KEYSIZE if @a keysize is too large (at least 4
 *              keys must fit in a page)
 * @return @ref HAM_WOULD_BLOCK if another process has locked the file
 * @return @ref HAM_DATABASE_ALREADY_OPEN if @a db is already in use
 *
 * @sa ham_data_access_modes
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_create_ex(ham_db_t *db, const char *filename,
        ham_u32_t flags, ham_u32_t mode, const ham_parameter_t *param);

/**
 * Opens an existing Database
 *
 * This function is a simplified version of @sa ham_open_ex.
 * It is recommended to use @ref ham_open_ex instead.
 *
 * @sa ham_open_ex
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_open(ham_db_t *db, const char *filename, ham_u32_t flags);

/**
 * Opens an existing Database - extended version
 *
 * This function is a shortcut for a sequence of @ref ham_env_open_ex
 * followed by @ref ham_env_open_db.
 *
 * It opens an (internal, hidden) Environment and in this Environment it
 * opens the first Database that was created.
 *
 * As a consequence, it is no problem to open a Database with
 * @ref ham_open_ex, even if it was created with @ref ham_env_create_ex.
 *
 * The internal Environment handle can be retrieved with @ref ham_get_env.
 *
 * @param db A valid Database handle
 * @param filename The filename of the Database file
 * @param flags Optional flags for opening the Database, combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *       <li>@ref HAM_READ_ONLY Opens the file for reading only.
 *            Operations which need write access (i.e. @ref ham_insert) will
 *            return @ref HAM_DB_READ_ONLY.
 *       <li>@ref HAM_WRITE_THROUGH Flushes all file handles after
 *            committing or aborting a Transaction using fsync(), fdatasync()
 *            or FlushFileBuffers(). This flag has no effect
 *            if Transactions are disabled. Slows down performance but makes
 *            sure that all file handles and operating system caches are
 *            transferred to disk, thus providing a stronger durability in case
 *            the computer crashes.
 *       <li>@ref HAM_DISABLE_VAR_KEYLEN Do not allow the use of variable
 *            length keys. Inserting a key, which is larger than the
 *            B+Tree index key size, returns @ref HAM_INV_KEYSIZE.
 *       <li>@ref HAM_DISABLE_MMAP Do not use memory mapped files for I/O.
 *            By default, hamsterdb checks if it can use mmap,
 *            since mmap is faster than read/write. For performance
 *            reasons, this flag should not be used.
 *       <li>@ref HAM_CACHE_STRICT Do not allow the cache to grow larger
 *            than @a cachesize. If a Database operation needs to resize the
 *            cache, it will return @ref HAM_CACHE_FULL.
 *            If the flag is not set, the cache is allowed to allocate
 *            more pages than the maximum cache size, but only if it's
 *            necessary and only for a short time.
 *       <li>@ref HAM_CACHE_UNLIMITED Do not limit the cache. Nearly as
 *            fast as an In-Memory Database. Not allowed in combination
 *            with @ref HAM_CACHE_STRICT or a limited cache size.
 *       <li>@ref HAM_DISABLE_FREELIST_FLUSH This flag is deprecated.
 *       <li>@ref HAM_LOCK_EXCLUSIVE Place an exclusive lock on the
 *            file. Only one process may hold an exclusive lock for
 *            a given file at a given time.<br>
 *            Deprecated - this is now the default.
 *       <li>@ref HAM_ENABLE_RECOVERY Enables logging/recovery for this
 *            Database. Will return @ref HAM_NEED_RECOVERY, if the Database
 *            is in an inconsistent state. Not allowed in combination
 *            with @ref HAM_IN_MEMORY_DB, @ref HAM_DISABLE_FREELIST_FLUSH
 *            and @ref HAM_WRITE_THROUGH.
 *       <li>@ref HAM_AUTO_RECOVERY Automatically recover the Database,
 *            if necessary. This flag implies @ref HAM_ENABLE_RECOVERY.
 *       <li>@ref HAM_ENABLE_TRANSACTIONS Enables Transactions for this
 *            Database.<br>
 *            <b>Remark</b> Transactions were introduced in hamsterdb 1.0.4,
 *            but with certain limitations. Please read the README file
 *            for details.<br>
 *            This flag implies @ref HAM_ENABLE_RECOVERY.
 *       <li>@ref HAM_SORT_DUPLICATES Sort duplicate keys for this
 *            Database. Only allowed if the Database was created with the flag
 *            @ref HAM_ENABLE_DUPLICATES. A compare function can be set with
 *            @ref ham_set_duplicate_compare_func. This flag is not persistent.
 *      </ul>
 *
 * @param param An array of ham_parameter_t structures. The following
 *        parameters are available:
 *      <ul>
 *        <li>@ref HAM_PARAM_CACHESIZE The size of the Database cache,
 *            in bytes. The default size is defined in src/config.h
 *            as @ref HAM_DEFAULT_CACHESIZE - usually 2 MByte, i.e. 512 pages on
 *            UNIX where 4K pages are usual (or 32 pages on Win32/Win64 where
 *            64K pages are usual).
 *        <li>@ref HAM_PARAM_DATA_ACCESS_MODE Gives a hint regarding data
 *            access patterns. The default setting optimizes hamsterdb
 *            for random read/write access (@ref HAM_DAM_RANDOM_WRITE).
 *            Use @ref HAM_DAM_SEQUENTIAL_INSERT for sequential inserts (this
 *            is automatically set for record number Databases). Use
 *            @ref HAM_DAM_DEFAULT if you want to open this Environment with
 *            hamsterdb versions <= 1.0.9.
 *            Data Access Mode hints can be set for individual Databases, too
 *            (see also @ref ham_create_ex) but are applied globally to all
 *            Databases within a single Environment.
 *            For more information about available DAM (Data Access Mode)
 *            flags, see @ref ham_data_access_modes.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a db pointer is NULL or an
 *              invalid combination of flags was specified
 * @return @ref HAM_FILE_NOT_FOUND if the file does not exist
 * @return @ref HAM_IO_ERROR if the file could not be opened or reading failed
 * @return @ref HAM_INV_FILE_VERSION if the Database version is not
 *              compatible with the library version
 * @return @ref HAM_OUT_OF_MEMORY if memory could not be allocated
 * @return @ref HAM_WOULD_BLOCK if another process has locked the file
 * @return @ref HAM_NEED_RECOVERY if the Database is in an inconsistent state
 * @return @ref HAM_LOG_INV_FILE_HEADER if the logfile is corrupt
 * @return @ref HAM_DATABASE_ALREADY_OPEN if @a db is already in use
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_open_ex(ham_db_t *db, const char *filename,
        ham_u32_t flags, const ham_parameter_t *param);

/**
 * @defgroup ham_database_flags hamsterdb Database Access Flags
 * @{
 *
 * These flags can be bitwise-OR-ed together.
 */

/** Flag for @ref ham_open, @ref ham_open_ex, @ref ham_create,
 * @ref ham_create_ex.
 *
 * This flag is non persistent. */
#define HAM_WRITE_THROUGH            0x00000001u

/* unused                            0x00000002u */

/** Flag for @ref ham_open, @ref ham_open_ex.
 *
 * This flag is non persistent. */
#define HAM_READ_ONLY                0x00000004u

/* unused                            0x00000008u */

/**
Use a B+-tree index for the database.

Flag for @ref ham_create, @ref ham_create_ex.

This flag is persisted in the Database.

@remark This is the default index. Not selecting any index implies you selected this one.
*/
#define HAM_USE_BTREE                0x00000010u

/**
Use a hash index for the Database.

Flag for @ref ham_create, @ref ham_create_ex.

This flag is persisted in the Database. */
#define HAM_USE_HASH                 0x00000020u

/**
Use a custom database/index backend ('algorithm') for the Database.

You can do so by registering your own backend with the @ref ham_db_t Database handle
before you actually create or open the database itself (an action which will 'activate'
your backend by calling its 'create' method/callback), i.e. you must register your
custom backend before invoking any of these:

- @ref ham_create
- @ref ham_open
- @ref ham_env_create_db
- @ref ham_env_open_db

Flag for @ref ham_create, @ref ham_create_ex.

This flag is persisted in the Database. */
#define HAM_USE_CUSTOM_DB_ALGO       0x00000030u

/** Bitmask to aid extraction of the database
storage algorithm (B+-tree, Cuckoo hash, custom)
from the flag bitset.

This is not a flag per se but rather an aid for code which needs to extract
the 'index type' from the flag bitset.
*/
#define HAM_USE_DB_ALGO_MASK         0x00000030u

/**
Disable Variable key length support for this Database.

This means that <em>this Database does not support extended keys</em>.
You still may store and access keys of 'variable length' but all keys
<em>must</em> have a length equal or less than the configured maximum
key length for this Database (which is set up using the
@ref HAM_PARAM_KEYSIZE parameter) -- the default maximum key length is 21.

Flag for @ref ham_create, @ref ham_create_ex.
 *
 * This flag is persisted in the Database.
 */
#define HAM_DISABLE_VAR_KEYLEN       0x00000040u

/**
Instruct the hamster to create a RAM-based Environment/Database.

@note These RAM-based databases are intended as temporary stores: nothing is persisted
      beyond the first related occurrence of
      @ref ham_close / @ref ham_delete / @ref ham_env_close / @ref ham_env_delete .

Flag for @ref ham_create, @ref ham_create_ex, @ref ham_env_create, @ref ham_env_create_ex.

This flag is 'persisted', though not for long:
In-memory Databases are discarded on close anyhow.
*/
#define HAM_IN_MEMORY_DB             0x00000080u

/* reserved: DB_USE_MMAP (not persistent)      0x00000100u */

/** Do not use Memory Mapped I/O facilities available in
 * your Operating System, but use standard read/write
 * (@ref pread / @ref pwrite) instead.
 *
 * Flag for @ref ham_open, @ref ham_open_ex, @ref ham_create,
 * @ref ham_create_ex, @ref ham_env_open, @ref ham_env_open_ex, @ref ham_env_create,
 * @ref ham_env_create_ex.
 *
 * This flag is non persistent.
 *
 * @note This flag is also used as a @ref ham_device_t flag.
 *
 * @sa ham_device_t
 */
#define HAM_DISABLE_MMAP             0x00000200u

/** Flag for @ref ham_open, @ref ham_open_ex, @ref ham_create,
 * @ref ham_create_ex.
 *
 * This flag is non persistent. */
#define HAM_CACHE_STRICT             0x00000400u

/** @deprecated Flag for @ref ham_open, @ref ham_open_ex, @ref ham_create,
 * @ref ham_create_ex.
 *
 * This flag is non persistent. */
#define HAM_DISABLE_FREELIST_FLUSH   0x00000800u

/** Flag for @ref ham_open, @ref ham_open_ex, @ref ham_create,
 * @ref ham_create_ex */
#define HAM_LOCK_EXCLUSIVE           0x00001000u

/** Flag for @ref ham_create, @ref ham_create_ex, @ref ham_env_create_db.
 * This flag is persisted in the Database. */
#define HAM_RECORD_NUMBER            0x00002000u

/** Flag for @ref ham_create, @ref ham_create_ex.
 * This flag is persisted in the Database.
 *
 * @sa HAM_SORT_DUPLICATES
 */
#define HAM_ENABLE_DUPLICATES        0x00004000u

/** Flag for @ref ham_create_ex, @ref ham_open_ex, @ref ham_env_create_ex,
 * @ref ham_env_open_ex.
 *
 * This flag is non persistent. */
#define HAM_ENABLE_RECOVERY          0x00008000u

/**
 * Perform a crash recovery automatically when opening the Environment or
 * Database which is found not to be properly terminated during the previous
 * run. Without this flag, such Environments and Databases will produce the
 * @ref HAM_NEED_RECOVERY error code when the store is found not to be
 * properly terminated during the previous run, after which you would have
 * to call @ref ham_open or @ref ham_env_open with the @ref HAM_AUTO_RECOVERY
 * flag set after all to allow the hamster to perform the required recovery
 * from the old crash recovery log files.
 *
 * Flag for @ref ham_open_ex, @ref ham_env_open_ex.
 *
 * This flag is non persistent. */
#define HAM_AUTO_RECOVERY            0x00010000u

/**
 * Enable transaction support in the Environment. Implies @ref HAM_ENABLE_RECOVERY
 * so expect a lower performance then without transactions.
 *
 * Flag for @ref ham_create_ex, @ref ham_open_ex, @ref ham_env_create_ex,
 * @ref ham_env_open_ex.
 *
 * This flag is non persistent. */
#define HAM_ENABLE_TRANSACTIONS      0x00020000u

/** Flag for @ref ham_open, @ref ham_open_ex, @ref ham_create,
 * @ref ham_create_ex.
 *
 * This flag is non persistent. */
#define HAM_CACHE_UNLIMITED          0x00040000u

/* reserved: DB_ENV_IS_PRIVATE (not persistent)      0x00080000u */

/** Flag for @ref ham_create, @ref ham_create_ex, @ref ham_env_create_db,
 * @ref ham_open, @ref ham_open_ex, @ref ham_env_open_db
 *
 * This flag is persisted in the Database.
 *
 * @sa HAM_ENABLE_DUPLICATES
 * @sa ham_cursor_insert
 */
#define HAM_SORT_DUPLICATES          0x00100000u

/* reserved: DB_IS_REMOTE     (not persistent)       0x00200000u */

/**
The B+tree nodes are augmented with a 'fast index':

This is a 16bit-word-per-key index at the start of the Btree node which indicates
where each key is located in the key array of the node.

It is a 'fast index' as it is designed to reduce the O(M) insert/erase constant
by keeping the sort order intact through this index only, which cuts down
significantly on memmove() overhead on insert and delete (erase).

A 'find' operation sampling keys needs to go through this index: the sort order
is guaranteed in this index instead of the key array itself, which means each key
load costs another (word-based) indirection: key = fast_index[index_no], i.e. one memory
load per key.

An 'insert' operation storing a key can simply 'dump' the key at the end of the
key array and only needs to adjust the 'fast index' to keep sort order intact: the
cost is reduced from an average memmove() of O((kz+11)*M/2) to O(2*M/2) where
M is the total number of keys in the node and kz is the key size (the +11 accounts for
the per-key overhead in the Btree node key array).
For a default key size of 21 the cost reduction is therefore nearly a factor of 16 already:
kz=21 --> C=kz+11=32 vs C=2 for same number of M.

A 'delete' operation removing a key has the same cost gain as was specified for the 'insert'
operation as this operation too needs to guarantee packed sort order of the key array,
which is now guaranteed through maintaining the 'fast index' instead: an 'erase' operation
now removes a key by overwriting its key array slot with the key data of the last key
in the key array, then decrementing the key array size by one to correct for the single key
overwrite/move action.
The classic code would memmove() M/2 keys on average instead, so the gain here is a little
less than for insert as one key gets moved instead of M/2:
O(2*M/2+(kz+11)*1) vs. O((kz+11)*M/2)

Simply put, when this gain compensates for the added indirection cost for 'find', which is
one additional indexed memory access to translate the key slot number, then this is an overall
speed gain. Assuming the default key size and not accounting for CPU/memory hardware cache lines etc.
a ratio of 1 insert or erase per 16 find operations is the guestimated break-even point: more
find versus insert or erase means that you should disable this optimization.
In actual practice, given CPU cache line and other hardware aspects, one may assume this
break-even ratio to be less than 16; benchmarking on your target hardware is mandatory to
determine the actual break-even point for your database and hardware configuration.

This flag is persisted in the Database.

@remark Only valid when the database uses a B+-tree index (@ref HAM_USE_BTREE)
*/
#define HAM_BTREE_NODES_HAVE_FAST_INDEX 0x00400000u

/**
 * @}
 */


/* WARNING: doxygen '@defgroup' lines cannot be 'wrapped':
 * they MUST remain a single line of text for doxygen to properly
 * extract the defgroup title */

/**
 * @defgroup ham_database_cfg_parameters hamsterdb Database Configuration Parameters
 * @{
 *
 * @sa ham_parameter_t
 */

/** Parameter name for @ref ham_open_ex, @ref ham_create_ex; sets the cache
 * size.
 *
 * The unit is BYTES, though for easy of use and backwards compatibility the number, when small enough (less than 512)
 * is regarded as the number of cached PAGES instead.
 *
 * Thus specifying a 'cache size' of 16 implies you wish hamsterdb to use a 16 PAGES cache, while a 'cache size' of
 * 65536 implies you wish hamsterdb to use 64KiB of memory space for the cache; the number of pages this would be
 * depends on the configured page size (@ref HAM_PARAM_PAGESIZE)
 *
 * @if ham_internals
 * @sa CACHE_AS_PAGES_MAX
 * @endif
 * @sa HAM_PARAM_PAGESIZE
 */
#define HAM_PARAM_CACHESIZE          0x00000100u

/** Parameter name for @ref ham_open_ex, @ref ham_create_ex

Sets/retrieves the 'cooked' pagesize, i.e. the pagesize which is used to store content.

This parameter may differ from the physical ('raw') pagesize when using an environment which contains
device chains which add headers and/or footers to the processed page content.
Integrity-checking and Authenticating cryptographical device layers come to mind as an example of such.  Page-level
compression device layers are another example of such.

When no database or environment is specified with the request, the 'cooked' pagesize will equal the
'raw' pagesize as hamsterdb will not know about any content-altering device layers (yet).
 */
#define HAM_PARAM_PAGESIZE           0x00000101u

/** Parameter name for @ref ham_create_ex; sets the key size */
#define HAM_PARAM_KEYSIZE            0x00000102u

/** Parameter name for @ref ham_env_create_ex; sets the number of maximum
 * Databases */
#define HAM_PARAM_MAX_ENV_DATABASES  0x00000103u

/** Parameter name for @ref ham_env_create_ex and @ref ham_env_open_ex; set the
 * expected access mode.
 */
#define HAM_PARAM_DATA_ACCESS_MODE   0x00000104u

/** Parameter name for @ref ham_env_create_ex and @ref ham_create; set the
 * initial database size (expressed in pages).
 *
 * @remark This is an advisory option and its use depends on the device
 * driver(s) which are used to store the database.
 */
#define HAM_PARAM_INITIAL_DB_SIZE    0x00000105u

/**
 * The parameter value provides a reference (pointer) to a custom device
 * instantiator method.
 *
 * @sa ham_parameter_function_t
 */
#define HAM_PARAM_CUSTOM_DEVICE      0x00000106u


/**
 * @defgroup ham_database_info_parameters hamsterdb Database Information Request Parameters
 * @{
 *
 * Parameter names to query information from the Database through calls
 * to @ref ham_env_get_parameters and  @ref ham_get_parameters.
 *
 * @sa ham_parameter_t
 */

/**
 * Retrieve the Database/Environment flags as were specified at the time of
 * @ref ham_create/@ref ham_env_create/@ref ham_open/@ref ham_env_open
 * invocation.
 */
#define HAM_PARAM_GET_FLAGS                0x00000200u

/**
 * Retrieve the filesystem file access mode as was specified at the time
 * of @ref ham_create/@ref ham_env_create/@ref ham_open/@ref ham_env_open
 * invocation.
 */
#define HAM_PARAM_GET_FILEMODE            0x00000201u

/**
 * Return a <code>const char *</code> pointer to the current
 * Environment/Database file name in the @ref ham_offset_t value
 * member, when the Database is actually stored on disc.
 *
 * In-memory Databases will return a @c NULL (0) pointer instead.
 */
#define HAM_PARAM_GET_FILENAME            0x00000202u

/**
 * Retrieve the Database 'name' number of this @ref ham_db_t Database within
 * the current @ref ham_env_t Environment.
 *
 * When the Database is not related to an Environment, the reserved 'name'
 * @ref HAM_FIRST_DATABASE_NAME is used for this Database.
*/
#define HAM_PARAM_GET_DATABASE_NAME       0x00000203u
#define HAM_PARAM_DBNAME                  HAM_PARAM_GET_DATABASE_NAME

/**
 * Retrieve the maximum number of keys per page; this number depends on the
 * currently active page and key sizes.
 *
 * When no Database or Environment is specified with the request, the default
 * settings for all of these will be assumed in order to produce a viable
 * ball park value for this one.
 */
#define HAM_PARAM_GET_KEYS_PER_PAGE        0x00000204u

/**
 * Retrieve the Data Access mode for the Database.

 @sa HAM_PARAM_DATA_ACCESS_MODE
 */
#define HAM_PARAM_GET_DATA_ACCESS_MODE     0x00000205u
#define HAM_PARAM_GET_DAM                  HAM_PARAM_GET_DATA_ACCESS_MODE

/**
 * Retrieve a @ref ham_statistics_t structure with the current statistics.
 *
 * @warning
 * Please, heed the warnings and notes listed in the @ref ham_statistics_t
 * documentation section and follow the advice given there to the letter.
 * Not adhering to these admonishions introduces the risk of hamsterdb
 * becoming unstable and exhibiting unreliable and downright faulty
 * behaviour over time. This includes, but is not limited to, core dumps or
 * comparable system crashes.
 *
 * @warning If you do not feel entirely qualified to use this particular section of
 * the hamsterdb API, refrain from using @ref HAM_PARAM_GET_STATISTICS and
 * @ref ham_statistics_t and consult a professional for assistance if your
 * application needs access to this kind of 'core data'.
 *
 * hamsterdb is, to the best of my knowledge, the @e only Database engine
 * which makes this degree of power available to the user. With using that
 * power comes a responsibility. Cave canem.
 *
 * @sa ham_statistics_t
 * @sa ham_get_parameters
 * @sa ham_env_get_parameters
 */
#define HAM_PARAM_GET_STATISTICS        0x00000206u

/**
 * Return the hamsterdb version in numeric format.
 *
 * The version Major.Minor.Revision
 * is encoded as follows:
 *
 * The major version number is stored in bits 24..31, minor in bits 16..23 and
 * the revision is stored in bits 0..15
 */
#define HAM_PARAM_GET_VERSION           0x00000207u

/**
 * Return the hamsterdb version as a string. This info is returned by modifying the
 * parameter value to point at the embedded <code>const char *</code> string.
 */
#define HAM_PARAM_GET_VERSION_STRING    0x00000208u

/**
 * Return a <code>const char *</code> pointer to the embedded hamsterdb licensee info.
 *
 * When no licensee has been encoded, the returned string will be empty.
 */
#define HAM_PARAM_GET_LICENSEE          0x00000209u

/**
 * Return the embedded hamsterdb license serial number as an integer value.
 *
 * When no serial number has been encoded, the returned value is zero (0).
 */
#define HAM_PARAM_GET_LICENSE_SERIALNO  0x00000210u


/**
 * @}
 */

/**
 * @}
 */

/**
 * Retrieve the current value for a given Database setting
 *
 * Only those values requested by the parameter array will be stored.
 *
 * The following parameters are supported:
 *      <ul>
 *        <li>HAM_PARAM_CACHESIZE  returns the cache size
 *        <li>HAM_PARAM_PAGESIZE  returns the page size
 *        <li>HAM_PARAM_KEYSIZE  returns the key size
 *        <li>HAM_PARAM_MAX_ENV_DATABASES  returns the max. number of
 *              Databases of this Database's Environment
 *        <li>HAM_PARAM_GET_FLAGS  returns the flags which were used to
 *              open or create this Database
 *        <li>HAM_PARAM_GET_FILEMODE  returns the @a mode parameter which
 *              was specified when creating this Database
 *        <li>HAM_PARAM_GET_FILENAME  returns the filename (the @a value
 *              of this parameter is a const char * pointer casted to a
 *              ham_u64_t variable)
 *        <li>HAM_PARAM_GET_DATABASE_NAME  returns the Database name
 *        <li>HAM_PARAM_GET_KEYS_PER_PAGE  returns the maximum number
 *              of keys per page
 *        <li>HAM_PARAM_GET_DATA_ACCESS_MODE  returns the Data Access Mode
 *      </ul>
 *
 * @param db A valid Database handle
 * @param param An array of ham_parameter_t structures
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a db pointer is NULL or
 *              @a param is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_get_parameters(ham_db_t *db, ham_parameter_t *param);


/**
 * Retrieve the flags which were specified when the Database was created
 * or opened
 *
 * @param db A valid Database handle
 *
 * @return The Database flags
 *
 * @deprecated This function was replaced by @ref ham_get_parameters
 * and @ref ham_env_get_parameters
 */
HAM_EXPORT ham_u32_t HAM_CALLCONV
ham_get_flags(ham_db_t *db);

/**
 * Retrieve the Environment handle of a Database
 *
 * Every Database belongs to an Environment, even if it was created with
 * ham_create[_ex] or ham_open[_ex].
 *
 * Therefore this function always returns a valid handle, if the Database
 * handle was also valid and initialized (otherwise it returns NULL).
 *
 * @param db A valid Database handle
 *
 * @return The Environment handle
 */
HAM_EXPORT ham_env_t *HAM_CALLCONV
ham_get_env(ham_db_t *db);

/**
 * Returns the kind of key match which produced this key as it was
 * returned by one of the @ref ham_find(), @ref ham_cursor_find() or
 * @ref ham_cursor_find_ex() functions
 *
 * This routine assumes the key was passed back by one of the @ref ham_find,
 * @ref ham_cursor_find or @ref ham_cursor_find_ex functions and not used
 * by any other hamsterdb functions after that.
 *
 * As such, this function produces an answer akin to the 'sign' of the
 * specified key as it was returned by the find operation.
 *
 * @param key A valid key
 *
 * @return 1 (greater than) or -1 (less than) when the given key is an
 *      approximate result / zero (0) otherwise. Specifically:
 *      <ul>
 *        <li>+1 when the key is greater than the item searched for (key
 *              was a GT match)
 *        <li>-1 when the key is less than the item searched for (key was
 *              a LT match)
 *        <li>zero (0) otherwise (key was an EQ (EXACT) match)
 *      </ul>
 */
HAM_EXPORT int HAM_CALLCONV
ham_key_get_approximate_match_type(ham_key_t *key);


/**
 * Returns the last error code
 *
 * @param db A valid Database handle
 *
 * @return The last error code which was returned by one of the
 *         hamsterdb API functions. Use @ref ham_strerror to translate
 *         this code to a descriptive string
 *
 * @deprecated This function has been removed from the API as it is not
 *             thread-safe by design. You should check and, where needed,
 *             store the error codes returned by the hamsterdb API
 *             functions in your application code.
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_get_error(ham_db_t *db);

/**
 * Typedef for a prefix comparison function
 *
 * This function compares two (possibly partial) index keys.
 * It returns -1 if @a lhs
 * ("left-hand side", the parameter on the left side) is smaller than
 * @a rhs ("right-hand side"), 0 if both keys are equal, and 1 if @a lhs
 * is larger than @a rhs.
 *
 * @remark If one of the keys is only partially loaded, but the comparison
 * function needs the full key, the return value should be
 * HAM_PREFIX_REQUEST_FULLKEY. The entire key will be loaded and the
 * comparison will be performed through the registered (or default)
 * @ref ham_compare_func_t function.
 *
 * @remark As hamsterdb allows zero-length keys, it may happen that
 * either @a lhs_length or @a rhs_length, or both are zero. When
 * zero-length keys are passed to this comparison function, their
 * related data pointers (@a rhs, @a lhs) MAY be NULL.
 *
 * @sa ham_set_prefix_compare_func
 * @sa ham_compare_func_t
 * @sa ham_set_compare_func
 */
typedef int HAM_CALLCONV (*ham_prefix_compare_func_t)
                                 (ham_db_t *db,
                                  const ham_u8_t *lhs, ham_size_t lhs_length,
                                  ham_size_t lhs_real_length,
                                  const ham_u8_t *rhs, ham_size_t rhs_length,
                                  ham_size_t rhs_real_length);

/**
 * Sets the prefix comparison function
 *
 * The prefix comparison function is called when an index uses
 * keys with variable length and at least one of the two keys is loaded
 * only partially.
 *
 * If @a foo is NULL, hamsterdb will not use any prefix comparison.
 *
 * @param db A valid Database handle
 * @param foo A pointer to the prefix compare function
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if the @a db parameter is NULL
 *
 * @sa ham_prefix_compare_func_t
 * @sa ham_compare_func_t
 * @sa ham_set_compare_func
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_set_prefix_compare_func(ham_db_t *db, ham_prefix_compare_func_t foo);

/**
 * Typedef for a key comparison function
 *
 * This function compares two index keys. It returns -1, if @a lhs
 * ("left-hand side", the parameter on the left side) is smaller than
 * @a rhs ("right-hand side"), 0 if both keys are equal, and 1 if @a lhs
 * is larger than @a rhs.
 *
 * @remark As hamsterdb allows zero-length keys, it may happen that
 * either @a lhs_length or @a rhs_length, or both are zero. When
 * zero-length keys are passed to this comparison function, their
 * related data pointers (@a rhs, @a lhs) MAY be NULL.
 *
 * @sa ham_set_compare_func
 * @sa ham_prefix_compare_func_t
 * @sa ham_set_prefix_compare_func
 */
typedef int HAM_CALLCONV (*ham_compare_func_t)(ham_db_t *db,
                                  const ham_u8_t *lhs, ham_size_t lhs_length,
                                  const ham_u8_t *rhs, ham_size_t rhs_length);

/**
 * Sets the comparison function
 *
 * The comparison function compares two index keys. It returns -1 if the
 * first key is smaller, +1 if the second key is smaller or 0 if both
 * keys are equal.
 *
 * If @a foo is NULL, hamsterdb will use the default compare
 * function (which is based on memcmp(3)).
 *
 * Note that if you use a custom comparison routine in combination with
 * extended keys, it might be useful to disable the prefix comparison, which
 * is based on memcmp(3). See @ref ham_set_prefix_compare_func for details.
 *
 * @param db A valid Database handle
 * @param foo A pointer to the compare function
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if one of the parameters is NULL
 *
 * @sa ham_set_prefix_compare_func
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_set_compare_func(ham_db_t *db, ham_compare_func_t foo);


/**
 * Typedef for a record comparison function, which is invoked when duplicate
 * key elements are inserted in the database: these are stored in record order.
 *
 * This function compares two records. It returns -1, if @a lhs
 * ("left-hand side", the parameter on the left side) is smaller than
 * @a rhs ("right-hand side"), 0 if both keys are equal, and 1 if @a lhs
 * is larger than @a rhs.
 *
 * @remark As hamsterdb allows zero-length records, it may happen that
 * either @a lhs_length or @a rhs_length, or both are zero. In this case
 * the related data pointers (@a rhs, @a lhs) <b>may</b> be NULL.
 *
 * <b>Warning</b> If duplicate sorting is enabled, and records are retrieved
 * with @ref HAM_DIRECT_ACCESS, the records must not be modified or the sort
 * order might get lost.
 *
 * @sa ham_set_compare_func
 * @sa ham_prefix_compare_func_t
 * @sa ham_set_prefix_compare_func
 */
typedef int HAM_CALLCONV (*ham_duplicate_compare_func_t)(ham_db_t *db,
                                  const ham_u8_t *lhs, ham_size_t lhs_length,
                                  const ham_u8_t *rhs, ham_size_t rhs_length);

/**
 * Sets the duplicate comparison function
 *
 * The comparison function compares two records which share the same key.
 * It returns -1 if the first record is smaller, +1 if the second record is
 * smaller or 0 if both records are equal.
 *
 * If @a foo is NULL, hamsterdb will use the default compare
 * function (which is based on memcmp(3)).
 *
 * To enable this function, the flag @ref HAM_SORT_DUPLICATES has to be
 * specified when creating or opening a Database.
 *
 * Sorting duplicate keys comes with a small performance penalty compared
 * to unsorted duplicates, since the records of other duplicates have to be
 * fetched for the comparison.
 *
 * <b>Warning</b> If duplicate sorting is enabled, and records are retrieved
 * with @ref HAM_DIRECT_ACCESS, the records must not be modified or the sort
 * order might get lost.
 *
 * @param db A valid Database handle
 * @param foo A pointer to the compare function
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if one of the parameters is NULL
 *
 * @sa HAM_ENABLE_DUPLICATES
 * @sa HAM_SORT_DUPLICATES
 * @sa ham_cursor_insert
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_set_duplicate_compare_func(ham_db_t *db, ham_duplicate_compare_func_t foo);


struct ham_lexorder_assist_t;
typedef struct ham_lexorder_assist_t ham_lexorder_assist_t;

struct ham_lexorder_assist_t
{
    void (*norm_uint16)(ham_u16_t *dst, ham_u16_t *src);
    void (*norm_uint32)(ham_u32_t *dst, ham_u32_t *src);
    void (*norm_uint64)(ham_u64_t *dst, ham_u64_t *src);
    void (*norm_flt)(float *dst, float *src);
    void (*norm_dbl)(double *dst, double *src);

    void (*denorm_uint16)(ham_u16_t *dst, ham_u16_t *src);
    void (*denorm_uint32)(ham_u32_t *dst, ham_u32_t *src);
    void (*denorm_uint64)(ham_u64_t *dst, ham_u64_t *src);
    void (*denorm_flt)(float *dst, float *src);
    void (*denorm_dbl)(double *dst, double *src);
};


/**
 * Typedef for a key normalizer function
 *
 * This function normalizes / denormalizes an index key. It returns an error
 * when the transformation failed.
 *
 * 'Normalizing' the key data means the data is transformed to a format which
 * enables a direct lexicographic ordering. For example, integers may be transformed
 * to their Big Endian representation, which has a bit/byte order which exhibits a
 * lexicographic order: the first bits are the most significant bits then and thus
 * would produce the same comparison result as a 'string compare' of the value
 * (lead-padded with zeroes to make all integers fixed-width strings).<br>
 * Floating point values may be stored in their Big Endian IEEE representation, where
 * the power preceedes the mantissa when the floating point number is read as a byte sequence.
 * For these basic type conversions, hamsterdb provides a series of conversion functions in the
 * @a assist 'class' (a struct with a set of function callbacks).
 *
 * @note hamsterdb allows zero-length keys, so this function must cope well with those.
 *
 * @sa ham_set_compare_func
 * @sa ham_prefix_compare_func_t
 * @sa ham_set_prefix_compare_func
 */
typedef ham_status_t HAM_CALLCONV (*ham_normalize_func_t)(ham_db_t *db,
                                  const ham_lexorder_assist_t *assist,
                                  const ham_u8_t *src, ham_size_t src_length,
                                  ham_u8_t *dst, ham_size_t dst_max_length,
                                  ham_bool_t normalize);


/**
 * Enables zlib compression for all inserted records
 *
 * This function enables zlib compression for all inserted Database records.
 *
 * The compression will be active till @ref ham_close is called. If the Database
 * handle is reused after calling @ref ham_close, the compression is no longer
 * active. @ref ham_enable_compression should be called immediately after
 * @ref ham_create[_ex] or @ref ham_open[_ex]. When opening the Database,
 * the compression has to be enabled again.
 *
 * Note that zlib usually has an overhead and often is not effective if the
 * records are small (i.e. < 128 bytes), but this highly depends
 * on the data that is inserted.
 *
 * The zlib compression filter does not allow queries (i.e. with @ref ham_find)
 * with user-allocated records and the flag @ref HAM_RECORD_USER_ALLOC. In this
 * case, the query-function will return @ref HAM_INV_PARAMETER.
 *
 * @param db A valid Database handle
 * @param level The compression level. 0 for the zlib default, 1 for
 *      best speed and 9 for minimum size
 * @param flags Optional flags for the compression; unused, set to 0
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db is NULL or @a level is not between
 *      0 and 9
 * @return @ref HAM_NOT_IMPLEMENTED if hamsterdb was compiled without support
 *      for compression
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_enable_compression(ham_db_t *db, ham_u32_t level, ham_u32_t flags);

/**
 * Searches an item in the Database
 *
 * This function searches the Database for @a key. If the key
 * is found, @a record will receive the record of this item and
 * @ref HAM_SUCCESS is returned. If the key is not found, the function
 * returns @ref HAM_KEY_NOT_FOUND.
 *
 * A ham_record_t structure should be initialized with
 * zeroes before it is being used. This can be done with the C library
 * routines memset(3) or bzero(2).
 *
 * If the function completes successfully, the @a record pointer is
 * initialized with the size of the record (in @a record.size) and the
 * actual record data (in @a record.data). If the record is empty,
 * @a size is 0 and @a data points to NULL.
 *
 * The @a data pointer is a temporary pointer and will be overwritten
 * by subsequent hamsterdb API calls. You can alter this behaviour by
 * allocating the @a data pointer in the application and setting
 * @a record.flags to @ref HAM_RECORD_USER_ALLOC. Make sure that the allocated
 * buffer is large enough.
 *
 * When specifying @ref HAM_DIRECT_ACCESS, the @a data pointer will point
 * directly to the record that is stored in hamsterdb; the data can be modified,
 * but the pointer must not be reallocated or freed. The flag
 * @ref HAM_DIRECT_ACCESS is only allowed in In-Memory Databases.
 *
 * @ref ham_find can not search for duplicate keys. If @a key has
 * multiple duplicates, only the first duplicate is returned.
 *
 * You can read only portions of the record by specifying the flag
 * @ref HAM_PARTIAL. In this case, hamsterdb will read
 * <b>record->partial_size</b> bytes of the record data at offset
 * <b>record->partial_offset</b>. If necessary, the record data will
 * be limited to the original record size. The number of bytes actually read
 * is returned in <b>record->size</b>.
 *
 * @param db A valid Database handle
 * @param txn A Transaction handle, or NULL
 * @param key The key of the item
 * @param record The record of the item
 * @param flags Optional flags for searching, which can be combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *        <li>@ref HAM_FIND_EXACT_MATCH (default). If the @a key exists,
 *              the cursor is adjusted to reference the record. Otherwise, an
 *              error is returned. Note that for backwards compatibility
 *              the value zero (0) can specified as an alternative when this
 *              option is not mixed with any of the others in this list.
 *        <li>@ref HAM_FIND_LT_MATCH Cursor 'find' flag 'Less Than': the
 *              cursor is moved to point at the last record which' key
 *              is less than the specified key. When such a record cannot
 *              be located, an error is returned.
 *        <li>@ref HAM_FIND_GT_MATCH Cursor 'find' flag 'Greater Than':
 *              the cursor is moved to point at the first record which' key is
 *              larger than the specified key. When such a record cannot be
 *              located, an error is returned.
 *        <li>@ref HAM_FIND_LEQ_MATCH Cursor 'find' flag 'Less or EQual':
 *              the cursor is moved to point at the record which' key matches
 *              the specified key and when such a record is not available
 *              the cursor is moved to point at the last record which' key
 *              is less than the specified key. When such a record cannot be
 *              located, an error is returned.
 *        <li>@ref HAM_FIND_GEQ_MATCH Cursor 'find' flag 'Greater or
 *              Equal': the cursor is moved to point at the record which' key
 *              matches the specified key and when such a record
 *              is not available the cursor is moved to point at the first
 *              record which' key is larger than the specified key.
 *              When such a record cannot be located, an error is returned.
 *        <li>@ref HAM_FIND_NEAR_MATCH Cursor 'find' flag 'Any Near Or
 *              Equal': the cursor is moved to point at the record which'
 *              key matches the specified key and when such a record is
 *              not available the cursor is moved to point at either the
 *              last record which' key is less than the specified key or
 *              the first record which' key is larger than the specified
 *              key, whichever of these records is located first.
 *              When such records cannot be located, an error is returned.
 *        <li>@ref HAM_DIRECT_ACCESS Only for In-Memory Databases!
 *              Returns a direct pointer to the data blob stored by the
 *              hamsterdb engine. This pointer must not be resized or freed,
 *              but the data in this memory can be modified.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db, @a key or @a record is NULL
 * @return @ref HAM_INV_PARAMETER if @a HAM_DIRECT_ACCESS is specified,
 *          but the Database is not an In-Memory Database.
 * @return @ref HAM_KEY_NOT_FOUND if the @a key does not exist
 * @return @ref HAM_KEYSIZE_TOO_SMALL if the user-specified key size is not
 *        large enough to store the entire key. Note also that
 *        @ref HAM_RECORD_NUMBER Databases require a fixed key size of 8.
 * @return @ref HAM_RECORDSIZE_TOO_SMALL if the user-specified record size
 *        is not large enough to store the entire record.
 *
 * @remark When either or both @ref HAM_FIND_LT_MATCH and/or @ref
 *        HAM_FIND_GT_MATCH have been specified as flags, the @a key structure
 *        will be overwritten when an approximate match was found: the
 *        @a key and @a record structures will then point at the located
 *        @a key and @a record. In this case the caller should ensure @a key
 *        points at a structure which must adhere to the same restrictions
 *        and conditions as specified for @ref ham_cursor_move(...,
 *        @ref HAM_CURSOR_NEXT).
 *
 * @sa HAM_RECORD_USER_ALLOC
 * @sa HAM_KEY_USER_ALLOC
 * @sa HAM_PARTIAL
 * @sa ham_record_t
 * @sa ham_key_t
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_find(ham_db_t *db, ham_txn_t *txn, ham_key_t *key,
        ham_record_t *record, ham_u32_t flags);

/**
 * Inserts a Database item
 *
 * This function inserts a key/record pair as a new Database item.
 *
 * If the key already exists in the Database, error @ref HAM_DUPLICATE_KEY
 * is returned.
 *
 * If you wish to overwrite an existing entry specify the
 * flag @ref HAM_OVERWRITE.
 *
 * You can write only portions of the record by specifying the flag
 * @ref HAM_PARTIAL. In this case, hamsterdb will write <b>partial_size</b>
 * bytes of the record data at offset <b>partial_offset</b>. The full record
 * size <em>must</em> always be given in <b>record->size</b>! If
 * partial_size+partial_offset exceed record->size then partial_size should have been
 * been limited to the record->size upper bound by the user. To shrink or grow the record, adjust record->size.
 * @ref HAM_PARTIAL automatically overwrites existing records.
 * Gaps will be filled with NUL-bytes if the record did not yet exist or when it is expanded.
 *
 * Using @ref HAM_PARTIAL is not allowed in combination with sorted
 * duplicates (@ref HAM_SORT_DUPLICATES).
 *
 * If you wish to insert a duplicate key specify the flag @ref HAM_DUPLICATE.
 * (Note that the Database has to be created with @ref HAM_ENABLE_DUPLICATES
 * in order to use duplicate keys.)
 * If no duplicate sorting is enabled (see @ref HAM_SORT_DUPLICATES), the
 * duplicate key is inserted after all other duplicate keys (see
 * @ref HAM_DUPLICATE_INSERT_LAST). Otherwise it is inserted in sorted order.
 *
 * Record Number Databases (created with @ref HAM_RECORD_NUMBER) expect
 * either an empty @a key (with a size of 0 and data pointing to NULL),
 * or a user-supplied key (with key.flag @ref HAM_KEY_USER_ALLOC, a size
 * of 8 and a valid data pointer).
 * If key.size is 0 and key.data is NULL, hamsterdb will temporarily
 * allocate memory for key->data, which will then point to an 8-byte
 * unsigned integer in host-endian.
 *
 * @param db A valid Database handle
 * @param txn A Transaction handle, or NULL
 * @param key The key of the new item
 * @param record The record of the new item
 * @param flags Optional flags for inserting. Possible flags are:
 *      <ul>
 *        <li>@ref HAM_OVERWRITE. If the @a key already exists, the record is
 *              overwritten. Otherwise, the key is inserted. Flag is not
 *              allowed in combination with @ref HAM_DUPLICATE.
 *        <li>@ref HAM_DUPLICATE. If the @a key already exists, a duplicate
 *              key is inserted. The key is inserted before the already
 *              existing key, or according to the sort order. Flag is not
 *              allowed in combination with @ref HAM_OVERWRITE.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db, @a key or @a record is NULL
 * @return @ref HAM_INV_PARAMETER if the Database is a Record Number Database
 *              and the key is invalid (see above)
 * @if vanilla_hamster
 * @return @ref HAM_INV_PARAMETER if @ref HAM_PARTIAL was specified <b>AND</b>
 *              duplicate sorting is enabled (@ref HAM_SORT_DUPLICATES)
 * @endif
 * @return @ref HAM_INV_PARAMETER if the flags @ref HAM_OVERWRITE <b>and</b>
 *              @ref HAM_DUPLICATE were specified, or if @ref HAM_DUPLICATE
 *              was specified, but the Database was not created with
 *              flag @ref HAM_ENABLE_DUPLICATES.
 * @return @ref HAM_INV_PARAMETER if @ref HAM_PARTIAL is specified and
 *              record->partial_offset+record->partial_size exceeds the
 *              record->size
 * @return @ref HAM_DB_READ_ONLY if you tried to insert a key in a read-only
 *              Database
 * @return @ref HAM_INV_KEYSIZE if the key size is larger than the @a keysize
 *              parameter specified for @ref ham_create_ex and variable
 *              key sizes are disabled (see @ref HAM_DISABLE_VAR_KEYLEN)
 *              OR if the @a keysize parameter specified for @ref ham_create_ex
 *              is smaller than 8.
 *
 * @sa HAM_DISABLE_VAR_KEYLEN
 * @sa HAM_PARTIAL
 * @sa ham_record_t
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_insert(ham_db_t *db, ham_txn_t *txn, ham_key_t *key,
        ham_record_t *record, ham_u32_t flags);

/**
 * @defgroup ham_insert_flags Flags for ham_insert and ham_cursor_insert
 * @{
 */

/**
 * Flag for @ref ham_insert and @ref ham_cursor_insert.
 *
 * When specified with @ref ham_insert and in case a key
 * is specified which stores duplicates in the Database, the first
 * duplicate record will be overwritten.
 *
 * When used with @ref ham_cursor_insert and assuming the same
 * conditions, the duplicate currently referenced by the Cursor
 * will be overwritten.

Mutually exclusive with any of the @ref HAM_DUPLICATE,
@ref HAM_DUPLICATE_INSERT_BEFORE, @ref HAM_DUPLICATE_INSERT_AFTER,
@ref HAM_DUPLICATE_INSERT_FIRST and @ref HAM_DUPLICATE_INSERT_LAST flags.

@note
This flag can still be used when ordered duplicate storage
(@ref HAM_SORT_DUPLICATES) has been configured for this
index/database; in that case the (key,record) tuple currently
addressed by the cursor will not only be replaced (overwritten)
but also re-inserted to ensure the duplicates remain stored
in strict order as determined through the callback (or default)
registered through the @ref ham_set_duplicate_compare_func API.

The <em>side effect</em> of this is that the cursor is moved
to the new (key,record) position, which means that calling
@ref ham_cursor_insert with the @a HAM_OVERWRITE flag set will
move the cursor with high probability. Hence coding sequences
like these

@code
@ref ham_cursor_find(...)
// edit two duplicates...
@ref ham_cursor_insert(..., flags=@a HAM_OVERWRITE)
@ref ham_cursor_move(..., flags=@ref HAM_CURSOR_NEXT | @ref HAM_DUPLICATES_ONLY)
@ref ham_cursor_insert(..., flags=@a HAM_OVERWRITE)
@endcode

will almost certainly @b not deliver the results one might
expect at first glance when these access a database which has
been set up with @ref HAM_SORT_DUPLICATES enabled.

@sa ham_cursor_overwrite
*/
#define HAM_OVERWRITE                   0x0001u

/**
Flag for @ref ham_insert and @ref ham_cursor_insert.
 *
 * When specified with @ref ham_insert and in case a key
 * is specified which already exists in the database, the
 * new (key, record) tuple will be added to the database
 * as a duplicate key entry. By default this duplicate entry
 * will be appended to the already existing list of
 * duplicates for the given key, unless duplicate key
 * ordering has been enabled during database setup
 * (@ref HAM_SORT_DUPLICATES) or one of these flags is
 * specified as well, overriding the default insert
 * position suggestion:
 *
 * <ul>
 * <li>@ref HAM_DUPLICATE_INSERT_BEFORE
 * <li>@ref HAM_DUPLICATE_INSERT_AFTER
 * <li>@ref HAM_DUPLICATE_INSERT_FIRST
 * <li>@ref HAM_DUPLICATE_INSERT_LAST
 * </ul>
 *
 * When used with @ref ham_cursor_insert and assuming the same
 * conditions, the new (key, record) tuple will be stored after
 * the tuple referenced by the Cursor, unless one of the
 * aforementioned flags is specified as well or duplicate key
 * ordering has been enabled (@ref HAM_SORT_DUPLICATES).
 */
#define HAM_DUPLICATE                   0x0002u

/**
Flag for @ref ham_cursor_insert

Suggests to insert the given (key, record) tuple immediately
before the current duplicate entry which already exists in the
database. Mutually exclusive with @ref HAM_OVERWRITE,
@ref HAM_DUPLICATE_INSERT_AFTER, @ref HAM_DUPLICATE_INSERT_FIRST
and @ref HAM_DUPLICATE_INSERT_LAST.

For example, given the instruction sequence

@code
ham_cursor_insert(key=DUP, record=C, flags=HAM_DUPLICATE_INSERT_BEFORE);
ham_cursor_insert(key=DUP, record=D, flags=HAM_DUPLICATE_INSERT_BEFORE);
@endcode

when a (key=DUP, record=A) and a (key=DUP, record=B) tuple
already exist in the database before and the cursor has been
moved so as to point to the @e second duplicate, that is
record=B, then the order in which the records are stored in
the database will be:

@code
A
D
C
B
@endcode

as the cursor will point at the stored duplicate after each
successful ham_cursor_insert operation.

@note
This flag can still be used when ordered duplicate storage
(@ref HAM_SORT_DUPLICATES) has been configured for this
index/database; in that case it merely serves as a @e hint for
the hamsterdb as it is used as a hint to where the insertion
point for the ordered insert would probably be in relation to
the current cursor position: when used properly, this will
(slightly) improve the performance of the database on insert.
*/
#define HAM_DUPLICATE_INSERT_BEFORE     0x0004u

/**
Flag for @ref ham_cursor_insert

Suggests to insert the given (key, record) tuple immediately
after the current duplicate entry which already exists in
the database. Mutually exclusive with @ref HAM_OVERWRITE,
@ref HAM_DUPLICATE_INSERT_BEFORE, @ref HAM_DUPLICATE_INSERT_FIRST
and @ref HAM_DUPLICATE_INSERT_LAST.

For example, given the instruction sequence

@code
ham_cursor_insert(key=DUP, record=C, flags=HAM_DUPLICATE_INSERT_AFTER);
ham_cursor_insert(key=DUP, record=D, flags=HAM_DUPLICATE_INSERT_AFTER);
@endcode

when a (key=DUP, record=A) and a (key=DUP, record=B) tuple
already exist in the database before and the cursor has been
moved so as to point to the @e first duplicate, that is
record=A, then the order in which the records are stored
in the database will be:

@code
A
C
D
B
@endcode

as the cursor will point at the stored duplicate after each
successful ham_cursor_insert operation.

@note
This flag can still be used when ordered duplicate storage
(@ref HAM_SORT_DUPLICATES) has been configured for this
index/database; in that case it merely serves as a @e hint
for the hamsterdb as it is used as a hint to where the
insertion point for the ordered insert would probably be
in relation to the current cursor position: when used
properly, this will (slightly) improve the performance of
the database on insert.
*/
#define HAM_DUPLICATE_INSERT_AFTER      0x0008u

/**
Flag for @ref ham_cursor_insert

Suggests to insert the given (key, record) tuple as the
first duplicate entry; all existing entries are shifted
one aft in the database. Mutually exclusive with
@ref HAM_OVERWRITE, @ref HAM_DUPLICATE_INSERT_AFTER,
@ref HAM_DUPLICATE_INSERT_BEFORE and @ref HAM_DUPLICATE_INSERT_LAST.

For example, given the instruction sequence

@code
ham_cursor_insert(key=DUP, record=C, flags=HAM_DUPLICATE_INSERT_FIRST);
ham_cursor_insert(key=DUP, record=D, flags=HAM_DUPLICATE_INSERT_FIRST);
@endcode

when a (key=DUP, record=A) and a (key=DUP, record=B) tuple
already exist in the database before and the cursor has
been moved so as to point to the @e second duplicate,
that is record=B, then the order in which the records
are stored in the database will be:

@code
D
C
A
B
@endcode

and the cursor will, of course, point at the last stored
duplicate record=D when done.

@note
This flag can still be used when ordered duplicate storage
(@ref HAM_SORT_DUPLICATES) has been configured for this
index/database; in that case it merely serves as a
@e hint for the hamsterdb as it is used as a hint to where
the insertion point for the ordered insert would probably
be in relation to the current cursor position: when used
properly, this will (slightly) improve the performance of
the database on insert.
*/
#define HAM_DUPLICATE_INSERT_FIRST      0x0010u

/**
Flag for @ref ham_cursor_insert

Suggests to insert the given (key, record) tuple as the
last duplicate entry. Mutually exclusive with
@ref HAM_OVERWRITE, @ref HAM_DUPLICATE_INSERT_AFTER,
@ref HAM_DUPLICATE_INSERT_FIRST and @ref HAM_DUPLICATE_INSERT_BEFORE.

For example, given the instruction sequence

@code
ham_cursor_insert(key=DUP, record=C, flags=HAM_DUPLICATE_INSERT_FIRST);
ham_cursor_insert(key=DUP, record=D, flags=HAM_DUPLICATE_INSERT_FIRST);
@endcode

when a (key=DUP, record=A) and a (key=DUP, record=B) tuple
already exist in the database before and the cursor has
been moved so as to point to the @e first duplicate, that
is record=A, then the order in which the records are
stored in the database will be:

@code
A
B
C
D
@endcode

and the cursor will, of course, point at the last stored
duplicate record=D when done.

@note
This flag can still be used when ordered duplicate storage
(@ref HAM_SORT_DUPLICATES) has been configured for this
index/database; in that case it merely serves as a @e hint
for the hamsterdb as it is used as a hint to where the
insertion point for the ordered insert would probably be
in relation to the current cursor position: when used
properly, this will (slightly) improve the performance
of the database on insert.
*/
#define HAM_DUPLICATE_INSERT_LAST       0x0020u

/**
@}
*/

/**
 * Flag for @ref ham_find, @ref ham_cursor_find, @ref ham_cursor_find_ex, @ref ham_cursor_move.
 *
 * Requires hamsterdb APIs to return a direct pointer to the data blob stored by the
 * hamsterdb engine. This pointer must not be resized or freed,
 * but the data in this memory can be modified.

@warning Only applicable for In-Memory Databases!
         (I.e. databases / environments which have been created
		 with the @ref HAM_IN_MEMORY_DB flag set.)
 */
#define HAM_DIRECT_ACCESS               0x0040u

/**
 Flag for @ref ham_insert, @ref ham_cursor_insert, @ref ham_find,
 @ref ham_cursor_find, @ref ham_cursor_find_ex, @ref ham_cursor_move

 Also serves as a flag for @ref ham_record_t.

 Working with @ref HAM_PARTIAL blobs is a bit more complex than one would imagine
 initially.

 Consider for instance these 'record read' scenarios (@ref ham_find,
 @ref ham_cursor_find, @ref ham_cursor_find_ex, @ref ham_cursor_move), in particular
 with the @ref HAM_RECORD_USER_ALLOC flag set:


 <h4>wicked scenario #1:</h4>

 Say the user passed in a record with the 'partial' elements set up,
 only to discover that the record stored in the database is smaller
 than expected so that the partial_offset points past the end?

 Do we consider this an error? No we don't.

 <h5>What does <em>hamsterdb</em> do?</h5>

 As the 'partial' attributes have precedence when activated, we must
 assume that the user wants us to deliver the requested number of
 'partial' bytes in the buffer pointed at by the 'data' attribute.

 Meanwhile, the 'size' attribute is, as always, used to report back the
 <em>actual size of the record</em>, that is, the total size, not just
 the length of the current partial piece. This behaviour is in line
 with our own requirements when receiving a record for storing in the
 database.

 Hence both partial offset and size may be adjusted to the current state
 of affairs following any @ref ham_find, , @ref ham_cursor_find, @ref ham_cursor_find_ex,
 @ref ham_cursor_move or any other hamsterdb interface function which
 operates on and fills records.

 In this scenario, that means that the 'partial_offset' member will be set to
 equal the 'size' record size, while the 'partial_size' will be set to zero(0).

 @warning
 Passing a @ref HAM_RECORD_USER_ALLOC flagged record structure into
 several hamsterdb interface functions in sequence without resetting
 its 'size' nor all of the 'partial' attributes in between each hamsterdb
 function invocation is therefore <em>strongly discouraged</em>.


 <h4>Wicked scenario #2:</h4>

 Say the user passes in a 'partial' record setup such that it receives
 exactly /zero/ bytes of content for the addressed record.<br>
 An error? No.<br>
 Side effects? No. Remember that having record->data and record->size values
 reset to zero are specifically meant to signal an empty record, which is
 a different thing altogether. In this scenario the 'partial_size' will
 be reset to zero(0) instead to indicate an empty <em>partial</em> load
 of the addressed record.

 Otherwise processing is the same as for scenario #1 above.


 <h4>Wicked scenario #3:</h4>

 Suppose the record has fewer bytes on offer for the current part than
 the user requested (@ref ham_record_t::partial_size). Do we adjust the 'partial_size'
 to signal the user that less bytes were written then one might hope for?

 Yes, we do. Because:

 <ol>
 <li> our output thus mimics the record setup we demand ourselves when
      the user is writing partial content to the database.
 <li> the user <em>could</em> decode this info from comparing the
      adjusted record 'size' parameter with the partial attributes, but
      us adjusting the partial attributes as well makes for simpler user
      code, hence less chances to make mistakes.
 </ol>

 <h5>Side Effects? Not Really.</h5>

 With or without 'partial' attributes activated, the user <em>must</em>
 be aware that record attributes are always modified by the hamsterdb
 interface functions, even when such changes are not immediately apparent.
 See 'wicked scenario #1' above for the relevant warning. Cave canem.


 @sa ham_record_t
 @sa HAM_RECORD_USER_ALLOC
 */
#define HAM_PARTIAL                     0x0080u


/**
 * @defgroup ham_find_matchmode_flags Key Match Criteria Flags for ham_find, ham_cursor_find, and ham_cursor_find_ex
 * @{
 */


/**
 * Cursor 'find' flag: return an exact match (default).
 *
 * Note: For backwards compatibility, you can specify zero (0) as an
 * alternative when this flag is used alone.
 *
 * Flag for @ref ham_find and @ref ham_cursor_find_ex
 */
#define HAM_FIND_EXACT_MATCH        0x4000u

/**
 * Cursor 'find' flag 'Less Than': return the nearest match below the
 * given key, whether an exact match exists or not.
 *
 * Flag for @ref ham_find and @ref ham_cursor_find_ex
 */
#define HAM_FIND_LT_MATCH           0x1000u

/**
 * Cursor 'find' flag 'Greater Than': return the nearest match above the
 * given key, whether an exact match exists or not.
 *
 * Flag for @ref ham_find and @ref ham_cursor_find_ex
 */
#define HAM_FIND_GT_MATCH           0x2000u

/**
 * Cursor 'find' flag 'Less or EQual': return the nearest match below the
 * given key, when an exact match does not exist.
 *
 * May be combined with @ref HAM_FIND_GEQ_MATCH to accept any 'near' key, or
 * you can use the @ref HAM_FIND_NEAR_MATCH constant as a shorthand for that.
 *
 * Flag for @ref ham_find and @ref ham_cursor_find_ex
 */
#define HAM_FIND_LEQ_MATCH          (HAM_FIND_LT_MATCH | HAM_FIND_EXACT_MATCH)

/**
 * Cursor 'find' flag 'Greater or Equal': return the nearest match above
 * the given key, when an exact match does not exist.
 *
 * May be combined with @ref HAM_FIND_LEQ_MATCH to accept any 'near' key,
 * or you can use the @ref HAM_FIND_NEAR_MATCH constant as a shorthand for that.
 *
 * Flag for @ref ham_find and @ref ham_cursor_find_ex
 */
#define HAM_FIND_GEQ_MATCH          (HAM_FIND_GT_MATCH | HAM_FIND_EXACT_MATCH)

/**
 * Cursor 'find' flag 'Any Near Or Equal': return a match directly below or
 * above the given key, when an exact match does not exist.
 *
 * Flag for @ref ham_find and @ref ham_cursor_find_ex
 *
 * @remark
 * Be aware that the returned match will either match the key exactly or
 * is either the first key available above or below the given key when an
 * exact match could not be found; 'find' does NOT spend any effort, in the
 * sense of determining which of both is the 'nearest' to the given key,
 * when both a key above and a key below the one given exist; 'find' will
 * simply return the first of both found. As such, this flag is the simplest
 * possible combination of the combined @ref HAM_FIND_LEQ_MATCH and
 * @ref HAM_FIND_GEQ_MATCH flags.
 */
#define HAM_FIND_NEAR_MATCH                                                 \
    (HAM_FIND_LT_MATCH | HAM_FIND_GT_MATCH | HAM_FIND_EXACT_MATCH)

/**
@}
*/




/**
 * @defgroup ham_hinting_flags hamsterdb Hinting Flags for Find, Insert, Move and Erase
 * @{
 *
 * These flags can be bitwise-OR mixed with the flags
 * as used with any of
 * @ref ham_cursor_insert, @ref ham_insert,
 * @ref ham_cursor_erase, @ref ham_erase, @ref ham_find,
 * @ref ham_cursor_find, @ref ham_cursor_find_ex and
 * @ref ham_cursor_move.
 *
 * These flags override the Database/Environment wide DAM (Data Access Mode)
 * preferences as set by @ref ham_env_create or @ref ham_create. When these
 * flags are not specified, performance hinting will be based on those DAM
 * preferences (see @ref ham_data_access_modes).
 *
 * @sa ham_data_access_modes
 */

/**
 * Flag for @ref ham_cursor_insert, @ref ham_insert,
 * @ref ham_cursor_erase, @ref ham_erase, @ref ham_find,
 * @ref ham_cursor_find and @ref ham_cursor_find_ex
 *
 * Mutually exclusive with flag @ref HAM_HINT_RANDOM_ACCESS.
 *
 * Hints the hamsterdb engine that access is in sequential order, or at least
 * that this operation uses a key that is very close to the previously
 * used one for the same operation (find, insert, erase).
 */
#define HAM_HINT_SEQUENTIAL           0x00010000u

/**
 * Flag for @ref ham_cursor_insert, @ref ham_insert,
 * @ref ham_cursor_erase, @ref ham_erase, @ref ham_find,
 * @ref ham_cursor_find and @ref ham_cursor_find_ex
 *
 * Mutually exclusive with flag @ref HAM_HINT_SEQUENTIAL.
 */
#define HAM_HINT_RANDOM_ACCESS        0x00020000u

/**
 * Flag for @ref ham_cursor_insert, @ref ham_insert,
 * @ref ham_cursor_erase, @ref ham_erase, @ref ham_find,
 * @ref ham_cursor_find and @ref ham_cursor_find_ex
 *
 * Can be used in conjunction with either @ref HAM_HINT_SEQUENTIAL
 * or @ref HAM_HINT_RANDOM_ACCESS.
 *
 * Hints the hamsterdb engine that maximum possible
 * execution speed is requested. The engine is granted permission
 * to employ optimizations and shortcuts that may or may not
 * adversely impact subsequent operations which do not have this
 * flag set. A known side effect is that the Database file will
 * grow faster (and shrink less) than when this flag is not used.
 *
 * @remark This flag can also be passed to @ref ham_get_key_count
 * to when all you need is a quick estimate instead of an exact
 * number.
 */
#define HAM_HINT_UBER_FAST_ACCESS     0x00040000u

/**
 * Flag for @ref ham_cursor_insert and @ref ham_insert
 *
 * Mutually exclusive with flag @ref HAM_HINT_PREPEND.
 *
 * Hints the hamsterdb engine that the current key will
 * compare as @e larger than any key already existing in the Database.
 * The hamsterdb engine will verify this postulation and when found not
 * to be true, will revert to a regular insert operation
 * as if this flag was not specified. The incurred cost then is only one
 * additional key comparison.
 */
#define HAM_HINT_APPEND               0x00080000u

/**
 * Flag for @ref ham_cursor_insert and @ref ham_insert
 *
 * Mutually exclusive with flag @ref HAM_HINT_APPEND.
 *
 * Hints the hamsterdb engine that the current key will
 * compare as @e smaller than any key already existing in the Database.
 * The hamsterdb engine will verify this postulation and when found not
 * to be true, will revert to a regular insert operation
 * as if this flag was not specified. The incurred cost then is only one
 * additional key comparison.
 */
#define HAM_HINT_PREPEND              0x00100000u

/**
 * Flag mask to extract the common hint flags from a find/move/insert/erase
 * flag value.
 */
#define HAM_HINTS_MASK                0x00FF0000u

/**
 * @}
 */

/**
 * Erases a Database item
 *
 * This function erases a Database item. If the item @a key
 * does not exist, @ref HAM_KEY_NOT_FOUND is returned.
 *
 * Note that ham_erase can not erase a single duplicate key. If the key
 * has multiple duplicates, all duplicates of this key will be erased. Use
 * @ref ham_cursor_erase to erase a specific duplicate key.
 *
 * @param db A valid Database handle
 * @param txn A Transaction handle, or NULL
 * @param key The key to delete
 * @param flags Optional flags for erasing; unused, set to 0
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a key is NULL
 * @return @ref HAM_DB_READ_ONLY if you tried to erase a key from a read-only
 *              Database
 * @return @ref HAM_KEY_NOT_FOUND if @a key was not found
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_erase(ham_db_t *db, ham_txn_t *txn, ham_key_t *key, ham_u32_t flags);

/**
 * Flushes the Environment
 *
 * This function flushes the Environment caches and writes the whole file
 * to disk. All Databases of this Environment are flushed as well.
 *
 * Since In-Memory Databases do not have a file on disk, the
 * function will have no effect and will return @ref HAM_SUCCESS.
 *
 * @param env A valid Environment handle
 * @param flags Optional flags for flushing; unused, set to 0
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_flush(ham_env_t *env, ham_u32_t flags);


/**
 * Flushes the Database
 *
 * This function flushes the Database cache and writes the whole file
 * to disk. If this Database was opened in an Environment, all other
 * Databases of this Environment are flushed as well.
 *
 * Since In-Memory Databases do not have a file on disk, the
 * function will have no effect and will return @ref HAM_SUCCESS.
 *
 * @param db A valid Database handle
 * @param flags Optional flags for flushing; unused, set to 0
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_flush(ham_db_t *db, ham_u32_t flags);



/**
 * Calculates the number of keys stored in the Database
 *
 * You can specify the @ref HAM_SKIP_DUPLICATES if you do now want
 * to include any duplicates in the count; if all you're after is
 * a quick estimate, you can specify the flag @ref HAM_FAST_ESTIMATE
 * (which implies @ref HAM_SKIP_DUPLICATES), which will improve the
 * execution speed of this operation significantly.
 *
 * @param db A valid Database handle
 * @param txn A Transaction handle, or NULL
 * @param flags Optional flags:
 *       <ul>
 *         <li>@ref HAM_SKIP_DUPLICATES. Excludes any duplicates from
 *             the count
 *         <li>@ref HAM_FAST_ESTIMATE. Excludes any duplicates from
 *             the count
 *       </ul>
 * @param keycount A reference to a variable which will receive
 *                 the calculated key count per page
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a keycount is NULL or when
 *         @a flags contains an invalid flag set
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_get_key_count(ham_db_t *db, ham_txn_t *txn, ham_u32_t flags,
            ham_offset_t *keycount);

/**
 * Flag for @ref ham_get_key_count
 */
#define HAM_FAST_ESTIMATE           0x0001u

/**
 * Closes the Database
 *
 * This function flushes the Database and then closes the file handle.
 * It does not free the memory resources allocated in the @a db handle -
 * use @ref ham_delete to free @a db.
 *
 * If the flag @ref HAM_AUTO_CLEANUP is specified, hamsterdb automatically
 * calls @ref ham_cursor_close on all open Cursors. This invalidates any
 * related ham_cursor_t handles!
 *
 * If the flag is not specified, the application must close all Database
 * Cursors with @ref ham_cursor_close to prevent memory leaks.
 *
 * This function removes all record-level filters installed
 * with @ref ham_add_record_filter (and hence also, implicitly,
 * the filter installed by @ref ham_enable_compression).
 *
 * This function also aborts all Transactions which were not yet committed,
 * and therefore renders all Transaction handles invalid. If the flag
 * @ref HAM_TXN_AUTO_COMMIT is specified, all Transactions will be committed.
 *
 * @param db A valid Database handle
 * @param flags Optional flags for closing the Database. Possible values are:
 *      <ul>
 *       <li>@ref HAM_AUTO_CLEANUP. Automatically closes all open Cursors
 *       <li>@ref HAM_TXN_AUTO_COMMIT. Automatically commit all open
 *          Transactions
 *       <li>@ref HAM_TXN_AUTO_ABORT. Automatically abort all open
 *          Transactions; this is the default behaviour
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db is NULL
 * @return @ref HAM_CURSOR_STILL_OPEN if not all Cursors of this Database
 *      were closed, and @ref HAM_AUTO_CLEANUP was not specified
 *
 * @sa HAM_AUTO_CLEANUP
 * @sa ham_env_close
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_close(ham_db_t *db, ham_u32_t flags);

/**
Flag for @ref ham_close, @ref ham_env_close.

When specified when calling @ref ham_close, the flag
@a HAM_AUTO_CLEANUP will automatically close all cursors
which reference the given database.

When specified when calling @ref ham_env_close, the flag
@a HAM_AUTO_CLEANUP will automatically close all cursors
which reference any database in the environment @e and
close all open databases in the environment as well.

When any cursors are still open by the time you call
@ref ham_close or @ref ham_env_close and you did
@e not specify this flag, that call will fail with
a @ref HAM_CURSOR_STILL_OPEN error return code.
*/
#define HAM_AUTO_CLEANUP            1

/**
@internal (Internal) flag for @ref ham_close, @ref ham_env_close
*/
#define HAM_DONT_CLEAR_LOG          2

/**
Flag for @ref ham_close, @ref ham_env_close.

Automatically abort all open Transactions (the default)
*/
#define HAM_TXN_AUTO_ABORT          4

/**
Flag for @ref ham_close, @ref ham_env_close.

Automatically commit all open Transactions.
*/
#define HAM_TXN_AUTO_COMMIT         8


/**
 * @}
 */










/**
 * @defgroup ham_cursor hamsterdb Cursor Functions
 * @{
 */

/**
 * Creates a Database Cursor
 *
 * Creates a new Database Cursor. Cursors can be used to
 * traverse the Database from start to end or vice versa. Cursors
 * can also be used to insert, delete or search Database items.
 *
 * A newly created Cursor does not point to any item in the Database.
 *
 * The application should close all Database Cursors before closing
 * the Database.
 *
 * @param db A valid Database handle
 * @param txn A Transaction handle, or NULL
 * @param flags Optional flags for creating the Cursor; unused, set to 0
 * @param cursor A pointer to a pointer which is allocated for the
 *          new Cursor handle
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a cursor is NULL
 * @return @ref HAM_OUT_OF_MEMORY if the new structure could not be allocated
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_create(ham_db_t *db, ham_txn_t *txn, ham_u32_t flags,
            ham_cursor_t **cursor);

/**
 * Clones a Database Cursor
 *
 * Clones an existing Cursor. The new Cursor will point to
 * exactly the same item as the old Cursor. If the old Cursor did not point
 * to any item, so will the new Cursor.
 *
 * If the old Cursor is bound to a Transaction, then the new Cursor will
 * also be bound to this Transaction.
 *
 * @param src The existing Cursor
 * @param dest A pointer to a pointer, which is allocated for the
 *          cloned Cursor handle
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a src or @a dest is NULL
 * @return @ref HAM_OUT_OF_MEMORY if the new structure could not be allocated
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_clone(ham_cursor_t *src, ham_cursor_t **dest);

/**
 * Moves the Cursor
 *
 * Moves the Cursor. Use the @a flags to specify the direction.
 * After the move, key and record of the item are returned, if @a key
 * and/or @a record are valid pointers.
 *
 * If the direction is not specified, the Cursor will not move. Do not
 * specify a direction if you want to fetch the key and/or record of
 * the current item.
 *
 * When specifying @ref HAM_DIRECT_ACCESS, the @a data pointer will point
 * directly to the record that is stored in hamsterdb; the data can be modified,
 * but the pointer must not be reallocated or freed. The flag @ref
 * HAM_DIRECT_ACCESS is only allowed in In-Memory Databases.
 *
 * You can write only portions of the record by specifying the flag
 * @ref HAM_PARTIAL. In this case, hamsterdb will write <b>partial_size</b>
 * bytes of the record data at offset <b>partial_offset</b>. The full record
 * size will always be given in <b>record->size</b>! If
 * partial_size+partial_offset exceed record->size then partial_size will
 * be limited. To shrink or grow the record, adjust record->size.
 * @ref HAM_PARTIAL automatically overwrites existing records.
 * Gaps will be filled with null-bytes if the record did not yet exist.
 * Using @ref HAM_PARTIAL is not allowed in combination with sorted
 * duplicates (@ref HAM_SORT_DUPLICATES).
 *
 * @param cursor A valid Cursor handle
 * @param key An optional pointer to a @ref ham_key_t structure. If this
 *      pointer is not NULL, the key of the new item is returned.
 *      Note that key->data will point to temporary data. This pointer
 *      will be invalidated by subsequent hamsterdb API calls. See
 *      @ref HAM_KEY_USER_ALLOC on how to change this behaviour.
 * @param record An optional pointer to a @ref ham_record_t structure. If this
 *      pointer is not NULL, the record of the new item is returned.
 *      Note that record->data will point to temporary data. This pointer
 *      will be invalidated by subsequent hamsterdb API calls. See
 *      @ref HAM_RECORD_USER_ALLOC on how to change this behaviour.
 * @param flags The flags for this operation. They are used to specify
 *      the direction for the "move". If you do not specify a direction,
 *      the Cursor will remain on the current position.
 *      <ul>
 *          <li>@ref HAM_CURSOR_FIRST positions the Cursor on the first
 *              item in the Database
 *          <li>@ref HAM_CURSOR_LAST positions the Cursor on the last
 *              item in the Database
 *          <li>@ref HAM_CURSOR_NEXT positions the Cursor on the next
 *              item in the Database; if the Cursor does not point to any
 *              item, the function behaves as if direction was
 *              @ref HAM_CURSOR_FIRST.
 *          <li>@ref HAM_CURSOR_PREVIOUS positions the Cursor on the
 *              previous item in the Database; if the Cursor does not point to
 *              any item, the function behaves as if direction was
 *              @ref HAM_CURSOR_LAST.
 *          <li>@ref HAM_SKIP_DUPLICATES skips duplicate keys of the
 *              current key. Not allowed in combination with
 *              @ref HAM_ONLY_DUPLICATES.
 *          <li>@ref HAM_ONLY_DUPLICATES only move through duplicate keys
 *              of the current key. Not allowed in combination with
 *              @ref HAM_SKIP_DUPLICATES.
 *        <li>@ref HAM_DIRECT_ACCESS Only for In-Memory Databases!
 *              Returns a direct pointer to the data blob stored by the
 *              hamsterdb engine. This pointer must not be resized or freed,
 *              but the data in this memory can be modified.
 *     </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a cursor is NULL, or if an invalid
 *              combination of flags was specified
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item, but
 *              key and/or record were requested
 * @return @ref HAM_KEY_NOT_FOUND if @a cursor points to the first (or last)
 *              item, and a move to the previous (or next) item was
 *              requested
 * @return @ref HAM_INV_PARAMETER if @a HAM_DIRECT_ACCESS is specified,
 *              but the Database is not an In-Memory Database.
 * @return @ref HAM_INV_PARAMETER if @ref HAM_PARTIAL is specified and
 *              record->partial_offset+record->partial_size exceeds the
 *              record->size
 * @return @ref HAM_KEYSIZE_TOO_SMALL if the user-specified key size is not
 *              large enough to store the entire key. Note also that
 *              @ref HAM_RECORD_NUMBER Databases require a fixed key size of 8.
 * @return @ref HAM_RECORDSIZE_TOO_SMALL if the user-specified record size
 *              is not large enough to store the entire record.
 *
 * @sa HAM_RECORD_USER_ALLOC
 * @sa HAM_KEY_USER_ALLOC
 * @sa HAM_PARTIAL
 * @sa ham_record_t
 * @sa ham_key_t
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_move(ham_cursor_t *cursor, ham_key_t *key,
        ham_record_t *record, ham_u32_t flags);

/**
Flag for @ref ham_cursor_move.

Positions the Cursor on the first item in the Database.

Mutually exclusive with @ref HAM_CURSOR_LAST, @ref HAM_CURSOR_NEXT
and @ref HAM_CURSOR_PREVIOUS.
*/
#define HAM_CURSOR_FIRST            0x0001u

/**
Flag for @ref ham_cursor_move.

Positions the Cursor on the last item in the Database.

Mutually exclusive with @ref HAM_CURSOR_FIRST, @ref HAM_CURSOR_NEXT
and @ref HAM_CURSOR_PREVIOUS.
 */
#define HAM_CURSOR_LAST             0x0002u

/**
Flag for @ref ham_cursor_move.

Positions the Cursor on the next item in the Database; if the Cursor
does not point to any item yet, the function behaves as if
@ref HAM_CURSOR_FIRST has been specified instead.

Mutually exclusive with @ref HAM_CURSOR_FIRST, @ref HAM_CURSOR_LAST
and @ref HAM_CURSOR_PREVIOUS.
*/
#define HAM_CURSOR_NEXT             0x0004u

/**
Flag for @ref ham_cursor_move.

Positions the Cursor on the previous item in the Database; if the
Cursor does not point to any item yet, the function behaves as
if @ref HAM_CURSOR_LAST has been specified instead.

Mutually exclusive with @ref HAM_CURSOR_FIRST, @ref HAM_CURSOR_LAST
and @ref HAM_CURSOR_NEXT.
 */
#define HAM_CURSOR_PREVIOUS         0x0008u

/**
Flag for @ref ham_cursor_move and @ref ham_get_key_count.

Skips duplicate keys for the current key. Mutually exclusive with
@ref HAM_ONLY_DUPLICATES.
 */
#define HAM_SKIP_DUPLICATES         0x0010u

/**
Flag for @ref ham_cursor_move.

Restrict cursor movement to duplicate keys of the current key only.
Mutually exclusive with @ref HAM_SKIP_DUPLICATES.
 */
#define HAM_ONLY_DUPLICATES         0x0020u

/**
 * Overwrites the current record
 *
 * This function overwrites the record of the current item.
 *
 * The use of this function <em>is</em> allowed if the item has duplicate keys
 * and the duplicate sorting is enabled (see @ref HAM_SORT_DUPLICATES).
 *
 * @warning
 * Be aware, however, that the cursor will implicitly 'jump' to a new position
 * within the ordered set of duplicates as the new record data is inserted so
 * as to guarantee a continued sorted order. I.e. when mixing this API with,
 * @ref ham_cursor_move (@ref HAM_CURSOR_NEXT / @ref HAM_CURSOR_PREVIOUS), you
 * <em>very</em> probably won't be visiting all duplicates and you may visit
 * several duplicates all over again when calling
 * @ref ham_cursor_move (@ref HAM_CURSOR_NEXT / @ref HAM_CURSOR_PREVIOUS)
 * after this call.
 *
 * @param cursor A valid Cursor handle
 * @param record A valid record structure
 * @param flags Optional flags for overwriting the item; unused, set to 0
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a cursor or @a record is NULL
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 *
 * @sa HAM_OVERWRITE
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_overwrite(ham_cursor_t *cursor, ham_record_t *record,
            ham_u32_t flags);

/**
 * Searches a key and points the Cursor to this key
 *
 * Searches for an item in the Database and points the
 * Cursor to this item. If the item could not be found, the Cursor is
 * not modified.
 *
 * Note that @ref ham_cursor_find can not search for duplicate keys. If @a key
 * has multiple duplicates, only the first duplicate is returned.
 *
 * When specifying @ref HAM_DIRECT_ACCESS, the @a data pointer will point
 * directly to the record that is stored in hamsterdb; the data can be modified,
 * but the pointer must not be reallocated or freed. The flag @ref
 * HAM_DIRECT_ACCESS is only allowed in In-Memory Databases.
 *
 * When either or both @ref HAM_FIND_LT_MATCH and/or @ref HAM_FIND_GT_MATCH
 * have been specified as flags, the @a key structure will be overwritten
 * when an approximate match was found: the @a key and @a record
 * structures will then point at the located @a key (and @a record).
 * In this case the caller should ensure @a key points at a structure
 * which must adhere to the same restrictions and conditions as specified
 * for @ref ham_cursor_move(...,HAM_CURSOR_*):
 * key->data will point to temporary data upon return. This pointer
 * will be invalidated by subsequent hamsterdb API calls. See
 * @ref HAM_KEY_USER_ALLOC on how to change this behaviour.
 *
 * Further note that the @a key structure must be non-const at all times as its
 * internal flag bits may be written to. This is done for your benefit, as
 * you may pass the returned @a key structure to
 * @ref ham_key_get_approximate_match_type() to retrieve additional info about
 * the precise nature of the returned key: the sign value produced
 * by @ref ham_key_get_approximate_match_type() tells you which kind of match
 * (equal, less than, greater than) occurred. This is very useful to
 * discern between the various possible successful answers produced by the
 * combinations of @ref HAM_FIND_LT_MATCH, @ref HAM_FIND_GT_MATCH and/or
 * @ref HAM_FIND_EXACT_MATCH.
 *
 * @param cursor A valid Cursor handle
 * @param key A pointer to a @a ham_key_t structure. If this
 *      pointer is not NULL, the key of the new item is returned.
 *      Note that key->data will point to temporary data. This pointer
 *      will be invalidated by subsequent hamsterdb API calls. See
 *      @a HAM_KEY_USER_ALLOC on how to change this behaviour.
 * @param flags Optional flags for searching, which can be combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *        <li>@ref HAM_FIND_EXACT_MATCH (default). If the @a key exists,
 *              the cursor is adjusted to reference the record. Otherwise, an
 *              error is returned. Note that for backwards compatibility
 *              the value zero (0) can specified as an alternative when this
 *              option is not mixed with any of the others in this list.
 *        <li>@ref HAM_FIND_LT_MATCH Cursor 'find' flag 'Less Than': the
 *              cursor is moved to point at the last record which' key
 *              is less than the specified key. When such a record cannot
 *              be located, an error is returned.
 *        <li>@ref HAM_FIND_GT_MATCH Cursor 'find' flag 'Greater Than':
 *              the cursor is moved to point at the first record which' key is
 *              larger than the specified key. When such a record cannot be
 *              located, an error is returned.
 *        <li>@ref HAM_FIND_LEQ_MATCH Cursor 'find' flag 'Less or EQual':
 *              the cursor is moved to point at the record which' key matches
 *              the specified key and when such a record is not available
 *              the cursor is moved to point at the last record which' key
 *              is less than the specified key. When such a record cannot be
 *              located, an error is returned.
 *        <li>@ref HAM_FIND_GEQ_MATCH Cursor 'find' flag 'Greater or
 *              Equal': the cursor is moved to point at the record which' key
 *              matches the specified key and when such a record
 *              is not available the cursor is moved to point at the first
 *              record which' key is larger than the specified key.
 *              When such a record cannot be located, an error is returned.
 *        <li>@ref HAM_FIND_NEAR_MATCH Cursor 'find' flag 'Any Near Or
 *              Equal': the cursor is moved to point at the record which'
 *              key matches the specified key and when such a record is
 *              not available the cursor is moved to point at either the
 *              last record which' key is less than the specified key or
 *              the first record which' key is larger than the specified
 *              key, whichever of these records is located first.
 *              When such records cannot be located, an error is returned.
 *        <li>@ref HAM_DIRECT_ACCESS Only for In-Memory Databases!
 *              Returns a direct pointer to the data blob stored by the
 *              hamsterdb engine. This pointer must not be resized or freed,
 *              but the data in this memory can be modified.
 *      </ul>
 *
 * <b>Remark</b>
 * For Approximate Matching the returned match will either match the
 * key exactly or is either the first key available above or below the
 * given key when an exact match could not be found; 'find' does NOT
 * spend any effort, in the sense of determining which of both is the
 * 'nearest' to the given key, when both a key above and a key below the
 * one given exist; 'find' will simply return the first of both found.
 * As such, this flag is the simplest possible combination of the
 * combined @ref HAM_FIND_LEQ_MATCH and @ref HAM_FIND_GEQ_MATCH flags.
 *
 * Note that these flags may be bitwise OR-ed to form functional combinations.
 *
 * @ref HAM_FIND_LEQ_MATCH, @ref HAM_FIND_GEQ_MATCH and
 * @ref HAM_FIND_NEAR_MATCH are themselves shorthands created using
 *      the bitwise OR operation like this:
 *      <ul>
 *          <li>@ref HAM_FIND_LEQ_MATCH == (@ref HAM_FIND_LT_MATCH |
 *                @ref HAM_FIND_EXACT_MATCH)
 *        <li>@ref HAM_FIND_GEQ_MATCH == (@ref HAM_FIND_GT_MATCH |
 *              @ref HAM_FIND_EXACT_MATCH)
 *        <li>@ref HAM_FIND_NEAR_MATCH == (@ref HAM_FIND_LT_MATCH |
 *              @ref HAM_FIND_GT_MATCH | @ref HAM_FIND_EXACT_MATCH)
 *        <li>The remaining bit-combination (@ref HAM_FIND_LT_MATCH |
 *              @ref HAM_FIND_GT_MATCH) has no shorthand, but it will function
 *              as expected nevertheless: finding only 'neighbouring' records
 *              for the given key.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success. Mind the remarks about the
 *         @a key flags being adjusted and the useful invocation of
 *         @ref ham_key_get_approximate_match_type() afterwards.
 * @return @ref HAM_INV_PARAMETER if @a db, @a key or @a record is NULL
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 * @return @ref HAM_KEY_NOT_FOUND if no suitable @a key (record) exists
 * @return @ref HAM_INV_PARAMETER if @a HAM_DIRECT_ACCESS is specified,
 *              but the Database is not an In-Memory Database.
 * @return @ref HAM_KEYSIZE_TOO_SMALL if the user-specified key size is not
 *         large enough to store the entire key. Note also that
 *         @ref HAM_RECORD_NUMBER Databases require a fixed key size of 8.
 *
 * @sa HAM_KEY_USER_ALLOC
 * @sa ham_key_t
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_find(ham_cursor_t *cursor, ham_key_t *key, ham_u32_t flags);


/**
 * Searches with a key and points the Cursor to the key found, retrieves
 * the located record
 *
 * This function is identical to @ref ham_cursor_find, but it immediately
 * retrieves the located record if the lookup operation was successful.
 *
 * Searches for an item in the Database and points the
 * Cursor to this item. If the item could not be found, the Cursor is
 * not modified.
 *
 * Note that @ref ham_cursor_find can not search for duplicate keys. If @a key
 * has multiple duplicates, only the first duplicate is returned.
 *
 * When specifying @ref HAM_DIRECT_ACCESS, the @a data pointer will point
 * directly to the record that is stored in hamsterdb; the data can be modified,
 * but the pointer must not be reallocated or freed. The flag @ref
 * HAM_DIRECT_ACCESS is only allowed in In-Memory Databases.
 *
 * You can read only portions of the record by specifying the flag
 * @ref HAM_PARTIAL. In this case, hamsterdb will read
 * <b>record->partial_size</b> bytes of the record data at offset
 * <b>record->partial_offset</b>. If necessary, the record data will
 * be limited to the original record size. The number of actually read
 * bytes is returned in <b>record->size</b>.
 *
 * When either or both @ref HAM_FIND_LT_MATCH and/or @ref HAM_FIND_GT_MATCH
 * have been specified as flags, the @a key structure will be overwritten
 * when an approximate match was found: the @a key and @a record
 * structures will then point at the located @a key (and @a record).
 * In this case the caller should ensure @a key points at a structure
 * which must adhere to the same restrictions and conditions as specified
 * for @ref ham_cursor_move(...,HAM_CURSOR_*):
 * key->data will point to temporary data upon return. This pointer
 * will be invalidated by subsequent hamsterdb API calls. See
 * @ref HAM_KEY_USER_ALLOC on how to change this behaviour.
 *
 * Further note that the @a key structure must be non-const at all times as its
 * internal flag bits may be written to. This is done for your benefit, as
 * you may pass the returned @a key structure to
 * @ref ham_key_get_approximate_match_type() to retrieve additional info about
 * the precise nature of the returned key: the sign value produced
 * by @ref ham_key_get_approximate_match_type() tells you which kind of match
 * (equal, less than, greater than) occurred. This is very useful to
 * discern between the various possible successful answers produced by the
 * combinations of @ref HAM_FIND_LT_MATCH, @ref HAM_FIND_GT_MATCH and/or
 * @ref HAM_FIND_EXACT_MATCH.
 *
 * @param cursor A valid Cursor handle
 * @param key A pointer to a @ref ham_key_t structure. If this
 *      pointer is not NULL, the key of the new item is returned.
 *      Note that key->data will point to temporary data. This pointer
 *      will be invalidated by subsequent hamsterdb API calls. See
 *      @a HAM_KEY_USER_ALLOC on how to change this behaviour.
 * @param record A pointer to a @ref ham_record_t structure. If this
 *      pointer is not NULL, the record of the new item is returned.
 *      Note that record->data will point to temporary data. This pointer
 *      will be invalidated by subsequent hamsterdb API calls. See
 *      @ref HAM_RECORD_USER_ALLOC on how to change this behaviour.
 * @param flags Optional flags for searching, which can be combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *        <li>@ref HAM_FIND_EXACT_MATCH (default). If the @a key exists,
 *              the cursor is adjusted to reference the record. Otherwise, an
 *              error is returned. Note that for backwards compatibility
 *              the value zero (0) can specified as an alternative when this
 *              option is not mixed with any of the others in this list.
 *        <li>@ref HAM_FIND_LT_MATCH Cursor 'find' flag 'Less Than': the
 *              cursor is moved to point at the last record which' key
 *              is less than the specified key. When such a record cannot
 *              be located, an error is returned.
 *        <li>@ref HAM_FIND_GT_MATCH Cursor 'find' flag 'Greater Than':
 *              the cursor is moved to point at the first record which' key is
 *              larger than the specified key. When such a record cannot be
 *              located, an error is returned.
 *        <li>@ref HAM_FIND_LEQ_MATCH Cursor 'find' flag 'Less or EQual':
 *              the cursor is moved to point at the record which' key matches
 *              the specified key and when such a record is not available
 *              the cursor is moved to point at the last record which' key
 *              is less than the specified key. When such a record cannot be
 *              located, an error is returned.
 *        <li>@ref HAM_FIND_GEQ_MATCH Cursor 'find' flag 'Greater or
 *              Equal': the cursor is moved to point at the record which' key
 *              matches the specified key and when such a record
 *              is not available the cursor is moved to point at the first
 *              record which' key is larger than the specified key.
 *              When such a record cannot be located, an error is returned.
 *        <li>@ref HAM_FIND_NEAR_MATCH Cursor 'find' flag 'Any Near Or
 *              Equal': the cursor is moved to point at the record which'
 *              key matches the specified key and when such a record is
 *              not available the cursor is moved to point at either the
 *              last record which' key is less than the specified key or
 *              the first record which' key is larger than the specified
 *              key, whichever of these records is located first.
 *              When such records cannot be located, an error is returned.
 *        <li>@ref HAM_DIRECT_ACCESS Only for In-Memory Databases!
 *              Returns a direct pointer to the data blob stored by the
 *              hamsterdb engine. This pointer must not be resized or freed,
 *              but the data in this memory can be modified.
 *      </ul>
 *
 * <b>Remark</b>
 * For Approximate Matching the returned match will either match the
 * key exactly or is either the first key available above or below the
 * given key when an exact match could not be found; 'find' does NOT
 * spend any effort, in the sense of determining which of both is the
 * 'nearest' to the given key, when both a key above and a key below the
 * one given exist; 'find' will simply return the first of both found.
 * As such, this flag is the simplest possible combination of the
 * combined @ref HAM_FIND_LEQ_MATCH and @ref HAM_FIND_GEQ_MATCH flags.
 *
 * Note that these flags may be bitwise OR-ed to form functional combinations.
 *
 * @ref HAM_FIND_LEQ_MATCH, @ref HAM_FIND_GEQ_MATCH and
 * @ref HAM_FIND_NEAR_MATCH are themselves shorthands created using
 *      the bitwise OR operation like this:
 *      <ul>
 *          <li>@ref HAM_FIND_LEQ_MATCH == (@ref HAM_FIND_LT_MATCH |
 *                @ref HAM_FIND_EXACT_MATCH)
 *        <li>@ref HAM_FIND_GEQ_MATCH == (@ref HAM_FIND_GT_MATCH |
 *              @ref HAM_FIND_EXACT_MATCH)
 *        <li>@ref HAM_FIND_NEAR_MATCH == (@ref HAM_FIND_LT_MATCH |
 *              @ref HAM_FIND_GT_MATCH | @ref HAM_FIND_EXACT_MATCH)
 *        <li>The remaining bit-combination (@ref HAM_FIND_LT_MATCH |
 *              @ref HAM_FIND_GT_MATCH) has no shorthand, but it will function
 *              as expected nevertheless: finding only 'neighbouring' records
 *              for the given key.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success. Mind the remarks about the
 *         @a key flags being adjusted and the useful invocation of
 *         @ref ham_key_get_approximate_match_type() afterwards.
 * @return @ref HAM_INV_PARAMETER if @a db, @a key or @a record is NULL
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 * @return @ref HAM_KEY_NOT_FOUND if no suitable @a key (record) exists
 * @return @ref HAM_INV_PARAMETER if @a HAM_DIRECT_ACCESS is specified,
 *              but the Database is not an In-Memory Database.
@return @ref HAM_KEYSIZE_TOO_SMALL if the user-specified key size is not large enough to store the
             entire key. Note also that @ref HAM_RECORD_NUMBER databases require a fixed key size of 8.
@return @ref HAM_RECORDSIZE_TOO_SMALL if the user-specified record size is not large enough to store the
             entire record.
 *
 * @sa HAM_KEY_USER_ALLOC
 * @sa ham_key_t
 * @sa HAM_RECORD_USER_ALLOC
 * @sa HAM_PARTIAL
 * @sa ham_record_t
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_find_ex(ham_cursor_t *cursor, ham_key_t *key,
            ham_record_t *record, ham_u32_t flags);

/**
 * Inserts a Database item and points the Cursor to the inserted item
 *
 * This function inserts a key/record pair as a new Database item.
 * If the key already exists in the Database, error @ref HAM_DUPLICATE_KEY
 * is returned.
 *
 * If you wish to overwrite an existing entry specify the
 * flag @ref HAM_OVERWRITE. The use of this flag is not allowed in combination
 * with @ref HAM_DUPLICATE.
 *
 * If you wish to insert a duplicate key specify the flag @ref HAM_DUPLICATE.
 * (Note that the Database has to be created with @ref HAM_ENABLE_DUPLICATES,
 * in order to use duplicate keys.)
 * By default, the duplicate key is inserted after all other duplicate keys
 * (see @ref HAM_DUPLICATE_INSERT_LAST). This behaviour can be overwritten by
 * specifying @ref HAM_DUPLICATE_INSERT_FIRST, @ref HAM_DUPLICATE_INSERT_BEFORE
 * or @ref HAM_DUPLICATE_INSERT_AFTER.
 *
 * You can write only portions of the record by specifying the flag
 * @ref HAM_PARTIAL. In this case, hamsterdb will write <b>partial_size</b>
 * bytes of the record data at offset <b>partial_offset</b>. If necessary, the
 * record data will grow. Gaps will be filled with null-bytes, if the record
 * did not yet exist. Using @ref HAM_PARTIAL is not allowed in combination
 * with sorted duplicates (@ref HAM_SORT_DUPLICATES).
 *
 * If a sort order is specified (see @ref HAM_SORT_DUPLICATES) then
 * the record is inserted in sorted order per key. In this case, the use of @ref
 * HAM_DUPLICATE_INSERT_FIRST, @ref HAM_DUPLICATE_INSERT_LAST, @ref
 * HAM_DUPLICATE_INSERT_BEFORE and @ref HAM_DUPLICATE_INSERT_AFTER <em>is</em>
 * still allowed, but these will merely be considered as <em>hints</em>
 * for positioning the record to be inserted in the sorted order as the order
 * is solely and entirely determined by the callback set up through
 * @ref ham_set_duplicate_compare_func().
 *
 * Specify the flag @ref HAM_HINT_APPEND if you insert sequential data
 * and the current @a key is higher than any other key in this Database.
 * In this case hamsterdb will optimize the insert algorithm. hamsterdb will
 * verify that this key is the highest; if not, it will perform a normal
 * insert. This is the default for Record Number Databases.
 *
 * Specify the flag @ref HAM_HINT_PREPEND if you insert sequential data
 * and the current @a key is lower than any other key in this Database.
 * In this case hamsterdb will optimize the insert algorithm. hamsterdb will
 * verify that this key is the lowest; if not, it will perform a normal
 * insert.
 *
 * After inserting, the Cursor will point to the new item. If inserting
 * the item failed, the Cursor is not modified.
 *
 * Record Number Databases (created with @ref HAM_RECORD_NUMBER) expect
 * either an empty @a key (with a size of 0 and data pointing to NULL),
 * or a user-supplied key (with key.flag @ref HAM_KEY_USER_ALLOC, a size
 * of 8 and a valid data pointer).
 * If key.size is 0 and key.data is NULL, hamsterdb will temporarily
 * allocate memory for key->data, which will then point to an 8-byte
 * unsigned integer in host-endian.
 *
 * @param cursor A valid Cursor handle
 * @param key A valid key structure
 * @param record A valid record structure
 * @param flags Optional flags for inserting the item, combined with
 *        bitwise OR. Possible flags are:
 *      <ul>
 *        <li>@ref HAM_OVERWRITE. If the @a key already exists, the record is
 *              overwritten. Otherwise, the key is inserted. Not allowed in
 *              combination with @ref HAM_DUPLICATE. Allowed if duplicate sorting is enabled.
 *        <li>@ref HAM_DUPLICATE. If the @a key already exists, a duplicate
 *              key is inserted. Same as @ref HAM_DUPLICATE_INSERT_LAST.
 *              Not allowed in combination with @ref HAM_OVERWRITE. Allowed if duplicate sorting is enabled.
 *        <li>@ref HAM_DUPLICATE_INSERT_BEFORE. If the @a key already exists,
 *              a duplicate key is inserted before the duplicate pointed
 *              to by the Cursor. Allowed if duplicate sorting is enabled.
 *        <li>@ref HAM_DUPLICATE_INSERT_AFTER. If the @a key already exists,
 *              a duplicate key is inserted after the duplicate pointed
 *              to by the Cursor. Allowed if duplicate sorting is enabled.
 *        <li>@ref HAM_DUPLICATE_INSERT_FIRST. If the @a key already exists,
 *              a duplicate key is inserted as the first duplicate of
 *              the current key. Allowed if duplicate sorting is enabled.
 *        <li>@ref HAM_DUPLICATE_INSERT_LAST. If the @a key already exists,
 *              a duplicate key is inserted as the last duplicate of
 *              the current key. Allowed if duplicate sorting is enabled.
 *        <li>@ref HAM_HINT_APPEND. Hints the hamsterdb engine that the
 *              current key will compare as @e larger than any key already
 *              existing in the Database. The hamsterdb engine will verify
 *              this postulation and when found not to be true, will revert
 *              to a regular insert operation as if this flag was not
 *              specified. The incurred cost then is only one additional key
 *              comparison. Mutually exclusive with flag @ref HAM_HINT_PREPEND.
 *              This is the default for Record Number Databases.
 *        <li>@ref HAM_HINT_PREPEND. Hints the hamsterdb engine that the
 *              current key will compare as @e lower than any key already
 *              existing in the Database. The hamsterdb engine will verify
 *              this postulation and when found not to be true, will revert
 *              to a regular insert operation as if this flag was not
 *              specified. The incurred cost then is only one additional key
 *              comparison. Mutually exclusive with flag @ref HAM_HINT_APPEND.
 *      </ul>
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a key or @a record is NULL
 * @return @ref HAM_INV_PARAMETER if the Database is a Record Number Database
 *              and the key is invalid (see above)
 * @return @ref HAM_INV_PARAMETER if @ref HAM_PARTIAL was specified <b>AND</b>
 *              duplicate sorting is enabled (@ref HAM_SORT_DUPLICATES)
 * @return @ref HAM_INV_PARAMETER if the flags @ref HAM_OVERWRITE <b>and</b>
 *              @ref HAM_DUPLICATE were specified, or if @ref HAM_DUPLICATE
 *              was specified, but the Database was not created with
 *              flag @ref HAM_ENABLE_DUPLICATES.
 * @return @ref HAM_DB_READ_ONLY if you tried to insert a key to a read-only
 *              Database.
 * @return @ref HAM_INV_KEYSIZE if the key's size is larger than the @a keysize
 *              parameter specified for @ref ham_create_ex and variable
 *              key sizes are disabled (see @ref HAM_DISABLE_VAR_KEYLEN)
 *              OR if the @a keysize parameter specified for @ref ham_create_ex
 *              is smaller than 8.
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 *
 * @sa HAM_DISABLE_VAR_KEYLEN
 * @sa HAM_SORT_DUPLICATES
 * @sa HAM_PARTIAL
 * @sa ham_record_t
 * @sa ham_set_duplicate_compare_func
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_insert(ham_cursor_t *cursor, ham_key_t *key,
            ham_record_t *record, ham_u32_t flags);

/**
 * Erases the current key
 *
 * Erases a key from the Database. If the erase was
 * successful, the Cursor is invalidated and does no longer point to
 * any item. In case of an error, the Cursor is not modified.
 *
 * If the Database was opened with the flag @ref HAM_ENABLE_DUPLICATES,
 * this function erases only the duplicate item to which the Cursor refers.
 *
 * @param cursor A valid Cursor handle
 * @param flags Optional flags for erasing the key; unused, set to 0.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a cursor is NULL
 * @return @ref HAM_DB_READ_ONLY if you tried to erase a key from a read-only
 *              Database
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_erase(ham_cursor_t *cursor, ham_u32_t flags);

/**
 * Gets the number of duplicate keys
 *
 * Returns the number of duplicate keys of the item to which the
 * Cursor currently refers.
 * Returns 1 if the key has no duplicates.
 *
 * @param cursor A valid Cursor handle
 * @param count Returns the number of duplicate keys
 * @param flags Optional flags; unused, set to 0.
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 * @return @ref HAM_INV_PARAMETER if @a cursor or @a count is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_get_duplicate_count(ham_cursor_t *cursor,
        ham_size_t *count, ham_u32_t flags);

/**
 * Closes a Database Cursor
 *
 * Closes a Cursor and frees allocated memory. All Cursors
 * should be closed before closing the Database (see @ref ham_close).
 *
 * @param cursor A valid Cursor handle
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_CURSOR_IS_NIL if the Cursor does not point to an item
 * @return @ref HAM_INV_PARAMETER if @a cursor is NULL
 *
 * @sa ham_close
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_cursor_close(ham_cursor_t *cursor);

/**
* Retrieves the Database handle of a Cursor
*
* @param cursor A valid Cursor handle
*
* @return @a The Database handle of @a cursor
*/
HAM_EXPORT ham_db_t * HAM_CALLCONV
ham_cursor_get_database(ham_cursor_t *cursor);

/**
 * @}
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_HAMSTERDB_H__ */
