// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_string_traits_h
#define oString_string_traits_h

// For when std::char_traits does not provide the desired API.

#include <oString/string.h>

namespace ouro {

template<typename charT> struct string_traits
{
	typedef charT char_type;
	typedef size_t size_type;

	// strlcpy semantics
	static size_type copy(char* dst, const char_type* src, size_type dst_size);
	static size_type copy(wchar_t* dst, const char_type* src, size_type dst_size);

	// strlcat semantics
	static size_type cat(char_type* dst, const char_type* src, size_type dst_size);
	
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
	static size_type copy(char* dst, const char_type* src, size_type dst_size) { return strlcpy(dst, safe(src), dst_size); }
	static size_type copy(wchar_t* dst, const char_type* src, size_type dst_size) { return mbsltowsc(dst, src, dst_size); }
	static size_type copy(char_type* dst, const wchar_t* src, size_type dst_size) { return wcsltombs(dst, src, dst_size); }
	static size_type cat(char_type* dst, const char_type* src, size_type dst_size) { return strlcat(dst, src, dst_size); }
	static const char_type* safe(const char_type* _String) { return _String ? _String : ""; }
	static size_type length(const char_type* _String) { return strlen(_String); }
	static int compare(const char_type* a, const char_type* b) { return strcmp(a, b); }
	static int comparei(const char_type* a, const char_type* b) { return _stricmp(a, b); }
};

template<> struct string_traits<wchar_t>
{
	typedef wchar_t char_type;
	typedef size_t size_type;
	static size_type copy(char* dst, const char_type* src, size_type dst_size) { return wcsltombs(dst, safe(src), dst_size); }
	static size_type copy(wchar_t* dst, const char_type* src, size_type dst_size) { return wcslcpy(dst, src, dst_size); }
	static size_type cat(char_type* dst, const char_type* src, size_type dst_size) { return wcslcat(dst, src, dst_size); }
	static const char_type* safe(const char_type* _String) { return _String ? _String : L""; }
	static size_type length(const char_type* _String) { return wcslen(_String); }
	static int compare(const char_type* a, const char_type* b) { return wcscmp(a, b); }
	static int comparei(const char_type* a, const char_type* b) { return _wcsicmp(a, b); }
};

}

#endif
