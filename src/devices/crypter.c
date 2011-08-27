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
@brief A 'virtual device' which encrypts the data to be saved.

The space allocation methods take into account that certain 'rid zones' will NOT be encrypted; 
these methods will ensure that sensitive data is encrypted at all times.

Note that this device will adjust the page size as a crypto header and/or footer will be added
per page.
*/


#include "internal_preparation.h"


/**
* @endcond 
*/

