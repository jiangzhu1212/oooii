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
#ifndef oStandards_h
#define oStandards_h

#include <stdarg.h>

// This contains patterns and standards that programmers at OOOii Realtime follow
// when implementing various products.

namespace oConsoleReporting
{
	enum REPORT_TYPE
	{
		DEFAULT,
		SUCCESS,
		INFO,
		HEADING,
		WARN,
		ERR,
		CRIT,
		NUM_REPORT_TYPES,
	};

	void VReport(REPORT_TYPE _Type, const char* _Format, va_list _Args);
	inline void Report(REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); VReport(_Type, _Format, args); va_end( args ); }
}

// Moves the mouse cursor offscreen so even in the system unhides it (if the app
// crashes), the mouse should be in an inconspicuous spot.
bool oMoveMouseCursorOffscreen();

// Returns platform-native handle to a file that should be closed
// using appropriate platform API. On Windows this returns an HICON
// that should be treated appropriately, including final destruction
// with DeleteObject().
void* oLoadStandardIcon();

// Returns the path to a .txt file with the name of the current exe concatenated 
// with the (optionally) specified suffix and a sortable timestamp in the 
// filename to ensure uniqueness.
errno_t oGetLogFilePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _ExeSuffix = 0);
template<size_t size> inline errno_t oGetLogFilePath(char (&_StrDestination)[size], const char* _ExeSuffix = 0) { return oGetLogFilePath(_StrDestination, size, _ExeSuffix); }

#endif // oStandards_h
