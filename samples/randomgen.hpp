
#ifndef __MARSAGLIA_SRNG_H__
#define __MARSAGLIA_SRNG_H__

#include <ham/types.h>
#include <time.h>


/**
 * @file randomgen.hpp
 * @author Ger Hobbelt, ger@hobbelt.com
 * @version 1.1.2
 *
A high quality SRNG (Semi Random Number Generator)

This code is an implementation of one of Marsaglia's random generators and
derived from the C source code as created by Agner Fog, 2007.

The new cross-platform portable and reproducable random generator is
derived from the randomc package by Agner Fog, available at:

   http://www.agner.org/random/

***************Derived from MOTHER.CPP ****************** AgF 2007-08-01 *
*  'Mother-of-All' random number generator                               *
*                                                                        *
*  This is a multiply-with-carry type of random number generator         *
*  invented by George Marsaglia.  The algorithm is:                      *
*  S = 2111111111*X[n-4] + 1492*X[n-3] + 1776*X[n-2] + 5115*X[n-1] + C   *
*  X[n] = S modulo 2^32                                                  *
*  C = floor(S / 2^32)                                                   *
*                                                                        *
*  Note:                                                                 *
*  This implementation uses 64-bit integers for intermediate             *
*  calculations. Works only on compilers that support 64-bit integers.   *
*                                                                        *
* (c) 1999 - 2007 A. Fog.                                                *
* GNU General Public License www.gnu.org/copyleft/gpl.html               *
**************************************************************************
*/
class random_generator
{
private:
	ham_u32_t rand_store[5];

public:
	random_generator()
	{
        init((ham_u32_t)time(NULL));
	}
	random_generator(ham_u32_t seed)
	{
        init(seed);
	}

	~random_generator()
	{
	}


	/**
	Output 32 random bits
	*/
	ham_u32_t rand32(void)
	{
		ham_u64_t sum;

		sum = (ham_u64_t)2111111111UL * (ham_u64_t)rand_store[3]
			  + (ham_u64_t)1492 * (ham_u64_t)(rand_store[2])
			  + (ham_u64_t)1776 * (ham_u64_t)(rand_store[1])
			  + (ham_u64_t)5115 * (ham_u64_t)(rand_store[0])
			  + (ham_u64_t)rand_store[4];
		rand_store[3] = rand_store[2];
		rand_store[2] = rand_store[1];
		rand_store[1] = rand_store[0];
		rand_store[4] = (ham_u32_t)(sum >> 32);          // Carry
		rand_store[0] = (ham_u32_t)(sum);                // Low 32 bits of sum
		return rand_store[0];
	}

	/**
	returns a random number between 0 and 1
	*/
	double frand(void)
	{
		return (double)rand32() * (1. / (65536. * 65536.));
	}

	/**
	initializes the random number generator
	*/
	void init(ham_u32_t seed)
	{
		int i;
		ham_u32_t s = seed;

		// make random numbers and put them into the buffer
		for (i = 0; i < 5; i++)
		{
			s = s * 29943829 - 1;
			rand_store[i] = s;
		}
		// randomize some more
		for (i = 0; i < 19; i++)
		{
			rand32();
		}
	}
};


#endif /* __MARSAGLIA_SRNG_H__ */


