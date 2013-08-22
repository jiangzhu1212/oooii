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

// assert() by itself doesn't organize enough debug information. It also breaks
// inside utility functions rather than the assertion itself. To present a 
// richer, more convenient assertion experience use the macros defined here.
#pragma once
#ifndef oStd_assert_h
#define oStd_assert_h

#include <oStd/config.h>
#include <cstdarg>
#include <cstdlib>

#ifndef oENABLE_RELEASE_ASSERTS
	#define oENABLE_RELEASE_ASSERTS 0
#endif

#ifndef oENABLE_ASSERTS
	#ifdef oDEBUG
		#define oENABLE_ASSERTS 1
	#else
		#define oENABLE_ASSERTS 0
	#endif
#endif

#ifndef oENABLE_RELEASE_TRACES
	#define oENABLE_RELEASE_TRACES oENABLE_RELEASE_ASSERTS
#endif

#ifndef oENABLE_TRACES
	#define oENABLE_TRACES oENABLE_ASSERTS
#endif

namespace oStd {

	namespace assert_type { enum value { trace, assertion }; } 
	namespace assert_action { enum value { abort, debug, ignore, ignore_always }; } 

	struct assert_context
	{
		const char* Expression;
		const char* Function;
		const char* Filename;
		int Line;
		assert_type::value Type;
		assert_action::value DefaultResponse;
	};

	// This function is inserted by oAssert macros.
	// functionality.
	extern assert_action::value vtracef(const assert_context& _Assertion, const char* _Format, va_list _Args);
	inline assert_action::value tracef(const assert_context& _Assertion, const char* _Format, ...) { va_list args; va_start(args, _Format); oStd::assert_action::value action = vtracef(_Assertion, _Format, args); va_end(args); return action; }

} // namespace oStd

// _____________________________________________________________________________
// Main macro wrapper for print callback that ensures a break points to the 
// assertion itself rather than inside some debug function.
#if oENABLE_ASSERTS == 1 || oENABLE_RELEASE_ASSERTS == 1 || oENABLE_TRACES == 1 || oENABLE_RELEASE_TRACES == 1
	#define oASSERT_TRACE(_Type, _DefaultResponse, _StrCondition, _Format, ...) do \
	{	static bool oAssert_IgnoreFuture = false; \
		if (!oAssert_IgnoreFuture) \
		{	oStd::assert_context assertion__; assertion__.Expression = _StrCondition; assertion__.Function = __FUNCTION__; assertion__.Filename = __FILE__; assertion__.Line = __LINE__; assertion__.Type = _Type; assertion__.DefaultResponse = _DefaultResponse; \
			oStd::assert_action::value action__ = oStd::tracef(assertion__, _Format "\n", ## __VA_ARGS__); \
			switch (action__) { case oStd::assert_action::abort: abort(); break; case oStd::assert_action::debug: oDEBUG_BREAK(); break; case oStd::assert_action::ignore_always: oAssert_IgnoreFuture = true; break; default: break; } \
		} \
	} while(false)
#endif

// _____________________________________________________________________________
// Always-macros (debug or release)

#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
	#define oASSERTA(_Condition, _Format, ...) do { if (!(_Condition)) { oASSERT_TRACE(oStd::assert_type::assertion, oStd::assert_action::abort, #_Condition, _Format, ## __VA_ARGS__); } } while(false)
#else
	#define oASSERTA(_Format, ...) oNOOP
#endif

#if oENABLE_RELEASE_TRACES == 1 || oENABLE_TRACES == 1
	#define oTRACEA(_Format, ...) oASSERT_TRACE(oStd::assert_type::trace, oStd::assert_action::ignore, "", _Format, ## __VA_ARGS__)
	#define oTRACEA_ONCE(_Format, ...) oASSERT_TRACE(oStd::assert_type::trace, oStd::assert_action::ignore_always, "", _Format, ## __VA_ARGS__)
#else
	#define oTRACEA(_Format, ...) oNOOP
	#define oTRACEA_ONCE(_Format, ...) oNOOP
#endif

// _____________________________________________________________________________
// Debug-only macros

#if oENABLE_ASSERTS == 1
	#define oASSERT(_Condition, _Format, ...) do { if (!(_Condition)) { oASSERT_TRACE(oStd::assert_type::assertion, oStd::assert_action::abort, #_Condition, _Format, ## __VA_ARGS__); } } while(false)
#else
	#define oASSERT(_Format, ...) oNOOP
#endif

#if oENABLE_TRACES == 1
	#define oTRACE(_Format, ...) oASSERT_TRACE(oStd::assert_type::trace, oStd::assert_action::ignore, "", _Format, ## __VA_ARGS__)
	#define oTRACE_ONCE(_Format, ...) oASSERT_TRACE(oStd::assert_type::trace, oStd::assert_action::ignore_always, "", _Format, ## __VA_ARGS__)
#else
	#define oTRACE(_Format, ...) oNOOP
	#define oTRACE_ONCE(_Format, ...) oNOOP
#endif

#endif
