/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

/**
* @cond ham_internals
*/

/**
 * @brief device management
 *
 * A device encapsulates the physical device, either a
 * file or memory chunks (for in-memory-databases).
 *
 */

#ifndef HAM_DEVICE_H__
#define HAM_DEVICE_H__

#include "internal_fwd_decl.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 Contains additional space allocation request attributes.

 This info can be used by partitioners and other devices to determine which
 child device is going to service the request, i.e. in which partition the
 requested space will be allocated.
*/
struct dev_alloc_request_info_ex_t
{
    /** [input] the type of space requested. One of the @ref page_type_codes */
    ham_u32_t space_type;
    /** [input] whether the request is for an entire page or just some chunks */
    ham_bool_t entire_page;
    /** [input] DAM (Data Access Mode hint flags */
    ham_u16_t dam;
    /** [input] the Database requesting the space */
    ham_db_t *db;
    /** [input] the Environment */
    ham_env_t *env;
    /** [input] the Key which triggered this request */
    const ham_key_t *key;
    const int_key_t *int_key;
    /** [input] the Record which triggered this request */
    const ham_record_t *record;
    /** [input] the related 'rid' which is managed by the freelist to pick; used with @ref PAGE_TYPE_FREELIST type requests */
    ham_offset_t related_address;
    /** [input] the master request of which this request is a part */
    dev_alloc_request_info_ex_t *master;
    /** [input */
    ham_u32_t insert_flags;

    /** [output] the device which serviced the request */
    ham_device_t *dev_servicer;
    /** [output] the freelist which serviced the request */
    freelist_cache_t *freelist_servicer;
    /** [output] the base rid/offset of the space range managed by the servicing device */
    ham_offset_t dev_base_addr;
    /** [output] the absolute maximum range of the space managed by the servicing device */
    ham_offset_t dev_addr_range;
};



/**
 * the device structure
 */
struct ham_device_t {
    /**
     * create a new device/store
     */
    ham_status_t (*create)(ham_device_t *self, const char *fname,
            ham_u32_t flags, ham_u32_t mode);

    /**
     * open an existing device/store
     */
    ham_status_t (*open)(ham_device_t *self, const char *fname,
            ham_u32_t flags);

    /**
     * close the device/store
     */
    ham_status_t (*close)(ham_device_t *self);

    /**
     * flushes the device/store
     */
    ham_status_t (*flush)(ham_device_t *self);

    /**
     * truncate/resize the device/store
     */
    ham_status_t (*truncate)(ham_device_t *self, ham_offset_t newsize);

    /**
     * returns true if the device/store is open
     */
    ham_bool_t (*is_open)(ham_device_t *self);

    /**
     * get the current file/storage size
     */
    ham_status_t (*get_filesize)(ham_device_t *self, ham_offset_t *length);

    /**
     * seek position in a file/store
     */
    ham_status_t (*seek)(ham_device_t *self, ham_offset_t offset, int whence);

    /**
     * tell the position in a file/store
     */
    ham_status_t (*tell)(ham_device_t *self, ham_offset_t *offset);

    /**
     * reads from the device/store; this function does not use mmap
     */
    ham_status_t (*read)(ham_device_t *self,
            ham_offset_t offset, void *buffer, ham_offset_t size);

    /**
     * writes to the device/store; this function does not use mmap,
     * the data is run through the file filters
     */
    ham_status_t (*write)(ham_device_t *self,
            ham_offset_t offset, void *buffer, ham_offset_t size);

    /**
     * reads a page from the device/store; this function CAN use mmap
     *
     * @return @ref HAM_LIMITS_REACHED when the underlying storage system is <em>temporarily</em> exhausted, that is:
     *         the underlying system, for example a memory mapped file, cannot map another page as the
     *         operating system signals that no more slots are available for memory mapping. This failure
     *         is <em>temporary</em> in that it makes sense for the caller to flush/release lingering cached pages
     *         in order to release some kernel resources, after which the caller may successfully retry this
     *         @a read_page() operation.
     *
     * @note The @ref HAM_LIMITS_REACHED error is the only error which will see some form of 'retry' mechanism
     *       happening in the calling layer.
     *
     * @sa page_fetch
     */
    ham_status_t (*read_page)(ham_device_t *self, ham_page_t *page,
            ham_size_t size);

    /**
     * writes a page to the device/store
     */
    ham_status_t (*write_page)(ham_device_t *self, ham_page_t *page);

    /**
     * get the pagesize for this device/store
     */
    ham_size_t (*get_pagesize)(ham_device_t *self);

    /**
     * set the pagesize for this device/store
     */
    ham_status_t (*set_pagesize)(ham_device_t *self, ham_size_t pagesize);

    /**
     * set the device/store flags
     */
    void (*set_flags)(ham_device_t *self, ham_u32_t flags);

    /**
     * get the device/store flags
     */
    ham_u32_t (*get_flags)(ham_device_t *self);

    /**
     Retrieve the device/store info (with or without the underlying devices taken into account:
     when @a inclusive is set to 'true', the device graph below the specified device will
     be traversed to determine the aggregated info values).

     @note The @ref ham_device_info_t @a info struct must be initialized before invoking this
     method, preferably with all zeroes.
     */
    ham_status_t (*get_device_info)(ham_device_t *self, ham_device_info_t *pinfo, ham_bool_t inclusive);

    /**
     Retrieve device class info, i.e. info which is common for all device instances of this device type.

     @note The @ref ham_device_class_info_t @a info struct must be initialized before invoking this
     method, preferably with all zeroes.
     */
    ham_status_t (*get_device_class_info)(ham_device_t *self, ham_device_class_info_t *pinfo);

    /**
     * allocate storage space from this device/store; this function
     * will *NOT* use mmap.
     */
    ham_status_t (*alloc)(ham_device_t *self, ham_size_t size,
            ham_offset_t *address, dev_alloc_request_info_ex_t *extra_dev_alloc_info);

    /**
     * allocate storage space for a page from this device/store; this function
     * @e can use mmap.
     *
     * @note
     * The caller is responsible for flushing the page; the @ref free_page function will
     * assert that the page is not dirty.
     *
     * @warning @ref alloc_page and @ref free_page are @e significantly different
     *          from the @ref request_space and @ref release_space methods which
     *          address the device-specific freelist manager instead.
     *
     * @return @ref HAM_LIMITS_REACHED when the underlying storage system is <em>temporarily</em> exhausted, that is:
     *         the underlying system, for example a memory mapped file, cannot allocate another page as the
     *         operating system signals that no more slots are available for memory mapping. This failure
     *         is <em>temporary</em> in that it makes sense for the caller to flush/release lingering cached pages
     *         in order to release some kernel resources, after which the caller may successfully retry this
     *         @a alloc_page() operation.
     *
     * @note The @ref HAM_LIMITS_REACHED error is the only error which will see some form of 'retry' mechanism
     *       happening in the calling layer.
     *
     * @sa page_alloc
     */
    ham_status_t (*alloc_page)(ham_device_t *self, ham_page_t *page,
            ham_size_t size, dev_alloc_request_info_ex_t *extra_dev_alloc_info);

    /**
     * frees a page on the device/store; plays counterpoint to @ref alloc_page.
     *
     * @warning @ref alloc_page and @ref free_page are @e significantly different
     *          from the @ref request_space and @ref release_space methods which
     *          address the device-specific freelist manager instead.
     */
    ham_status_t (*free_page)(ham_device_t *self, ham_page_t *page);

    /**
     * destroy the device object, free all memory
     */
    ham_status_t (*destroy)(ham_device_t **self_reference);

    /**
     * register another @a target device; adds an outgoing edge to the device
     * graph of which the current device is a part.
     *
     * @param partition_index is zero or higher and indicates which partition must be forwarded
     *        to the specified device. For non-partitioning devices, the @a partition_index
     *        should be set to zero(0).
     */
    ham_status_t (*add_outgoing)(ham_device_t *self, ham_u32_t partition_index, ham_device_t *target);

    /**
     * register another @a source device; adds an incoming edge to the device
     * graph of which the current device is a part.
     */
    ham_status_t (*add_incoming)(ham_device_t *self, ham_device_t *source);

    /**
     * the memory allocator
     */
    mem_allocator_t *_malloc;

    /**
    * the environment which employs this device
    */
    ham_env_t *_env;

    /**
     stores the device class common info, e.g.:

     <ul>
     <li>the class descriptive name,</li>
     <li>the size ranges for page headers and footers</li>
     </ul>
    */
    ham_device_class_info_t *_class_info;

    /**
     stores the device/store info, e.g.:

     <ul>
     <li>the pagesize on disc,</li>
     <li>the pagesize as conceived by the user of this device, i.e. after the environment
     filters and device chain are through with the 'raw' page, and</li>
     <li>the surplus head and tail space required by the page filter chain</li>
     </ul>
    */
    ham_device_info_t _info;

    /**
     * Flags of this device.
     *
     * Currently, these flags are used (at least):
     * - @ref HAM_DISABLE_MMAP do not use mmap but pread/pwrite
     * - @ref DB_USE_MMAP use memory mapped I/O (this bit is not observed through here, though)
     * - @ref HAM_READ_ONLY this is a read-only device
     */
    ham_u32_t _flags;

    /**
     * some private data for this device
     */
    void *_private;

    /**
     * device graph links
     */
    ham_device_graph_connection_t in;
    ham_device_graph_connection_t out;

    /**
    The freelist cache: the freelist is managed by the device so it can be parallelized
    and/or managed per partition without having to feed a lot of unnecessary data into
    the @ref ham_backend_t database layer or @ref ham_env_t / @ref ham_db_t containers.
    */
    freelist_cache_t *_freelist_cache;
};


/*
 * get the allocator of this device
 */
#define device_get_allocator(dev)          (dev)->_malloc

/*
 * set the allocator of this device
 */
#define device_set_allocator(dev, a)       (dev)->_malloc = (a)

/*
* get the environment of this device
*/
#define device_get_env(dev)          (dev)->_env

/*
* set the environment of this device
*/
#define device_set_env(dev, e)          (dev)->_env = (e)

/*
 * get the flags of this device
 */
#define device_get_flags(dev)              (dev)->_flags

/*
 * set the flags of this device
 */
#define device_set_flags(dev, f)           (dev)->_flags=(f)

/*
 * get the private data of this device
 */
#define device_get_private(dev)            (dev)->_private

/*
 * set the private data of this device
 */
#define device_set_private(dev, p)         (dev)->_private=(p)

#define device_set_freelist_cache(dev, cache) (dev)->_freelist_cache=(cache)

#define device_get_freelist_cache(dev)     (dev)->_freelist_cache


/**
 * Create a new device structure of the specified type.
 * These types are known, though some may not (yet) be supported in
 * the implementation):
 *
 *<dl>
 *<dt>single file</dt>
    <dd>The classic single file HamsterDB database. The default.</dd>

  <dt>in-memory database</dt>
    <dd>The classic nonpersistent in-memory HamsterDB database. Will be
        instantiated when you specify the @ref HAM_IN_MEMORY_DB flag
        in @a flags and no custom device override has been specified
        in @a param.</dd>

  <dt>custom database storage device</dt>
    <dd>May be anything from Flash Memory to network storage. Will
        be instantiated when you specify the @ref HAM_PARAM_CUSTOM_DEVICE
        @a param parameter. The value must be function reference to
        a function which can create a @ref ham_device_t instance.

        These custom device generator functions are available:

        <ul>
          <li>@ref ham_device_mallocmem_new - identical to the 'in-memory database' type above.</li>

          <li>@ref ham_device_flatfile_new - identical to the 'single file' type above.</li>

          <li>@ref ham_device_flashfile_new - offers a basic 'flat memory' store, which
                                              is, for instance, suitable to address a
                                              Flash Memory chip in embedded environments.
          </li>
        </ul>

        See also @ref ham_parameter_function_t
    </dd>
  </dl>
 */
extern ham_device_t *
ham_device_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);


/* forward decl so struct can self-reference */
struct ham_device_invocation_t;
typedef struct ham_device_invocation_t ham_device_invocation_t;

/**
 Keeps track of which device invoked whom; this way any device can
 traverse up the call tree in a portable and deterministic fashion.
*/
struct ham_device_invocation_t
{
    ham_device_t *me;
    ham_device_invocation_t *parent;
};



ham_device_t *
ham_device_mallocmem_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);

ham_device_t *
ham_device_flatfile_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);

ham_device_t *
ham_device_flashfile_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_DEVICE_H__ */

/**
* @endcond
*/

