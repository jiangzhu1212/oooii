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
// Approximate equality for float/double types. This uses absolute bit 
// differences rather than epsilon or some very small float value because eps
// usage is not valid across the entire range of floating precision. With this
// oStd::equal API it can be guaranteed that the statement "equal within N 
// minimal bits of difference" will be true.

// ULPS = "units of last place". Number of float-point steps of error. At 
// various sizes, 1 bit of difference in the floating point number might mean 
// large or small deltas, so just doing an epsilon difference is not valid 
// across the entire spectrum of floating point representations. With ULPS, you 
// specify the maximum number of floating-point steps, not absolute (fixed) 
// value that something should differ by, so it scales across all of float's 
// range.
#pragma once
#ifndef oStd_equal_h
#define oStd_equal_h

#include <stdexcept>

#define oSTD_DEFAULT_ULPS 5

namespace oStd {

template<typename T> inline bool equal(const T& A, const T& B, unsigned int _MaxUlps) { return A == B; }
template<typename T> inline bool equal(const T& A, const T& B) { return equal(A, B, oSTD_DEFAULT_ULPS); }

template<> inline bool equal(const float& A, const float& B, unsigned int _MaxUlps)
{
	/** <citation
		usage="Implementation" 
		reason="Apparently using eps isn't good enough." 
		author="Bruce Dawson"
		description="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		modification="add exception support"
	/>*/
	// $(CitedCodeBegin)
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	if (_MaxUlps >= (4 * 1024 * 1024))
		throw std::overflow_error("ulps overflow");
	int aInt = *(int*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x80000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	int bInt = *(int*)&B;
	if (bInt < 0)
		bInt = 0x80000000 - bInt;
	unsigned int intDiff = abs(aInt - bInt);
	if (intDiff <= _MaxUlps)
		return true;
	return false;
	// $(CitedCodeEnd)
}

template<> inline bool equal(const double& A, const double& B, unsigned int _MaxUlps)
{
	typedef long long intT;
	typedef unsigned long long uintT;

	/** <citation
		usage="Adaptation" 
		reason="Apparently using eps isn't good enough." 
		author="Bruce Dawson"
		description="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		modification="add exception support and types to accommodate doubles. Express abs() explicitly because VC9 doesn't seem to have a long long implementation"
	/>*/
	// $(CitedCodeBegin)
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	if (_MaxUlps >= (4 * 1024 * 1024))
		throw std::overflow_error("ulps underflow");
	intT aInt = *(intT*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x8000000000000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	intT bInt = *(intT*)&B;
	if (bInt < 0)
		bInt = 0x8000000000000000 - bInt;
	intT diff = aInt - bInt;
	uintT intDiff = diff >= 0 ? diff : -diff; // abs() seems only implemented for int/long in VC9
	if (intDiff <= _MaxUlps)
		return true;
	return false;
	// $(CitedCodeEnd)
}

} // namespace oStd

#endif
