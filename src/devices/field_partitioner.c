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
@brief An augmented partitioner, which maps key and record fields to 'rid zones' when assigning rids to new
space requests; these 'rid zones' are then used to transfer the data to the corresponding partition
once it gets saved (or is reloaded at a later time).
*/

#include "internal_preparation.h"


/**
* @endcond 
*/

