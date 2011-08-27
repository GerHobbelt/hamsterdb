/**
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


#include "config.h"

#include <string.h>

#include "db.h"
#include "device.h"
#include "error.h"
#include "mem.h"
#include "os.h"
#include "page.h"
#include "env.h"



static __inline void
__set_flags(ham_device_t *self, ham_u32_t flags)
{
    device_set_flags(self, flags);
}

static __inline ham_u32_t
__get_flags(ham_device_t *self)
{
    return (device_get_flags(self));
}





ham_device_t *
ham_device_new(ham_env_t *env, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    if (param)
    {
        for ( ; param->name; param++)
        {
            if (param->name == HAM_PARAM_CUSTOM_DEVICE)
            {
                ham_parameter_function_t *fn = param->value.fn;

                if (fn)
                {
                    return (ham_device_t *)(*fn)(env, db, flags, param);
                }
                else
                {
                    return NULL;
                }
            }
        }
    }

    if (flags & HAM_IN_MEMORY_DB)
    {
        return ham_device_mallocmem_new(env, db, flags, param);
    }
    else
    {
        return ham_device_flatfile_new(env, db, flags, param);
    }
}


/**
* @endcond
*/

