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
// This contains patterns and standards that programmers at OOOii follow when
// implementing various products.

#pragma once
#ifndef oStandards_h
#define oStandards_h

#include <oBase/fixed_string.h>
#include <stdarg.h>

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
	inline void Report(REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); VReport(_Type, _Format, args); va_end(args); }
}

// Moves the mouse cursor offscreen so even in the system unhides it (if the app
// crashes), the mouse should be in an inconspicuous spot. This is primarily 
// useful for fullscreen apps.
bool oMoveMouseCursorOffscreen();

// Returns platform-native handle to a file that should be closed
// using appropriate platform API. On Windows this returns an HICON
// that should be treated appropriately, including final destruction
// with DeleteObject().
void* oLoadStandardIcon();

// Finds the path to the specified ini according to OOOii's override rules which 
// are ordered as follows:
// /..
// AppDir/..
// /.
// /AppDir/..
// This ordering is used as our typical installation has the launcher running from
// the /.. directory.
bool oINIFindPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _pININame);
template<size_t size> bool oINIFindPath(char (&_StrDestination)[size], const char* _pININame) { return oINIFindPath(_StrDestination, size, _pININame); }
template<size_t CAPACITY> bool oINIFindPath(ouro::fixed_string<char, CAPACITY>& _StrDestination, const char* _pININame) { return oINIFindPath(_StrDestination, _StrDestination.capacity(), _pININame); }

#endif
