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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// for C++ HLSL simple intrinsic types.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLTypes_h
#define oHLSLTypes_h

#ifndef oHLSL

	#include <oHLSL/oHLSLTypesInternal.h>

	// half is becoming less relevant these days, so since it's an external 
	// dependency allow for compilation to ignore the issue.

	#ifndef oHLSL_USE_HALF
		#define oHLSL_USE_HALF 1	
	#endif

	#if oHLSL_USE_HALF != 0
		#include <half.h>
		namespace std {
			template<> struct is_floating_point<half> : true_type {};
			// @oooii-tony: Should we spoof is_class to false here?
		}
	#endif

	typedef unsigned int uint;

	typedef TVEC2<int> int2; typedef TVEC2<uint> uint2;
	typedef TVEC3<int> int3; typedef TVEC3<uint> uint3;
	typedef TVEC4<int> int4; typedef TVEC4<uint> uint4;

	typedef TVEC2<float> float2; typedef TVEC2<double> double2;
	typedef TVEC3<float> float3; typedef TVEC3<double> double3;
	typedef TVEC4<float> float4; typedef TVEC4<double> double4;

	typedef TMAT3<float> float3x3; typedef TMAT3<double> double3x3;
	typedef TMAT4<float> float4x4; typedef TMAT4<double> double4x4;

	template<typename T> struct is_hlsl
	{
		static const bool value = 
			std::is_floating_point<T>::value ||
			std::is_same<bool,std::remove_cv<T>::type>::value ||
			std::is_same<int,std::remove_cv<T>::type>::value ||
			std::is_same<uint,std::remove_cv<T>::type>::value ||
			std::is_same<int2,std::remove_cv<T>::type>::value ||
			std::is_same<int3,std::remove_cv<T>::type>::value ||
			std::is_same<int4,std::remove_cv<T>::type>::value ||
			std::is_same<uint2,std::remove_cv<T>::type>::value ||
			std::is_same<uint3,std::remove_cv<T>::type>::value ||
			std::is_same<uint4,std::remove_cv<T>::type>::value ||
			std::is_same<float2,std::remove_cv<T>::type>::value ||
			std::is_same<float3,std::remove_cv<T>::type>::value ||
			std::is_same<float4,std::remove_cv<T>::type>::value ||
			std::is_same<double2,std::remove_cv<T>::type>::value ||
			std::is_same<double3,std::remove_cv<T>::type>::value ||
			std::is_same<double4,std::remove_cv<T>::type>::value ||
			std::is_same<float3x3,std::remove_cv<T>::type>::value ||
			std::is_same<float4x4,std::remove_cv<T>::type>::value ||
			std::is_same<double3x3,std::remove_cv<T>::type>::value ||
			std::is_same<double4x4,std::remove_cv<T>::type>::value;
	};

#endif
#endif
