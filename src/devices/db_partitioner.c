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
@brief A basic partitioner, which segments the 'rid' ID space into multiple partitions based on the
database ID/name.

This partitioner enables the parallel use of multiple databases within a single environment
as this partitioner does away with the freelist bottleneck: now, each database (db)
has its own freelist, while a classic (single file) hamster env/db would only have a
single freelist per environment, i.e. the freelist would be SHARED among multiple databases
in he 'classic' case.

The only thing SHARED between the databases within the environment is the header page: page 0.

To facilitate parallel access, this 'page 0' header page is 'tweaked' in such a way that the
backend still is allowed to believe that there is a freelist entry in 'page 0': it is forced
to be empty, precluding any need for freelist locking when databases are accessed
simultaneously from different threads.

WARNING: that last remark implies that the LOG FILES for RECOVERY are also partitioned per DB
instead of per ENV!
*/

#include "internal_preparation.h"




/**
* @endcond
*/

