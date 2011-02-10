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

	// As it appears in the debugger's UI. nativeHandle of 0 means "this thread"
	void SetThreadName(const char* _Name, void* _NativeThreadHandle = 0);

	// Format printf-style output to the debugger's output window
	int vprintf(const char* _Format, va_list _Args);
	int printf(const char* _Format, ...);

	// Direct-print without any formatting. This can be faster than vprintf/printf
	void print(const char* _String);

	// True if the debugger is watching this process
	bool IsAttached();

	// Cause a system break on the requested alloc ordinal
	void BreakOnAlloc(long _RequestNumber);

	// Summarize all malloc()'s not free()'ed
	void ReportLeaksOnExit(bool _Report = true);

	// Throw more exceptions when bad things happen to floats and doubles
	bool FloatExceptionsEnabled();
	void EnableFloatExceptions(bool _Enable);

	// gets the address of the function calls from main to the point of calling.
	// offset is the number of symbols to skip before recording starts
	size_t GetCallstack(unsigned long long* _pAddresses, size_t _NumAddresses, size_t _Offset);
	template<size_t size> inline size_t GetCallstack(unsigned long long (&_pAddresses)[size], size_t _Offset) { return GetCallstack(_pAddresses, size, _Offset); }

	// Convert symbol into more descriptive strings
	bool TranslateSymbol(SYMBOL* _pSymbol, unsigned long long _Address);
};

#endif
