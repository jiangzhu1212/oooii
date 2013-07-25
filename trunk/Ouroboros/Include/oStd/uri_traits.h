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
// Traits of uri rules used in oStd::uri
#pragma once
#ifndef oStd_uri_traits_h
#define oStd_uri_traits_h

#include <oStd/path_traits.h>

#define oSTD_MAX_URI 2048

namespace oStd {

template<typename charT, typename hashT, typename path_traitsT = default_posix_path_traits<charT>>
struct uri_traits
{
	typedef charT char_type;
	typedef size_t size_type;
	typedef hashT hash_type;
	typedef path_traitsT path_traits_type;
	typedef basic_path<char_type, path_traits_type> path_type;
	static const size_type capacity = oSTD_MAX_URI;

	static const char_type* empty_str();
	static const char_type* scheme_str();
	static const char_type* sep_str();
	static const char_type* double_sep_str();
	static const char_type* query_str();
	static const char_type* fragment_str();
	static const char_type* file_scheme_prefix_str();

	static bool is_file_scheme(const char_type* _URI);

	static int compare(const char_type* _URIStringA, const char_type* _URIStringB);
};

template<> struct uri_traits<char, unsigned long long, default_posix_path_traits<char>>
{
	typedef char char_type;
	typedef size_t size_type;
	typedef unsigned long long hash_type;
	typedef default_posix_path_traits<char_type> path_traits_type;
	typedef basic_path<char_type, path_traits_type> path_type;
	static const size_type capacity = oSTD_MAX_URI;

	static const char_type* empty_str() { return ""; }
	static const char_type* scheme_str() { return ":"; }
	static const char_type* sep_str() { return "/"; }
	static const char_type* double_sep_str() { return "//"; }
	static const char_type* query_str() { return "?"; }
	static const char_type* fragment_str() { return "#"; }
	static const char_type* file_scheme_prefix_str() { return "file://"; }

	static bool is_file_scheme(const char_type* _URI) { return !!_memicmp("file", _URI, 4); }
	static int compare(const char_type* _URIStringA, const char_type* _URIStringB) { return strcmp(_URIStringA, _URIStringB); }
};

template<> struct uri_traits<wchar_t, unsigned long long, default_posix_path_traits<wchar_t>>
{
	typedef wchar_t char_type;
	typedef size_t size_type;
	typedef unsigned long long hash_type;
	typedef default_posix_path_traits<char_type> path_traits_type;
	typedef basic_path<char_type, path_traits_type> path_type;
	static const size_type capacity = oSTD_MAX_URI;

	static const char_type* empty_str() { return L""; }
	static const char_type* scheme_str() { return L":"; }
	static const char_type* sep_str() { return L"/"; }
	static const char_type* double_sep_str() { return L"//"; }
	static const char_type* query_str() { return L"?"; }
	static const char_type* fragment_str() { return L"#"; }
	static const char_type* file_scheme_prefix_str() { return L"file://"; }

	static bool is_file_scheme(const char_type* _URI) { return !!_memicmp(L"file", _URI, 8); }
	static int compare(const char_type* _URIStringA, const char_type* _URIStringB) { return wcscmp(_URIStringA, _URIStringB); }
};

} // namespace oStd

#endif
