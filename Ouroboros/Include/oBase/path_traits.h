// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Traits of file path rules used in ouro::path.
#pragma once
#ifndef oBase_path_traits_h
#define oBase_path_traits_h

#include <oBase/assert.h>
#include <oBase/fixed_string.h>
#include <oBase/fnv1a.h>
#include <oString/string_path.h>
#include <oBase/throw.h>

namespace ouro {

template<typename charT, bool _Posix>
struct base_path_traits
{
	typedef charT char_type;
	static const bool posix = _Posix;
	static bool is_dot(char_type c);
	static bool is_sep(char_type c);
	static bool is_native_sep(char_type c);
	static bool is_unc(const char_type* p);
	static bool has_vol(const char_type* p);
	static char_type* generic_sep_str();
	static char_type* native_sep_str();
	static char_type* dot_str();
	static char_type* dotdot_str();
	static char_type* empty_str();
	static size_t cmnroot(const char_type* _Path1, const char_type* _Path2);
	static int compare(const char_type* a, const char_type* b); // strcmp/stricmp
	static size_t hash(const char_type* _Path);
};

template<> struct base_path_traits<char, false>
{
	typedef char char_type;
	static const bool posix = false;
	static bool is_dot(char_type c) { return c == '.'; }
	static bool is_sep(char_type c) { return c == '/' || c == '\\'; }
	static bool is_native_sep(char_type c) { return c == '\\'; }
	static bool is_unc(const char_type* p) { return p && is_sep(p[0]) && is_sep(p[1]) && !is_sep(p[2]); }
	static bool has_vol(const char_type* p) { return p && p[0] && p[1] == ':';  }
	static char_type generic_sep_chr() { return '/'; }
	static char_type native_sep_chr() { return '\\'; }
	static char_type* generic_sep_str() { return "/"; }
	static char_type* native_sep_str() { return "\\"; }
	static char_type* dot_str() { return "."; }
	static char_type* dotdot_str() { return ".."; }
	static char_type* empty_str() { return ""; }
	static size_t cmnroot(const char_type* _Path1, const char_type* _Path2) { return cmnroot(_Path1, _Path2); }
	static int compare(const char_type* a, const char_type* b) { return _stricmp(a, b); }
	static size_t hash(const char_type* _Path) { return fnv1ai<size_t>(_Path); }
};

template<> struct base_path_traits<char, true>
{
	typedef char char_type;
	static const bool posix = true;
	static bool is_dot(char_type c) { return c == '.'; }
	static bool is_sep(char_type c) { return c == '/'; }
	static bool is_native_sep(char_type c) { return c == '/'; }
	static bool is_unc(const char_type* p) { return p && is_sep(p[0]) && is_sep(p[1]) && !is_sep(p[2]); }
	static bool has_vol(const char_type* p) { return false;  }
	static char_type generic_sep_chr() { return '/'; }
	static char_type native_sep_chr() { return '/'; }
	static char_type* generic_sep_str() { return "/"; }
	static char_type* native_sep_str() { return "/"; }
	static char_type* dot_str() { return "."; }
	static char_type* dotdot_str() { return ".."; }
	static char_type* empty_str() { return ""; }
	static size_t cmnroot(const char_type* _Path1, const char_type* _Path2) { return cmnroot(_Path1, _Path2); }
	static int compare(const char_type* a, const char_type* b) { return strcmp(a, b); }
	static size_t hash(const char_type* _Path) { return fnv1a<size_t>(_Path); }
};

template<> struct base_path_traits<wchar_t, false>
{
	typedef wchar_t char_type;
	static const bool posix = false;
	static bool is_dot(char_type c) { return c == L'.'; }
	static bool is_sep(char_type c) { return c == L'/' || c == L'\\'; }
	static bool is_native_sep(char_type c) { return c == L'\\'; }
	static bool is_unc(const char_type* p) { return p && is_sep(p[0]) && is_sep(p[1]) && !is_sep(p[2]); }
	static bool has_vol(const char_type* p) { return p && p[0] && p[1] == L':';  }
	static char_type generic_sep_chr() { return L'/'; }
	static char_type native_sep_chr() { return L'/'; }
	static char_type* generic_sep_str() { return L"/"; }
	static char_type* native_sep_str() { return L"\\"; }
	static char_type* dot_str() { return L"."; }
	static char_type* dotdot_str() { return L".."; }
	static char_type* empty_str() { return L""; }
	static size_t cmnroot(const char_type* _Path1, const char_type* _Path2) { return wcmnroot(_Path1, _Path2); }
	static int compare(const char_type* a, const char_type* b) { return _wcsicmp(a, b); }
	static size_t hash(const char_type* _Path) { return fnv1ai<size_t>(_Path); }
};

template<> struct base_path_traits<wchar_t, true>
{
	typedef wchar_t char_type;
	static const bool posix = true;
	static bool is_dot(char_type c) { return c == L'.'; }
	static bool is_sep(char_type c) { return c == L'/'; }
	static bool is_native_sep(char_type c) { return c == L'/'; }
	static bool is_unc(const char_type* p) { return p && is_sep(p[0]) && is_sep(p[1]) && !is_sep(p[2]); }
	static bool has_vol(const char_type* p) { return false;  }
	static char_type generic_sep_chr() { return L'/'; }
	static char_type native_sep_chr() { return L'/'; }
	static char_type* generic_sep_str() { return L"/"; }
	static char_type* native_sep_str() { return L"/"; }
	static char_type* dot_str() { return L"."; }
	static char_type* dotdot_str() { return L".."; }
	static char_type* empty_str() { return L""; };
	static size_t cmnroot(const char_type* _Path1, const char_type* _Path2) { return wcmnroot(_Path1, _Path2); }
	static int compare(const char_type* a, const char_type* b) { return wcscmp(a, b); }
	static size_t hash(const char_type* _Path) { return fnv1a<size_t>(_Path); }
};

template<typename charT, bool _Posix, bool _AlwaysClean>
struct path_traits : base_path_traits<charT, _Posix>
{
	static const bool always_clean = _AlwaysClean;
};

template<typename charT>
struct default_posix_path_traits : path_traits<charT, true, true> {};

template<typename charT>
struct default_windows_path_traits : path_traits<charT, false, true> {};

}

#endif
