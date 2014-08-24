/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// Make HLSL a part of base C++ and define other types consistently with HLSL
// naming conventions.
#pragma once
#ifndef oBase_types_h
#define oBase_types_h

#include <oHLSL/oHLSLTypes.h>
#include <oMemory/equal.h>
#include <oBase/uint128.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

typedef TVEC2<char> char2; typedef TVEC2<uchar> uchar2;
typedef TVEC3<char> char3; typedef TVEC3<uchar> uchar3;
typedef TVEC4<char> char4; typedef TVEC4<uchar> uchar4;

typedef TVEC2<short> short2; typedef TVEC2<ushort> ushort2;
typedef TVEC3<short> short3; typedef TVEC3<ushort> ushort3;
typedef TVEC4<short> short4; typedef TVEC4<ushort> ushort4;

typedef TVEC2<long> long2; typedef TVEC2<ulong> ulong2;
typedef TVEC3<long> long3; typedef TVEC3<ulong> ulong3;
typedef TVEC4<long> long4; typedef TVEC4<ulong> ulong4;

typedef TVEC2<llong> llong2; typedef TVEC2<ullong> ullong2;
typedef TVEC3<llong> llong3; typedef TVEC3<ullong> ullong3;
typedef TVEC4<llong> llong4; typedef TVEC4<ullong> ullong4;

typedef TVEC2<half> half2;
typedef TVEC3<half> half3;
typedef TVEC4<half> half4;

template<typename U, typename T> U as_type(const T& _X) { U X = (U)_X; T X2 = T(X); if (X2 != _X) throw std::bad_cast(); return X; }
template<typename T> char as_char(const T& _X) { return as_type<char>(_X); }
template<typename T> uchar as_uchar(const T& _X) { return as_type<uchar>(_X); }
template<typename T> short as_short(const T& _X) { return as_type<short>(_X); }
template<typename T> ushort as_ushort(const T& _X) { return as_type<ushort>(_X); }
template<typename T> int as_int(const T& _X) { return as_type<int>(_X); }
template<typename T> uint as_uint(const T& _X) { return as_type<uint>(_X); }
template<typename T> long as_long(const T& _X) { return as_type<long>(_X); }
template<typename T> ulong as_ulong(const T& _X) { return as_type<ulong>(_X); }
template<typename T> llong as_llong(const T& _X) { return as_type<llong>(_X); }
template<typename T> ullong as_ullong(const T& _X) { return as_type<ullong>(_X); }
template<typename T> size_t as_size_t(const T& _X) { return as_type<size_t>(_X); }

// the static value is true if the specified type is a linear algebra type
// this is modeled after std::is_arithmetic for the linear algrebra types found
// in shader/compute languages as well as appearing commonly in Ouroboros C++.
template<typename T> struct is_linear_algebra
{
	static const bool value = 
		std::is_same<char2,std::remove_cv<T>::type>::value ||
		std::is_same<char3,std::remove_cv<T>::type>::value ||
		std::is_same<char4,std::remove_cv<T>::type>::value ||
		std::is_same<uchar2,std::remove_cv<T>::type>::value ||
		std::is_same<uchar3,std::remove_cv<T>::type>::value ||
		std::is_same<uchar4,std::remove_cv<T>::type>::value ||
		std::is_same<short2,std::remove_cv<T>::type>::value ||
		std::is_same<short3,std::remove_cv<T>::type>::value ||
		std::is_same<short4,std::remove_cv<T>::type>::value ||
		std::is_same<ushort2,std::remove_cv<T>::type>::value ||
		std::is_same<ushort3,std::remove_cv<T>::type>::value ||
		std::is_same<ushort4,std::remove_cv<T>::type>::value ||
		std::is_same<int2,std::remove_cv<T>::type>::value ||
		std::is_same<int3,std::remove_cv<T>::type>::value ||
		std::is_same<int4,std::remove_cv<T>::type>::value ||
		std::is_same<uint2,std::remove_cv<T>::type>::value ||
		std::is_same<uint3,std::remove_cv<T>::type>::value ||
		std::is_same<uint4,std::remove_cv<T>::type>::value ||
		std::is_same<llong2,std::remove_cv<T>::type>::value ||
		std::is_same<llong3,std::remove_cv<T>::type>::value ||
		std::is_same<llong4,std::remove_cv<T>::type>::value ||
		std::is_same<ullong2,std::remove_cv<T>::type>::value ||
		std::is_same<ullong3,std::remove_cv<T>::type>::value ||
		std::is_same<ullong4,std::remove_cv<T>::type>::value ||
		std::is_same<float2,std::remove_cv<T>::type>::value ||
		std::is_same<float3,std::remove_cv<T>::type>::value ||
		std::is_same<float4,std::remove_cv<T>::type>::value ||
		std::is_same<float4x4,std::remove_cv<T>::type>::value;
};

// _____________________________________________________________________________
// ouro::equal support

namespace ouro {

template<> inline bool equal(const TVEC2<float>& a, const TVEC2<float>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps); }
template<> inline bool equal(const TVEC3<float>& a, const TVEC3<float>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps) && equal(a.z, b.z, maxUlps); }
template<> inline bool equal(const TVEC4<float>& a, const TVEC4<float>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps) && equal(a.z, b.z, maxUlps) && equal(a.w, b.w, maxUlps); }
template<> inline bool equal(const TMAT3<float>& a, const TMAT3<float>& b, unsigned int maxUlps) { return equal(a.Column0, b.Column0, maxUlps) && equal(a.Column1, b.Column1, maxUlps) && equal(a.Column2, b.Column2, maxUlps); }
template<> inline bool equal(const TMAT4<float>& a, const TMAT4<float>& b, unsigned int maxUlps) { return equal(a.Column0, b.Column0, maxUlps) && equal(a.Column1, b.Column1, maxUlps) && equal(a.Column2, b.Column2, maxUlps) && equal(a.Column3, b.Column3, maxUlps); }
template<> inline bool equal(const TVEC2<double>& a, const TVEC2<double>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps); }
template<> inline bool equal(const TVEC3<double>& a, const TVEC3<double>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps) && equal(a.z, b.z, maxUlps); }
template<> inline bool equal(const TVEC4<double>& a, const TVEC4<double>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps) && equal(a.z, b.z, maxUlps) && equal(a.w, b.w, maxUlps); }
template<> inline bool equal(const TMAT3<double>& a, const TMAT3<double>& b, unsigned int maxUlps) { return equal(a.Column0, b.Column0, maxUlps) && equal(a.Column1, b.Column1, maxUlps) && equal(a.Column2, b.Column2, maxUlps); }
template<> inline bool equal(const TMAT4<double>& a, const TMAT4<double>& b, unsigned int maxUlps) { return equal(a.Column0, b.Column0, maxUlps) && equal(a.Column1, b.Column1, maxUlps) && equal(a.Column2, b.Column2, maxUlps) && equal(a.Column3, b.Column3, maxUlps); }

}

#endif
