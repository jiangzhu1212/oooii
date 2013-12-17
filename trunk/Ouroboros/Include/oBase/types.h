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
// Make HLSL a part of base C++ and define other types consistently with HLSL
// naming conventions.
#pragma once
#ifndef oBase_types_h
#define oBase_types_h

#include <oHLSL/oHLSLTypes.h>
#include <oBase/uint128.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long llong;
typedef unsigned long long ullong;

typedef TVEC2<char> char2; typedef TVEC2<uchar> uchar2;
typedef TVEC3<char> char3; typedef TVEC3<uchar> uchar3;
typedef TVEC4<char> char4; typedef TVEC4<uchar> uchar4;

typedef TVEC2<short> short2; typedef TVEC2<ushort> ushort2;
typedef TVEC3<short> short3; typedef TVEC3<ushort> ushort3;
typedef TVEC4<short> short4; typedef TVEC4<ushort> ushort4;

typedef TVEC2<llong> llong2; typedef TVEC2<ullong> ullong2;
typedef TVEC3<llong> llong3; typedef TVEC3<ullong> ullong3;
typedef TVEC4<llong> llong4; typedef TVEC4<ullong> ullong4;

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

#endif
