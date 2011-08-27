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
 * This file contains macros for little endian/big endian byte swapping.
 *
 * @note The database is always stored in little-endian format.
 */

#ifndef HAM_ENDIANSWAP_H__
#define HAM_ENDIANSWAP_H__

#include <ham/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * the endian-architecture of the host computer; set this to
 * HAM_LITTLE_ENDIAN or HAM_BIG_ENDIAN
 */
#ifndef HAM_LITTLE_ENDIAN
#   ifndef HAM_BIG_ENDIAN
#       error "neither HAM_LITTLE_ENDIAN nor HAM_BIG_ENDIAN defined"
#   endif
#endif


/*
 * byte swapping macros - we use little endian
 */
#ifdef HAM_BIG_ENDIAN

#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)

#   define ham_h2db16(x)            _ham_byteswap16(x)
#   define ham_h2db32(x)            _ham_byteswap32(x)
#   define ham_h2db_rid(d, x)       ((d).rid64 = _ham_byteswap64(x))
#   define ham_h2db_recno(d, x)     ((d).recno64 = _ham_byteswap64(x))
#   define ham_h2db_size32(x)       _ham_byteswap32(x)
#   define ham_h2db_size64(d, x)    ((d).size64 = _ham_byteswap64(x))
#   define ham_db2h16(x)            _ham_byteswap16(x)
#   define ham_db2h32(x)            _ham_byteswap32(x)
#   define ham_db2h_rid(x)          _ham_byteswap64((x).rid64)
#   define ham_db2h_recno(x)        _ham_byteswap64((x).recno64)
#   define ham_db2h_size32(x)       _ham_byteswap32(x)
#   define ham_db2h_size64(x)       _ham_byteswap64((x).size64)

/**
swap recno from native to persistent (Little Endian) or vice-versa.
*/
#define ham_recno_byteswap(/*(ham_pers_recno_t *)*/recno)                   \
    do {                                                                    \
        ham_pers_recno_t *tmp_recno_ptr = (recno);                          \
        ham_u64_t tmp_recno = tmp_recno_ptr->recno64;                       \
        _ham_byteswap64(tmp_recno);                                         \
        tmp_recno_ptr->recno64 = tmp_recno;                                 \
    } while (0)

#else /* else no 64-bit int support */

#   define ham_h2db16(x)            _ham_byteswap16(x)
#   define ham_h2db32(x)            _ham_byteswap32(x)
#   define ham_h2db_rid(d, x)       do { (d).rid32[0] = _ham_byteswap32(x); (d).rid32[1] = 0; } while (0)
#   define ham_h2db_recno(d, x)     do { (d).recno32[0] = _ham_byteswap32(x); (d).recno32[1] = 0; } while (0)
#   define ham_h2db_size32(x)       _ham_byteswap32(x)
#   define ham_h2db_size64(d, x)    do { (d).size32[0] = _ham_byteswap32(x); (d).size32[1] = 0; } while (0)
#   define ham_db2h16(x)            _ham_byteswap16(x)
#   define ham_db2h32(x)            _ham_byteswap32(x)
#   define ham_db2h_rid(x)          _ham_byteswap32((x).rid32[0])
#   define ham_db2h_recno(x)        _ham_byteswap32((x).recno32[0])
#   define ham_db2h_size32(x)       _ham_byteswap32(x)
#   define ham_db2h_size64(x)       _ham_byteswap32((x).size32[0])

/**
swap recno from native to persistent (Little Endian) or vice-versa.
*/
#define ham_recno_byteswap(/*(ham_pers_recno_t *)*/recno)               \
    do {                                                                \
        ham_pers_recno_t *tmp_recno_ptr = (recno);                      \
        ham_u32_t tmp_recnoL = tmp_recno_ptr->recno32[0];               \
        ham_u32_t tmp_recnoH = tmp_recno_ptr->recno32[1];               \
        _ham_byteswap32(tmp_recnoL);                                    \
        _ham_byteswap32(tmp_recnoH);                                    \
        tmp_recno_ptr->recno32[0] = tmp_recnoH;                         \
        tmp_recno_ptr->recno32[1] = tmp_recnoL;                         \
    } while (0)

#endif /* HAM_64BIT */

#else /* HAM_LITTLE_ENDIAN */

#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)

#   define ham_h2db16(x)            (x)
#   define ham_h2db32(x)            (x)
#   define ham_h2db_rid(d, x)       ((d).rid64 = (x))
#   define ham_h2db_recno(d, x)     ((d).recno64 = (x))
#   define ham_h2db_size32(x)       (x)
#   define ham_h2db_size64(d, x)    ((d).size64 = (x))
#   define ham_db2h16(x)            (x)
#   define ham_db2h32(x)            (x)
#   define ham_db2h_rid(x)          (x).rid64
#   define ham_db2h_recno(x)        (x).recno64
#   define ham_db2h_size32(x)       (x)
#   define ham_db2h_size64(x)       (x).size64

/**
swap recno from native to persistent (Little Endian) or vice-versa.
*/
#define ham_recno_byteswap(/*(ham_pers_recno_t *)*/recno)                   \
    (void)0

#else /* else no 64-bit int support */

#   define ham_h2db16(x)            (x)
#   define ham_h2db32(x)            (x)
#   define ham_h2db_rid(d, x)       do { (d).rid32[0] = (x); (d).rid32[1] = 0; } while (0)
#   define ham_h2db_recno(d, x)     do { (d).recno32[0] = (x); (d).recno32[1] = 0; } while (0)
#   define ham_h2db_size32(x)       (x)
#   define ham_h2db_size64(d, x)    do { (d).size32[0] = (x); (d).size32[1] = 0; } while (0)
#   define ham_db2h16(x)            (x)
#   define ham_db2h32(x)            (x)
#   define ham_db2h_rid(x)          (x).rid32[0]
#   define ham_db2h_recno(x)        (x).recno32[0]
#   define ham_db2h_size32(x)       (x)
#   define ham_db2h_size64(x)       (x).size32[0]

/**
swap recno from native to persistent (Little Endian) or vice-versa.
*/
#define ham_recno_byteswap(/*(ham_pers_recno_t *)*/recno)                   \
        (void)0

#endif /* HAM_64BIT */

#endif /* HAM_LITTLE_ENDIAN */



#define _ham_byteswap16(x)                       \
        ((((x) >> 8) & 0xffu) |                  \
         (((x) & 0xffu) << 8))

#define _ham_byteswap32(x)                       \
        ((((x) & 0xff000000u) >> 24) |           \
         (((x) & 0x00ff0000u) >>  8) |           \
         (((x) & 0x0000ff00u) <<  8) |           \
         (((x) & 0x000000ffu) << 24))

#if (HAM_FAST_ONLY_32BIT != 0) || defined(HAM_64BIT)

#define _ham_byteswap64(x)                       \
        ((((x) & 0xff00000000000000ull) >> 56) | \
         (((x) & 0x00ff000000000000ull) >> 40) | \
         (((x) & 0x0000ff0000000000ull) >> 24) | \
         (((x) & 0x000000ff00000000ull) >>  8) | \
         (((x) & 0x00000000ff000000ull) <<  8) | \
         (((x) & 0x0000000000ff0000ull) << 24) | \
         (((x) & 0x000000000000ff00ull) << 40) | \
         (((x) & 0x00000000000000ffull) << 56))

#endif

#if (HAM_FAST_ONLY_32BIT != 0)

/**
Support macros for extracting and assigning values to 64 bit persistent types such as the RID
*/
#define pers_get_rid(rid)                                                   \
        ((rid).rid64)
#define pers_set_rid(pers_rid, val)                                         \
        ((pers_rid).rid64 = (val))

#define pers_get_size64(sz)                                                 \
        ((sz).size64)
#define pers_set_size64(pers_sz, val)                                       \
        ((pers_sz).size64 = (val))

#else /* else no 64-bit int support */

/**
Support macros for extracting and assigning values to 64 bit persistent types such as the RID
*/
#define pers_get_rid(rid)                                                   \
        ((rid).rid32[0])
#define pers_set_rid(pers_rid, val)                                         \
        do {                                                                \
            (pers_rid).rid32[0] = (val);                                    \
            (pers_rid).rid32[1] = 0;                                        \
        } while (0)

#define pers_get_size64(sz)                                                 \
        ((sz).size32[0])
#define pers_set_size64(pers_sz, val)                                       \
        do {                                                                \
            (pers_sz).size32[0] = (val);                                    \
            (pers_sz).size32[1] = 0;                                        \
        } while (0)

#endif /* HAM_64BIT */




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_ENDIANSWAP_H__ */

/**
* @endcond
*/

