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

// assert() by itself doesn't organize enough debug information and doesn't even
// break to the assertion itself, but instead inside utility functions. To 
// present a richer, more convenient assertion experience, use the macros 
// defined here.
#pragma once
#ifndef oAssert_h
#define oAssert_h

#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oHash.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef oENABLE_RELEASE_ASSERTS
	#define oENABLE_RELEASE_ASSERTS 0
#endif

#ifndef oENABLE_ASSERTS
	#ifdef _DEBUG
		#define oENABLE_ASSERTS 1
	#endif
#endif

#ifndef oHAS_STATIC_ASSERT
	#define static_assert(_Condition, _StrLiteral) /*_STATIC_ASSERT(_Condition)*/
#endif

#if defined(_WIN32) || defined(_WIN64)
	#define oDEBUGBREAK() __debugbreak()
	#define oASSERT_NOOP __noop
	#ifdef oHAS_ASSUME
	#define oASSERT_NOEXECUTION do { oASSERT(0, "Unexpected code execution"); __assume(0); } while(false)
	#else
		#define oASSERT_NOEXECUTION oASSERT(0, "Unexpected code execution")
	#endif
#else
	#error Unsupported platform
#endif

// Use this in switch statements where there is no valid default behavior. On 
// Some compilers this can generate more efficient code.
#define oNODEFAULT default: oASSERT_NOEXECUTION

enum oASSERT_TYPE
{
	oASSERT_TRACE,
	oASSERT_WARNING,
	oASSERT_ASSERTION,
};

enum oASSERT_ACTION
{
	oASSERT_ABORT,
	oASSERT_BREAK,
	oASSERT_IGNORE_ONCE,
	oASSERT_IGNORE_ALWAYS,
};

struct oASSERTION
{
	const char* Expression;
	const char* Function;
	const char* Filename;
	size_t ID;
	oASSERT_TYPE Type;
	oASSERT_ACTION DefaultResponse;
	int Line;
};

// Callback for handling an assertion/trace. All parameters derived from code
// are packed into the specified _Assertion.
typedef oASSERT_ACTION (*oAssertVPrintfFn)(const oASSERTION& _Assertion, const char* _Format, va_list _Args);

// Client code is required to implement this function for oASSERT functionality,
// this way all formatting and presentation can be made consistent with other 
// client reporting facilities.
extern oASSERT_ACTION oAssertVPrintf(const oASSERTION& _Assertion, const char* _Format, va_list _Args);

inline oASSERT_ACTION oAssertPrintf(const oASSERTION& _Assertion, const char* _Format, ...) { va_list args; va_start(args, _Format); oASSERT_ACTION action = oAssertVPrintf(_Assertion, _Format, args); va_end(args); return action; }

// _____________________________________________________________________________
// Main macro wrapper for print callback that ensures a break points to the 
// assertion itself rather than inside some debug function.
#if (defined(_DEBUG) && oENABLE_ASSERTS == 1) || oENABLE_RELEASE_ASSERTS == 1
	#define oASSERT_PRINT(_Type, _DefaultResponse, _StrCondition, _Format, ...) do \
	{	static bool oAssert_IgnoreFuture = false; \
		if (!oAssert_IgnoreFuture) \
		{	oASSERTION assertion__; assertion__.Expression = _StrCondition; assertion__.Function = __FUNCTION__; assertion__.Filename = __FILE__; assertion__.Line = __LINE__; assertion__.Type = _Type; assertion__.DefaultResponse = _DefaultResponse; assertion__.ID = oHash_stlp(_Format); \
			oASSERT_ACTION action__ = oAssertPrintf(assertion__, _Format "\n", ## __VA_ARGS__); \
			switch (action__) { case oASSERT_ABORT: abort(); break; case oASSERT_BREAK: oDEBUGBREAK(); break; case oASSERT_IGNORE_ALWAYS: oAssert_IgnoreFuture = true; break; default: break; } \
		} \
	} while(false)

#endif

// _____________________________________________________________________________
// Always-macros (debug or release)

#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
	#define oTRACEA(_Format, ...) oASSERT_PRINT(oASSERT_TRACE, oASSERT_IGNORE_ONCE, "", _Format, ## __VA_ARGS__)
	#define oTRACEA_ONCE(_Format, ...) oASSERT_PRINT(oASSERT_TRACE, oASSERT_IGNORE_ALWAYS, "", _Format, ## __VA_ARGS__)
	#define oWARNA(_Format, ...) oASSERT_PRINT(oASSERT_WARNING, oASSERT_IGNORE_ONCE, "", _Format, ## __VA_ARGS__)
	#define oWARNA_ONCE(_Format, ...) oASSERT_PRINT(oASSERT_WARNING, oASSERT_IGNORE_ALWAYS, "", _Format, ## __VA_ARGS__)
	#define oASSERTA(_Condition, _Format, ...) do { if (!(_Condition)) { oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_ABORT, #_Condition, _Format, ## __VA_ARGS__); } } while(false)
#else
	#define oTRACEA(_Format, ...) oASSERT_NOOP
	#define oTRACEA_ONCE(_Format, ...) oASSERT_NOOP
	#define oWARNA(_Format, ...) oASSERT_NOOP
	#define oWARNA_ONCE(_Format, ...) oASSERT_NOOP
	#define oASSERTA(_Format, ...) oASSERT_NOOP
#endif

// _____________________________________________________________________________
// Debug-only macros

#if oENABLE_ASSERTS == 1
	#define oTRACE(_Format, ...) oASSERT_PRINT(oASSERT_TRACE, oASSERT_IGNORE_ONCE, "", _Format, ## __VA_ARGS__)
	#define oTRACE_ONCE(_Format, ...) oASSERT_PRINT(oASSERT_TRACE, oASSERT_IGNORE_ALWAYS, "", _Format, ## __VA_ARGS__)
	#define oWARN(_Format, ...) oASSERT_PRINT(oASSERT_WARNING, oASSERT_IGNORE_ONCE, "", _Format, ## __VA_ARGS__)
	#define oWARN_ONCE(_Format, ...) oASSERT_PRINT(oASSERT_WARNING, oASSERT_IGNORE_ALWAYS, "", _Format, ## __VA_ARGS__)
	#define oASSERT(_Condition, _Format, ...) do { if (!(_Condition)) { oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_ABORT, #_Condition, _Format, ## __VA_ARGS__); } } while(false)
#else
	#define oTRACE(_Format, ...) oASSERT_NOOP
	#define oTRACE_ONCE(_Format, ...) oASSERT_NOOP
	#define oWARN(_Format, ...) oASSERT_NOOP
	#define oWARN_ONCE(_Format, ...) oASSERT_NOOP
	#define oASSERT(_Format, ...) oASSERT_NOOP
#endif

#endif
