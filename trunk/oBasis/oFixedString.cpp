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
#include <oBasis/oFixedString.h>
#include <oBasis/oPlatformFeatures.h>
#include <string.h>
#include <wchar.h>

template<typename T> T SS(T _String) { return _String ? _String : T(""); }

void oFixedStringCopy(char* _StrDestination, size_t _NumDestinationChars, const char* _StrSource)
{
	strcpy_s(_StrDestination, _NumDestinationChars, SS(_StrSource));
}

void oFixedStringCopy(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource)
{
	wcscpy_s(_StrDestination, _NumDestinationChars, SS(_StrSource));
}

void oFixedStringCopy(char* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource)
{
	size_t sz = 0;
	const wchar_t* src = SS(_StrSource);
	wcsrtombs_s(&sz, _StrDestination, _NumDestinationChars, &src, _NumDestinationChars, nullptr);
}

void oFixedStringCopy(wchar_t* _StrDestination, size_t _NumDestinationChars, const char* _StrSource)
{
	size_t sz = 0;
	const char* src = SS(_StrSource);
	mbsrtowcs_s(&sz, _StrDestination, _NumDestinationChars, &src, _NumDestinationChars, nullptr);
}

size_t oFixedStringLength(const char* _StrSource)
{
	return strlen(SS(_StrSource));
}

size_t oFixedStringLength(const wchar_t* _StrSource)
{
	return wcslen(SS(_StrSource));
}
