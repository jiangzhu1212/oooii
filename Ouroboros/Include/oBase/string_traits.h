/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// For when std::char_traits does not provide the desired API.
#pragma once
#ifndef oBase_string_traits_h
#define oBase_string_traits_h

#include <oBase/string.h>

namespace ouro {

template<typename charT> struct string_traits
{
	typedef charT char_type;
	typedef size_t size_type;

	// strlcpy semantics
	static size_type copy(char* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount);
	static size_type copy(wchar_t* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount);

	// strlcat semantics
	static size_type cat(char_type* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount);
	
	// never return null
	static const char* safe(const char_type* s);

	// return length of string
	static size_type length(const char_type* s);

	 // strcmp-style compare
	static int compare(const char_type* a, const char_type* b);

	 // stricmp-style compare
	static int comparei(const char_type* a, const char_type* b);
};

template<> struct string_traits<char>
{
	typedef char char_type;
	typedef size_t size_type;
	static size_type copy(char* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount) { return strlcpy(_StrDestination, safe(_StrSource), _StrDestinationCount); }
	static size_type copy(wchar_t* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount) { return mbsltowsc(_StrDestination, _StrSource, _StrDestinationCount); }
	static size_type copy(char_type* _StrDestination, const wchar_t* _StrSource, size_type _StrDestinationCount) { return wcsltombs(_StrDestination, _StrSource, _StrDestinationCount); }
	static size_type cat(char_type* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount) { return strlcat(_StrDestination, _StrSource, _StrDestinationCount); }
	static const char_type* safe(const char_type* _String) { return _String ? _String : ""; }
	static size_type length(const char_type* _String) { return strlen(_String); }
	static int compare(const char_type* a, const char_type* b) { return strcmp(a, b); }
	static int comparei(const char_type* a, const char_type* b) { return _stricmp(a, b); }
};

template<> struct string_traits<wchar_t>
{
	typedef wchar_t char_type;
	typedef size_t size_type;
	static size_type copy(char* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount) { return wcsltombs(_StrDestination, safe(_StrSource), _StrDestinationCount); }
	static size_type copy(wchar_t* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount) { return wcslcpy(_StrDestination, _StrSource, _StrDestinationCount); }
	static size_type cat(char_type* _StrDestination, const char_type* _StrSource, size_type _StrDestinationCount) { return wcslcat(_StrDestination, _StrSource, _StrDestinationCount); }
	static const char_type* safe(const char_type* _String) { return _String ? _String : L""; }
	static size_type length(const char_type* _String) { return wcslen(_String); }
	static int compare(const char_type* a, const char_type* b) { return wcscmp(a, b); }
	static int comparei(const char_type* a, const char_type* b) { return _wcsicmp(a, b); }
};

}

#endif
