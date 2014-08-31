// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Traits of uri rules used in ouro::uri
#pragma once
#ifndef oBase_uri_traits_h
#define oBase_uri_traits_h

#include <oBase/path_traits.h>

#define oSTD_MAX_URI 2048

namespace ouro {

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

	static bool is_file_scheme(const char_type* _URI) { return 0 == _memicmp("file", _URI, 4); }
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

	static bool is_file_scheme(const char_type* _URI) { return 0 == _memicmp(L"file", _URI, 8); }
	static int compare(const char_type* _URIStringA, const char_type* _URIStringB) { return wcscmp(_URIStringA, _URIStringB); }
};

}

#endif
