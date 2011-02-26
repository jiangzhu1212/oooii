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
// This is a runtime allocated alternative to temporary character buffers.
// Buffers are allocated when needed and reused when available.

#pragma once
#ifndef oTempString_h
#define oTempString_h

#include <oooii/oString.h>

struct oStringBuffer;

struct oTempString
{
	static const size_t DefaultSize = (4 * 1024);
	static const size_t BigSize		= (128 * 1024);

	oTempString(size_t _size = DefaultSize);
	~oTempString();

	oTempString(const oTempString& rhs) { *this = rhs; }
	oTempString& operator=(const oTempString& rhs);

	operator char*();
	char* c_str();

	oTempString operator+(size_t _offset) { return oTempString(pStrBuffer, offset + _offset); }
	oTempString operator[](size_t _offset) { return oTempString(pStrBuffer, offset + _offset); }

	size_t size() const;

private:
	oTempString(oStringBuffer* _pStrBuffer, size_t _offset);

	oStringBuffer*	pStrBuffer;
	size_t			offset;
};

// Common string functions overloads to make it easier to use the safe versions

inline int sprintf_s (oTempString string, const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	return vsprintf_s(string, string.size(), format, arglist);
}

inline int vsprintf_s (oTempString string, const char *format, va_list ap) { return vsprintf_s(string, string.size(), format, ap); }
inline errno_t strcpy_s(oTempString string, const char *strSource){ return strcpy_s(string, string.size(), strSource); }

inline char* oNewlinesToDos(oTempString _StrDestination, const char* _StrSource) { return oNewlinesToDos(_StrDestination, _StrDestination.size(), _StrSource); }

#endif // oTempString_h
