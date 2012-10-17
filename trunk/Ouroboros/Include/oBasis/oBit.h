/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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

// Utility functions helpful when dealing with bitfields. This is written to use
// the same API as HLSL Shader Model 5.0.
#pragma once
#ifndef oBit_h
#define oBit_h

#include <oBasis/oIntrinsic.h>

inline unsigned int countbits(unsigned long n)
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions" 
		author="Gurmeet Singh Manku"
		description="http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	// MIT HAKMEM Count
	/* works for 32-bit numbers only    */
	/* fix last line for 64-bit numbers */
	register unsigned int tmp;
	tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
	return ((tmp + (tmp >> 3)) & 030707070707) % 63;
	// $(CitedCodeEnd)
}

inline unsigned int countbits(unsigned long long i) 
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions" 
		author="Maciej H"
		description="http://stackoverflow.com/questions/2709430/count-number-of-bits-in-a-64-bit-long-big-integer"
		license="*** Assumed Public Domain ***"
		licenseurl="http://stackoverflow.com/questions/2709430/count-number-of-bits-in-a-64-bit-long-big-integer"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	// Hamming weight
	i = i - ((i >> 1) & 0x5555555555555555); 
	i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333); 
	return (((i + (i >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56; 
	// $(CitedCodeEnd)
}

inline unsigned int reversebits(unsigned int v)
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions"
		author="Edwin Freed"
		description="http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious"
		license="*** Assumed Public Domain ***"
		licenseurl="http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	//unsigned int v; // 32-bit word to reverse bit order
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ... 
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = (v >> 16            )   | (v               << 16);
	return v;
	// $(CitedCodeEnd)
}

inline int reversebits(int v) { unsigned int r = reversebits(*(unsigned int*)&v); return *(int*)&r; }

// Returns the next power of 2 from the current value.
// So if _Value is 512 this will return 1024
inline unsigned long oNextPow2(unsigned long _Value)
{
	unsigned long pow2;
	_BitScanReverse(&pow2, _Value);
	++pow2;
	return 1 << pow2;
}

// firstbithigh and firstbitlow flavors return -1 if value is 0, or the index
// of the first bit set.

inline int firstbithigh(unsigned int value)
{
	unsigned long index;
	return _BitScanReverse(&index, value) == 0 ? -1 : index;
}

inline int firstbitlow(unsigned int value)
{
	unsigned long index;
	return _BitScanForward(&index, value) == 0 ? -1 : index;
}

#ifdef _M_X64
	inline int firstbithigh(unsigned long long value)
	{
		unsigned long index;
		return _BitScanReverse64(&index, value) == 0 ? -1 : index;
	}

	inline int firstbitlow(unsigned long long value)
	{
		unsigned long index;
		return _BitScanForward64(&index, value) == 0 ? -1 : index;
	}

#else
	inline int firstbithigh(unsigned long long value)
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

	inline int firstbitlow(unsigned long long value)
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

inline int firstbithigh(int value) { return firstbithigh(*(unsigned int*)&value); }
inline int firstbitlow(int value) { return firstbitlow(*(unsigned int*)&value); }
inline int firstbithigh(long long value) { return firstbithigh(*(unsigned long long*)&value); }
inline int firstbitlow(long long value) { return firstbitlow(*(unsigned long long*)&value); }

inline int firstbitset(int value) { return value & -value; }
inline unsigned int firstbitset(unsigned int value) { return value & (~value + 1); }

// Returns whether the value is between the _Start and the _End (inclusive of _Start and _End) taking into account integer rollover
template<typename T>
inline bool oBetweenInclusive( T _Start, T _End, T _Value )
{
	// @oooii-kevin: This cast is necessary because the MS compiler is implicitly converting the intermediate computation to 32-bit (which is probably a compiler bug)
	return (T)(_Value - _Start)  <= (T)(_End - _Start);
}

#endif
