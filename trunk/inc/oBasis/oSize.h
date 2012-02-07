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

// To protect against bugs between 32-bit and 64-bit builds, create a type
// called oSizeXX that is fixed in bit-size for all builds. Since different
// APIs treat size differently (STL: size_t, DirectX, unsigned int), all the
// operators for oSizeXX are overloaded to ensure there is no overflow.
// Prefer constructing an oSize type rather than C-casting or static_cast.
#pragma once
#ifndef oSize_h
#define oSize_h

#include <oBasis/oCheckedAssignment.h>

namespace detail {

template<typename T> class oSizeT
{
	T Size;

	oDEFINE_RANGE_CHECKS(oSizeT, T, Size, oNumericLimits<T>::GetMax());
	oDEFINE_ALWAYS_VALID_CHECK(T);

public:
	typedef T value_type;
	oSizeT() : Size(0) {}
	template<typename U> oSizeT(const U& _Size) { CheckMax(_Size, "ctor"); Size = static_cast<T>(_Size); }
	oDEFINE_CHECKED_OPERATORS(oSizeT, T, Size);
};

} // namespace detail

template<typename T> detail::oSizeT<T> operator*(const detail::oSizeT<T>& a, const detail::oSizeT<T>& b) { return a.operator*(b); }
template<typename T, typename U> detail::oSizeT<T> operator*(const U& a, const detail::oSizeT<T>& b) { return b * a; }

template<typename T> detail::oSizeT<T> operator+(const detail::oSizeT<T>& a, const detail::oSizeT<T>& b) { return a.operator+(b); }
template<typename T, typename U> detail::oSizeT<T> operator+(const U& a, const detail::oSizeT<T>& b) { return b + a; }

template<typename T, typename U> bool operator<(const U& a, const detail::oSizeT<T>& b) { return !(b >= a); }
template<typename T, typename U> bool operator<=(const U& a, const detail::oSizeT<T>& b) { return !(b > a); }
template<typename T, typename U> bool operator>(const U& a, const detail::oSizeT<T>& b) { return !(b <= a); }
template<typename T, typename U> bool operator>=(const U& a, const detail::oSizeT<T>& b) { return !(b < a); }

template<typename T> bool operator<(const detail::oSizeT<T>& a, const detail::oSizeT<T>& b) { return a.operator<(b); }
template<typename T> bool operator<=(const detail::oSizeT<T>& a, const detail::oSizeT<T>& b) { return a.operator<=(b); }
template<typename T> bool operator>(const detail::oSizeT<T>& a, const detail::oSizeT<T>& b) { return a.operator>(b); }
template<typename T> bool operator>=(const detail::oSizeT<T>& a, const detail::oSizeT<T>& b) { return a.operator>=(b); }

typedef detail::oSizeT<unsigned short> oSize16;
typedef detail::oSizeT<unsigned int> oSize32;
typedef detail::oSizeT<unsigned long long> oSize64;

#endif
