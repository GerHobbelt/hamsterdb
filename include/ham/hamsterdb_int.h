/**
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 *
 *
 * @file hamsterdb_int.h
 * @brief Internal hamsterdb Embedded Storage functions.
 * @author Christoph Rupp, chris@crupp.de
 *
 * @warning Please be aware that the interfaces in this file are mostly for internal
 * use. Unlike those in hamsterdb.h they are not stable and can be changed
 * with every new version.
 *
 */

#ifndef HAM_HAMSTERDB_INT_H__
#define HAM_HAMSTERDB_INT_H__

#include <ham/hamsterdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ham_extended_api hamsterdb Enhanced API
 * @{
 */

/**
 * @defgroup ham_special_db_names Special Database Names
 * @{
 */

/**
 * A reserved Database name for those Databases, who are created without
 * an Environment (and therefore do not have a name).
 *
 * Note that this value also serves as the upper bound for allowed
 * user specified Database names as passed to @a ham_env_create_db
 * or @a ham_env_open_db.
 */
#define HAM_DEFAULT_DATABASE_NAME     0xf000u

/**
 * A reserved Database name which automatically picks the first Database
 * in an Environment
 */
#define HAM_FIRST_DATABASE_NAME       0xf001u

/**
 * A reserved Database name for a dummy Database which only reads/writes
 * the header page
 */
#define HAM_DUMMY_DATABASE_NAME       0xf002u

/**
@}
*/


/**
 * Verifies the integrity of the Database
 *
 * This function is only interesting if you want to debug hamsterdb.
 *
 * @param db A valid Database handle
 * @param txn A Transaction handle, or NULL
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INTEGRITY_VIOLATED if the Database is broken
 * @return @ref HAM_NOT_IMPLEMENTED if hamsterdb was built without
 *              internal functions.
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_check_integrity(ham_db_t *db, ham_txn_t *txn);

/**
 * Estimates the number of keys stored per page in the Database
 *
 * @param db A valid Database handle
 * @param keycount A reference to a variable which will receive
 *                 the calculated key count per page
 * @param keysize The size of the key
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a keycount is NULL
 * @return @ref HAM_INV_KEYSIZE if the @a keycount turns out to be huge (i.e.
 *         larger than 65535); in this case @a keycount still contains a
 *         valid value, but this error indicates this keysize won't be
 *         usable with the given Database.
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_calc_maxkeys_per_page(ham_db_t *db, ham_size_t *keycount,
            ham_u16_t keysize);

/**
 * Set a user-provided context pointer
 *
 * This function sets a user-provided context pointer. This can be any
 * arbitrary pointer; it is stored in the Database handle and can be
 * retrieved with @a ham_get_context_data. It is mainly used by Wrappers
 * and language bindings.
 *
 * @param db A valid Database handle
 * @param data The pointer to the context data
 */
HAM_EXPORT void HAM_CALLCONV
ham_set_context_data(ham_db_t *db, void *data);

/**
 * Retrieves a user-provided context pointer
 *
 * This function retrieves a user-provided context pointer. This can be any
 * arbitrary pointer which was previously stored with @a ham_set_context_data.
 *
 * @param db A valid Database handle
 *
 * @return The pointer to the context data
 */
HAM_EXPORT void * HAM_CALLCONV
ham_get_context_data(ham_db_t *db);

/**
* Set a user-provided context pointer
*
* This function sets a user-provided context pointer. This can be any
* arbitrary pointer; it is stored in the Environment handle and can be
* retrieved with @a ham_env_get_context_data. It is mainly used by Wrappers
* and language bindings.
*
* @param env A valid Environment handle
* @param data The pointer to the context data
*/
HAM_EXPORT void HAM_CALLCONV
ham_env_set_context_data(ham_env_t *env, void *data);

/**
* Retrieves a user-provided context pointer
*
* This function retrieves a user-provided context pointer. This can be any
* arbitrary pointer which was previously stored with @a ham_env_set_context_data.
*
* @param env A valid Environment handle
*
* @return The pointer to the context data
*/
HAM_EXPORT void * HAM_CALLCONV
ham_env_get_context_data(ham_env_t *env);


#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

struct ham_file_filter_t;
typedef struct ham_file_filter_t ham_file_filter_t;

#endif


struct ham_device_t;
typedef struct ham_device_t ham_device_t;


/**
stores the connections from one device to any others in the device graph
*/
typedef struct ham_device_graph_connection_t
{
    ham_size_t count;   /**< number of edges connecting to other devices */

    /**
    an array of @a count connected devices; the pointer points at an allocated array of ham_device_t pointers.
    */
    ham_device_t **devs;

} ham_device_graph_connection_t;


/**
This value means the given minimum/maximum size value is 'unknown'.

@sa ham_minmax_size_t
@sa ham_device_class_info_t
*/
#define HAM_DEV_MINMAX_SIZE_UNKNOWN   HAM_MAX_SIZE_T

/**
Basic container for minimum/maximum size specifications.

When a maximum size is 'unknown' such is signalled by setting the value to @ref HAM_DEV_MINMAX_SIZE_UNKNOWN
*/
typedef struct ham_minmax_sizes_t
{
    ham_size_t minimum;
    ham_size_t maximum;
} ham_minmax_size_t;


/**
This struct defines the properties which are common to all instances of a given device/filter. These datums will be provided by the
page filter @ref class_info_cb callback method.
*/
typedef struct ham_device_class_info_t
{
    /**
    Descriptive name of the targeted device class. Will be a non-NULL pointer to a non-modyfiable ANSI string.
    */
    const char *device_name;

    /**
     @defgroup device_characteristics_flags The device type / characteristics flags

     Flags which answer questions like: is this a virtual and/or physical device,
     does it support memory-mapping, does it require additional per-page header/footer space,
     etc.

     @{
     */

    unsigned is_physical_device: 1;                     /**< can act as a physical end node */
    unsigned is_virtual_device: 1;                      /**< can act as a virtual device, i.e. requires at least 1 physical device 'downwind' */
    unsigned adds_header_or_footer_per_page: 1;         /**< @sa header_size, footer_size */
    unsigned adds_header_or_footer_in_first_page: 1;    /**< @sa first_extra_header_size, @sa first_extra_footer_size */
    unsigned supports_direct_IO: 1;                     /**< supports memory-mapping or other/similar non-copying I/O technology */
    unsigned is_a_partitioner: 1;                       /**< offers partitioning functionality, i.e. splits a flat range into multiple sections */
    unsigned is_growth_front: 1;                        /**< this device is allowed to 'grow' its storage space when the need to increase
                                                             the storage space arises for hamsterdb. Can only be true when the device can grow its
                                                             storage space: @sa can_be_growth_front */
    unsigned can_be_growth_front: 1;                    /**< this device can 'grow' its storage space when the need arises. */

    /**
    @}
    */

    /**
     @defgroup device_characteristics_chunk_sizes The device type / characteristics minimum / maximum size specifications

     @{
     */

    /**
    The minimum/maximum size of the header added by this device/filter for each page of data.
    */
    ham_minmax_size_t header_size;

    /**
    The minimum/maximum size of the footer added by this device/filter for each page of data.
    */
    ham_minmax_size_t footer_size;

    /**
    The minimum/maximum size of the additional file header added by this device/filter for the first page of the file.

    Hence, the total header size requested by this filter for the first page is first_extra_header_size+header_size
    and just header_size for any other page of the database file. This way we enable devices/filters to store
    a device/filter-specific file header with each database file.

    Take for example an advanced encryption filter: it would need to store identifiers for the encryption and
    authentication/validation algorithms used, plus probably a unique per-file IV, i.e. a chunk of cryptographically
    strong random data, which is used to create unique per-page nonces/IVs for this database, and maybe, say,
    a public key or certificate for nonrepudiation. This kind of data needs to be stored only once per file,
    so stretching the per-page header size to accommodate these items would be, at best, a file space utilization
    efficiency reduction. (And then, imagine what reserving space per page for storing a certificate would cost,
    while it only needs to be stored once, in the file header!)
    */
    ham_minmax_size_t first_extra_header_size;

    /**
    As for @ref first_extra_header_size: the minimum/maximum footer size occupied in the first database page.
    */
    ham_minmax_size_t first_extra_footer_size;

    /**
    The size of the 'trailing surplus' space needed by this device/filter for each page of data

    Some filters benefit from having an additional 'tail space' available beyond the end
    of the data; this is a kind of 'scratch space' which can be used by the algorithms to improve the
    transformation performance. For example, some high performance implementations of certain
    encryption algorithms will 'overshoot' the data output size, temporarily writing a few more
    bytes; these are good to have available for such algorithms, e.g. for systems which store
    truncated MACs with the encrypted data: the full MAC can be written, while only a specified,
    reduced number of bytes is transferred to the next stage (tail truncation).

    Hence, the 'trailing surplus' does @e not impact the pagesize as seen by hamsterdb, but @e does
    impact the number of bytes allocated by hamsterdb for each page of data before invoking either
    the @ref before_write_cb or @ref after_read_db callbacks in here.

    The same applies to the 'leading_surplus_size' value below, which is good to have when, for
    example, using encryption methods, which require a data prefix, such as an IV, while such data
    can be calculated by the system and does not need to be stored in the header. (Counter-based
    nonces using the @ref file_offset parameter as a per-database-unique counter
    come to mind.)
    */
    ham_minmax_size_t trailing_surplus_size;

    /**
    @sa trailing_surplus_size
    */
    ham_minmax_size_t leading_surplus_size;

    /**
    @}
    */

    /**
    (For internal use)
    
    The reference count, i.e. how many device instances of this class have been generated (including possibly hidden instances).
    */
    ham_size_t instance_count;

} ham_device_class_info_t;


/**
These define the file/page layout for a given device/filter. These datums must be provided by the
page filter @ref info_cb callback method and must be static and valid before the first 'cooked' data
can be read of written from the database file.

@ref first_extra_header_size,
@ref header_size,
@ref footer_size,
@ref trailing_surplus_size and
@ref leading_surplus_size
must be specified by the page filter.

@ref raw_pagesize and
@ref cooked_pagesize
are both derived values, calculated by the hamsterdb itself; these datums will be valid from the
first data write or read operation onwards.

@warning
Note that there is one very special exception to the above: when an existing database is opened,
the header is read. This is the very first read after opening the database. In this particular case
we do not know yet which header/footer/etc. settings were used when the database was created
previously; this data has to be divulged by the database header. Which may be compressed, encrypted
or otherwise transformed using yet incompletely known settings.

To successfully complete this reading of the database header data, the initial read will be an
iterative process at which' start few elements are known (raw page size only), and each iteration
will resolve the data for at least one filter in the chain (as that filter will receive the raw data
and sufficient and correct context to successfully complete the 'cooking' process (raw->cooked).

Thus the precise reality is that this data will be complete and correct at the successful @e end of
the initial read after open device operation.
*/
typedef struct ham_device_info_t
{
    unsigned dirty_info: 1; /**< info has been changed; derived elements are invalid. This bit is set when device settings
                                have been updated, but not yet entirely processed. */

    unsigned is_physical_device: 1; /**< can act as a physical end node */
    unsigned is_virtual_device: 1;  /**< can act as a virtual device, i.e. requires at least 1 physical device 'downwind' */
    unsigned supports_direct_IO: 1; /**< supports memory-mapping or other/similar non-copying I/O technology */
    unsigned is_growth_front: 1; /**< this device is allowed to 'grow' its storage space when the need to increase
                                     the storage space arises for hamsterdb. Can only be true when the device can grow its
                                     storage space: @sa can_be_growth_front */

    /**
     The lowest addressable unit for this device

     When a chunk is read or written, the hamsterdb will always identify it using a unique 'rid' which is
     similar to a file offset for regular files.

     Devices receive this 'rid' along with the request to retrieve or store a chunk of data; each device can
     quickly determine whether the addressed space is managed by the given device by comparing the given 'rid'
     with the @a base_offset value: when the 'rid' is equal to or larger than the @a base_offset, then
     the given device is targeted.

     A similar comparison is applied for the upper bound, by comparing 'rid'+data length with the
     @a upper_bound_offset.
    */
    ham_offset_t base_offset; /**< lower bound of addressable range (inclusive) */
    ham_offset_t upper_bound_offset; /**< upper bound of addressable range (inclusive!); <code>~(ham_offset_t)0</code> means 'infinity' */

#define HAM_DEVICE_OFFSET_INFINITY    HAM_MAX_OFFSET_T

    /**
    The size of the header added by this device/filter for each page of data
    */
    ham_size_t header_size;

    /**
    The size of the footer added by this device/filter for each page of data
    */
    ham_size_t footer_size;

    /**
    The size of the additional file header added by this device/filter for the first page of the file

    Hence, the total header size requested by this filter for the first page is first_extra_header_size+header_size
    and just header_size for any other page of the database file. This way we enable devices/filters to store
    a device/filter-specific file header with each database file.

    Take for example an advanced encryption filter: it would need to store identifiers for the encryption and
    authentication/validation algorithms used, plus probably a unique per-file IV, i.e. a chunk of cryptographically
    strong random data, which is used to create unique per-page nonces/IVs for this database, and maybe, say,
    a public key or certificate for nonrepudiation. This kind of data needs to be stored only once per file,
    so stretching the per-page header size to accommodate these items would be, at best, a file space utilization
    efficiency reduction. (And then, imagine what reserving space per page for storing a certificate would cost,
    while it only needs to be stored once, in the file header!)
    */
    ham_size_t first_extra_header_size;

    ham_size_t first_extra_footer_size;

    /**
    The size of the 'trailing surplus' space needed by this device/filter for each page of data

    Some filters benefit from having an additional 'tail space' available beyond the end
    of the data; this is a kind of 'scratch space' which can be used by the algorithms to improve the
    transformation performance. For example, some high performance implementations of certain
    encryption algorithms will 'overshoot' the data output size, temporarily writing a few more
    bytes; these are good to have available for such algorithms, e.g. for systems which store
    truncated MACs with the encrypted data: the full MAC can be written, while only a specified,
    reduced number of bytes is transferred to the next stage (tail truncation).

    Hence, the 'trailing surplus' does @e not impact the pagesize as seen by hamsterdb, but @e does
    impact the number of bytes allocated by hamsterdb for each page of data before invoking either
    the @ref before_write_cb or @ref after_read_db callbacks in here.

    The same applies to the 'leading_surplus_size' value below, which is good to have when, for
    example, using encryption methods, which require a data prefix, such as an IV, while such data
    can be calculated by the system and does not need to be stored in the header. (Counter-based
    nonces using the @ref file_offset parameter as a per-database-unique counter
    come to mind.)
    */
    ham_size_t trailing_surplus_size;

    /**
    @sa trailing_surplus_size
    */
    ham_size_t leading_surplus_size;

    /**
    The number of bytes allocated by hamsterdb before invoking either
    the @ref before_write_cb or @ref after_read_db callbacks
    */
    ham_size_t allocated_file_size;

    /**
    The size of a raw database page, i.e. a database page as it is mapped to
    disc space.

    Usually a constant input value for all callbacks, but may be modified by the
    @ref after_read callback method while it decoded the header page.

    @sa HAM_INVALID_PAGEFILTER_PAGESIZE_ERROR
    */
    ham_size_t raw_pagesize;

    /**
    The size of a database page as perceived by hamsterdb, i.e. a database page which
    includes both the hamsterdb-written data and all headers and footers as they are
    added by the devices/filters at the front of this filter chain (i.e. the filters
    which are invoked before this filter)
    */
    ham_size_t cooked_pagesize;

    /**
    pagesize bitmask: defines the minimum required alignment of the device page.

    Works like an IP mask, i.e. all -bits identify allowable address bits, so that
    an aligned address may be obtained through the bit-wise AND operation, e.g.

    <pre>
    aligned_address = (address & info.pagesize_mask);
    </pre>
    */
    ham_size_t pagesize_mask;

} ham_device_info_t;



#if 0 /* [i_a] obsoleted; use the new device graph architecture instead */

/**
The structure passed by the hamster to the file filter read/write callbacks.

This structure lists the input buffer reference and size and the available
output/scratch buffer and size.

Note that both the preset source and destination references in here are already 'adjusted'
for the needs of the currently invoked filter, given it's header/footer/surplus requirements,
which means that the total data space of each buffer is at least the specified size @e plus
the current filter's head and tail surplus. The filter's head surplus is 'hidden' in the
sense that it is available as 'negative indexed space' in both the @ref source and @ref dest
buffers. This info can also be gleaned from the extended parameters in this structure.
*/
typedef struct ham_file_filter_bufinfo_t
{
    /**
    The offset of the data within the file as it is perceived by hamsterdb.
    */
    ham_offset_t file_offset;

    /** reference to the source data */
    ham_u8_t *source;
    /** the size of the source buffer */
    ham_size_t sourcewidth;

    /** reference to the destination data */
    ham_u8_t *dest;
    /** the size of the destination buffer */
    ham_size_t destwidth;

    /** the amount of 'negative indexed' head surplus space available in the source buffer */
    ham_size_t source_head_surplus;
    /** the amount of tail surplus space available in the source buffer */
    ham_size_t source_tail_surplus;

    /** the amount of 'negative indexed' head surplus space available in the destination buffer */
    ham_size_t dest_head_surplus;
    /** the amount of tail surplus space available in the destination buffer */
    ham_size_t dest_tail_surplus;

    /** whether source and destination buffer are the same */
    ham_bool_t dest_is_source;
    /** whether the destination buffer is memory mapped */
    ham_bool_t dest_is_mmap;
    /** whether the destination buffer is allocated on the heap (using the database-registered allocator) */
    ham_bool_t dest_is_heap;

    /**
    A reference to the current database handle which triggered this file I/O operation.

    @warning
    This does not imply the data written is related to that database handle per se:
    cache flush operations can flush or fetch pages which (partially) contain data for
    different databases in the given environment.
    */
    ham_db_t *triggering_db;

} ham_file_filter_bufinfo_t;



/**
 * A callback function for a file-level filter; called before the
 * data is written to disk
 */
typedef ham_status_t ham_file_filter_before_write_cb_t(
            ham_file_filter_t *filter,
            ham_env_t *env,
            ham_file_filter_bufinfo_t *data);

/**
 * A callback function for a file-level filter; called immediately after the
 * data is read from disk
 */
typedef ham_status_t ham_file_filter_after_read_cb_t(
            ham_file_filter_t *filter,
            ham_env_t *env,
            ham_file_filter_bufinfo_t *data);

/**
 * A callback function for a file-level filter; called immediately before the
 * Environment is opened/created. Can be used for filter setup.
 *
 * By this time, the environment will not yet have a header page in place: this
 * callback is invoked before any page is created or written.
 *
 * @param creating will be HAM_TRUE when the environment is being created from scratch,
 *                 and HAM_FALSE when an existing environment is opened.
 */
typedef ham_status_t ham_file_filter_init_cb_t(
            ham_file_filter_t *filter,
            ham_env_t *env,
            ham_bool_t creating);

/**
 * A callback function for a file-level filter; called immediately before the
 * Environment is closed. Can be used to avoid memory leaks.
 */
typedef ham_status_t ham_file_filter_close_cb_t(ham_file_filter_t *filter,
                                                ham_env_t *env);

/**
* A callback function for a file-level filter; called when the data is explicitly flushed
* to the raw device.
*/
typedef ham_status_t ham_file_filter_flush_cb_t(ham_file_filter_t *filter,
                                                ham_env_t *env);


/**
 * A callback function for a file-level filter; called while
 * or after the Environment is created
 * or opened (with @ref ham_env_create[_ex] or @ref ham_env_open[_ex]).
 * Is used by hamsterdb to collect file-specific information, such as
 * the amount of space per page used by custom headers (or footers)
 * as they are added by the file filter write operations.
 *
 * The info is retrieved by hamsterdb through inspecting the content of
 * the @ref ham_file_filter_t.dev_info_ref data reference.
 */
typedef ham_status_t ham_file_filter_get_info_cb_t(ham_file_filter_t *filter,
                                                   ham_env_t *env);

/**
 * A handle for file-level filtering
 *
 * File-level filters can modify the page data before some data is
 * written to disk, and immediately after it's read from disk.
 *
 * File-level filters can be used for example for writing encryption filters.
 * See @a ham_env_enable_encryption() to create a simple filter for AES-based
 * encryption.
 *
 * Any of the callback functions may be NULL.
 *
 * Before this structure is registered/used, it has to be initialized with zeroes.
 * You may set up the values in the @ref info section of the structure before
 * registration or alternatively fill it at the first invocation of the @ref info_cb
 * callback method.
 *
 * @note Filter Chain Invocation Order
 *
 * <ul>
 * <li> @ref init_cb
 *       page filter @e init callbacks are executed in 'write/forward' order, i.e.
 *       the filter which was registered first, and consequently will be the first in the
 *       chain when a page is @e written, will also be the first filter having its
 *       init callback invoked.
 *
 * <li> @ref write_cb
 *       page filter @e write callbacks are executed in 'write/forward' order, i.e.
 *       the filter which was registered first will be the first in the
 *       chain when a page is @e written.
 *
 * <li> @ref read_cb
 *       page filter @e read callbacks are executed in 'read/reverse' order, i.e.
 *       the filter which was registered @e last will be the first in the
 *       chain when a page is @e read. As the chain will be traversed in the reverse
 *       order compared to @e write, properly stacked processing of the page content is
 *       assured.
 *
 * <li> @ref close_cb
 *       page filter @e close callbacks are executed in 'write/forward' order, i.e.
 *       the filter which was registered first, and consequently will be the first in the
 *       chain when a page is @e written, will also be the first filter having its
 *       close callback invoked.
 *
 *       One might have expected that the @e close chain might have been selected to execute
 *       in the reverse order of @e init, i.e. in 'read/reverse' order, but this would prevent
 *       stateful filters from correctly flushing remaining data at this last notice, as then
 *       the rest of the 'write' chain would already have been closed/destroyed.
 *
 *       As this was deemed a hazard for page filter design, the only alternative was to have
 *       @e close chain execute in 'write/forward' order, which may be counterintuitive for
 *       first time page filter designers.
 *
 * <li> @ref info_cb
 *       page filter @e info callbacks are executed in 'write/forward' order, i.e.
 *       the filter which was registered first, and consequently will be the first in the
 *       chain when a page is @e written, will also be the first filter having its
 *       info callback invoked.
 *
 * </ul>
 */
struct ham_file_filter_t
{
    /** user data, freely assignable.

 @warning
 Any referenced data must have a scope/lifetime
 equal or larger/longer than the @ref ham_env_t environment they are registered with.
 Of course you can change this reference or the data referenced at run-time, but you
 must this lifetime warning, or alternatively remove the reference to shorter-lived data
 particles and ensure your filter can cope with that.

 Failure to adhere to this rule will result in spurious run-time failures.
    */
    void *userdata;

    /** The function which is called before the page is written to the raw device */
    ham_file_filter_before_write_cb_t *write_cb;

    /** The function which is called after the page is read from the raw device */
    ham_file_filter_after_read_cb_t *read_cb;

    /** The function which is called when the data is explicitly flushed to the raw device */
    ham_file_filter_flush_cb_t *flush_cb;

    /** The function which is called when the database environment is created / opened through
    an invocation of @ref ham_env_create or @ref ham_env_open */
    ham_file_filter_init_cb_t *init_cb;

    /** The function which is called when the database environment is closed through an
    invocation of @ref ham_env_close */
    ham_file_filter_close_cb_t *close_cb;

    /** The function which is called to calculate the cooked pagesize, etc., given the returned
    header and footer sizes.

    @warning
    This function may be invoked before the database environment is created / opened and your
    implementation must be able to cope with this scenario as well. */
    ham_file_filter_get_info_cb_t *info_cb;

    /**
    the 'device info' applicable to this filter stage; this data has to be initialized
    through invoking the @ref info_cb callback
    */
    ham_device_info_t info;

    /**
    Whether or not the @ref read_cb callback processes data 'inline', i.e. is capable
    of transforming the input data in the buffer itself, without the need for a
    target buffer.
    */
    ham_bool_t read_happens_inline;

    /**
    Whether or not the @ref write_cb callback processes data 'inline', i.e. is capable
    of transforming the output data in the buffer itself, without the need for a
    target buffer.
    */
    ham_bool_t write_happens_inline;

    /** For internal use */
    ham_bool_t has_been_initialized;

    /** For internal use */
    ham_u32_t _flags;

    /** For internal use */
    ham_file_filter_t *_next;
    ham_file_filter_t *_prev;

};

/**
 * A function to install a file-level filter.
 *
 * Filters are appended in the @e write direction, i.e. adding filters A and B
 * in that order will see them executed in exactly that same order when a
 * page of data is @e written to the device (~ file), while, of course, these
 * filters are executed in @e reverse order when a page of data is @e read
 * from that same device.
 *
 * Hence filters can be chained or regarded as a stack or long pipe where data
 * travels from front to back towards the raw device and bubbles up from the
 * raw device to the 'cooked' data perused by the hamster database.
 *

 @warning
 * This function has to be called after the Environment handle has been
 * allocated (with @ref ham_env_new) and before the Environment is created
 * or opened (with @ref ham_env_create / @ref ham_env_create_ex or @ref ham_env_open / @ref ham_env_open_ex).

 @warning
 Filters and their associated data as referenced in the @ref ham_file_filter_t
 structure as well as the @ref ham_file_filter_t structure itself must have a scope/lifetime
 equal or larger/longer than the @ref ham_env_t environment they are registered with.
 Failure to adhere to this rule will result in spurious run-time failures.

 * @param env A valid Environment handle
 * @param filter A pointer to a ham_file_filter_t structure
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a env or @a filter is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_add_file_filter(ham_env_t *env, ham_file_filter_t *filter);

/**
 * A function to remove a file-level filter.
 *
 * This function is usually not necessary - the lifetime of a file-filter
 * usually starts before the first Database Environment operation, and ends at or beyond when that
 * environment is closed.

@warning
It is not recommended to use this function.
 *
 * @param env A valid Environment handle
 * @param filter A pointer to a ham_file_filter_t structure
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a env or @a filter is NULL
 * @return @ref HAM_FILTER_NOT_FOUND if @a filter was not registered
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_remove_file_filter(ham_env_t *env, ham_file_filter_t *filter);

#endif

struct ham_record_filter_t;
typedef struct ham_record_filter_t ham_record_filter_t;


/**
These are set up by hamsterdb before each invocation of the record filter callbacks
to allow callbacks access to important parts of the hamsterdb, e.g. the current
transaction.

Without these additional datums, functionality implemented through callbacks would
be severely restricted (as it was before hamsterdb release 1.1.5) as callbacks
could not tie recursive hamsterdb access occuring in the callback to the pending
transaction or parent operation in any way.
*/
typedef struct ham_rec_filter_info_io_t
{
    /**
    The key related to this record.
    */
    ham_key_t *key;

    /**
    The transaction within which' context this record I/O operation is performed
    */
    ham_txn_t *txn;

    /**
    The cursor which' triggered this record I/O operation
    */
    ham_cursor_t *triggering_cursor;

} ham_rec_filter_info_io_t;


/**
 * A callback function for a record-level filter; called before the
 * record is written to disk (inserted/updated)
 */
typedef ham_status_t (*ham_record_filter_before_write_cb_t)(ham_db_t *db,
        ham_record_filter_t *filter, ham_record_t *record, ham_rec_filter_info_io_t *info);

/**
 * A callback function for a record-level filter; called immediately after the
 * record is read from disk, and before it is returned to the user.
 */
typedef ham_status_t (*ham_record_filter_after_read_cb_t)(ham_db_t *db,
        ham_record_filter_t *filter, ham_record_t *record, ham_rec_filter_info_io_t *info);

/**
 * A callback function for a record-level filter; called immediately before the
 * Database is closed. Can be used to avoid memory leaks.
 */
typedef void (*ham_record_filter_close_cb_t)(ham_db_t *db,
        ham_record_filter_t *filter, ham_rec_filter_info_io_t *info);

/**
 * A handle for record-level filtering
 *
 * Record-level filters can modify and resize the record data before
 * the record is inserted, and before it is returned to the user.
 *
 * Record-level filters can be used for example for writing compression
 * filters.  See @ref ham_enable_compression() to create a filter for zlib-based
 * compression.
 *
 * Each of the three callback functions can be NULL.
 *
 * Before this structure is used, it has to be initialized with zeroes.
 */
struct ham_record_filter_t
{
    /** The user data */
    void *userdata;

    /** The function which is called before the record is written (inserted/updated) */
    ham_record_filter_before_write_cb_t before_write_cb;

    /** The function which is called after the record is read from disk */
    ham_record_filter_after_read_cb_t after_read_cb;

    /** The function which is when the Database is closed */
    ham_record_filter_close_cb_t close_cb;

    /** For internal use */
    ham_u32_t _flags;

    /** For internal use */
    ham_record_filter_t *_next, *_prev;

};

/**
 * A function to install a record-level filter.
 *
 * Record-level filters are usually installed immediately after the Database
 * is created with @ref ham_create / @ref ham_create_ex / @ref ham_env_create_db or opened with @ref ham_open / @ref ham_open_ex / @ref ham_env_open_db .
 *
 * @param db A valid Database handle
 * @param filter A pointer to a ham_record_filter_t structure
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a filter is NULL
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_add_record_filter(ham_db_t *db, ham_record_filter_t *filter);

/**
 * A function to remove a record-level filter.
 *
 * This function is usually not necessary - the lifetime of a record-filter
 * usually starts before the first Database operation, and ends when the
 * Database is closed. It is not recommended to use this function.
 *
 * @param db A valid Database handle
 * @param filter A pointer to a ham_record_filter_t structure
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a filter is NULL
 * @return @ref HAM_FILTER_NOT_FOUND if @a filter was not registered
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_remove_record_filter(ham_db_t *db, ham_record_filter_t *filter);

/**
 * Install a custom device object
 *
 * Custom device objects can be used to overwrite the functions which
 * open, create, read, write etc. to/from the file.
 *
 * The device structure is defined in src/device.h. The default device
 * objects (for file-based access and for in-memory access) are implemented
 * in src/device.c.
 *
 * This function has to be called after the Environment handle has been
 * allocated (with @ref ham_env_new ) and before the Environment is created
 * or opened (with @ref ham_env_create / @ref ham_env_create_ex or @ref ham_env_open / @ref ham_env_open_ex).
 *
 * @param env A valid Environment handle
 * @param device A pointer to a ham_device_t structure
 *
 * @return @ref HAM_SUCCESS upon success
 * @return @ref HAM_INV_PARAMETER if @a db or @a device is NULL
 * @return @ref HAM_ALREADY_INITIALIZED if this function was already called
 *            for this Environment
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_set_device(ham_env_t *env, ham_device_t *device);



typedef struct ham_backend_t ham_backend_t;

/**
Register a backend for one of these identifiable backends:
B-tree, Hash, and Custom, which are identified by the @a type

<ul>
<li>@ref HAM_USE_BTREE
<li>@ref HAM_USE_HASH
<li>@ref HAM_USE_CUSTOM_DB_ALGO
</ul>

so that @a type equals

<pre>
flags & @ref HAM_USE_DB_ALGO_MASK
</pre>

Any of these backends can be overriden at the individual db level
through registering a different (overruling) backend with that
@ref ham_db_t by invoking @ref ham_register_backend.

@note backends are automatically cleaned up and unregistered when
      you call @ref ham_env_close.

@sa ham_env_register_backend
*/
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_env_register_backend(ham_env_t *env, int type, ham_backend_t *backend);

/**
Register a backend with this particular database @a db
for one of these identifiable backends:
B-tree, Hash, and Custom, which are identified by the @a type

<ul>
<li>@ref HAM_USE_BTREE
<li>@ref HAM_USE_HASH
<li>@ref HAM_USE_CUSTOM_DB_ALGO
</ul>

so that @a type equals

<pre>
flags & @ref HAM_USE_DB_ALGO_MASK
</pre>

Any of these backends can be overriden at the individual db level
through registering a different (overruling) backend with that
@ref ham_db_t by invoking @ref ham_register_backend.

@note backends are automatically cleaned up and unregistered when
      you call @ref ham_env_close.

@sa ham_env_register_backend
*/
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_register_backend(ham_db_t *env, int type, ham_backend_t *backend);



/**
Writes the specified Database Access Flags (see @ref ham_database_flags)
as a string to the buffer.

@param buf points to a buffer of size @a buflen
@param buflen the size of the buffer, generally this is sizeof(buf) if
       buf is an array type.
@param db_flags the database access flags

@sa ham_database_flags
*/
HAM_EXPORT const char * HAM_CALLCONV
ham_create_flags2str(char *buf, ham_size_t buflen, ham_u32_t db_flags);

/**
Writes a database configuration parameter id (see @ref ham_database_cfg_parameters)
as a string to the buffer.

@param buf points to a buffer of size @a buflen
@param buflen the size of the buffer, generally this is sizeof(buf) if
       buf is an array type.
@param name the parameter id

@sa ham_database_cfg_parameters
*/
HAM_EXPORT const char * HAM_CALLCONV
ham_param2str(char *buf, ham_size_t buflen, ham_u32_t name);





typedef struct
{
    /** The actual number of bytes allocated. */
    ham_size_t alloc_size;

    /**
     * flags which provide additional info about the allocated space.
     * when @ref HAM_DIRECT_ACCESS is set, hamster is signalling that the allocated storage space is
     * 'direct access', e.g. backed by a memory-mapped sorage space.
     */
    ham_u32_t flags;

    /** Points at the start of the allocated space */
    void *data;

    /**
     * The 'record ID' (RID) which is a reference to the storage space / location which can
     * itself be persisted, e.g. by storing this RID as part of a key or record in a database.
     *
     * You'll need to pass this RID to @ref ham_fetch_dedicated_storage_space in any future run
     * to obtain access to the data you persisted though @ref ham_alloc_dedicated_storage_space
     * plus @ref ham_flush_dedicated_storage_space .
     */
    ham_pers_rid_t rid;

} ham_dedicated_storage_space_t;

/**
 * Allocate one or more storage pages for direct access (possibly mmap assisted) storage space in the hamsterdb file.
 *
 * Users may use this call to obtain dedicated storage space through the same device access channels as used by the hamster.
 * On the usual platforms this in particular means that you can hence keep @e all your persisted storage in a single file
 * (the hamster environment) while using the optimized device interfaces which come with the hamster.
 *
 * As the hamster only knows the thusly allocated space is in use by the user, but nothing else, you must invoke
 * @ref ham_release_dedicated_storage_space to return the reserved storage space to the hamster space manager.
 *
 * When you allocate the storage space, this function delivers a pointer to the user space. Note that each reserved storage space
 * is preceeded by a page header identifying the storage as such, so you will not receive an entire page's worth of bytes
 * for each allocated page.
 *
 * The actually allocated space size is returned in the variable referenced by the @a alloc_size_ref reference
 * reference. The hamster will allocate an suitable series of disc pages to fit your request size, so the returned size
 * may be equal or larger than your requested size as specified in the @a request_size parameter.
 *
 * @return When an error occurred during the allocation, the related response code is returned as the function result.
 *         The function returns @ref HAM_SUCCESS when the allocation request was succesful.
 *
 * @warning This storage is not transaction-safe, meaning that any write activity is non-recoverable. If you want
 *          transaction-safe, recoverable persistent storage, save your data using regular hamster records (@ref ham_record_t)
 *          instead.
 *          This type of 'direct access storage' is meant for storing and manipulating large to very large amounts of
 *          data which must be persisted, but where fast access to the persisted storage is the prevalent factor and
 *          recovery and transaction safety measures are either handled at the application level or neglected entirely.
 *
 * @sa ham_release_dedicated_storage_space
 * @sa ham_fetch_dedicated_storage_space
 * @sa ham_flush_dedicated_storage_space
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_alloc_dedicated_storage_space(ham_env_t *env, ham_dedicated_storage_space_t *storage_info_ref, ham_size_t request_size);

/**
 * Release the space allocated by a previous call to @ref ham_alloc_dedicated_storage_space
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_release_dedicated_storage_space(ham_env_t *env, ham_dedicated_storage_space_t *storage_info_ref);

/**
 * Load the dedicated direct access persisted storage space from the hamsterdb database store which was previously allocated
 * through a call to @ref ham_alloc_dedicated_storage_space
 *
 * The storage space is identified by the 'record ID' (a.k.a. RID) which was returned as part of the original allocation request
 * in the @ref ham_dedicated_storage_space_t structure which was filled by @ref ham_alloc_dedicated_storage_space on succesful
 * allocation.
 *
 * @return Returns HAM_SUCCESS when the data was succesfully loaded (mapped into memory); any other status code indicates an
 *         error occurred while trying to access/map the indicated storage space.
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_fetch_dedicated_storage_space(ham_env_t *env, ham_dedicated_storage_space_t *storage_info_ref, ham_pers_rid_t rid);

/**
 * Write/flush the modified data in the allocated direct access storage space to persisted storage.
 */
HAM_EXPORT ham_status_t HAM_CALLCONV
ham_flush_dedicated_storage_space(ham_env_t *env, ham_pers_rid_t rid);


/**
 * @}
 */


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_HAMSTERDB_INT_H__ */
