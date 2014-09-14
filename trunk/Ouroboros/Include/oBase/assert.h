// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// assert() by itself doesn't organize enough debug information. It also breaks
// inside utility functions rather than the assertion itself. To present a 
// richer, more convenient assertion experience use the macros defined here.

#pragma once
#include <oCompiler.h>
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

namespace ouro {

	namespace assert_type { enum value { trace, assertion }; } 
	namespace assert_action { enum value { abort, debug, ignore, ignore_always }; } 

	struct assert_context
	{
		const char* expression;
		const char* function;
		const char* filename;
		int line;
		assert_type::value type;
		assert_action::value default_response;
	};

	// This function is inserted by oAssert macros.
	// functionality.
	extern assert_action::value vtracef(const assert_context& _Assertion, const char* _Format, va_list _Args);
	inline assert_action::value tracef(const assert_context& _Assertion, const char* _Format, ...) { va_list args; va_start(args, _Format); ouro::assert_action::value action = vtracef(_Assertion, _Format, args); va_end(args); return action; }

}

// _____________________________________________________________________________
// Main macro wrapper for print callback that ensures a break points to the 
// assertion itself rather than inside some debug function.
#if oENABLE_ASSERTS == 1 || oENABLE_RELEASE_ASSERTS == 1 || oENABLE_TRACES == 1 || oENABLE_RELEASE_TRACES == 1
	#define oASSERT_TRACE(_Type, _DefaultResponse, _StrCondition, _Format, ...) do \
	{	static bool oAssert_IgnoreFuture = false; \
		if (!oAssert_IgnoreFuture) \
		{	ouro::assert_context assertion__; assertion__.expression = _StrCondition; assertion__.function = __FUNCTION__; assertion__.filename = __FILE__; assertion__.line = __LINE__; assertion__.type = _Type; assertion__.default_response = _DefaultResponse; \
			ouro::assert_action::value action__ = ouro::tracef(assertion__, _Format "\n", ## __VA_ARGS__); \
			switch (action__) { case ouro::assert_action::abort: abort(); break; case ouro::assert_action::debug: oDEBUGBREAK; break; case ouro::assert_action::ignore_always: oAssert_IgnoreFuture = true; break; default: break; } \
		} \
	} while(false)
#endif

// _____________________________________________________________________________
// Always-macros (debug or release)

#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
	#define oASSERTA(_Condition, _Format, ...) do { if (!(_Condition)) { oASSERT_TRACE(ouro::assert_type::assertion, ouro::assert_action::abort, #_Condition, _Format, ## __VA_ARGS__); } } while(false)
#else
	#define oASSERTA(_Format, ...) do {} while(false)
#endif

#if oENABLE_RELEASE_TRACES == 1 || oENABLE_TRACES == 1
	#define oTRACEA(_Format, ...) oASSERT_TRACE(ouro::assert_type::trace, ouro::assert_action::ignore, "", _Format, ## __VA_ARGS__)
	#define oTRACEA_ONCE(_Format, ...) oASSERT_TRACE(ouro::assert_type::trace, ouro::assert_action::ignore_always, "", _Format, ## __VA_ARGS__)
#else
	#define oTRACEA(_Format, ...) do {} while(false)
	#define oTRACEA_ONCE(_Format, ...) do {} while(false)
#endif

// _____________________________________________________________________________
// Debug-only macros

#if oENABLE_ASSERTS == 1
	#define oASSERT(_Condition, _Format, ...) do { if (!(_Condition)) { oASSERT_TRACE(ouro::assert_type::assertion, ouro::assert_action::abort, #_Condition, _Format, ## __VA_ARGS__); } } while(false)
#else
	#define oASSERT(_Format, ...) do {} while(false)
#endif

#if oENABLE_TRACES == 1
	#define oTRACE(_Format, ...) oASSERT_TRACE(ouro::assert_type::trace, ouro::assert_action::ignore, "", _Format, ## __VA_ARGS__)
	#define oTRACE_ONCE(_Format, ...) oASSERT_TRACE(ouro::assert_type::trace, ouro::assert_action::ignore_always, "", _Format, ## __VA_ARGS__)
#else
	#define oTRACE(_Format, ...) do {} while(false)
	#define oTRACE_ONCE(_Format, ...) do {} while(false)
#endif
