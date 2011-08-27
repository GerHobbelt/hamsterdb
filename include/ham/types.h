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
 * @file types.h
 * @brief Portable typedefs for hamsterdb Embedded Storage.
 * @author Christoph Rupp, chris@crupp.de
 */

#ifndef HAM_TYPES_H__
#define HAM_TYPES_H__


#ifdef _MSC_VER
/* suppress some warnings in the system header files: */
#pragma warning(push)
#pragma warning(disable: 4548) /* expression before comma has no effect; expected expression with side-effect */
#endif


/*
 * Check the operating system and word size
 */
#ifdef UNDER_CE
#   undef WIN32
#   define WIN32 1
#   define HAM_OS_WINCE 1
#endif

#if defined(WIN32) || defined(_WIN32)
#   undef  HAM_OS_WIN32
#   define HAM_OS_WIN32 1
#   if defined(WIN64) || defined(_M_IA64) || defined(_M_X64) || defined(_WIN64)
#       undef  HAM_64BIT
#       define HAM_64BIT 1
#   elif defined(WIN32) || defined(_WIN32)
#       undef  HAM_32BIT
#       define HAM_32BIT 1
#   endif
#else /* posix? */
#   undef  HAM_OS_POSIX
#   define HAM_OS_POSIX 1
#   if defined(__LP64__) || defined(__LP64) || __WORDSIZE==64
#       undef  HAM_64BIT
#       define HAM_64BIT 1
#   else
#       undef  HAM_32BIT
#       define HAM_32BIT 1
#   endif
#endif

#if defined(HAM_OS_POSIX) && defined(HAM_OS_WIN32)
#    error "Unknown arch - neither HAM_OS_POSIX nor HAM_OS_WIN32 defined"
#endif

/*
 * windows.h is needed for HANDLE
 */
#if defined(HAM_OS_WIN32)
#   define WIN32_MEAN_AND_LEAN
#   include <windows.h>
#endif

/*
 * improve memory debugging on WIN32 by using crtdbg.h (only MSVC
 * compiler and debug builds!)
 *
 * make sure crtdbg.h is loaded before malloc.h!
 */
#if defined(_MSC_VER) && defined(HAM_OS_WIN32)
#   if !defined(UNDER_CE)
#      if defined(DEBUG) || defined(_DEBUG)
#         if !defined(_CRTDBG_MAP_ALLOC)
#            define _CRTDBG_MAP_ALLOC 1
#         endif
#      endif
#      include <crtdbg.h>
#      include <malloc.h>
#   endif
#endif

/*
 * Create the EXPORT macro for Microsoft Visual C++
 */
#ifndef HAM_EXPORT
#   ifdef _MSC_VER
#       define HAM_EXPORT __declspec(dllexport)
#   else
#       define HAM_EXPORT extern
#   endif
#endif

/*
 * The default calling convention is cdecl
 */
#ifndef HAM_CALLCONV
#   define HAM_CALLCONV
#endif

/*
 * enable structure padding warnings at all times!
 */
#ifdef _MSC_VER
//#pragma warning(push)
#pragma warning(error: 4820)
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * typedefs for 32bit operating systems
 */
#ifdef HAM_32BIT
#   ifdef _MSC_VER
typedef signed __int64     ham_s64_t;
typedef unsigned __int64   ham_u64_t;
#   else
typedef signed long long   ham_s64_t;
typedef unsigned long long ham_u64_t;
#   endif
typedef signed int         ham_s32_t;
typedef unsigned int       ham_u32_t;
typedef signed short       ham_s16_t;
typedef unsigned short     ham_u16_t;
typedef signed char        ham_s8_t;
typedef unsigned char      ham_u8_t;
#endif

/**
 * typedefs for 64bit operating systems; on Win64,
 * longs do not have 64bit! (LLP64 vs. LP64)
 */
#ifdef HAM_64BIT
#   ifdef _MSC_VER
typedef signed __int64     ham_s64_t;
typedef unsigned __int64   ham_u64_t;
#   else
typedef signed long        ham_s64_t;
typedef unsigned long      ham_u64_t;
#   endif
typedef signed int         ham_s32_t;
typedef unsigned int       ham_u32_t;
typedef signed short       ham_s16_t;
typedef unsigned short     ham_u16_t;
typedef signed char        ham_s8_t;
typedef unsigned char      ham_u8_t;
#endif

/*
 * Set up a sensible default setting: we prefer fast action.
 */
#if !defined(HAM_FAST_ONLY_32BIT)
#ifdef HAM_64BIT
#define HAM_FAST_ONLY_32BIT	1
#else
#define HAM_FAST_ONLY_32BIT 0
#endif
#endif


/*
 * Undefine macros to avoid macro redefinitions
 */
#undef HAM_INVALID_FD
#undef HAM_FALSE
#undef HAM_TRUE

/*
 * typedefs for POSIX
 */
#ifdef HAM_OS_POSIX
typedef int                ham_fd_t;
#   define HAM_INVALID_FD  (-1)
#endif

/*
 * typedefs for Windows 32- and 64-bit
 */
#ifdef HAM_OS_WIN32
#   ifdef CYGWIN
typedef int                ham_fd_t;
#   else
typedef HANDLE             ham_fd_t;
#   endif
#   define HAM_INVALID_FD  (0)
#endif

/**
 * a boolean type
 */
typedef unsigned short int ham_bool_t;
#define HAM_FALSE          0
#define HAM_TRUE           (!HAM_FALSE)

/**
 * typedef for error- and status-code
 */
typedef int                ham_status_t;


#include <ham/packstart.h>

/**
 * Typedef for <em>storing</em> an address (a.k.a. RID) in the persistent
 * store / file. This limits the absolute maximum file size to '64 bit'
 * i.e. 2-to-the-power-64 which is a little over 18 ExaByte (18E+18).
 *
 * @remark If you change this datatype, you also have to change
 * the endian-macros in src/endianswap.h (@ref ham_db2h_rid / @ref ham_h2db_rid)
 *
 * The related variable type used when processing this type, e.g.
 * when using it in calculations, is most often @ref ham_offset_t which
 * is a type designed to span the entire RID storage addressing range
 * on your platform.
 *
 * @note HamsterDB includes an optimization for @e very tiny records:
 *       those are stored inside the RID, instead of having the RID
 *       point at their storage location in the database file.
 *       This optimization applies for records with any of the flags
 *       @ref KEY_BLOB_SIZE_TINY, @ref KEY_BLOB_SIZE_SMALL or
 *       @ref KEY_BLOB_SIZE_EMPTY
 */
typedef HAM_PACK_0 union HAM_PACK_1 ham_pers_rid_t
{
#if (HAM_FAST_ONLY_32BIT != 0)
    ham_u64_t          rid64;
#endif
    ham_u32_t          rid32[2];
	ham_u8_t           rid8[8]; /**< for direct 'tiny record' access */
} HAM_PACK_2 ham_pers_rid_t;

#include <ham/packstop.h>



/**
 * typedef for when working on (or performing calculations with) RIDs (see
 * @ref ham_pers_rid_t and @ref ham_rid_t) at run-time.
 * Note that only when your platform supports 64-bit integers will you be able
 * to address persistent storage space (i.e. files) significantly larger
 * than 4GByte (2^32 bytes).
 *
 * This type will be the largest available <em>unsigned</em> integer type on
 * your platform (compiler / OS combo)
 *
 * @sa ham_signed_off_t
 * @sa ham_pers_rid_t
 */
#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT) /* _INTEGRAL_MAX_BITS */
typedef ham_u64_t          ham_offset_t;
#else
typedef ham_u32_t          ham_offset_t;
#endif


/**
 * typedef for storing RIDs at run-time.
 *
 * @note this type must be able to store all bytes in the @ref ham_pers_rid_t
 * in particular circumstances, but in regular situations you should only access
 * the @a rid member of this union when you wish to treat a RID as a native integer
 * value.
 */
typedef union
{
	ham_offset_t rid;
	ham_u8_t raw_data[8];
} ham_rid_t;


/**
 * typedef for when performing calculations at run-time which require
 * you to use the largest available signed integer type available on
 * your platform.
 * Note that only when your platform supports 64-bit integers will you
 * be able to address persistent storage space (i.e. files) significantly
 * larger than 4GByte (2^32 bytes).
 *
 * This type will be the largest available <em>signed</em> integer type
 * on your platform (compiler / OS combo)
 *
 * @sa ham_offset_t
 * @sa ham_pers_rid_t
 */
#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)
typedef ham_s64_t          ham_signed_off_t;
#else
typedef ham_s32_t          ham_signed_off_t;
#endif


/**
 * typedef for persisted 32-bit sizes.
 *
 * @remark If you change this data type, you also have to change
 * the endian-macros in src/endianswap.h
 *
 * The related variable type used when processing this type, e.g.
 * when using it in calculations, is most often @ref ham_size_t
 * which is a type designed to span the 32 bit range on your platform.
 *
 * @if ham_internals
 * @sa ham_db2h_size32
 * @sa ham_h2db_size32
 * @endif
 */
typedef ham_u32_t          ham_pers_size32_t;


#include <ham/packstart.h>

/**
 * typedef for persisted 64-bit sizes.
 *
 * @remark If you change this data type, you also have to change
 * the endian-macros in src/endianswap.h
 *
 * The related variable type used when processing this type, e.g.
 * when using it in calculations, is most often @ref ham_offset_t
 * which is a type designed to span the entire RID storage
 * addressing range on your platform.
 *
 * @if ham_internals
 * @sa ham_db2h_size64
 * @sa ham_h2db_size64
 * @endif
 */
typedef HAM_PACK_0 union HAM_PACK_1 ham_pers_size64_t
{
#if (HAM_FAST_ONLY_32BIT != 0)
    ham_u64_t          size64;
#endif
    ham_u32_t          size32[2];
} HAM_PACK_2 ham_pers_size64_t;

#include <ham/packstop.h>


/**
 * typedef for sizes, which limits data blobs to 32 bits
 *
 * @remark If you change this datatype, you also have to change
 * the endian-macros in src/endian.h (ham_db2h_size/ham_h2db_size)
 */
typedef ham_u32_t          ham_size_t;

/**
Transaction ID / counter integer type
*/
#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)
typedef ham_u64_t          ham_txn_id_t;
#else
typedef ham_u32_t          ham_txn_id_t;
#endif


#include <ham/packstart.h>

/**
persisted RECNO record number type
*/
typedef HAM_PACK_0 union HAM_PACK_1 ham_pers_recno_t
{
#if (HAM_FAST_ONLY_32BIT != 0)
    ham_u64_t          recno64;
#endif
    ham_u32_t          recno32[2];
} HAM_PACK_2 ham_pers_recno_t;

#include <ham/packstop.h>

/**
run-time RECNO record number type
*/
#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)
typedef ham_u64_t ham_recno_t;
#else
typedef ham_u32_t ham_recno_t;
#endif

#include <ham/packstart.h>

/**
typedef for arbitrary integer type payloads which dangle at
the end of defined structures.

This type is specially crafted to make modern compilers, such as
GCC 4.4.x happy when maximum optimizations are dialed up.

(See 'pointer aliasing and gcc errors' issues mentioned on the internet.)
*/
typedef HAM_PACK_0 union HAM_PACK_1 ham_pers_multitype_t
{
    double    sizefp[1]; /* 'double' is the only 64-bit type on older 32-bit boxes */
#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)
    ham_u64_t size64[1];
#endif
    ham_u32_t size32[2];
    ham_u16_t size16[4];
    ham_u8_t  size08[8];
} HAM_PACK_2 ham_pers_multitype_t;

#include <ham/packstop.h>




/**
 * Fixed point arithmetic type used in fast calculations of fractional numbers such as ratios.
 *
 * The fixed point is located between bit 12 and 13, hence 8192 ~ 1.0 (@ref HAM_FLOAT_1)
 *
 * It also means the range of this 'floating point' type is +256K-1 .. -256K. This range is explicitly chosen
 * for this ensures that we can add and subtract at least up to three pages worth of keys (3 * absolute maxmimum key count per page ~ 3 * 64K)
 * without risking type overflow.
 */
typedef ham_s32_t ham_float_t;

/** The value 1.0 as represented in a @ref ham_float_t type instance */
#define HAM_FLOAT_1					8192

/** Converts any value to a @ref ham_float_t value. */
#define MK_HAM_FLOAT(val)           ((ham_float_t)((val) * HAM_FLOAT_1))





/**
 * @defgroup max_values_for_types Maximum / minimum values which can be stored in the related ham_[type]_t type
 *
 * @{
*/

/** absolute maximum value for @ref ham_u16_t, which is an unsigned type */
#define HAM_MAX_U16             (~(ham_u16_t)0)
/** absolute maximum value for @ref ham_u32_t, which is an unsigned type */
#define HAM_MAX_U32             (~(ham_u32_t)0)
/** absolute maximum value for @ref ham_s16_t, which is an signed type */
#define HAM_MAX_S16             ((ham_s16_t)((~(ham_u16_t)0) >> 1))					// make sure the shift-right is a LOGICAL SHIFT RIGHT!
/** absolute maximum value for @ref ham_s32_t, which is an signed type */
#define HAM_MAX_S32             ((ham_s32_t)((~(ham_u32_t)0) >> 1))					// make sure the shift-right is a LOGICAL SHIFT RIGHT!
/** absolute minimum value for @ref ham_s16_t, which is an signed type */
#define HAM_MIN_S16             ((ham_s16_t)((~(ham_u16_t)0) >> 1 + 1))				// make sure the shift-right is a LOGICAL SHIFT RIGHT!
/** absolute minimum value for @ref ham_s32_t, which is an signed type */
#define HAM_MIN_S32             ((ham_s32_t)((~(ham_u32_t)0) >> 1 + 1))				// make sure the shift-right is a LOGICAL SHIFT RIGHT!
/** absolute maximum value for @ref ham_size_t, which is an unsigned type */
#define HAM_MAX_SIZE_T          (~(ham_size_t)0)
/** absolute maximum value for @ref ham_offset_t, which is an unsigned type */
#define HAM_MAX_OFFSET_T        (~(ham_offset_t)0)
/** absolute maximum value for @ref ham_signed_off_t, which is an @e signed type */
#define HAM_MAX_SIGNED_OFF_T    ((ham_signed_off_t)((~(ham_offset_t)0) >> 1))		// make sure the shift-right is a LOGICAL SHIFT RIGHT!
/** absolute minimum value for @ref ham_signed_off_t, which is an @e signed type */
#define HAM_MIN_SIGNED_OFF_T    ((ham_signed_off_t)((~(ham_offset_t)0) >> 1 + 1))	// make sure the shift-right is a LOGICAL SHIFT RIGHT!
/** absolute maximum value for @ref ham_float_t, which is an signed type */
#define HAM_MAX_FLOAT           HAM_MAX_S32
/** absolute minimum value for @ref ham_float_t, which is an signed type */
#define HAM_MIN_FLOAT           HAM_MIN_S32


/**
absolute maximum supported key length.

@sa ham_key_t
*/
#define HAM_MAX_KEY_SIZE_T      HAM_MAX_U16
/**
absolute maximum supported record size.

@sa ham_record_t
*/
#define HAM_MAX_RECORD_SIZE_T   HAM_MAX_SIZE_T

/**
 * @}
 */


#ifdef __cplusplus
} // extern "C"
#endif


#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif /* HAM_TYPES_H__ */
