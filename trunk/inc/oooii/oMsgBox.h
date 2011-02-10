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
// Printf-style interface for display OS message boxes
#pragma once
#ifndef oMsgBox_h
#define oMsgBox_h

#include <stdarg.h>

struct oMsgBox
{
	enum TYPE
	{
		INFO,
		WARN,
		YESNO,
		ERR,
		DEBUG,
	};

	enum RESULT
	{
		NO,
		YES, // any "OK" button returns YES
		ABORT,
		BREAK,
		CONTINUE,
		IGNORE_ALWAYS,
	};

	static RESULT tvprintf(TYPE _Type, unsigned int _Timeout, const char* _Title, const char* _Format, va_list _Args);
	static RESULT tprintf(TYPE _Type, unsigned int _Timeout, const char* _Title, const char* _Format, ...);
	static RESULT vprintf(TYPE _Type, const char* _Title, const char* _Format, va_list _Args);
	static RESULT printf(TYPE _Type, const char* _Title, const char* _Format, ...);
};

#endif
