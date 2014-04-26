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
// This header contains defines and workarounds for specific hardware/platform/
// compiler combinations.
#pragma once
#ifndef oBase_config_h
#define oBase_config_h

#include <oBase/memcfg.h>

#if defined(_DEBUG) && defined(NDEBUG)
	#error both _DEBUG and NDEBUG are defined
#endif

#if defined(_DEBUG) || (!defined(oDEBUG) && !defined(NDEBUG))
	#define oDEBUG 1
#endif

#ifdef _MSC_VER

	// @todo: replace usage with alignas and memcfg values
	#define oALIGN(amount) __declspec(align(amount))
	#define oCACHE_ALIGNED(_Declaration) \
		__pragma(warning(disable:4324)) /* structure was padded due to _declspec(align()) */ \
		oALIGN(oCACHE_LINE_SIZE) _Declaration; \
		__pragma(warning(default:4324))
	
	#if _MSC_VER >= 1600
		#define oHAS_AUTO 1
		#define oHAS_NULLPTR 1
		#define oHAS_STATIC_ASSERT 1
		#define oHAS_MOVE_CTOR 1
	#endif

	#ifdef _WIN64
	#else
		#define oHAS_DOUBLE_WIDE_ATOMIC_BUG 1
	#endif

	#ifdef _WIN32
		// http://www.gamedev.net/topic/591975-c-unsigned-__int64-tofrom-double-conversions/
		#define oHAS_BAD_DOUBLE_TO_ULLONG_CONVERSION
	#endif

	// @tony: there needs to be more #ifdef'ing of when this is true, but I 
	// don't have test platforms at the moment to ensure this is pre-vista 
	// compatible (and I don't have an answer of what to do when I don't have 
	// 64-bit atomics yet)
	#define oHAS_64BIT_ATOMICS

	#define oASSUME(x) __assume(x)
	#define oDEBUG_BREAK() __debugbreak()
	#define oFORCEINLINE __forceinline
	#define oNOOP __noop
	#define oRESTRICT __restrict
	#define oSELECTANY __declspec(selectany)

	#pragma warning(disable:4481) // nonstandard extension used: override specifier 'override'

	#if _MSC_VER < 1700 // VS2012 has this c++11 feature. but sealed in VS2010 does the same thing.
		#define final sealed
		#define oHAS_DXSDK 1
	#elif _MSC_VER < 1800
		#define oHAS_MEMBER_DELETE 1
		#define oHAS_MAKE_EXCEPTION_PTR 1
	#else
		#define oHAS_MEMBER_DELETE 1
		#define oHAS_NOEXCEPT 1
		#define oHAS_THREAD_LOCAL 1
		#define oHAS_MAKE_EXCEPTION_PTR 1
	#endif

#define oNEEDS_CBEGIN

#else
	#define override
#endif

#ifndef oHAS_NULLPTR
	#define nullptr NULL
#endif

#ifndef oHAS_ALIGNAS
	#define alignas(x) __declspec(align(x))
#endif

#ifndef oHAS_ALIGNOF
	#define alignof __alignof
#endif

#ifdef oHAS_NOEXCEPT
	#define oNOEXCEPT noexcept
#else
	#define oNOEXCEPT
#endif

#ifndef oASSUME
	#define oASSUME(x) do { if (!(x)) oDEBUG_BREAK(); } while(false)
#endif

#ifndef oNOOP
	#define oNOOP do {} while(false)
#endif

#ifndef oHAS_THREAD_LOCAL
	#ifdef _MSC_VER
		// @tony: This is negligent: VS2012 enforces that no keyword is macro-ized, but then doesn't
		// support thread_local. How dare they prevent override of something that does't exist.
		// My solution: comment out the broken and inappropriate code in VS2012's xkeycheck.h because
		// more of the code remains compliant rather than endlessly creating wrapper macros for supposed
		// language intrinsics.
		#define thread_local __declspec(thread)
	#else
		#error unsupported target
	#endif
#endif

#ifndef oHAS_MAKE_EXCEPTION_PTR
	#include <exception>
	namespace std { inline exception_ptr make_exception_ptr(exception e) { return copy_exception(e); } }
#endif

#if !defined(oHAS_MOVE_CTOR) || oHAS_MOVE_CTOR != 1
	#error This code has not been tested on compilers without move ctors. In addition to functionality concern, there may be race conditions in concurrency implementations that might no longer explicitly call an object's destructor.
#endif

#endif
