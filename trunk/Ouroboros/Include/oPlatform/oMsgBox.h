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
// Printf-style interface for displaying OS message boxes
#pragma once
#ifndef oMsgBox_h
#define oMsgBox_h

#include <oBasis/oGUI.h>
#include <oBasis/oStddef.h>
#include <stdarg.h>

enum oMSGBOX_TYPE
{
	oMSGBOX_INFO,
	oMSGBOX_WARN,
	oMSGBOX_YESNO,
	oMSGBOX_ERR,
	oMSGBOX_DEBUG,
	oMSGBOX_NOTIFY,
	oMSGBOX_NOTIFY_INFO,
	oMSGBOX_NOTIFY_WARN,
	oMSGBOX_NOTIFY_ERR,
};

enum oMSGBOX_RESULT
{
	oMSGBOX_NO,
	oMSGBOX_YES,
	oMSGBOX_ABORT,
	oMSGBOX_BREAK,
	oMSGBOX_CONTINUE,
	oMSGBOX_IGNORE,
};

struct oMSGBOX_DESC
{
	oMSGBOX_DESC(oMSGBOX_TYPE _Type = oMSGBOX_INFO, const char* _Title = "", oGUI_WINDOW _hParent = nullptr, unsigned int _TimeoutMS = oInfiniteWait)
		: Type(_Type)
		, Title(nullptr)
		, hParent(nullptr)
		, TimeoutMS(_TimeoutMS)
	{}

	oMSGBOX_TYPE Type;
	unsigned int TimeoutMS;
	oGUI_WINDOW hParent;
	const char* Title;
};


// For no timeout, specify oInfiniteWait
oMSGBOX_RESULT oMsgBoxV(const oMSGBOX_DESC& _Desc, const char* _Format, va_list _Args);
inline oMSGBOX_RESULT oMsgBox(const oMSGBOX_DESC& _Desc, const char* _Format, ...) { va_list args; va_start(args, _Format); oMSGBOX_RESULT r = oMsgBoxV(_Desc, _Format, args); va_end(args); return r; }

#endif
