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

#include "../src/config.h"

#include "../src/env.h"

#include "memtracker.h"

#include <stdexcept>
#include <cstdlib>
#include <cstring>




#define MAGIC_START 0x12345678u

#define MAGIC_STOP  0x98765432u


static memdesc_t *
get_descriptor(const void *p)
{
    return (memdesc_t *)(static_cast<ham_u8_t *>(const_cast<void *>(p))-OFFSETOF(memdesc_t, data));
}

static void
verify_mem_desc(memdesc_t *desc)
{
    memdesc_magic_edge_t magic=MAGIC_STOP;
    if (desc->size==0)
        throw std::out_of_range("memory blob size is 0");
    if (desc->magic_start!=MAGIC_START)
        throw std::out_of_range("memory blob descriptor is corrupt");
    memcpy(&magic, ((ham_u8_t *)(&desc->data[0]))+desc->size, sizeof(magic));
    if (magic!=MAGIC_STOP)
        throw std::out_of_range("memory blob was corrupted after end");
}

static void *
alloc_impl(mem_allocator_t *self, const char *file, int line, ham_u32_t size)
{
    memdesc_magic_edge_t magic=MAGIC_STOP;
    memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);
#if defined(_CRTDBG_MAP_ALLOC)
    memdesc_t *desc=(memdesc_t *)_malloc_dbg(OFFSETOF(memdesc_t, data)+size+sizeof(magic),
            _NORMAL_BLOCK, file, line);
#else
    memdesc_t *desc=(memdesc_t *)malloc(OFFSETOF(memdesc_t, data)+size+sizeof(int));
#endif
    if (!desc)
        return (0);
    memset(desc, 0, OFFSETOF(memdesc_t, data));
#if 0
#if defined(_CRTDBG_MAP_ALLOC)
    _CrtCheckMemory();
#endif
#endif
    desc->file=file;
    desc->line=line;
    desc->size=size;
    desc->magic_start=MAGIC_START;
    /* avoid unaligned access on sparc */
    memcpy(((ham_u8_t *)(&desc->data[0]))+size, &magic, sizeof(magic));
#if 0
#if defined(_CRTDBG_MAP_ALLOC)
    _CrtCheckMemory();
#endif
#endif

    desc->next=priv->header;
    if (priv->header)
        priv->header->previous=desc;
    priv->header=desc;
    ham_assert(priv->header->previous == NULL, (0));
    priv->total+=size;

    return desc->data;
}

static void
free_impl(mem_allocator_t *self, const char *file, int line, const void *ptr)
{
    memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);
    memdesc_t *desc, *p, *n;

    if (!ptr)
        throw std::logic_error("tried to free a null-pointer");

    desc=get_descriptor(ptr);
    verify_mem_desc(desc);

    p=desc->previous;
    n=desc->next;
    if (p) {
        p->next=n;
    }
    if (n) {
        n->previous=p;
    }
    ham_assert(priv->header->previous == NULL, (0));
    if (priv->header==desc) {
        priv->header=n;
    }
    ham_assert(priv->header ? priv->header->previous == NULL : 1, (0));

    priv->total-=desc->size;
    ham_assert(!priv->header ? priv->total == 0 : 1, (0));

#if defined(_CRTDBG_MAP_ALLOC)
    _free_dbg(desc, _NORMAL_BLOCK);
#else
    free(desc);
#endif
}

static void *
realloc_impl(mem_allocator_t *self, const char *file, int line,
        const void *ptr, ham_size_t size)
{
    memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);
    void *newptr=_allocator_alloc(self, file, line, size);
    memdesc_t *desc;

    if (!newptr) {
        allocator_free(self, ptr);
        return (0);
    }

    if (!ptr)
        return (newptr);

    ham_assert(priv->header, (0));
    ham_assert(priv->header->previous == NULL, (0));
    desc=get_descriptor(ptr);

    if (size > (ham_size_t)desc->size)
        size = desc->size;
    memcpy(newptr, ptr, size);
    allocator_free(self, ptr);
    ham_assert(priv->header, (0));
    ham_assert(priv->header->previous == NULL, (0));
    return (newptr);
}

static void
destroy_impl(mem_allocator_t **self_ref)
{
    ham_assert(self_ref, (0));
    if (*self_ref)
    {
        mem_allocator_t *self = *self_ref;
        memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);

        ham_assert(priv->header ? priv->header->previous == NULL : 1, (0));
        priv->instance_counter--;
        ham_assert(priv->instance_counter >= 0, ("HamsterDB MUST balance the memory allocator instantiations vs. destructions"));

#if defined(_CRTDBG_MAP_ALLOC)
        _free_dbg(self, _NORMAL_BLOCK);
#else
        free(self);
#endif
        *self_ref = NULL;
    }
}

static mem_allocator_t *
instantiate_impl(mem_allocator_t *self, const char *fname, const int lineno)
{
    memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);
    mem_allocator_t *m;

    m=(mem_allocator_t *)
#if defined(_CRTDBG_MAP_ALLOC)
        _calloc_dbg(1, sizeof(*m), _NORMAL_BLOCK, fname, lineno);
#else
        calloc(1, sizeof(*m));
#endif
    if (!m)
        return (0);

    *m = *self;
    priv->instance_counter++;

    return m;
}

static void
validate_heap_impl(mem_allocator_t *self)
{
    memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);
    memdesc_t *desc;

    ham_assert(priv->header ? priv->header->previous == NULL : 1, (0));
    for (desc = priv->header; desc; desc = desc->next)
    {
        verify_mem_desc(desc);
    }

#if defined(_CRTDBG_MAP_ALLOC)
    _CrtCheckMemory();
#endif
}



mem_allocator_t *
memtracker_new(void)
{
    static mem_allocator_t m;
    static memtracker_priv_t p;

    memset(&m, 0, sizeof(m));
    memset(&p, 0, sizeof(p));
    m.alloc=alloc_impl;
    m.free =free_impl;
    m.realloc=realloc_impl;
    m.destroy=destroy_impl;
    m.instantiate=instantiate_impl;
    m.check_integrity=validate_heap_impl;
    m.priv=&p;

    return &m;
}

unsigned long
memtracker_get_leaks(mem_allocator_t *self)
{
    memtracker_priv_t *priv = static_cast<memtracker_priv_t *>(self->priv);

    ham_assert(priv->header ? priv->header->previous == NULL : 1, (0));
    self->check_integrity(self);

#if 0
    return (priv->total);
#else
    return 0;
#endif
}


