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
// This contains patterns and standards that programmers at OOOii follow when
// implementing various products.

#pragma once
#ifndef oStandards_h
#define oStandards_h

#include <oBasis/oFixedString.h>
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

// Many applications are part of an auto-boot system that treats the operating
// system like a console/embedded-one-app system. The idea is to come up as 
// quickly as possible at 100% performance. To avoid the "what happened there?"
// behavior of running a performance app while the OS is still loading services
// and other on-boot systems, put in a pause here until CPU usage and other 
// system metrics are in a steady state. This way the "why is it slow?" is 
// explicit. Most for-deployment apps should call this very early in their oMAIN
// function (@oooii-tony: Perhaps we should consider moving this into oMAIN?)
// _ContinueIdling is an optional function that is called while waiting.  If it 
// ever returns false the wait will terminate.
bool oWaitForSystemSteadyState(oFUNCTION<bool()> _ContinueIdling = nullptr);

// For our multi-screen produces, exclusive fullscreen doesn't work when one
// computer is controlling several fullscreen windows, so we use cooperative
// fullscreen - always-on-top. Windows thinks it deserves the right to always
// show the taskbar, so it can fight the idea that we don't want it shown. Many
// of our apps are console-style apps - i.e. we 100% control the interaction 
// experience, so close the regular user interface of windows when our app takes
// over. Remember it's Ctrl-Shift-Esc to bring up the task manager and then
// File|Run "explorer.exe" to bring it all back. Since our apps tend to shut 
// down the machine if they close or crash, there is no auto-restore explorer at 
// this time.
void oKillExplorer();

// Returns platform-native handle to a file that should be closed
// using appropriate platform API. On Windows this returns an HICON
// that should be treated appropriately, including final destruction
// with DeleteObject().
void* oLoadStandardIcon();

// Returns the path to a .txt file with the name of the current exe concatenated 
// with the (optionally) specified suffix and a sortable timestamp in the 
// filename to ensure uniqueness.
char* oGetLogFilePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _ExeSuffix = nullptr);
template<size_t size> char* oGetLogFilePath(char (&_StrDestination)[size], const char* _ExeSuffix = nullptr) { return oGetLogFilePath(_StrDestination, size, _ExeSuffix); }
template<size_t CAPACITY> char* oGetLogFilePath(oFixedString<char, CAPACITY>& _StrDestination, const char* _ExeSuffix = nullptr) { return oGetLogFilePath(_StrDestination, _StrDestination.capacity(), _ExeSuffix); }


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
template<size_t CAPACITY> bool oINIFindPath(oFixedString<char, CAPACITY>& _StrDestination, const char* _pININame) { return oINIFindPath(_StrDestination, _StrDestination.capacity(), _pININame); }


#endif
