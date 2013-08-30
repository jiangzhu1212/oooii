/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
// String manipulation functions that really should be in the C/C++ standard.
#pragma once
#ifndef oStd_string_h
#define oStd_string_h

#include <oStd/config.h>
#include <oStd/macros.h>

// The most-standard (but not standard) secure strcpy/strcats. The 3rd parameter
// is in number of characters, not bytes.
size_t strlcat(char* _StrDestination, const char* _StrSource, size_t _StrDestinationCount);
size_t strlcpy(char* _StrDestination, const char* _StrSource, size_t _StrDestinationCount);
size_t wcslcat(wchar_t* _StrDestination, const wchar_t* _StrSource, size_t _StrDestinationCount);
size_t wcslcpy(wchar_t* _StrDestination, const wchar_t* _StrSource, size_t _StrDestinationCount);
size_t mbsltowsc(wchar_t* _StrDestination, const char* _StrSource, size_t _StrDestinationCount);
size_t wcsltombs(char* _StrDestination, const wchar_t* _StrSource, size_t _StrDestinationCount);

template<size_t count> size_t strlcat(char (&_StrDestination)[count], const char* _StrSource) { return strlcat(_StrDestination, _StrSource, count); }
template<size_t count> size_t strlcpy(char (&_StrDestination)[count], const char* _StrSource) { return strlcpy(_StrDestination, _StrSource, count); }
template<size_t count> size_t wcslcat(wchar_t (&_StrDestination)[count], const wchar_t* _StrSource) { return wcslcat(_StrDestination, _StrSource, count); }
template<size_t count> size_t wcslcpy(wchar_t (&_StrDestination)[count], const wchar_t* _StrSource) { return wcslcpy(_StrDestination, _StrSource, count); }
template<size_t count> size_t mbsltowsc(wchar_t (&_StrDestination)[count], const char* _StrSource) { return mbsltowsc(_StrDestination, _StrSource, count); }
template<size_t count> size_t wcsltombs(char* _StrDestination, const wchar_t* _StrSource, size_t _StrDestinationCount) { return wcsltombs(_StrDestination, _StrSource, count); }

#ifdef _MSC_VER

#include <cstdarg>
#include <cstdio>

namespace oStd {

	inline int vsnprintf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args)
	{
		#pragma warning(disable:4996) // secure CRT warning
		int l = ::vsnprintf(_StrDestination, _SizeofStrDestination, _Format, _Args);
		#pragma warning(default:4996)
		return l;
	}

	inline int vsnwprintf(wchar_t* _StrDestination, size_t _NumChars, const wchar_t* _Format, va_list _Args)
	{
		#pragma warning(disable:4996) // secure CRT warning
		int l = ::_vsnwprintf(_StrDestination, _NumChars, _Format, _Args);
		#pragma warning(default:4996)
		return l;
	}
}

inline int snprintf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, ...)
{
	va_list args; va_start(args, _Format);
	int l = oStd::vsnprintf(_StrDestination, _SizeofStrDestination, _Format, args);
	va_end(args);
	return l;
}

template<size_t size> int snprintf(char (&_StrDestination)[size], const char* _Format, ...)
{
	va_list args; va_start(args, _Format);
	int l = oStd::vsnprintf(_StrDestination, size, _Format, args);
	va_end(args);
	return l;
}

// Posix form of a safer strtok
inline char* strtok_r(char* _strToken, const char* _strDelim, char** _Context)
{
	return strtok_s(_strToken, _strDelim, _Context);
}

#endif

namespace oStd {

// Adds "...\0" to the end of the buffer - useful when recovering from string 
// overruns that can be truncated.
char* ellipsize(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> char* ellipsize(char (&_StrDestination)[size]) { return ellipsize(_StrDestination, size); }

wchar_t* wcsellipsize(wchar_t* _StrDestination, size_t _NumChars);
template<size_t n> wchar_t* ellipsize(wchar_t (&_StrDestination)[n]) { return wcsellipsize(_StrDestination, n); }

// passing this to std::transform brings up warnings that are hard to disable,
// so create a wrapper that is same type in and out.
template<typename charT> charT tolower(const charT& c) { return (charT)::tolower(c); }
template<typename charT> charT toupper(const charT& c) { return (charT)::toupper(c); }

// copy _strToken first for multi-threaded or non-destructive strtok. If the 
// parsing exits early end_strtok() must be called on _Context. If parsing
// exits due to a nullptr return value, cleanup need not be called.
char* strtok(const char* _strToken, const char* _strDelim, char** _Context);
void end_strtok(char** _Context);

// This function takes a single value (usually an enum value) and returns a 
// constant string representation of it.
typedef const char* (*as_string_fn)(int _SingleFlag);

// Fill _StrDestination with a string representation of _Flags by going through
// each bit set and calling _AsString on that value. If no bits are set, 
// _StrDestination will be set to _AllZerosValue.
char* strbitmask(char* _StrDestination, size_t _SizeofStrDestination, int _Flags, const char* _AllZerosValue, as_string_fn _AsString);
template<size_t size> char* strbitmask(char (&_StrDestination)[size], int _Flags, const char* _AllZerosValue, as_string_fn _AsString) { return strbitmask(_StrDestination, size, _Flags, _AllZerosValue, _AsString); }
template<typename T> char* strbitmask(char* _StrDestination, size_t _SizeofStrDestination, int _Flags, const char* _AllZerosValue, const char* (*_AsString)(T _Value)) { return strbitmask(_StrDestination, _SizeofStrDestination, _Flags, _AllZerosValue, (as_string_fn)_AsString); }
template<typename T, size_t size> char* strbitmask(char (&_StrDestination)[size], int _Flags, const char* _AllZerosValue, const char* (*_AsString)(T _Value)) { return strbitmask(_StrDestination, size, _Flags, _AllZerosValue, (as_string_fn)_AsString); }

// does a formatted strcat. This returns the total new length of the string. If 
// that length is >= _SizeofStrDestination, then the destination is not big 
// enough to hold the concatenation. This never returns -1 like snprintf.
inline int vsncatf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args)
{
	int len = static_cast<int>(strlen(_StrDestination ? _StrDestination : ""));
	int result = vsnprintf(_StrDestination + len, _SizeofStrDestination - len, _Format, _Args);
	if (result < -1 || static_cast<size_t>(result) > (_SizeofStrDestination - len))
		result = vsnprintf(nullptr, 0, _Format, _Args);
	return len + result;
}

inline int sncatf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, ...)
{
	va_list args; va_start(args, _Format);
	int l = vsncatf(_StrDestination, _SizeofStrDestination, _Format, args);
	va_end(args);
	return l;
}

template<size_t size> int vsncatf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args) { return vsncatf(_StrDestination, size, _Format, _Args): }
template<size_t size> int sncatf(char (&_StrDestination)[size], const char* _Format, ...)
{
	va_list args; va_start(args, _Format);
	int l = vsncatf(_StrDestination, size, _Format, args);
	va_end(args);
	return l;
}

// Fills the specified buffer with a size in days hours minutes seconds (and 
// milliseconds). This returns the length the string would be but writing stops
// at the end of _SizeofStrDestination. If length >= _SizeofStrDestination then
// the destination is not large enough to hold the nul-terminated string. This
// will not return -1 like snprintf does.
int format_duration(char* _StrDestination, size_t _SizeofStrDestination, double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true);
template<size_t size> int format_duration(char (&_StrDestination)[size], double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true) { return format_duration(_StrDestination, size, _TimeInSeconds, _Abbreviated, _IncludeMS); }

// Fills the specified buffer with a size in either bytes, KB, MB, GB, or TB 
// depending on the number of bytes specified. This returns the same result as 
// snprintf.
int format_bytes(char* _StrDestination, size_t _SizeofStrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits);

// Remove all chars found in _ToTrim from the beginning of the string. _Trimmed 
// can be the same as _StrSource. Returns _Trimmed.
char* trim_left(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim);
template<size_t size> char* trim_left(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_left(_Trimmed, size, _StrSource, _ToTrim); }

// Remove all chars found in _ToTrim form the end of the string. _Trimmed can 
// be the same as strSource. Returns _Trimmed.
char* trim_right(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim);
template<size_t size> char* trim_right(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_right(_Trimmed, size, _StrSource, _ToTrim); }

// Trims both the left and right side of a string
inline char* trim(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_right(_Trimmed, _SizeofTrimmed, trim_left(_Trimmed, _SizeofTrimmed, _StrSource, _ToTrim), _ToTrim); }
template<size_t size> char* trim(char (&_Trimmed)[size], const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim(_Trimmed, size, _StrSource, _ToTrim); }

// Returns the appropriate suffix [st nd rd th] for a number
const char* ordinal(size_t _Number);

// first param must be pointing into a string at the open brace. From there this 
// will find the brace at the same level of recursion - internal pairs are 
// ignored. The 'brace' can be any single character. This will return nullptr
// if the braces are unbalanced.
const char* next_matching(const char* _pPointingAtOpenBrace, char _CloseBrace);
char* next_matching(char* _pPointingAtOpenBrace, char _CloseBrace);

// Same as the single-char next_matching, but operates on braces that are 
// multiple characters. (c-style comments, #if #endif <!-- --> and other such 
// delimiters).
const char* next_matching(const char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace);
char* next_matching(char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace);

// _____________________________________________________________________________
// Text file format encoding.

// Text file formats are human readable! Except when storing human-readable 
// data. This section includes format-specific encode/decode functions.

#define oRESERVED_URI_CHARS "!*'();:@&=+$,?#[]/"
#define oRESERVED_URI_CHARS_WITH_SPACE oRESERVED_URI_CHARS " "

// encode a string with URI-compliant percent encoding. 
// _StrDestination and _StrSource must not overlap.
char* percent_encode(char* oRESTRICT _StrDestination, size_t _SizeofStrDestination, const char* oRESTRICT _StrSource, const char* _StrReservedChars = oRESERVED_URI_CHARS);
template<size_t size> char* percent_encode(char (&_StrDestination)[size], const char* _StrSource) { return percent_encode(_StrDestination, size, _StrSource, _StrReservedChars); }

// decode a string encoded with URI-compliant percent encoding.
// _StrDestination and _StrSource can be the same pointer.
char* percent_decode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> char* percent_decode(char (&_StrDestination)[size], const char* _StrSource) { return percent_decode(_StrDestination, size, _StrSource); }

// ensures all percent encodings use lower-case letter values. For exmaple this
// will convert %7A to %7a. // _StrDestination and _StrSource can be the same 
// pointer. (NOTE: percent_encode always returns lower-case values, so this is
// only necessary on percent-encoded strings that were not encoded with 
// percent_encode.)
char* percent_to_lower(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> char* percent_to_lower(char (&_StrDestination)[size], const char* _StrSource) { return percent_to_lower(_StrDestination, size, _StrSource); }

// encode a string with XML-compliant ampersand encoding.
char* ampersand_encode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> char* ampersand_encode(char (&_StrDestination)[size], const char* _StrSource) { return ampersand_encode(_StrDestination, size, _StrSource); }

// decode a string encoded with XML-compliant ampersand encoding.
char* ampersand_decode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> char* ampersand_decode(char (&_StrDestination)[size], const char* _StrSource) { return ampersand_decode(_StrDestination, size, _StrSource); }

// encode a string with JSON-compliant escape encoding.
char* json_escape_encode(char* _StrDestination, size_t _SizeofStrDestination, const char* _Source);
template<size_t size> char* json_escape_encode(char (&_StrDestination)[size], const char* _StrSource) { return json_escape_encode(_StrDestination, size, _StrSource); }

// decode a string encoded with JSON-compliant escape encoding.
char* json_escape_decode(char* _StrDestination, size_t _SizeofStrDestination, const char* _Source);
template<size_t size> char* json_escape_decode(char (&_StrDestination)[size], const char* _StrSource) { return json_escape_decode(_StrDestination, size, _StrSource); }

// _____________________________________________________________________________
// File path utilities

// If _ZeroBuffer is true, all extra chars in the destination will be set to 
// zero such that a memcmp of two cleaned buffers would be reliable.
char* clean_path(char* _StrDestination, size_t _SizeofStrDestination, const char* _SourcePath, char _FileSeparator = '/', bool _ZeroBuffer = false);
template<size_t size> char* clean_path(char (&_StrDestination)[size], const char* _SourcePath, char _FileSeparator = '/', bool _ZeroBuffer = false) { return clean_path(_StrDestination, _SizeofStrDestination, _SourcePath, _FileSeparator, _ZeroBuffer); }

// Fills _StrDestination with a version of FullPath that has all the common 
// parts between _BasePath and _FullPath removed and additionally has ../ 
// relative paths inserted until it is relative to the base path.
char* relativize_path(char* _StrDestination, size_t _SizeofStrDestination, const char* _BasePath, const char* _FullPath);
template<size_t size> char* relativize_path(char (&_StrDestination)[size], const char* _BasePath, const char* _FullPath) { return relativize_path(_StrDestination, size, _BasePath, _FullPath); }

} // namespace oStd

// The macros below are an inner-loop optimization with 5+% performance 
// implications, so please bear with the oddity. Basically this set of API is a 
// specialization of strspn and strcspn for text-file/string parsing to move 
// between whitespace-delimited words where a word is a series of characters 
// that are not whitespace. This is for parsing large data files like OBJ 3D 
// model files or for parsing C++/HLSL in small compilation efforts, or for just
// having fast next-word parsing for any text data. The LUT nature of the 
// implementations has a big impact, but factoring this code out undid a lot of
// the impact, so the solution was to factor definition of all this into a macro
// so that code locality advantages can be retained. Call 
// oDEFINE_WHITESPACE_PARSING() in a .cpp file and use the API documented below
// for pretty-darn-fast parsing capabilities.

// Returns true if c is any kind of whitespace
// inline bool is_whitespace(int c)

// Returns true if c is whitespace that is not a newline
// inline bool is_line_whitespace(int c)

// Returns true if c is a newline
// inline bool is_newline(int c)

// Updates the specified pointer to a string until it points at a non-whitespace 
// character or a newline. To reiterate, this does not skip newline whitespace.
// inline void move_past_line_whitespace(const char** _ppString)

// Updates the specified pointer to a string until it points at a whitespace 
// character, either line or newline whitespace.
// inline void move_to_whitespace(const char** _ppString);

// Updates the specified pointer to consume the current non-whitespace and then 
// skip any other non-newline whitespace, thus pointing at the next word, i.e.
// next non-whitespace block of characters that is not the current block of 
// characters.
// inline void move_next_word(const char** _ppString);

// Updates the specified pointer to a string until it points at a newline 
// character.
// inline void move_to_line_end(const char** _ppString);

// Updates the specified pointer to a string until it points at the first non-
// newline character.
// inline void move_past_newline(const char** _ppString);

#define oZ16 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define oZ16_12 oZ16, oZ16, oZ16, oZ16, oZ16, oZ16, oZ16, oZ16, oZ16, oZ16, oZ16, oZ16
#define oDEFINE_WHITESPACE_FUNCTION(_Name, _Criteria) inline void _Name(const char** _ppString) { while (**_ppString && _Criteria(**_ppString)) ++*_ppString; }
#define oDEFINE_WHITESPACE_PARSING() \
	namespace { \
		oALIGN(16) static const unsigned char is_whitespace__[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, oZ16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, oZ16, oZ16_12 }; \
		oALIGN(16) static const unsigned char is_line_whitespace__[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, oZ16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, oZ16, oZ16_12 }; \
		oALIGN(16) static const unsigned char is_newline__[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, oZ16, oZ16, oZ16, oZ16_12 }; \
	} namespace oStd { \
		inline bool is_whitespace(int c) { return !!is_whitespace__[c]; } \
		inline bool is_line_whitespace(int c) { return !!is_line_whitespace__[c]; } \
		inline bool is_newline(int c) { return !!is_newline__[c]; } \
		oDEFINE_WHITESPACE_FUNCTION(move_past_line_whitespace, is_line_whitespace) \
		oDEFINE_WHITESPACE_FUNCTION(move_to_whitespace, !is_whitespace) \
		oDEFINE_WHITESPACE_FUNCTION(move_to_line_end, !is_newline) \
		oDEFINE_WHITESPACE_FUNCTION(move_past_newline, is_newline) \
		inline void move_next_word(const char** _ppString) { move_to_whitespace(_ppString); move_past_line_whitespace(_ppString); } \
	}

#endif
