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
@brief Physical storage device which stores each 'rid' (blob, etc.) in a separate file.

The file mapping is done using a 'micro-database', i.e. a simple hamster database
which contains a translation table converting 'rid's to file paths and vice versa.
*/

#include "internal_preparation.h"


/**
* @endcond 
*/

