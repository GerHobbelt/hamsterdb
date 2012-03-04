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
@brief Classic physical storage device which stores data in a single ('flat') file.
*/

#include "internal_preparation.h"



typedef struct
{
    ham_fd_t fd;
    unsigned mmap_tested: 1;
} dev_file_t;

static ham_status_t
__f_create(ham_device_t *self, const char *fname, ham_u32_t flags,
            ham_u32_t mode)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);

    device_set_flags(self, flags);
    t->mmap_tested = 0;

    return (os_create(fname, flags, mode, &t->fd));
}

static ham_status_t
__f_open(ham_device_t *self, const char *fname, ham_u32_t flags)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);

    device_set_flags(self, flags);
    t->mmap_tested = 0;

    return (os_open(fname, flags, &t->fd));
}

static ham_status_t
__f_close(ham_device_t *self)
{
    ham_status_t st;
    dev_file_t *t=(dev_file_t *)device_get_private(self);
    t->mmap_tested = 0;

    /* already closed before? */
    if (t->fd == HAM_INVALID_FD)
        return HAM_SUCCESS;

    st=os_close(t->fd, device_get_flags(self));
    if (st == HAM_SUCCESS)
        t->fd = HAM_INVALID_FD;
    return (st);
}

static ham_status_t
__f_flush(ham_device_t *self)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);

    return (os_flush(t->fd));
}



static ham_status_t
__f_truncate(ham_device_t *self, ham_offset_t newsize)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);

    return (os_truncate(t->fd, newsize));
}

static ham_bool_t
__f_is_open(ham_device_t *self)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);
    return (HAM_INVALID_FD!=t->fd);
}

static ham_size_t
__f_get_pagesize(ham_device_t *self)
{
    ham_size_t s;

    if (self && self->_info.raw_pagesize)
    {
        ham_assert(self->_info.raw_pagesize == self->_info.cooked_pagesize, (0));
        return self->_info.raw_pagesize;
    }

    s = os_get_pagesize();
    if (self)
    {
        self->_info.raw_pagesize = s;
        self->_info.cooked_pagesize = s;
    }

    return s;
}

/**
 * get the size of the database file
 */
static ham_status_t
__f_get_filesize(ham_device_t *self, ham_offset_t *length)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);

    *length = 0;
    return os_get_filesize(t->fd, length);
}

static ham_status_t
__f_read(ham_device_t *self, ham_offset_t offset,
        void *buffer, ham_offset_t size)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);
    ham_status_t st;

    ham_assert(size <= HAM_MAX_SIZE_T, ("no ham_size_t overflow"));
    st=os_pread(t->fd, offset, buffer, size);
    if (st)
        return st;

    /*
     * we're done
     */
    return HAM_SUCCESS;
}

static ham_status_t
__f_read_page(ham_device_t *self, ham_page_t *page, ham_size_t size)
{
    ham_u8_t *buffer;
    ham_status_t st;
    dev_file_t *t=(dev_file_t *)device_get_private(self);
    ham_env_t *env = device_get_env(self);
    
    ham_assert(page_get_owner(page) ? env == db_get_env(page_get_owner(page)) : 1, (0));

    if (!size)
        size=self->get_pagesize(self);
    ham_assert(size, (0));

retry_with_classic_rw:
    if (device_get_flags(self)&HAM_DISABLE_MMAP)
    {
        if (page_get_pers(page)==0)
        {
            ham_assert(size <= HAM_MAX_SIZE_T, ("ham_size_t overflow"));
            buffer=(ham_u8_t *)allocator_alloc(device_get_allocator(self), size);
            if (!buffer)
                return (HAM_OUT_OF_MEMORY);
            page_set_pers(page, (ham_perm_page_union_t *)buffer);
            page_add_npers_flags(page, PAGE_NPERS_MALLOC);
        }
        else
            ham_assert(!(page_get_npers_flags(page)&PAGE_NPERS_MALLOC), (0));

        return (__f_read(self, page_get_self(page),
                    page_get_pers(page), size));
    }

    ham_assert(page_get_pers(page)==0, (0));
    ham_assert(!(page_get_npers_flags(page)&PAGE_NPERS_MALLOC), (0));

    ham_assert(size <= HAM_MAX_SIZE_T, ("no ham_size_t overflow"));
    /*
    Win32 will only b0rk on small page sizes when the SECOND page is fetched, i.e. when the
    PAGE OFFSET is NOT PRPERLY ALIGNED.

    But /we/ need to know by the time we're fetching page 0, so we tweak the behaviour a
    little here by doing some chking of our own and fake an error when we recognize an error
    condition lurking in the shadows there...

    As we try to load an entire page each time, we can check this one against the actual
    granularity.
    */
    if (!t->mmap_tested && size == os_get_pagesize() && (size % os_get_granularity() != 0))
    {
        st = HAM_NOT_IMPLEMENTED;
        buffer = NULL; // shut up MSVC compiler code flow analysis warning
    }
    else
    {
        st=os_mmap(t->fd, page_get_mmap_handle_ptr(page),
                   page_get_self(page), size,
                   device_get_flags(self)&HAM_READ_ONLY, &buffer);
    }
    if (st)
    {
        /*
        When the filesystem / platform does not support memory mapping for this file,
        fall back to 'classic' read/write I/O:
        */
        if (!t->mmap_tested && st == HAM_NOT_IMPLEMENTED)
        {
            device_set_flags(self, device_get_flags(self) | HAM_DISABLE_MMAP);
            env_modify_rt_flags(env, HAM_DISABLE_MMAP, 0);
            t->mmap_tested = HAM_TRUE;
            goto retry_with_classic_rw;
        }
        t->mmap_tested = HAM_TRUE;
        return (st);
    }
    t->mmap_tested = HAM_TRUE;

    page_set_pers(page, (ham_perm_page_union_t *)buffer);
    return HAM_SUCCESS;
}

static ham_status_t
__f_alloc(ham_device_t *self, ham_size_t size, ham_offset_t *address,
          dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_status_t st;

    st=__f_get_filesize(self, address);
    if (st)
        return (st);
    st=__f_truncate(self, (*address)+size);
    if (st)
        return (st);

    return HAM_SUCCESS;
}

static ham_status_t
__f_alloc_page(ham_device_t *self, ham_page_t *page, ham_size_t size,
               dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_status_t st;
    ham_offset_t pos;

    st=__f_get_filesize(self, &pos);
    if (st)
        return (st);

    st=__f_truncate(self, pos+size);
    if (st)
        return (st);

    page_set_self(page, pos);
    return (__f_read_page(self, page, size));
}


/**
 * seek position in a file
 */
static ham_status_t
__f_seek(ham_device_t *self, ham_offset_t offset, int whence)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);

    return os_seek(t->fd, offset, whence);
}

/**
 * tell the position in a file
 */
static ham_status_t
__f_tell(ham_device_t *self, ham_offset_t *offset)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);
    return os_tell(t->fd, offset);
}

static ham_status_t
__f_write(ham_device_t *self, ham_offset_t offset, void *buffer,
            ham_offset_t size)
{
    dev_file_t *t=(dev_file_t *)device_get_private(self);
    
    /*
     * run page through page-level filters
     */
    ham_assert(size <= HAM_MAX_SIZE_T, ("no ham_size_t overflow"));
    return (os_pwrite(t->fd, offset, buffer, size));
}

static ham_status_t
__f_write_page(ham_device_t *self, ham_page_t *page)
{
    return (__f_write(self, page_get_self(page),
                page_get_pers(page), self->get_pagesize(self)));
}

static ham_status_t
__f_free_page(ham_device_t *self, ham_page_t *page)
{
    ham_status_t st;

    if (page_get_pers(page))
    {
        if (page_get_npers_flags(page) & PAGE_NPERS_MALLOC)
        {
            allocator_free(device_get_allocator(self), page_get_pers(page));
            page_remove_npers_flags(page, PAGE_NPERS_MALLOC);
        }
        else
        {
            st = os_munmap(page_get_mmap_handle_ptr(page),
                    page_get_pers(page), self->get_pagesize(self));
            if (st)
                return (st);
        }
    }

    page_set_pers(page, 0);
    return (HAM_SUCCESS);
}

static ham_status_t
__f_destroy(ham_device_t **self_ref)
{
    ham_assert(self_ref, (0));
    if (*self_ref)
    {
        ham_device_t *self = *self_ref;

        ham_assert(!__f_is_open(self), ("destroying a device which is open"));

        allocator_free(device_get_allocator(self), device_get_private(self));
        allocator_free(device_get_allocator(self), self);
        *self_ref = NULL;
    }
    return (HAM_SUCCESS);
}


static void
__f_set_flags(ham_device_t *self, ham_u32_t flags)
{
    device_set_flags(self, flags);
}

static ham_u32_t
__f_get_flags(ham_device_t *self)
{
    return (device_get_flags(self));
}



/**
 * register another @a target device; adds an outgoing edge to the device
 * graph of which the current device is a part.
 *
 * @param self the current device instance (this function/method is part of this one)
 * @param partition_index is zero or higher and indicates which partition must be forwarded
 *        to the specified device. For non-partitioning devices, the @a partition_index
 *        should be set to zero(0).
 * @param target the device instance to add to the device graph, where the current
 *        device (@a self) has an ougoing edge towards this @a target device.
 */
static ham_status_t
__f_add_outgoing(ham_device_t *self, ham_u32_t partition_index, ham_device_t *target)
{
    return HAM_SUCCESS;
}

/**
 * register another @a source device; adds an incoming edge to the device
 * graph of which the current device is a part.
 */
static ham_status_t
__f_add_incoming(ham_device_t *self, ham_device_t *source)
{
    return HAM_SUCCESS;
}


/**
 Retrieve device class info, i.e. info which is common for all device instances of this device type.

 @note The @ref ham_device_class_info_t @a info struct must be initialized before invoking this
 method, preferably with all zeroes.
 */
static ham_status_t
__f_get_device_class_info(ham_device_t *self, ham_device_class_info_t *pinfo)
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
__f_get_device_info(ham_device_t *self, ham_device_info_t *pinfo, ham_bool_t inclusive)
{
    if (pinfo)
    {
        memcpy(pinfo, &self->_info, sizeof(*pinfo));
    }
    return HAM_SUCCESS;
}


/**
 * set the pagesize for this device
 */
static ham_status_t
__f_set_pagesize(ham_device_t *self, ham_size_t pagesize)
{
    ham_assert(self, (0));
    self->_info.raw_pagesize = pagesize;
    self->_info.cooked_pagesize = pagesize;

    return HAM_SUCCESS;
}





ham_device_t *
ham_device_flatfile_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    mem_allocator_t *alloc = env_get_allocator(env);
    dev_file_t *t;
    ham_device_t *dev = (ham_device_t *)allocator_calloc(alloc, sizeof(*dev));
    if (!dev)
        return NULL;

    device_set_allocator(dev, alloc);
    device_set_env(dev, env);

    t = (dev_file_t *)allocator_alloc(alloc, sizeof(*t));
    if (!t)
        return NULL;
    t->fd = HAM_INVALID_FD;
    device_set_private(dev, t);

    dev->create       = __f_create;
    dev->open         = __f_open;
    dev->close        = __f_close;
    dev->flush        = __f_flush;
    dev->truncate     = __f_truncate;
    dev->is_open      = __f_is_open;
    dev->get_pagesize = __f_get_pagesize;
    dev->get_filesize = __f_get_filesize;
    dev->set_flags    = __f_set_flags;
    dev->get_flags    = __f_get_flags;
    dev->alloc        = __f_alloc;
    dev->alloc_page   = __f_alloc_page;
    dev->seek         = __f_seek;
    dev->tell         = __f_tell;
    dev->read         = __f_read;
    dev->write        = __f_write;
    dev->read_page    = __f_read_page;
    dev->write_page   = __f_write_page;
    dev->free_page    = __f_free_page;
    dev->destroy      = __f_destroy;
    dev->add_incoming = __f_add_incoming;
    dev->add_outgoing = __f_add_outgoing;
    dev->get_device_class_info = __f_get_device_class_info;
    dev->get_device_info = __f_get_device_info;
    dev->set_pagesize = __f_set_pagesize;

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

