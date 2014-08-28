// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_string_traits_h
#define oBase_string_traits_h

// For when std::char_traits does not provide the desired API.

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
