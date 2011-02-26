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
#include <oooii/oMsgBox.h>
#include <stdarg.h>

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
		int Line;
		int MsgId;
	};

	typedef ACTION (*VPrintMessageFn)(const ASSERTION* _pAssertion, const char* _Format, va_list _Args);

	ACTION VPrintMessage(const ASSERTION* _pAssertion, const char* _Format, va_list _Args);
	inline ACTION PrintMessage(const ASSERTION* _pAssertion, const char* _Format, ...) { va_list args; va_start(args, _Format); return VPrintMessage(_pAssertion, _Format, args); }

	struct DESC
	{
		DESC()
			: VPrintMessage(0)
			, LogFilePath(0)
			, EnableWarnings(true)
			, PrintCallstack(true)
			, PrefixFileLine(true)
			, PrefixThreadId(true)
			, PrefixMsgType(true)
			, PrefixMsgId(false)
		{}

		// User VPrintMessage function. If none is specified, a default one will be
		// used. The function will be passed an ASSERTION struct containing 
		// information derived from the usage. If the message is TYPE_TRACE or 
		// TYPE_WARNING, then Expression will be "!", implying no relevant expression
		// was specified. From the user function, GetDesc() can be called to 
		// determine various user-specified options.
		VPrintMessageFn VPrintMessage;
		
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

	// Enable PrefixMsgId in the DESC to get the id of any annoying messages to 
	// be disabled.
	void AddMessageFilter(int _MsgId);
	void RemoveMessageFilter(int _MsgId);

	// Generate a message ID based on the constant string used in message macros
	// Mostly this is used internally by the macros below.
	int GetMsgId(const char* _Format);

} // namespace oAssert

#if defined(_WIN32) || defined(_WIN64)
	#include <crtdbg.h>
	#define oSTATICASSERT(cond) _STATIC_ASSERT(cond)
	#define oDEBUGBREAK() __debugbreak()
	#define PROCESS_ACTION__(action) do { if (action == oAssert::ABORT) ::exit(__LINE__); else if (action == oAssert::BREAK) oDEBUGBREAK(); else if (action == oAssert::IGNORE_ALWAYS) bIgnoreFuture = true; } while(0)
	#define PREPARE_ASSERTION__(type, cond, msg, a) do { a.Expression = #cond; a.Function = __FUNCTION__; a.Filename = __FILE__; a.Line = __LINE__; a.Type = oAssert::type; a.MsgId = oAssert::GetMsgId(msg); } while(0)
	#define PRINTMESSAGE__(type, cond, msg, ...) do { static bool bIgnoreFuture = false; if (!bIgnoreFuture) { oAssert::ASSERTION a; PREPARE_ASSERTION__(type, !, msg "\n", a); oAssert::ACTION action = oAssert::PrintMessage(&a, msg "\n", ## __VA_ARGS__); PROCESS_ACTION__(action); } } while(0)
	#define oTRACEA(msg, ...) PRINTMESSAGE__(TYPE_TRACE, !, msg, ## __VA_ARGS__)
	#define oWARNA(msg, ...) PRINTMESSAGE__(TYPE_WARNING, !, msg, ## __VA_ARGS__)
	#define oWARNA_ONCE(msg, ...) do { static bool bIgnoreOnce = false; if (!bIgnoreOnce) { oWARNA(msg, ## __VA_ARGS__); bIgnoreOnce = true; } } while(0)
	#define oTRACEA_ONCE(msg, ...) do { static bool bIgnoreOnce = false; if (!bIgnoreOnce) { oTRACEA(msg ## __VA_ARGS__); bIgnoreOnce = true; } } while(0)
	#if oENABLE_ASSERTS == 1
		#define oASSERT(cond, msg, ...) do { if (!(cond)) { PRINTMESSAGE__(TYPE_ASSERT, cond, msg, ## __VA_ARGS__); } } while(0)
		#define oWARN(msg, ...) oWARNA(msg, ## __VA_ARGS__)
		#define oWARN_ONCE(msg, ...) oWARNA_ONCE(msg, ## __VA_ARGS__)
		#define oTRACE(msg, ...) oTRACEA(msg, ## __VA_ARGS__)
		#define oTRACE_ONCE(msg, ...) oTRACEA_ONCE(msg, ## __VA_ARGS__)
		#define oASSUME(x) do { oASSERT(0, "Unexpected code execution"); __assume(0); } while(0)
		#define oVERIFY(fn) do { if (!(fn)) { PRINTMESSAGE__(TYPE_ASSERT, fn, "Error %s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc()); } } while(0)

		// error check wrappers for WIN32 API. These interpret HRESULTS, so should not 
		// be used on anything but WIN32 API. oVB is for the return false, GetLastError()
		// pattern, and oV is for direct HRESULT return values.
		extern int oGetNativeErrorDesc(char* _StrDestination, size_t _SizeofStrDestination, size_t _NativeError);
		#define oVB(fn) { if (!(fn)) { HRESULT HR__ = ::GetLastError(); char errDesc[1024]; oGetNativeErrorDesc(errDesc, _countof(errDesc), HR__); PRINTMESSAGE__(TYPE_ASSERT, fn, "HRESULT 0x%08x: %s", HR__, errDesc); } } while(0)
		#define oV(fn) { HRESULT HR__ = fn; if (FAILED(HR__)) { char errDesc[1024]; oGetNativeErrorDesc(errDesc, _countof(errDesc), HR__); PRINTMESSAGE__(TYPE_ASSERT, fn, "HRESULT 0x%08x: %s", HR__, errDesc); } } while(0)

		// Convenience wrapper for quick scoped leak checking
		class oLeakCheck
		{
			const char* Name;
			_CrtMemState StartState;
		public:
			oLeakCheck(const char* _ConstantName = "") : Name(_ConstantName ? _ConstantName : "(unnamed)") { _CrtMemCheckpoint(&StartState); }
			~oLeakCheck()
			{
				_CrtMemState endState, stateDiff;
				_CrtMemCheckpoint(&endState);
				_CrtMemDifference(&stateDiff, &StartState, &endState);
				oTRACE("---- Mem diff for %s ----", Name);
				_CrtMemDumpStatistics(&stateDiff);
			}
		};
	#else
			#define oASSERT(cond, msg, ...) __noop
			#define oWARN(msg, ...) do { oMsgBox::printf(oMsgBox::WARN, "OOOii Debug Library", "Release Warning!\n%s() %s(%u)\n\n" msg, __FUNCTION__, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)
			#define oWARN_ONCE(msg, ...) oWARN(msg, ## __VA_ARGS__);
			#define oTRACE(msg, ...) __noop
			#define oTRACE_ONCE(msg, ...) __noop
			#define oASSUME(x) __assume(0)
			#define oVERIFY(fn) fn
			#define	oVB(fn) fn
			#define oV(fn) fn
		#endif
	#else
		#error Unsupported platform
	#endif
#endif
