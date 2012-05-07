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

#include "../src/mem.h"

typedef unsigned int  memdesc_magic_edge_t;

typedef struct memdesc_t
{
    const char *file;
    int line;
    int size;
    struct memdesc_t *next;
    struct memdesc_t *previous;
    memdesc_magic_edge_t magic_start;

    union
    {
        double d;
        void *p;
        long l;
    } data[1]; /* ensure regular malloc alignment: this is the 'biggest' type we'll need for that */
} memdesc_t;

typedef struct
{
    memdesc_t *header;
    unsigned long total;
    int instance_counter;
} memtracker_priv_t;


extern mem_allocator_t *
memtracker_new(void);

extern unsigned long
memtracker_get_leaks(mem_allocator_t *mt);


