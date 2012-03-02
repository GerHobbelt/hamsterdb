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
@brief An augmented partitioner which converts key value ranges to 'rid zones' when new data space is requested.

These 'rid zones' are recognized and used later on to transfer the data to the appropriate partition
when the data is saved or reloaded.
*/

#include "internal_preparation.h"


/**
* @endcond 
*/
