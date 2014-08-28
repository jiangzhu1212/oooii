// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_string_platform_h
#define oString_string_platform_h

// Abstract secureCRT and other platform-specific permutations of stdc string
// manipulators.

#include <oCompiler.h>

#ifdef _MSC_VER

#include <cstdarg>
#include <cstdio>

namespace ouro {

	inline int vsnprintf(char* dst, size_t dst_size, const char* fmt, va_list args)
	{
		#pragma warning(disable:4996) // secure CRT warning
		int l = ::vsnprintf(dst, dst_size, fmt, args);
		#pragma warning(default:4996)
		return l;
	}
	template<size_t size> int vsnprintf(char (&dst)[size], const char* fmt, va_list args) { return vsnprintf(dst, size, fmt, args); }

	inline int vsnwprintf(wchar_t* dst, size_t num_chars, const wchar_t* fmt, va_list args)
	{
		#pragma warning(disable:4996) // secure CRT warning
		int l = ::_vsnwprintf(dst, num_chars, fmt, args);
		#pragma warning(default:4996)
		return l;
	}
	template<size_t size> int vsnwprintf(wchar_t (&dst)[size], const wchar_t* fmt, va_list args) { return vsnwprintf(dst, size, fmt, args); }

	inline char* strncpy(char* dst, size_t dst_size, const char* src, size_t num_chars)
	{
		return strncpy_s(dst, dst_size, src, num_chars) ? nullptr : dst;
	}

	template<size_t size> char* strncpy(char (&dst)[size], const char* src, size_t num_chars) { return strncpy(dst, size, src, num_chars); }

	inline wchar_t* wcsncpy(wchar_t* dst, size_t _NumDestinationChars, const wchar_t* src, size_t num_chars)
	{
		return wcsncpy_s(dst, _NumDestinationChars, src, num_chars) ? nullptr : dst;
	}

	template<size_t size> char* wcsncpy(wchar_t (&dst)[size], const wchar_t* src, size_t num_chars) { return wcsncpy(dst, size, src, num_chars); }
}

inline int snprintf(char* dst, size_t dst_size, const char* fmt, ...)
{
	va_list args; va_start(args, fmt);
	int l = ouro::vsnprintf(dst, dst_size, fmt, args);
	va_end(args);
	return l;
}

template<size_t size> int snprintf(char (&dst)[size], const char* fmt, ...)
{
	va_list args; va_start(args, fmt);
	int l = ouro::vsnprintf(dst, size, fmt, args);
	va_end(args);
	return l;
}

// Posix form of a safer strtok
inline char* strtok_r(char* _StrToken, const char* _StrDelim, char** ctx)
{
	return strtok_s(_StrToken, _StrDelim, ctx);
}

#endif

#endif
