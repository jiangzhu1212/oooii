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

// Interface for working with an attached command line display
#pragma once
#ifndef oConsole_h
#define oConsole_h

#include <oStd/color.h>
#include <oStd/function.h>
#include <oBasis/oMathTypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace oConsole
{
	const unsigned short DEFAULT = 0x8000;

	// Return true to short-circuit any default action. Return false to defer
	// to default behavior.
	typedef oFUNCTION<bool()> EventFn;

	enum EVENT
	{
		CTRLC,
		CTRLBREAK,
		CLOSE, // you've got 5-sec on Windows before it force-closes
		LOGOFF,
		SHUTDOWN,
	};
	
	struct DESC
	{

		DESC()
			: BufferWidth(DEFAULT)
			, BufferHeight(DEFAULT)
			, Top(DEFAULT)
			, Left(DEFAULT)
			, Width(DEFAULT)
			, Height(DEFAULT)
			, Foreground(0)
			, Background(0)
			, Show(true)
		{}

		// NOTE: BufferWidth/Height must be the same size or larger than Width/Height

		unsigned short BufferWidth; // in characters
		unsigned short BufferHeight;  // in characters
		unsigned short Top; // in pixels
		unsigned short Left;  // in pixels
		unsigned short Width;  // in characters
		unsigned short Height;  // in characters
		oStd::color Foreground; // 0 means don't set color
		oStd::color Background; // 0 means don't set color
		bool Show;
	
		// If specified and a valid, accessible file path, all output through this 
		// console will be written to the specified file. 
		oStd::path_string LogFilePath;
	};

	void* GetNativeHandle(); // returns HWND on Windows

	// Get the pixel width/height of the console window
	int2 GetSizeInPixels();
	int2 GetSizeInCharacters();

	void SetDesc(const DESC* _pDesc);
	void GetDesc(DESC* _pDesc);
	void SetTitle(const char* _Title);
	void GetTitle(char* _StrDestination, size_t _SizeofStrDestination);
	template<size_t size> inline void GetTitle(char (&_StrDestination)[size]) { return GetTitle(_StrDestination, size); }

	// If you specify DEFAULT for one of these, then it will be whatever it was
	// before.
	void SetCursorPosition(const int2& _Position);
	int2 GetCursorPosition();

	void Clear();

	bool HasFocus();

	int vfprintf(FILE* _pStream, oStd::color _Foreground, oStd::color _Background, const char* _Format, va_list _Args);
	inline int fprintf(FILE* _pStream, oStd::color _Foreground, oStd::color _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(_pStream, _Foreground, _Background, _Format, args); va_end(args); return n; }
	inline int printf(oStd::color _Foreground, oStd::color _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(stdout, _Foreground, _Background, _Format, args); va_end(args); return n; }

	void HookEvent(EVENT _Event, EventFn _Function);
};

#endif
