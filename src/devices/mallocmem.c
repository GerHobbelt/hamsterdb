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
@brief A 'physical' storage device which stores the data in heap-allocated memory
*/

#include "internal_preparation.h"



typedef struct 
{
    ham_bool_t is_open;
} dev_inmem_t;



static ham_status_t 
__m_create(ham_device_t *self, const char *fname, ham_u32_t flags, 
            ham_u32_t mode)
{
    dev_inmem_t *t=(dev_inmem_t *)device_get_private(self);

    (void)fname;
    (void)flags;
    (void)mode;

    ham_assert(!t->is_open, (0));
    t->is_open=HAM_TRUE;
    return (0);
}

static ham_status_t 
__m_open(ham_device_t *self, const char *fname, ham_u32_t flags)
{
    (void)fname;
    (void)flags;
    ham_assert(!"can't open an in-memory-device", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t
__m_close(ham_device_t *self)
{
    dev_inmem_t *t=(dev_inmem_t *)device_get_private(self);
    ham_assert(t->is_open, (0));
    t->is_open=HAM_FALSE;
    return (HAM_SUCCESS);
}

static ham_status_t 
__m_flush(ham_device_t *self)
{
    (void)self;
    return (HAM_SUCCESS);
}

static ham_status_t 
__m_truncate(ham_device_t *self, ham_offset_t newsize)
{
    (void)self;
    (void)newsize;
    return (HAM_SUCCESS);
}

static ham_bool_t 
__m_is_open(ham_device_t *self)
{
    dev_inmem_t *t=(dev_inmem_t *)device_get_private(self);
    return (t->is_open);
}

static ham_size_t 
__m_get_pagesize(ham_device_t *self)
{
    ham_size_t s;

    if (self && self->_info.raw_pagesize)
    {
        ham_assert(self->_info.raw_pagesize == self->_info.cooked_pagesize, (0));
        return self->_info.raw_pagesize;
    }

    s = (1024*4);
    if (self)
    {
        self->_info.raw_pagesize = s;
        self->_info.cooked_pagesize = s;
    }
    return s;
}

static ham_status_t 
__m_get_filesize(ham_device_t *self, ham_offset_t *length)
{
    (void)self;
    ham_assert(!"can't request the filesize from an in-memory-device", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t 
__m_alloc(ham_device_t *self, ham_size_t size, ham_offset_t *address,
          dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    (void)self;
    (void)size;
    (void)address;
    ham_assert(!"can't alloc from an in-memory-device", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t 
__m_alloc_page(ham_device_t *self, ham_page_t *page, ham_size_t size,
               dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_u8_t *buffer;

    ham_assert(page_get_pers(page)==0, (0));

    ham_assert(size <= HAM_MAX_SIZE_T, ("ham_size_t overflow"));
    buffer=(ham_u8_t *)allocator_alloc(device_get_allocator(self), size);
    if (!buffer)
        return (HAM_OUT_OF_MEMORY);
    page_set_pers(page, (ham_perm_page_union_t *)buffer);
    page_add_npers_flags(page, PAGE_NPERS_MALLOC);
    page_set_self(page, (ham_offset_t)buffer);

    return (HAM_SUCCESS);
}

/**
 * seek position in a file
 */
static ham_status_t 
__m_seek(ham_device_t *self, ham_offset_t offset, int whence)
{
    ham_assert(!"can't seek an in-memory-device", (0));
    return (HAM_NOT_IMPLEMENTED);
}

/**
 * tell the position in a file
 */
static ham_status_t 
__m_tell(ham_device_t *self, ham_offset_t *offset)
{
    ham_assert(!"can't seek/tell an in-memory-device", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t 
__m_read(ham_device_t *self, ham_offset_t offset, 
        void *buffer, ham_offset_t size)
{
    (void)self;
    (void)offset;
    (void)buffer;
    (void)size;
    ham_assert(!"this operation is not possible for in-memory-databases", (0));
    return (HAM_NOT_IMPLEMENTED);
}


static ham_status_t 
__m_write(ham_device_t *self, ham_offset_t offset, void *buffer, 
            ham_offset_t size)
{
    (void)self;
    (void)offset;
    (void)buffer;
    (void)size;
    ham_assert(!"this operation is not possible for in-memory-databases", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t
__m_read_page(ham_device_t *self, ham_page_t *page, ham_size_t size)
{
    (void)self;
    (void)page;
    ham_assert(!"this operation is not possible for in-memory-databases", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t 
__m_write_page(ham_device_t *self, ham_page_t *page)
{
    (void)self;
    (void)page;
    ham_assert(!"this operation is not possible for in-memory-databases", (0));
    return (HAM_NOT_IMPLEMENTED);
}

static ham_status_t 
__m_free_page(ham_device_t *self, ham_page_t *page)
{
    ham_assert(page_get_pers(page)!=0, (0));
    ham_assert(page_get_npers_flags(page) & PAGE_NPERS_MALLOC, (0));

    allocator_free(device_get_allocator(self), page_get_pers(page));
    page_set_pers(page, 0);
    page_remove_npers_flags(page, PAGE_NPERS_MALLOC);

    return (HAM_SUCCESS);
}

static ham_status_t 
__m_destroy(ham_device_t **self_ref)
{
    ham_assert(self_ref, (0));
    if (*self_ref)
    {
        ham_device_t *self = *self_ref;

        ham_assert(!__m_is_open(self), ("destroying a device which is open"));

        allocator_free(device_get_allocator(self), device_get_private(self));
        allocator_free(device_get_allocator(self), self);
        *self_ref = NULL;
    }
    return (HAM_SUCCESS);
}

static void 
__set_flags(ham_device_t *self, ham_u32_t flags)
{
    device_set_flags(self, flags);
}

static ham_u32_t 
__get_flags(ham_device_t *self)
{
    return (device_get_flags(self));
}

/**
 * register another @a target device; adds an outgoing edge to the device 
 * graph of which the current device is a part.
 *
 * @param partition_index is zero or higher and indicates which partition must be forwarded
 *        to the specified device. For non-partitioning devices, the @a partition_index
 *        should be set to zero(0).
 */
static ham_status_t 
__m_add_outgoing(ham_device_t *self, ham_u32_t partition_index, ham_device_t *target)
{
    return 0;
}

/**
 * register another @a source device; adds an incoming edge to the device 
 * graph of which the current device is a part.
 */
static ham_status_t 
__m_add_incoming(ham_device_t *self, ham_device_t *source)
{
    return 0;
}


/**
 Retrieve device class info, i.e. info which is common for all device instances of this device type.

 @note The @ref ham_device_class_info_t @a info struct must be initialized before invoking this
 method, preferably with all zeroes.
 */
static ham_status_t 
__m_get_device_class_info(ham_device_t *self, ham_device_class_info_t *pinfo)
{
    if (pinfo)
    {
        memcpy(pinfo, self->_class_info, sizeof(*pinfo));
    }
    return HAM_SUCCESS;
}


/**
 Retrieve the device info (with or without the underlying devices taken into account:
 when @a inclusive is set to 'true', the device graph below the specified device will
 be traversed to determine the aggregated info values).

 @note The @ref ham_device_info_t @a info struct must be initialized before invoking this
 method, preferrably with all zeroes.
 */
static ham_status_t 
__m_get_device_info(ham_device_t *self, ham_device_info_t *pinfo, ham_bool_t inclusive)
{
    if (pinfo)
    {
        memcpy(pinfo, &self->_info, sizeof(*pinfo));
    }
    return 0;
}


/**
 * set the pagesize for this device
 */
static ham_status_t 
__m_set_pagesize(ham_device_t *self, ham_size_t pagesize)
{
    if (self)
    {
        self->_info.raw_pagesize = pagesize;
        self->_info.cooked_pagesize = pagesize;
    }
    return 0;
}




ham_device_t *
ham_device_mallocmem_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    mem_allocator_t *alloc = env_get_allocator(env);
    dev_inmem_t *t;
    ham_device_t *dev = (ham_device_t *)allocator_calloc(alloc, sizeof(*dev));
    if (!dev)
        return (0);

    device_set_allocator(dev, alloc);
    device_set_env(dev, env);

    t = (dev_inmem_t *)allocator_alloc(alloc, sizeof(*t));
    if (!t)
        return (0);
    t->is_open = 0;
    device_set_private(dev, t);

    dev->create       = __m_create;
    dev->open         = __m_open;
    dev->close        = __m_close;
    dev->flush        = __m_flush;
    dev->truncate     = __m_truncate;
    dev->is_open      = __m_is_open;
    dev->get_pagesize = __m_get_pagesize;
    dev->get_filesize = __m_get_filesize;
    dev->set_flags    = __set_flags;
    dev->get_flags    = __get_flags;
    dev->alloc        = __m_alloc;
    dev->alloc_page   = __m_alloc_page;
    dev->seek         = __m_seek;
    dev->tell         = __m_tell;
    dev->read         = __m_read;
    dev->write        = __m_write;
    dev->read_page    = __m_read_page;
    dev->write_page   = __m_write_page;
    dev->free_page    = __m_free_page;
    dev->destroy      = __m_destroy;
    dev->add_incoming = __m_add_incoming;
    dev->add_outgoing = __m_add_outgoing;
    dev->get_device_class_info = __m_get_device_class_info;
    dev->get_device_info = __m_get_device_info;
    dev->set_pagesize = __m_set_pagesize;

    /*
     * initialize the pagesize with a default value - this will be
     * overwritten i.e. by ham_open, ham_create when the pagesize 
     * of the file is known
     */
    (void)dev->get_pagesize(dev);

    return (dev);
}


/**
* @endcond 
*/

