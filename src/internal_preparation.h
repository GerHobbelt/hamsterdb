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
 * @brief internal preparation macros and a common start for precompiled headers
 *
 * This header file should be included as the very first in every HamsterDB source file.
 *
 * It defines very few things which are used throughout the code, plus it configures your
 * compiler to produce a desired set of optimizations and/or warnings.
 *
 * It is also used to mark the start (and end) of the section which can be precompiled
 * (assuming your compiler supports precompiled headers).
 */

#ifndef HAM_INTERNAL_PREPARATION_H__
#define HAM_INTERNAL_PREPARATION_H__


#include "config.h"


#ifdef _MSC_VER
#pragma warning(error: 4003) /* make warning fail compile phase: not enough actual parameters for macro 'ham_assert' */

/* suppress some warnings in the system header files: */
#pragma warning(push)
#pragma warning(disable: 4548) /* expression before comma has no effect; expected expression with side-effect */
#endif


#ifdef HAVE_MALLOC_H
#  include <malloc.h>
#else
#  include <stdlib.h>
#endif
#include <string.h>
#include <math.h>
#include <float.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include "internal_fwd_decl.h"


#if defined(HAM_DEBUG)
#ifdef _MSC_VER
#pragma warning(disable: 4710) /* warning disabled: function not inlined */
#pragma warning(disable: 4100) /* warning disabled: unreferenced formal parameter */
#endif
#endif


#include "endianswap.h"
#include "error.h"
#include "util.h"

#include "os.h"
#include "mem.h"
#include "txn.h"
#include "env.h"
#include "db.h"
#include "backend.h"
#include "device.h"
#include "btree_key.h"
#include "log.h"
#include "page.h"
#include "btree_stats.h"
#include "blob.h"
#include "cache.h"
#include "cursor.h"
#include "extkeys.h"
#include "freelist.h"
#include "freelist_statistics.h"
#include "serial.h"
#include "version.h"
#include "util.h"

#include "btree.h"
#include "btree_cursor.h"
#include "journal.h"
#include "txn_cursor.h"

#include "changeset.h"
#include "errorinducer.h"

#include "duplicates.h"

#ifdef __cplusplus
extern "C" {
#endif




/**
 * a macro to cast pointers to u64 and vice versa to avoid compiler
 * warnings if the sizes of ptr and u64 are not equal
 */
#if defined(HAM_32BIT) && !defined(_MSC_VER)
#   define U64_TO_PTR(p)  ((ham_u8_t *)(unsigned int)(p))
#   define PTR_TO_U64(p)  ((ham_offset_t)(unsigned int)(p))
#else
#   define U64_TO_PTR(p)  (p)
#   define PTR_TO_U64(p)  (p)
#endif




/*
 * macro to help diagnose stack references outside the stack frame where they reside.
 */
#if defined(HAM_DEBUG)
#define ham_nuke_stack_space(elem)      memset(&(elem), 0xA5U, sizeof(elem))
#else
#define ham_nuke_stack_space(elem)      /**/
#endif






/*
 * helper for #pragma message(...)
 */
#if defined(_MSC_VER)
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#endif



#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif

/**
* @endcond
*/

