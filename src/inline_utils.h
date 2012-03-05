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
 * @brief utility functions
 *
 */

#ifndef HAM_INLINE_UTILS_H__
#define HAM_INLINE_UTILS_H__

#include <ham/types.h>

#include <stdarg.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
* inline function (must be fast!) which calculates the smallest
* encompassing power-of-16 for the given value. The integer equivalent
* of roundup(log16(value)).
*
* @return Returned value range: 0..16
*/
static __inline ham_u16_t ham_log16(ham_offset_t v)
{
#if 0
    register ham_size_t value = v;
    register ham_u16_t power = 0;

    if (value)
    {
        do
        {
            power++;
        } while (value >>= 4);
    }

    return power;
#else
    /**
    Inspired by Hacker's Delight, pp. 77-84 (Counting Bits chapter) + pp 215-217 (Integer logarithms)

    Effectively a bitlevel bsearch. Contrast that with the naive solution
    which is a O(N) loop (for N=16 or N=8 as we're processing a ham_size_t here).
    */

#if defined(HAM_64BIT)

    if (v & 0xFFFFFFFF00000000ull)
    {
        register unsigned int x = (unsigned int)(v >> 32);

        if (x & 0xFFFF0000u)
            if (x & 0xFF000000u)
                if (x & 0xF0000000u)
                    return 8+8;
                else
                    return 7+8;
            else
                if (x & 0x00F00000u)
                    return 6+8;
                else
                    return 5+8;
        else
            if (x & 0x0000FF00u)
                if (x & 0x0000F000u)
                    return 4+8;
                else
                    return 3+8;
            else
                if (x & 0x000000F0u)
                    return 2+8;
                else
                    if (x /* & 0x0000000Fu */)
                        return 1+8;
                    else
                        return 0+8;
    }
    else
    {
        register unsigned int x = (unsigned int)v;

        if (x & 0xFFFF0000u)
            if (x & 0xFF000000u)
                if (x & 0xF0000000u)
                    return 8;
                else
                    return 7;
            else
                if (x & 0x00F00000u)
                    return 6;
                else
                    return 5;
        else
            if (x & 0x0000FF00u)
                if (x & 0x0000F000u)
                    return 4;
                else
                    return 3;
            else
                if (x & 0x000000F0u)
                    return 2;
                else
                    if (x /* & 0x0000000Fu */)
                        return 1;
                    else
                        return 0;
    }

#else

    register ham_offset_t x = v;

    if (x & 0xFFFF0000u)
        if (x & 0xFF000000u)
            if (x & 0xF0000000u)
                return 8;
            else
                return 7;
        else
            if (x & 0x00F00000u)
                return 6;
            else
                return 5;
    else
        if (x & 0x0000FF00u)
            if (x & 0x0000F000u)
                return 4;
            else
                return 3;
        else
            if (x & 0x000000F0u)
                return 2;
            else
                if (x /* & 0x0000000Fu */)
                    return 1;
                else
                    return 0;

#endif
#endif
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HAM_UTIL_H__ */

/**
* @endcond
*/

