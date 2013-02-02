/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Define integer types that do robust error checking for overflow and type 
// incompatibility risks. Prefer these to raw types as much as possible, 
// especially when dealing with always-64-bit sizes like file sizes or size_t
// as is ubiquitous in STL.
#pragma once
#ifndef oInt_h
#define oInt_h

#include "SafeInt3.h"
#include <type_traits>

typedef SafeInt<char> oChar;
typedef SafeInt<unsigned char> oUChar;
typedef SafeInt<short> oShort;
typedef SafeInt<unsigned short> oUShort;
typedef SafeInt<int> oInt;
typedef SafeInt<unsigned int> oUInt;
typedef SafeInt<long> oLong;
typedef SafeInt<unsigned long> oULong;
typedef SafeInt<long long> oLLong;
typedef SafeInt<unsigned long long> oULLong;

typedef SafeInt<size_t> oSizeT;

namespace std {

template<class _Ty>
struct _Arithmetic_traits<SafeInt<_Ty> >
	: _Arithmetic_traits<_Ty>
{
};

template<class _Ty>
struct is_integral<SafeInt<_Ty> >
	: _Is_integral<typename remove_cv<_Ty>::type>
{	// determine whether _Ty is integral
};

template<class _Ty>
struct tr1::_Has_signed_vals<SafeInt<_Ty> >
	: _Cat_base<(typename remove_cv<_Ty>::type)(-1)
	< (typename remove_cv<_Ty>::type)(0)>
{	// integral type can represent negative values
};

template<class _Ty0, class _Ty1>
struct common_type<SafeInt<_Ty0>, SafeInt<_Ty1> >
{	// determine common type of arithmetic types _Ty0 and _Ty1
	typedef typename _Ipromo<_Ty0>::_Type _PromoTy0;
	typedef typename _Ipromo<_Ty1>::_Type _PromoTy1;
	typedef typename _Common_type<_PromoTy0, _PromoTy1,
		_Arithmetic_traits<_PromoTy0>::_Rank,
		_Arithmetic_traits<_PromoTy1>::_Rank>::_Type type;
};

} // namespace std

#endif
