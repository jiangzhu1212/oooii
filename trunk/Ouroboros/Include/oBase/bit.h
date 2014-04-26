/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// Utility functions helpful when dealing with bits
#pragma once
#ifndef oBase_bit_h
#define oBase_bit_h

#ifdef _MSC_VER
#ifdef __cplusplus
	extern "C" {
#endif
unsigned char _BitScanForward(unsigned long* Index, unsigned long Mask);
unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask);
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#ifdef _M_X64
	unsigned char _BitScanForward64(unsigned long* Index, unsigned __int64 Mask);
	unsigned char _BitScanReverse64(unsigned long* Index, unsigned __int64 Mask);
	#pragma intrinsic(_BitScanReverse64)
	#pragma intrinsic(_BitScanForward64)
#endif
#ifdef __cplusplus
	}
#endif
#else
#error unsupported platform
#endif

namespace ouro {

inline unsigned int bit_count(unsigned long n)
{
	// MIT HAKMEM http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx
	register unsigned int tmp;
	tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
	return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}

inline unsigned int bit_count(unsigned long long i) 
{
	// MIT HAKMEM http://stackoverflow.com/questions/2709430/count-number-of-bits-in-a-64-bit-long-big-integer
	i = i - ((i >> 1) & 0x5555555555555555); 
	i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333); 
	return (((i + (i >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56; 
}

inline unsigned int bit_reverse(unsigned int v)
{
	// http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
	//unsigned int v; // 32-bit word to reverse bit order
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) <<  1); // swap odd and even bits
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) <<  2); // swap consecutive pairs
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) <<  4); // swap nibbles ... 
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) <<  8); // swap bytes
	v = ( v >> 16             ) | ( v               << 16); // swap 2-byte long pairs
	return v;
}

inline unsigned int bit_reverse(int v) { unsigned int r = bit_reverse(*(unsigned int*)&v); return r; }

inline int bit_high(unsigned int value)
{
	unsigned long index;
	return _BitScanReverse(&index, value) == 0 ? -1 : index;
}

inline int bit_low(unsigned int value)
{
	unsigned long index;
	return _BitScanForward(&index, value) == 0 ? -1 : index;
}

#ifdef _M_X64
	inline int bit_high(unsigned long long value)
	{
		unsigned long index;
		return _BitScanReverse64(&index, value) == 0 ? -1 : index;
	}

	inline int bit_low(unsigned long long value)
	{
		unsigned long index;
		return _BitScanForward64(&index, value) == 0 ? -1 : index;
	}

#else
	inline int bit_high(unsigned long long value)
	{
		unsigned long index;
		if (_BitScanReverse(&index, (value>>32)) == 0)
		{
			if (_BitScanReverse(&index, static_cast<unsigned long>(value)) == 0)
				return -1;
		}
		else
			index += 32;
		return index;
	}

	inline int bit_low(unsigned long long value)
	{
		unsigned long index;
		if (_BitScanForward(&index, static_cast<unsigned long>(value)) == 0)
		{
			if (_BitScanForward(&index, (value>>32)) == 0)
				return -1;
			else
				index += 32;
		}
		return index;
	}
#endif

inline int bit_high(int value) { return bit_high(*(unsigned int*)&value); }
inline int bit_low(int value) { return bit_low(*(unsigned int*)&value); }
inline int bit_high(long long value) { return bit_high(*(unsigned long long*)&value); }
inline int bit_low(long long value) { return bit_low(*(unsigned long long*)&value); }

inline int bit_first_set(int value) { return value & -value; }
inline unsigned int bit_first_set(unsigned int value) { return value & (~value + 1); }


} // namespace ouro

#endif
