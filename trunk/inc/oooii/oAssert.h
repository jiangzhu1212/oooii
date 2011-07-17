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

// This file contains typical "message only in debug" mode type macros. 
// oENABLE_ASSERTS can be defined in any build to have the macros do something
// meaningful. Various other highlights include:
// * macros that execute meaningfully in all builds
// * standard wrappers for oGetLastError()-style functions as well as Windows
//   GetLastError()-style and HRESULT-style functions.
// * MSVC-clickable messages
// * Various identifications for the message including process and thread ids as 
//   well as a unique id per message
// * Based on message ID, messages can be filtered/ignored programmatically.
// * The reporting function can be overridden by the user, however be warned,
//   the overriding function takes ALL responsibility for any data coming from
//   the original message in the form of an ASSERTION struct and DESC struct. 
//   If you want a  minor tweak, all the functionality of assembling prefixes
//   and directing output to logs, message boxes and the debugger will have to 
//   be recreated.
#pragma once
#ifndef oAssert_h
#define oAssert_h

#include <oooii/oErrno.h>
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

namespace oAssert
{
	enum TYPE
	{
		TYPE_TRACE,
		TYPE_WARNING,
		TYPE_ASSERT,
	};

	enum ACTION
	{
		ABORT,
		BREAK,
		IGNORE_ONCE,
		IGNORE_ALWAYS,
	};

	struct ASSERTION
	{
		const char* Expression;
		const char* Function;
		const char* Filename;
		TYPE Type;
		ACTION DefaultResponse;
		int Line;
		int MsgId;
	};

	typedef ACTION (*VPrintMessageFn)(const ASSERTION& _Assertion, void* _hLogFile, const char* _Format, va_list _Args);

	ACTION VPrintMessage(const ASSERTION& _Assertion, const char* _Format, va_list _Args);
	inline ACTION PrintMessage(const ASSERTION& _Assertion, const char* _Format, ...) { va_list args; va_start(args, _Format); return VPrintMessage(_Assertion, _Format, args); }

	struct DESC
	{
		DESC()
			: LogFilePath(0)
			, EnableWarnings(true)
			, PrintCallstack(true)
			, PrefixFileLine(true)
			, PrefixThreadId(true)
			, PrefixMsgType(true)
			, PrefixMsgId(false)
		{}

		// If specified and a valid, accessible file path, all output through 
		// PrintMessage should also be recorded to the specified file.
		const char* LogFilePath;
		bool EnableWarnings:1;
		bool PrintCallstack:1;
		bool PrefixFileLine:1;
		bool PrefixThreadId:1;
		bool PrefixMsgType:1;
		bool PrefixMsgId:1;
	};

	// If passed null, SetDesc() will restore default settings.
	// These are NOT threadsafe.
	void SetDesc(const DESC* _pDesc);
	void GetDesc(DESC* _pDesc);

	// Push and pops a user VPrintMessage function. By default there is a 
	// very low-level reporting system at the base used during early bootstrap
	// and a more robust reporting system that is pushed when all prerequisits
	// are initialized. This can return false if the push fails because the
	// stack is full.
	bool PushMessageHandler(VPrintMessageFn _VPrintMessage);
	VPrintMessageFn PopMessageHandler();

	// oAssert is useful even during static init/deinit. The singleton 
	// initializes on-demand well, but we need this to ensure against premature
	// deinitialization.
	void Reference();
	void Release();

	// Enable PrefixMsgId in the DESC to get the id of any annoying messages to 
	// be disabled.
	void AddMessageFilter(int _MsgId);
	void RemoveMessageFilter(int _MsgId);

	// Generate a message ID based on the constant string used in message macros
	// Mostly this is used internally by the macros below.
	int GetMsgId(const char* _Format);

} // namespace oAssert

// _____________________________________________________________________________
// Platform-specific macros

#if defined(_WIN32) || defined(_WIN64)
	#define oSTATICASSERT(cond) _STATIC_ASSERT(cond)
	#define oDEBUGBREAK() __debugbreak()
	#define oASSERT_NOOP __noop
	#define oASSERT_NOEXEC __assume(0)
#else
	#error Unsupported platform (oSTATICASSERT)
	#error Unsupported platform (oDEBUGBREAK)
	#error Unsupported platform (oASSERT_NOOP)
	#error Unsupported platform (oASSERT_NOEXEC)
#endif

// _____________________________________________________________________________
// Main entry point that ensures all __FILE__ and __LINE__ macros are 
// accurately expanded as well as ensuring that a break occurs on the instance 
// of the assert itself rather than inside a utility function.

#if (defined(_DEBUG) && oENABLE_ASSERTS == 1) || oENABLE_RELEASE_ASSERTS == 1

	#define oASSERT_PRINT_MESSAGE(type, defaultresponse, cond, msg, ...) do \
		{	static bool oAssert_IgnoreFuture = false; \
			if (!oAssert_IgnoreFuture) \
			{	oAssert::ASSERTION a; a.Expression = #cond; a.Function = __FUNCTION__; a.Filename = __FILE__; a.Line = __LINE__; a.Type = oAssert::type; a.DefaultResponse = defaultresponse; a.MsgId = oAssert::GetMsgId(msg); \
				oAssert::ACTION action__ = oAssert::PrintMessage(a, msg "\n", ## __VA_ARGS__); \
				switch (action__) { case oAssert::ABORT: ::exit(__LINE__); break; case oAssert::BREAK: oDEBUGBREAK(); break; case oAssert::IGNORE_ALWAYS: oAssert_IgnoreFuture = true; break; default: break; } \
			} \
		} while(0)

#endif

// _____________________________________________________________________________
// Always-macros (debug or release)

#if oENABLE_RELEASE_ASSERTS == 1 || oENABLE_ASSERTS == 1
	#define oTRACEA(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_TRACE, oAssert::IGNORE_ONCE, !, msg, ## __VA_ARGS__)
	#define oTRACEA_ONCE(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_TRACE, oAssert::IGNORE_ALWAYS, !, msg, ## __VA_ARGS__)
	#define oWARNA(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_WARNING, oAssert::IGNORE_ONCE, !, msg, ## __VA_ARGS__)
	#define oWARNA_ONCE(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_WARNING, oAssert::IGNORE_ALWAYS, !, msg, ## __VA_ARGS__)
	#define oASSERTA(cond, msg, ...) do { if (!(cond)) { oASSERT_PRINT_MESSAGE(TYPE_ASSERT, oAssert::IGNORE_ONCE, cond, msg, ## __VA_ARGS__); } } while(0)
#else
	#define oTRACEA(msg, ...) oASSERT_NOOP
	#define oTRACEA_ONCE(msg, ...) oASSERT_NOOP
	#define oWARNA(msg, ...) oASSERT_NOOP
	#define oWARNA_ONCE(msg, ...) oASSERT_NOOP
	#define oASSERTA(msg, ...) oASSERT_NOOP
#endif

// _____________________________________________________________________________
// Debug-only macros

#if oENABLE_ASSERTS == 1
	#define oTRACE(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_TRACE, oAssert::IGNORE_ONCE, !, msg, ## __VA_ARGS__)
	#define oTRACE_ONCE(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_TRACE, oAssert::IGNORE_ALWAYS, !, msg, ## __VA_ARGS__)
	#define oWARN(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_WARNING, oAssert::IGNORE_ONCE, !, msg, ## __VA_ARGS__)
	#define oWARN_ONCE(msg, ...) oASSERT_PRINT_MESSAGE(TYPE_WARNING, oAssert::IGNORE_ALWAYS, !, msg, ## __VA_ARGS__)
	#define oASSERT(cond, msg, ...) do { if (!(cond)) { oASSERT_PRINT_MESSAGE(TYPE_ASSERT, oAssert::IGNORE_ONCE, cond, msg, ## __VA_ARGS__); } } while(0)
	#define oASSUME(x) do { oASSERT(0, "Unexpected code execution"); oASSERT_NOEXEC; } while(0)
	#define oVERIFY(fn) do { if (!(fn)) { oASSERT_PRINT_MESSAGE(TYPE_ASSERT, oAssert::IGNORE_ONCE, fn, "Error %s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc()); } } while(0)
#else
	#define oTRACE(msg, ...) oASSERT_NOOP
	#define oTRACE_ONCE(msg, ...) oASSERT_NOOP
	#define oWARN(msg, ...) oASSERT_NOOP
	#define oWARN_ONCE(msg, ...) oASSERT_NOOP
	#define oASSERT(msg, ...) oASSERT_NOOP
	#define oASSUME(x) oASSERT_NOEXEC
	#define oVERIFY(fn) fn
#endif

#endif
