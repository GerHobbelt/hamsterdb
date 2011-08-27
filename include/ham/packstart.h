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
 * @brief macros for packing structures; should work with most compilers.
 *
 */

/**
 *  example usage:
 *
 *  #include "packstart.h"
 *
 *  typedef HAM_PACK_0 struct HAM_PACK_1 example_struct
 *  {
 *    oef_u16_t var1;
 *    oef_u8_t var2;
 *    oef_u32_t var3;
 *  } HAM_PACK_2 example_struct_t;
 *
 *  #include "packstop.h"
 *
 */


/*
 * enable structure padding warnings at all times!
 */
#ifdef _MSC_VER
#pragma warning(disable: 4103) /* shut up compiler about: alignment changed after including header, may be due to missing #pragma pack(pop) <-- that's intentional! And do this outside the push/pop as we still need it when we come out of <packstop.h> */

#pragma warning(push)
#pragma warning(error: 4820) /* compiler errors out when there'll be implicit padding bytes in any struct */
#endif


#undef HAM_PACK_0
#undef HAM_PACK_1
#undef HAM_PACK_2


#if defined(__GNUC__) && !defined(_NEWGNUC_)
#  if (((__GNUC__==2) && (__GNUC_MINOR__>=7)) || (__GNUC__>2))
#  define HAM_PACK_2 __attribute__ ((packed))
#  define _NEWGNUC_
#  endif
#endif

#ifdef __WATCOMC__
#  define HAM_PACK_0 _Packed
#endif

#if !defined(_NEWMSC_)
# if (defined(_MSC_VER) && (_MSC_VER >= 900)) || defined(__BORLANDC__)
#  define _NEWMSC_
# endif
#endif
#if !defined(_NEWGNUC_) && !defined(__WATCOMC__) && !defined(_NEWMSC_)
#  pragma pack(1)
#endif
#ifdef _NEWMSC_
#  pragma pack(push, 1)
#  define HAM_PACK_2 __declspec(align(1))
#endif

#if defined(_NEWMSC_) && !defined(_WIN32_WCE)
#  pragma pack(push, 1)
#  define HAM_PACK_2 __declspec(align(1))
#endif

#ifndef HAM_PACK_0
#  define HAM_PACK_0
#endif

#ifndef HAM_PACK_1
#  define HAM_PACK_1
#endif

#ifndef HAM_PACK_2
#  define HAM_PACK_2
#endif
