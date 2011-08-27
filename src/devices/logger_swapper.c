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
A device which manages two 'files' (sub-devices): when a flush is triggered,
the data is written to the currently active 'file' and then the files are
'swapped', i.e. the other file is made 'active'.

This behaviour assists recovery logging, which employs two log files, one of
which is always the 'active' one.
*/

#include "internal_preparation.h"


/**
* @endcond 
*/

