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
// This header describes meta-information about the platform and defines certain
// keywords in a way all systems can use them, whether they're meaningful on 
// the current platform or not (interface, override).
#pragma once
#ifndef oPlatformFeatures_h
#define oPlatformFeatures_h

#ifdef _MSC_VER
	#pragma warning(disable:4481) // nonstandard extension used: override specifier 'override'
	#define oHAS_ASSUME

	#if _MSC_VER >= 1600
		#define oHAS_AUTO
		#define oHAS_NULLPTR
		#define oHAS_STATIC_ASSERT
		#define oHAS_MOVE_CTOR
		#define oHAS_8BIT_ATOMICS
		#define	oHAS_16BIT_ATOMICS
		#define oHAS_TYPE_TRAITS
	#else
		#define oCLICKABLE_OUTPUT_REQUIRES_SPACE_COLON
	#endif

	#define oLITTLEENDIAN

	#ifdef _WIN64
		#define oPOINTERSIZE 8
		#define oDEFAULT_MEMORY_ALIGNMENT 16
		#define o64BIT 1
	#else
		#define oPOINTERSIZE 4
		#define oDEFAULT_MEMORY_ALIGNMENT 8
		#define o32BIT 1
	#endif

	#ifdef _WIN32
		// http://www.gamedev.net/topic/591975-c-unsigned-__int64-tofrom-double-conversions/
		#define oHAS_BAD_DOUBLE_TO_ULLONG_CONVERSION
	#endif

	#define oRESTRICT __restrict
	#define oFORCEINLINE __forceinline
	#define oALIGN(amount) __declspec(align(amount))

	// @oooii-tony: there needs to be more #ifdef'ing of when this is true, but I 
	// don't have test platforms at the moment to ensure this is pre-vista 
	// compatible (and I don't have an answer of what to do when I don't have 
	// 64-bit atomics yet)
	#define oHAS_64BIT_ATOMICS

#else
	#define override
#endif

#ifdef interface
	#define INTERFACE_DEFINED
#endif

#ifndef INTERFACE_DEFINED
	#ifdef _MSC_VER
		#define interface struct __declspec(novtable)
	#else
		#define interface struct
	#endif
	#define INTERFACE_DEFINED
#endif

#ifndef oHAS_NULLPTR
	#define nullptr NULL
#endif

#ifndef oHAS_THREAD_LOCAL
	#ifdef _MSC_VER
		#define thread_local __declspec(thread)
	#else
		#error Unsupported platform
	#endif
#endif

// Enable this in the compiler command line for debugging link/declaration issues
#ifndef oENABLE_BUILD_TRACES
	#define oENABLE_BUILD_TRACES 0
#endif

#ifndef oUSE_CLICKABLE_BUILD_TRACES
	#define oUSE_CLICKABLE_BUILD_TRACES 0
#endif

#if oENABLE_BUILD_TRACES == 0 
	#define oBUILD_TRACE(msg)
#else
	#if oUSE_CLICKABLE_BUILD_TRACES == 0
		#define oBUILD_TRACE(msg) __pragma(message("BUILD TRACE: " msg))
	#else
		#define oBUILD_TRACE(msg) __pragma(message(__FILE__ "(" #__LINE__ ") : BUILD TRACE: " msg))
	#endif
#endif

#if defined(_DLL) && !defined(oSTATICLIB)
	#ifdef _EXPORT_SYMBOLS
		oBUILD_TRACE("oAPI exporting dynamic module symbols")
		#define oAPI __declspec(dllexport)
	#else
		oBUILD_TRACE("oAPI importing dynamic module symbols")
		#define oAPI __declspec(dllimport)
	#endif
#else
	oBUILD_TRACE("oAPI linking static module symbols")
	#define oAPI
#endif

typedef unsigned char uchar;

#define oMODULE_DEBUG_PREFIX_A "DEBUG-"
#define oMODULE_DEBUG_SUFFIX_A "D"

#ifdef _DEBUG
	#define oMODULE_DEBUG_PREFIX oMODULE_DEBUG_PREFIX_A
	#define oMODULE_DEBUG_SUFFIX oMODULE_DEBUG_SUFFIX_A
#else
	#define oMODULE_DEBUG_PREFIX ""
	#define oMODULE_DEBUG_SUFFIX ""
#endif

#ifdef oHAS_BAD_DOUBLE_TO_ULLONG_CONVERSION
	#include <memory.h>
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
inline unsigned long long oDtoULL(double input)
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

#endif

#endif
