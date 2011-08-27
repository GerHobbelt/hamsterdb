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
Physical storage device similar to the 'malloc memory device', but here we store all
data in a single, fixed-sized, flat memory space, which can be mapped onto a flash device
or other memory-based persisted storage.
*/

#include "internal_preparation.h"




typedef struct 
{
    ham_u8_t *ptr;
    ham_offset_t alloc_size;
    ham_offset_t used;

    /* attribs */
    const char *fname;
    ham_u32_t flags;
    ham_u32_t mode;

    ham_offset_t curpos;
} dev_flashmem_t;



typedef struct
{
    /**
     An array of @ref dev_flashmem_t pointers; the array of pointers instead an array of instances is required 
     as the instances must stay pinned to their memory locations while the array may be redimensioned to accept
     more device instances alongside.
    */
    dev_flashmem_t **filelist;
    ham_size_t used;
    ham_size_t alloc;

    int refcount;
} dev_flashFS_t;


static dev_flashFS_t *filesystem = 0;


static ham_bool_t locate_fname_in_fs(const char *fname, ham_size_t *index)
{
    ham_size_t i;

    ham_assert(filesystem, (0));

    for (i = 0; i < filesystem->used; i++)
    {
        if (filesystem->filelist[i]->fname
            && !strcmp(filesystem->filelist[i]->fname, fname))
        {
            *index = i;
            return HAM_TRUE;
        }
    }

    *index = filesystem->used;
    return HAM_FALSE;
}

static ham_status_t assign_me_a_free_slot(ham_device_t *dev)
{
    dev_flashmem_t *t;
    ham_size_t i;
    mem_allocator_t *alloc;

    if (!dev)
        return HAM_IO_ERROR;

    alloc = device_get_allocator(dev);
    ham_assert(alloc != NULL, (0));

    t = NULL;
    device_set_private(dev, 0);
    if (filesystem->used < filesystem->alloc)
    {
        if (!filesystem->filelist[filesystem->used])
            filesystem->filelist[filesystem->used] = (dev_flashmem_t *)allocator_calloc(alloc, sizeof(*t));
        t = filesystem->filelist[filesystem->used];
        filesystem->used++;
        device_set_private(dev, t);
    }
    else
    {
        /* scan for empty slot */
        for (i = 0; i < filesystem->used; i++)
        {
            if (filesystem->filelist[i]->fname == NULL)
            {
                ham_assert(filesystem->filelist[i], (0));
                t = filesystem->filelist[i];
                device_set_private(dev, t);
                break;
            }
        }

        /* make sure there's at least one empty spot in there, always */
        if (!t)
        {
            ham_size_t count = filesystem->alloc;
            
            count++;
            count *= 3;
            count /= 2;

            filesystem->filelist = (dev_flashmem_t **)allocator_realloc(alloc, filesystem->filelist, count * sizeof(filesystem->filelist[0]));
            if (!filesystem->filelist)
                return HAM_OUT_OF_MEMORY;

            memset(filesystem->filelist + filesystem->alloc, 0, (count - filesystem->alloc) * sizeof(filesystem->filelist[0]));

            filesystem->alloc = count;

            if (!filesystem->filelist[filesystem->used])
                filesystem->filelist[filesystem->used] = (dev_flashmem_t *)allocator_calloc(alloc, sizeof(*t));
            t = filesystem->filelist[filesystem->used];
            filesystem->used++;
            device_set_private(dev, t);
        }
    }

    return (t ? HAM_SUCCESS : HAM_IO_ERROR); 
}

static ham_status_t 
__fm_create(ham_device_t *self, const char *fname, ham_u32_t flags, 
            ham_u32_t mode)
{
    ham_size_t file_index;
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    if (locate_fname_in_fs(fname, &file_index))
    {
        t = filesystem->filelist[file_index];
        if (t->ptr)
        {
            allocator_free(device_get_allocator(self), t->ptr);
            t->ptr = NULL;
        }
        if (t->fname)
        {
            allocator_free(device_get_allocator(self), t->fname);
            t->fname = NULL;
        }
        device_set_private(self, t);
    }

    device_set_flags(self, flags);

    t->alloc_size = 1024 * 1024;
    t->ptr = (ham_u8_t *)allocator_alloc(device_get_allocator(self), t->alloc_size);
    if (!t->ptr)
    {
        return HAM_IO_ERROR;
    }
    t->used = 0;
    t->fname = strdup(fname);
    if (!t->fname)
    {
        allocator_free(device_get_allocator(self), t->ptr);
        t->ptr = NULL;
        return HAM_IO_ERROR;
    }
    t->mode = mode;
    t->flags = flags;

    t->curpos = 0;

    return HAM_SUCCESS;
}

static ham_status_t 
__fm_open(ham_device_t *self, const char *fname, ham_u32_t flags)
{
    ham_size_t file_index;
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    if (locate_fname_in_fs(fname, &file_index))
    {
        t = filesystem->filelist[file_index];
        device_set_private(self, t);

        device_set_flags(self, flags);

        return HAM_SUCCESS;
    }

    return HAM_IO_ERROR;
}

static ham_status_t
__fm_close(ham_device_t *self)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    /* already closed before? */
    if (t->fname == NULL)
        return HAM_SUCCESS;

    /* no cleanup, just swap t for an empty one */
    return assign_me_a_free_slot(self);
}

static ham_status_t 
__fm_flush(ham_device_t *self)
{
    //dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    return HAM_SUCCESS;
}



static ham_status_t 
__fm_truncate(ham_device_t *self, ham_offset_t newsize)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    if (newsize <= t->alloc_size)
    {
        t->used = newsize;
    }
    else
    {
        ham_offset_t s = newsize;

        if (!t->ptr)
            return HAM_IO_ERROR;

        s += 1024 * 1024;
        s -= s % (1024 * 1024);
        t->ptr = (ham_u8_t *)allocator_realloc(device_get_allocator(self), t->ptr, s);
        if (!t->ptr)
            return HAM_IO_ERROR;
        t->alloc_size = s;
        t->used = newsize;
    }
    return HAM_SUCCESS;
}

static ham_bool_t 
__fm_is_open(ham_device_t *self)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);
    return (t->ptr != NULL);
}

static ham_size_t 
__fm_get_pagesize(ham_device_t *self)
{
    ham_size_t s;

    if (self && self->_info.raw_pagesize)
    {
        ham_assert(self->_info.raw_pagesize == self->_info.cooked_pagesize, (0));
        return self->_info.raw_pagesize;
    }

    s = 4096;
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
__fm_get_filesize(ham_device_t *self, ham_offset_t *length)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    *length = t->used;
    return HAM_SUCCESS;
}

static ham_status_t 
__fm_read(ham_device_t *self, ham_offset_t offset, 
        void *buffer, ham_offset_t size)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    ham_assert(size <= HAM_MAX_SIZE_T, ("no ham_size_t overflow"));
    
    if (offset > t->used || offset + size > t->used)
        return HAM_IO_ERROR;
    memcpy(buffer, t->ptr + offset, size);
    return HAM_SUCCESS;
}

static ham_status_t
__fm_read_page(ham_device_t *self, ham_page_t *page, ham_size_t size)
{
    ham_u8_t *buffer;
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);
    ham_env_t *env = device_get_env(self);
    ham_offset_t offset;

    ham_assert(page_get_owner(page) ? env == db_get_env(page_get_owner(page)) : 1, (0));

    offset = page_get_self(page);

    if (!size)
        size=self->get_pagesize(self);
    ham_assert(size, (0));

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
        {
            ham_assert(!(page_get_npers_flags(page)&PAGE_NPERS_MALLOC), (0));
        }

        return (__fm_read(self, page_get_self(page), 
                    page_get_pers(page), size));
    }

    ham_assert(page_get_pers(page)==0, (0));
    ham_assert(!(page_get_npers_flags(page)&PAGE_NPERS_MALLOC), (0));

    ham_assert(size <= HAM_MAX_SIZE_T, ("no ham_size_t overflow"));
    if (offset > t->used || offset + size > t->used)
        return HAM_IO_ERROR;
    //st=os_mmap(t->fd, page_get_mmap_handle_ptr(page), 
    //        page_get_self(page), size, 
    //        device_get_flags(self)&HAM_READ_ONLY, &buffer);
    page_set_pers(page, (ham_perm_page_union_t *)(t->ptr + offset));
    return HAM_SUCCESS;
}

static ham_status_t 
__fm_alloc(ham_device_t *self, ham_size_t size, ham_offset_t *address,
          dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_status_t st;

    st=__fm_get_filesize(self, address);
    if (st)
        return (st);
    st=__fm_truncate(self, (*address)+size);
    if (st)
        return (st);

    return HAM_SUCCESS;
}

static ham_status_t 
__fm_alloc_page(ham_device_t *self, ham_page_t *page, ham_size_t size,
               dev_alloc_request_info_ex_t *extra_dev_alloc_info)
{
    ham_status_t st;
    ham_offset_t pos;

    st=__fm_get_filesize(self, &pos);
    if (st)
        return (st);

    st=__fm_truncate(self, pos+size);
    if (st)
        return (st);

    page_set_self(page, pos);
    return __fm_read_page(self, page, size);
}


/**
 * seek position in a file
 */
static ham_status_t 
__fm_seek(ham_device_t *self, ham_offset_t offset, int whence)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    switch (whence)
    {
    default:
        return HAM_IO_ERROR;

    case HAM_OS_SEEK_SET:
        if (offset > t->used)
            return HAM_IO_ERROR;

        t->curpos = offset;
        break;

    case HAM_OS_SEEK_CUR:
        if (offset > t->used - t->curpos)
            return HAM_IO_ERROR;

        t->curpos += offset;
        break;

    case HAM_OS_SEEK_END:
        if (offset > t->used)
            return HAM_IO_ERROR;

        t->curpos = t->used - offset;
        break;
    }
    return HAM_SUCCESS;
}

/**
 * tell the position in a file
 */
static ham_status_t 
__fm_tell(ham_device_t *self, ham_offset_t *offset)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    *offset = t->curpos;
    return HAM_SUCCESS;
}

static ham_status_t 
__fm_write(ham_device_t *self, ham_offset_t offset, void *buffer, 
            ham_offset_t size)
{
    dev_flashmem_t *t=(dev_flashmem_t *)device_get_private(self);

    ham_assert(size <= HAM_MAX_SIZE_T, ("no ham_size_t overflow"));
    
    if (offset > t->used)
        return HAM_IO_ERROR;
    if (offset + size > t->used)
    {
        /* enlarge file */
        ham_offset_t endlen = offset + size;

        if (endlen <= t->alloc_size)
        {
            t->used = endlen;
        }
        else
        {
            /* resize */
            ham_status_t st = __fm_truncate(self, endlen);

            if (st)
                return st;
        }
    }
    
    memcpy(t->ptr + offset, buffer, size);

    return HAM_SUCCESS;
}

static ham_status_t 
__fm_write_page(ham_device_t *self, ham_page_t *page)
{
    return __fm_write(self, page_get_self(page), 
                page_get_pers(page), self->get_pagesize(self));
}

static ham_status_t 
__fm_free_page(ham_device_t *self, ham_page_t *page)
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
            st = 0;
        }
    }

    page_set_pers(page, 0);
    return HAM_SUCCESS;
}

static ham_status_t 
__fm_destroy(ham_device_t **self_ref)
{
    ham_assert(self_ref, (0));
    if (*self_ref)
    {
        ham_device_t *self = *self_ref;
        dev_flashmem_t *t = (dev_flashmem_t *)device_get_private(self);

        ham_assert(!__fm_is_open(self), ("destroying a device which is open"));

        if (t->ptr)
        {
            allocator_free(device_get_allocator(self), t->ptr);
            t->ptr = NULL;
        }
        if (t->fname)
        {
            allocator_free(device_get_allocator(self), t->fname);
            t->fname = NULL;
        }

        /* now see whether the 'filesystem' should remain active */
        ham_assert(filesystem->refcount > 0, (0));
        filesystem->refcount--;

        if (0 == filesystem->refcount)
        {
            /* delete the entire filesystem now */
            ham_size_t i;

            for (i = 0; i < filesystem->used; i++)
            {
                ham_assert(filesystem->filelist[i], (0));
                allocator_free(device_get_allocator(self), filesystem->filelist[i]);
            }
            allocator_free(device_get_allocator(self), filesystem->filelist);
            allocator_free(device_get_allocator(self), filesystem);
            filesystem = NULL;
        }

        //allocator_free(device_get_allocator(self), device_get_private(self));
        allocator_free(device_get_allocator(self), self);
        *self_ref = NULL;
    }

    return HAM_SUCCESS;
}


static void 
__fm_set_flags(ham_device_t *self, ham_u32_t flags)
{
    device_set_flags(self, flags);
}

static ham_u32_t 
__fm_get_flags(ham_device_t *self)
{
    return device_get_flags(self);
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
__fm_add_outgoing(ham_device_t *self, ham_u32_t partition_index, ham_device_t *target)
{
    return HAM_SUCCESS;
}

/**
 * register another @a source device; adds an incoming edge to the device 
 * graph of which the current device is a part.
 */
static ham_status_t 
__fm_add_incoming(ham_device_t *self, ham_device_t *source)
{
    return HAM_SUCCESS;
}



/**
 Retrieve device class info, i.e. info which is common for all device instances of this device type.

 @note The @ref ham_device_class_info_t @a info struct must be initialized before invoking this
 method, preferably with all zeroes.
 */
static ham_status_t 
__fm_get_device_class_info(ham_device_t *self, ham_device_class_info_t *pinfo)
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
 method, preferably with all zeroes.
 */
static ham_status_t 
__fm_get_device_info(ham_device_t *self, ham_device_info_t *pinfo, ham_bool_t inclusive)
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
__fm_set_pagesize(ham_device_t *self, ham_size_t pagesize)
{
    ham_assert(self, (0));
    self->_info.raw_pagesize = pagesize;
    self->_info.cooked_pagesize = pagesize;

    return HAM_SUCCESS;
}





ham_device_t *
ham_device_flashfile_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    mem_allocator_t *alloc = env_get_allocator(env);
    ham_device_t *dev = (ham_device_t *)allocator_calloc(alloc, sizeof(*dev));
    if (!dev)
        return NULL;

    device_set_allocator(dev, alloc);
    device_set_env(dev, env);

    if (!filesystem)
    {
        filesystem = (dev_flashFS_t *)allocator_calloc(alloc, sizeof(*filesystem));
        if (!filesystem)
            return NULL;
    }
    if (!filesystem->filelist)
    {
        ham_size_t count = 3;

        filesystem->filelist = (dev_flashmem_t **)allocator_calloc(alloc, count * sizeof(filesystem->filelist[0]));
        if (!filesystem->filelist)
            return NULL;
        filesystem->alloc = count;
        //filesystem->used = 0;
        filesystem->refcount = 1;
    }
    else
    {
        filesystem->refcount++;
    }

    //t = (dev_flashmem_t *)allocator_calloc(alloc, sizeof(*t));
    //if (!t)
    //  return (0);
    //t->ptr = NULL;
    //device_set_private(dev, t);

    /*
    always assign a flashmem item to the device; at create/open time this may be swapped for another. 
    
    simplifies the other callbacks: no need to check for NULL pointer
    */
    if (HAM_SUCCESS != assign_me_a_free_slot(dev))
        return NULL;

    dev->create       = __fm_create;
    dev->open         = __fm_open;
    dev->close        = __fm_close;
    dev->flush        = __fm_flush;
    dev->truncate     = __fm_truncate;
    dev->is_open      = __fm_is_open;
    dev->get_pagesize = __fm_get_pagesize;
    dev->get_filesize = __fm_get_filesize;
    dev->set_flags    = __fm_set_flags;
    dev->get_flags    = __fm_get_flags;
    dev->alloc        = __fm_alloc;
    dev->alloc_page   = __fm_alloc_page;
    dev->seek         = __fm_seek;
    dev->tell         = __fm_tell;
    dev->read         = __fm_read;
    dev->write        = __fm_write;
    dev->read_page    = __fm_read_page;
    dev->write_page   = __fm_write_page;
    dev->free_page    = __fm_free_page;
    dev->destroy      = __fm_destroy;
    dev->add_incoming = __fm_add_incoming;
    dev->add_outgoing = __fm_add_outgoing;
    dev->get_device_info = __fm_get_device_info;
    dev->get_device_class_info = __fm_get_device_class_info;
    dev->set_pagesize = __fm_set_pagesize;

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

