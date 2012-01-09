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
// VC90 must be supported, so wrap type_traits and add some implementations that
// VC() is missing. Also I don't fully understand how to apply common_type to
// some chrono objects, so include one below that works for my needs right now.
#pragma once
#ifndef oTypeTraits_h
#define oTypeTraits_h

#include <oBasis/oPlatformFeatures.h>
#include <type_traits>

#ifndef oHAS_TYPE_TRAITS
namespace std {
template<typename T> struct make_signed { typedef T type; };
template<> struct make_signed<unsigned char> { typedef char type; };
template<> struct make_signed<unsigned short> { typedef short type; };
template<> struct make_signed<unsigned int> { typedef int type; };
template<> struct make_signed<unsigned long> { typedef long type; };
template<> struct make_signed<unsigned long long> { typedef long long type; };
} // namespace std

#endif

namespace oStd {

// @oooii-tony: We can't use decltype yet because of VC9.0 support, and I don't
// want to type all this out, so only support where A == B.

// @oooii-tony: Really, we should be able to use type_trait's std::common_type,
// but I don't quite understand it enough yet, so use this all the time for now
// until I have time to look at it more closely.

template<typename A, typename B = void> struct common_type { typedef A type; };

#define oCMNT1(_AType) template<> struct common_type<_AType,void> { typedef _AType type; };
#define oCMNT2(_AType, _BType) template<> struct common_type<_AType,_BType> { typedef _AType type; }; template<> struct common_type<_BType,_AType> { typedef _AType type; };
oCMNT1(bool) oCMNT2(char,bool) oCMNT2(short,bool) oCMNT2(int,bool) oCMNT2(long,bool) oCMNT2(long long,bool) oCMNT2(float,bool) oCMNT2(double,bool)
	oCMNT1(char) oCMNT2(short,char) oCMNT2(int,char) oCMNT2(long,char) oCMNT2(long long,char) oCMNT2(float,char) oCMNT2(double,char)
	oCMNT1(short) oCMNT2(int,short) oCMNT2(long,short) oCMNT2(long long,short) oCMNT2(float,short) oCMNT2(double,short)
	oCMNT1(int) oCMNT2(long,int) oCMNT2(long long,int) oCMNT2(float,int) oCMNT2(double,int)
	oCMNT1(long) oCMNT2(long long,long) oCMNT2(float,long) oCMNT2(double,long)
	oCMNT1(long long) oCMNT2(float,long long) oCMNT2(double, long long)
	oCMNT1(float) oCMNT2(double,float)
	oCMNT1(double)
#undef oCMNT2
#undef oCMNT1

} // namespace oStd

#endif
