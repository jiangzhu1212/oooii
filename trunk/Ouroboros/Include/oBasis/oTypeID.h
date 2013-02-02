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
// Record a runtime generically accessible form of a type idenifier for either
// arithmetic or linear algebra types.
#pragma once
#ifndef oTypeID_h
#define oTypeID_h

#include <oBasis/oHLSLTypes.h>
#include <oBasis/oPlatformFeatures.h>
#include <type_traits>

enum oTYPE_ID
{
	oTYPE_UNKNOWN,

	// Arithmetic types
	// (Integral types)
	oTYPE_VOID,
	oTYPE_BOOL,
	oTYPE_CHAR,
	oTYPE_UCHAR,
	oTYPE_WCHAR,
	oTYPE_SHORT,
	oTYPE_USHORT,
	oTYPE_INT,
	oTYPE_UINT,
	oTYPE_LONG,
	oTYPE_ULONG,
	oTYPE_LLONG,
	oTYPE_ULLONG,
	// (Floating point types)
	oTYPE_FLOAT,
	oTYPE_DOUBLE,
	oTYPE_HALF,
	
	// Linear algebra types
	oTYPE_INT2,
	oTYPE_INT3,
	oTYPE_INT4,
	oTYPE_UINT2,
	oTYPE_UINT3,
	oTYPE_UINT4,
	oTYPE_FLOAT2,
	oTYPE_FLOAT3,
	oTYPE_FLOAT4,
	oTYPE_FLOAT4X4,

	oNUM_TYPES,
};

oAPI const char* oAsString(oTYPE_ID _TypeID);

oAPI bool oFromString(oTYPE_ID* _pTypeID, const char* _StrSource);
oAPI char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oTYPE_ID& _Value);

template<typename T> struct oTypeID
{
	static const oTYPE_ID value = 
		(std::is_void<std::remove_cv<T>::type>::value ? oTYPE_VOID : 
		(std::is_same<bool,std::remove_cv<T>::type>::value ? oTYPE_BOOL : 
		(std::is_same<char,std::remove_cv<T>::type>::value ? oTYPE_CHAR : 
		(std::is_same<unsigned char,std::remove_cv<T>::type>::value ? oTYPE_UCHAR : 
		(std::is_same<wchar_t,std::remove_cv<T>::type>::value ? oTYPE_WCHAR : 
		(std::is_same<short,std::remove_cv<T>::type>::value ? oTYPE_SHORT : 
		(std::is_same<unsigned short,std::remove_cv<T>::type>::value ? oTYPE_USHORT : 
		(std::is_same<int,std::remove_cv<T>::type>::value ? oTYPE_INT : 
		(std::is_same<unsigned int,std::remove_cv<T>::type>::value ? oTYPE_UINT : 
		(std::is_same<long,std::remove_cv<T>::type>::value ? oTYPE_LONG : 
		(std::is_same<unsigned long,std::remove_cv<T>::type>::value ? oTYPE_ULONG : 
		(std::is_same<long long,std::remove_cv<T>::type>::value ? oTYPE_LLONG : 
		(std::is_same<unsigned long long,std::remove_cv<T>::type>::value ? oTYPE_ULLONG : 
		(std::is_same<half,std::remove_cv<T>::type>::value ? oTYPE_HALF : 
		(std::is_same<float,std::remove_cv<T>::type>::value ? oTYPE_FLOAT : 
		(std::is_same<double,std::remove_cv<T>::type>::value ? oTYPE_DOUBLE : 
		(std::is_same<int2,std::remove_cv<T>::type>::value ? oTYPE_INT2 : 
		(std::is_same<int3,std::remove_cv<T>::type>::value ? oTYPE_INT3 : 
		(std::is_same<int4,std::remove_cv<T>::type>::value ? oTYPE_INT4 : 
		(std::is_same<uint2,std::remove_cv<T>::type>::value ? oTYPE_UINT2 : 
		(std::is_same<uint3,std::remove_cv<T>::type>::value ? oTYPE_UINT3 : 
		(std::is_same<uint4,std::remove_cv<T>::type>::value ? oTYPE_UINT4 : 
		(std::is_same<float2,std::remove_cv<T>::type>::value ? oTYPE_FLOAT2 : 
		(std::is_same<float3,std::remove_cv<T>::type>::value ? oTYPE_FLOAT3 : 
		(std::is_same<float4,std::remove_cv<T>::type>::value ? oTYPE_FLOAT4 : 
		(std::is_same<float4x4,std::remove_cv<T>::type>::value ? oTYPE_FLOAT4X4 : 
		oTYPE_UNKNOWN))))))))))))))))))))))))));
};

template<typename T> struct is_type_id
{
	static const bool value = std::is_arithmetic<T>::value || std::is_void<T>::value || is_linear_algebra<T>::value;
};

#endif
