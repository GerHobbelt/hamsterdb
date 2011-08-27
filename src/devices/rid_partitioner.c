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
@brief A basic partitioner, which segments the 'rid' ID space into multiple partitions

Data is transferred to the appropriate partition for each save and load operation.
*/

#include "internal_preparation.h"


/**
* @endcond 
*/

