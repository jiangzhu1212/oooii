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

// Interface for accessing common debugger features
#pragma once
#ifndef oDebugger_h
#define oDebugger_h

#include <stdarg.h>

namespace oDebugger
{
	struct SYMBOL
	{
		unsigned long long Address;
		char Module[512];
		char Name[512];
		char Filename[512];
		unsigned int SymbolOffset;
		unsigned int Line;
		unsigned int CharOffset;
	};

	// oDebugger is useful even during static init/deinit. The singleton 
	// initializes on-demand well, but we need this to ensure against premature
	// deinitialization.
	void Reference();
	void Release();

	// As it appears in the debugger's UI. _NativeThreadHandle of 0 means 
	// "this thread"
	void SetThreadName(const char* _Name, void* _NativeThreadHandle = 0);

	// Print to debugger output. Normally this ensures only one thread at a time
	// can write so no messages get garbled, but doing so requires oSingleton to
	// be valid to hold onto the mutex. With _EnsureThreadsafety = false, the 
	// oSingleton is not touched and print() is not threadsafe. This is useful in
	// late static deinitialization code where the mutex might already be 
	// destroyed and threading is less of a worry.
	void Print(const char* _String);

	// Cause a system break on the requested alloc ordinal
	void BreakOnAlloc(long _RequestNumber);

	// Summarize all malloc()'s not free()'ed
	void ReportLeaksOnExit(bool _Report = true, bool _UseDefaultReporter = false);

	// Throw more exceptions when bad things happen to floats and doubles
	bool FloatExceptionsEnabled();
	void EnableFloatExceptions(bool _Enable);

	// Fills the array pointed to by _pAddresses with up to _NumAddresses
	// addresses of functions in the current callstack from main(). This
	// includes every function that led to the GetCallstack() call. Offset
	// ignores the N functions at the top of the stack so a system can 
	// simplify/hide details of debug reporting and keep the callstack 
	// started from the meaningful user code where an error or assertion
	// occurred.
	// This returns the actual number of addresses retrieved.
	size_t GetCallstack(unsigned long long* _pAddresses, size_t _NumAddresses, size_t _Offset);
	template<size_t size> inline size_t GetCallstack(unsigned long long (&_pAddresses)[size], size_t _Offset) { return GetCallstack(_pAddresses, size, _Offset); }

	// Convert an address as retrieved using GetCallstack() into more 
	// descriptive strings.
	bool TranslateSymbol(SYMBOL* _pSymbol, unsigned long long _Address);
};

#endif
