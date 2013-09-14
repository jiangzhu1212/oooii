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
#include <memory.h>

namespace oStd {

/** <citation
	usage="Implementation" 
	reason="win32 compiler double -> unsigned long long is not correct, this is"
	author="Erik Rufelt"
	description="http://www.gamedev.net/topic/591975-c-unsigned-__int64-tofrom-double-conversions/page__st__20"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.gamedev.net/topic/591975-c-unsigned-__int64-tofrom-double-conversions/page__st__20"
	modification="assert -> static assert. uint64 -> unsigned long long"
/>*/
// $(CitedCodeBegin)
unsigned long long dtoull(double input)
{
	static_assert(sizeof(double) == 8, "sizeof(double) == 8");
	static_assert(sizeof(unsigned long long) == 8, "sizeof(unsigned long long) == 8");
	static_assert(sizeof(1ULL) == 8, "sizeof(1ull) == 8");

	// Get the bits representing the double
	double d = input;
	unsigned long long doubleBits;
	memcpy(reinterpret_cast<char*>(&doubleBits), reinterpret_cast<char*>(&d), 8);

	// Check for a negative number
	unsigned long long signBit = (doubleBits >> 63ULL) & 0x1ULL;
	if(signBit != 0ULL)
		return 0ULL;

	// Get the exponent
	unsigned long long exponent = (doubleBits >> 52ULL) & 0x7ffULL;

	// The number is larger than the largest uint64
	if(exponent > 1086ULL)
		return 0ULL;

	// The number is less than 1
	if(exponent < 1023ULL)
		return 0ULL;

	// Get the fraction
	unsigned long long fraction = (doubleBits & 0xfffffffffffffULL) | 0x10000000000000ULL;

	// Calculate and return integer part
	if(exponent >= 1075ULL)
		return fraction << (exponent - 1075ULL);
	else
		return fraction >> (1075ULL - exponent);
}
// $(CitedCodeEnd)

} // namespace oStd
