// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_String_h
#define oBase_String_h

// String manipulation functions that really should be in the C/C++ standard.

#include <oBase/compiler_config.h>
#include <oBase/macros.h>
#include <oBase/container_support.h>
#include <cstdint>
#include <ctype.h>
#include <functional>

// The most-standard (but not standard) secure strcpy/strcats. The 3rd parameter
// is in number of characters, not bytes.
size_t strlcat(char* dst, const char* src, size_t dst_size);
size_t strlcpy(char* dst, const char* src, size_t dst_size);
size_t wcslcat(wchar_t* dst, const wchar_t* src, size_t dst_size);
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size);
size_t mbsltowsc(wchar_t* dst, const char* src, size_t dst_size);
size_t wcsltombs(char* dst, const wchar_t* src, size_t dst_size);

template<size_t count> size_t strlcat(char (&dst)[count], const char* src) { return strlcat(dst, src, count); }
template<size_t count> size_t strlcpy(char (&dst)[count], const char* src) { return strlcpy(dst, src, count); }
template<size_t count> size_t wcslcat(wchar_t (&dst)[count], const wchar_t* src) { return wcslcat(dst, src, count); }
template<size_t count> size_t wcslcpy(wchar_t (&dst)[count], const wchar_t* src) { return wcslcpy(dst, src, count); }
template<size_t count> size_t mbsltowcs(wchar_t (&dst)[count], const char* src) { return mbsltowcs(dst, src, count); }
template<size_t count> size_t wcsltombs(char* dst, const wchar_t* src, size_t dst_size) { return wcsltombs(dst, src, count); }

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

	inline int vsnwprintf(wchar_t* dst, size_t _NumChars, const wchar_t* fmt, va_list args)
	{
		#pragma warning(disable:4996) // secure CRT warning
		int l = ::_vsnwprintf(dst, _NumChars, fmt, args);
		#pragma warning(default:4996)
		return l;
	}
	template<size_t size> int vsnwprintf(wchar_t (&dst)[size], const wchar_t* fmt, va_list args) { return vsnwprintf(dst, size, fmt, args); }

	inline char* strncpy(char* dst, size_t dst_size, const char* src, size_t _NumChars)
	{
		return strncpy_s(dst, dst_size, src, _NumChars) ? nullptr : dst;
	}

	template<size_t size> char* strncpy(char (&dst)[size], const char* src, size_t _NumChars) { return strncpy(dst, size, src, _NumChars); }

	inline wchar_t* wcsncpy(wchar_t* dst, size_t _NumDestinationChars, const wchar_t* src, size_t _NumChars)
	{
		return wcsncpy_s(dst, _NumDestinationChars, src, _NumChars) ? nullptr : dst;
	}

	template<size_t size> char* wcsncpy(wchar_t (&dst)[size], const wchar_t* src, size_t _NumChars) { return wcsncpy(dst, size, src, _NumChars); }
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

namespace ouro {

// _____________________________________________________________________________
// String formatting

// tolower doesn't work on wchar_ts and std::to_lower has annoying locale required
inline char to_lower(char c) { return (char)tolower(c); }
inline char to_upper(char c) { return (char)toupper(c); }
wchar_t to_lower(wchar_t wc);
wchar_t to_upper(wchar_t wc);

template<typename charT> void to_lower(charT* str) { while (*str) *str++ = to_lower(*str); }
template<typename charT> void to_upper(charT* str) { while (*str) *str++ = to_upper(*str); }
template<typename charT> void to_lower(charT* begin, charT* end) { while (begin < end) *begin++ = to_lower(*begin); }
template<typename charT> void to_upper(charT* begin, charT* end) { while (begin < end) *begin++ = to_upper(*begin); }

// Adds "...\0" to the end of the buffer - useful when recovering from string 
// overruns that can be truncated.
char* ellipsize(char* dst, size_t dst_size);
template<size_t size> char* ellipsize(char (&dst)[size]) { return ellipsize(dst, size); }

wchar_t* wcsellipsize(wchar_t* dst, size_t _NumChars);
template<size_t n> wchar_t* ellipsize(wchar_t (&dst)[n]) { return wcsellipsize(dst, n); }

// Fills the specified buffer with a size in days hours minutes seconds (and 
// milliseconds). This returns the length the string would be but writing stops
// at the end of dst_size. If length >= dst_size then
// the destination is not large enough to hold the nul-terminated string. This
// will not return -1 like snprintf does.
int format_duration(char* dst, size_t dst_size, double seconds, bool abbreviated = false, bool include_milliseconds = true);
template<size_t size> int format_duration(char (&dst)[size], double seconds, bool abbreviated = false, bool include_milliseconds = true) { return format_duration(dst, size, seconds, abbreviated, include_milliseconds); }

// Fills the specified buffer with a size in either bytes, KB, MB, GB, or TB 
// depending on the number of bytes specified. This returns the same result as 
// snprintf.
int format_bytes(char* dst, size_t dst_size, uint64_t bytes, size_t num_precision_digits);

// For numbers, this inserts commas where they ought to be (every 3 numbers)
// this returns nullptr on failure, else dst.
char* format_commas(char* dst, size_t dst_size, int number);
template<size_t size> char* format_commas(char (&dst)[size], int number) { return format_commas(dst, size, number); }
char* format_commas(char* dst, size_t dst_size, unsigned int number);
template<size_t size> char* format_commas(char (&dst)[size], unsigned int number) { return format_commas(dst, size, number); }

// Returns the appropriate suffix [st nd rd th] for a number
const char* ordinal(size_t number);

// does a formatted strcat. This returns the total new length of the string. If 
// that length is >= dst_size, then the destination is not big 
// enough to hold the concatenation. This never returns -1 like snprintf.
inline int vsncatf(char* dst, size_t dst_size, const char* fmt, va_list args)
{
	int len = static_cast<int>(strlen(dst ? dst : ""));
	int result = vsnprintf(dst + len, dst_size - len, fmt, args);
	if (result < -1 || static_cast<size_t>(result) > (dst_size - len))
		result = vsnprintf(nullptr, 0, fmt, args);
	return len + result;
}

inline int sncatf(char* dst, size_t dst_size, const char* fmt, ...)
{
	va_list args; va_start(args, fmt);
	int l = vsncatf(dst, dst_size, fmt, args);
	va_end(args);
	return l;
}

template<size_t size> int vsncatf(char* dst, size_t dst_size, const char* fmt, va_list args) { return vsncatf(dst, size, fmt, args): }
template<size_t size> int sncatf(char (&dst)[size], const char* fmt, ...)
{
	va_list args; va_start(args, fmt);
	int l = vsncatf(dst, size, fmt, args);
	va_end(args);
	return l;
}

// Remove all chars found in to_trim from the beginning of the string. trimmed 
// can be the same as src. Returns trimmed.
char* trim_left(char* trimmed, size_t trimmed_size, const char* src, const char* to_trim);
template<size_t size> char* trim_left(char (&trimmed)[size], const char* src, const char* to_trim = oWHITESPACE) { return trim_left(trimmed, size, src, to_trim); }

// Remove all chars found in to_trim form the end of the string. trimmed can 
// be the same as strSource. Returns trimmed.
char* trim_right(char* trimmed, size_t trimmed_size, const char* src, const char* to_trim);
template<size_t size> char* trim_right(char (&trimmed)[size], const char* src, const char* to_trim = oWHITESPACE) { return trim_right(trimmed, size, src, to_trim); }

// Trims both the left and right side of a string
inline char* trim(char* trimmed, size_t trimmed_size, const char* src, const char* to_trim = oWHITESPACE) { return trim_right(trimmed, trimmed_size, trim_left(trimmed, trimmed_size, src, to_trim), to_trim); }
template<size_t size> char* trim(char (&trimmed)[size], const char* src, const char* to_trim = oWHITESPACE) { return trim(trimmed, size, src, to_trim); }

// Replaces any run of whitespace with a single ' ' character. Returns dst
char* clean_whitespace(char* dst, size_t dst_size, const char* src, char replacement = ' ', const char* _ToPrune = oWHITESPACE);
template<size_t size> char* clean_whitespace(char (&dst)[size], const char* src, char replacement = ' ', const char* _ToPrune = oWHITESPACE) { return clean_whitespace(dst, dst_size, src, replacement, _ToPrune); }

// _____________________________________________________________________________
// Search

// like strstr, but finds the last occurrence as if searching in reverse
const char* rstrstr(const char* str, const char* substring);
char* rstrstr(char* str, const char* substring);

// behaves like strcspn but starts are str1 and goes through the string in
// reverse until buffer_start is reached.
size_t rstrcspn(const char* buffer_start, const char* str1, const char* str2);
size_t rstrcspn(char* buffer_start, char* str1, const char* str2);

// Insert one string into another in-place. insertion_point must point into 
// src. If replacement_length is non-zero then that number of characters 
// from insertion_point on will be overwritten by the insertion. This returns
// the position immediately after the new insertion. If there isn't enough room
// in the buffer, then nullptr is returned.
char* insert(char* src, size_t src_size, char* insertion_point, size_t replacement_length, const char* insertion);
template<size_t size> char* insert(char (&src)[size], char* insertion_point, size_t replacement_length, const char* insertion)  { return insert(src, size, insertion_point, replacement_length, insertion); }

// replace all occurrences of find in src with replace and copy 
// the result to dst. 
errno_t replace(char* oRESTRICT result, size_t result_size, const char* oRESTRICT src, const char* oRESTRICT find, const char* oRESTRICT replace);
template<size_t size> errno_t replace(char (&result)[size], const char* oRESTRICT src, const char* oRESTRICT find, const char* oRESTRICT replace) { return replace(result, size, src, find, replace); }

// char version of above
errno_t replace(char* oRESTRICT result, size_t result_size, const char* oRESTRICT src, char _ChrFind, char replace);
template<size_t size> errno_t replace(char (&result)[size], const char* oRESTRICT src, char _ChrFind, char replace) { return replace(result, size, src, _ChrFind, replace); }

// first param must be pointing into a string at the open brace. From there this 
// will find the brace at the same level of recursion - internal pairs are 
// ignored. The 'brace' can be any single character. This will return nullptr
// if the braces are unbalanced.
const char* next_matching(const char* p_open_brace, char close_brace);
char* next_matching(char* p_open_brace, char close_brace);

// first param must be pointing into a string at the close brace. From there this
// will find the brace at the same level of recursion - internal pairs are ignored.
// The 'brace' can be any single character. This will return nullptr if the braces
// are unbalanced.
const char* prev_matching(const char* buffer_start, const char* p_close_brace, char open_brace);
char* prev_matching(char* buffer_start, char* p_close_brace, char open_brace);

// Same as the single-char next_matching but operates on braces that are multiple 
// characters. (c-style comments, #if #endif <!-- --> and other such delimiters).
const char* next_matching(const char* p_open_brace, const char* open_brace, const char* close_brace);
char* next_matching(char* p_open_brace, const char* open_brace, const char* close_brace);

// Same as the single-char prev_matching but operates on braces that are multiple 
// characters. (c-stle comments, #if #endif <!-- --> and other such delimiters).
const char* prev_matching(const char* buffer_start, const char* p_close_brace, const char* open_brace, const char* close_brace);
char* prev_matching(char* buffer_start, char* p_close_brace, const char* open_brace, const char* close_brace);

// _____________________________________________________________________________
// Parsing

// copy token first for multi-threaded or non-destructive strtok. If the 
// parsing exits early end_strtok() must be called on ctx. If parsing
// exits due to a nullptr return value, cleanup need not be called.
char* strtok(const char* token, const char* delim, char** ctx);
void end_strtok(char** ctx);

// This function takes a single value (usually an enum value) and returns a 
// constant string representation of it.
typedef const char* (*asstr_fn)(const int& single_flag);

// Fill dst with a string representation of flags by going through
// each bit set and calling as_string on that value. If no bits are set, 
// dst will be set to all_zeros_value.
char* strbitmask(char* dst, size_t dst_size, int flags, const char* all_zeros_value, asstr_fn as_string);
template<size_t size> char* strbitmask(char (&dst)[size], int flags, const char* all_zeros_value, asstr_fn as_string) { return strbitmask(dst, size, flags, all_zeros_value, as_string); }
template<typename T> char* strbitmask(char* dst, size_t dst_size, int flags, const char* all_zeros_value, const char* (*as_string)(T value)) { return strbitmask(dst, dst_size, flags, all_zeros_value, (asstr_fn)as_string); }
template<typename T, size_t size> char* strbitmask(char (&dst)[size], int flags, const char* all_zeros_value, const char* (*as_string)(T value)) { return strbitmask(dst, size, flags, all_zeros_value, (asstr_fn)as_string); }

// Returns the pointer into typeinfo_name that represents just the name of the 
// user type, thus skipping any prefix [enum|class|struct|union]. This does not
// behave well for built-in types.
const char* type_name(const char* typeinfo_name);

// Like this very C++ comment! This function replaces the comment with the 
// specified char. comment_prefix for C++ would be "//" but could also be ";" or 
// "'" or "--" for other languages.
char* zero_line_comments(char* str, const char* comment_prefix, char replacement = ' ');

// Zeros-out all text sections delimited by the open and close braces, useful for 
// getting rid of block comments or #if 0/#endif blocks. This preserves newlines so 
// accurate line numbers can be reported in case of subsequent compile errors.
char* zero_block_comments(char* str, const char* open_brace, const char* close_brace, char replacement = ' ');

// This function uses the specified macros to go through and evaluate C-style
// #if* statements (#if, #ifdef, #elif, #else, #endif) to zero out undefined
// code. The final macro should be { nullptr, nullptr } as a nul terminator.
struct macro
{
	const char* symbol;
	const char* value;
};

char* zero_ifdefs(char* str, const macro* macros, char replacement);

// Combine the above 3 functions for the typical use case
inline char* zero_non_code(char* str, const macro* macros, char replacement = ' ')
{
	if (zero_block_comments(str, "/*", "*/", replacement)
		&& zero_ifdefs(str, macros, replacement)
		&& zero_line_comments(str, "//", replacement))
			return str;
	return nullptr;
}

// Convert a buffer into a C++ array. This is useful when you want to embed data 
// in code itself. This fills the destination string with a declaration of the 
// form:
// const <specifiedType> <specifiedName>[] = { <buffer data> };
// This also defines a function of the form:
// void get_<buffer_name>(const char** ppBufferName, const void** ppBuffer, size_t* pSize)
// that can be externed and used to access the buffer. Any extension '.' in the
// specified bufferName will be replaced with '_', so get_MyFile_txt(...)

size_t codify_data(char* dst
									 , size_t dst_size
									 , const char* buffer_name
									 , const void* buf
									 , size_t buf_size
									 , size_t word_size);

// _____________________________________________________________________________
// Encoding

// Text file formats are human readable! Except when storing human-readable 
// data. This section includes format-specific encode/decode functions.

#define oRESERVED_URI_CHARS "!*'();:@&=+$,?#[]/"
#define oRESERVED_URI_CHARS_WITH_SPACE oRESERVED_URI_CHARS " "

// encode a string with URI-compliant percent encoding. 
// dst and src must not overlap.
char* percent_encode(char* oRESTRICT dst, size_t dst_size, const char* oRESTRICT src, const char* reserved_chars = oRESERVED_URI_CHARS);
template<size_t size> char* percent_encode(char (&dst)[size], const char* src) { return percent_encode(dst, size, src, reserved_chars); }

// decode a string encoded with URI-compliant percent encoding.
// dst and src can be the same pointer.
char* percent_decode(char* dst, size_t dst_size, const char* src);
template<size_t size> char* percent_decode(char (&dst)[size], const char* src) { return percent_decode(dst, size, src); }

// ensures all percent encodings use lower-case letter values. For exmaple this
// will convert %7A to %7a. // dst and src can be the same 
// pointer. (NOTE: percent_encode always returns lower-case values, so this is
// only necessary on percent-encoded strings that were not encoded with 
// percent_encode.)
char* percent_to_lower(char* dst, size_t dst_size, const char* src);
template<size_t size> char* percent_to_lower(char (&dst)[size], const char* src) { return percent_to_lower(dst, size, src); }

// encode a string with XML-compliant ampersand encoding.
char* ampersand_encode(char* dst, size_t dst_size, const char* src);
template<size_t size> char* ampersand_encode(char (&dst)[size], const char* src) { return ampersand_encode(dst, size, src); }

// decode a string encoded with XML-compliant ampersand encoding.
char* ampersand_decode(char* dst, size_t dst_size, const char* src);
template<size_t size> char* ampersand_decode(char (&dst)[size], const char* src) { return ampersand_decode(dst, size, src); }

// encode a string with JSON-compliant escape encoding.
char* json_escape_encode(char* dst, size_t dst_size, const char* _Source);
template<size_t size> char* json_escape_encode(char (&dst)[size], const char* src) { return json_escape_encode(dst, size, src); }

// decode a string encoded with JSON-compliant escape encoding.
char* json_escape_decode(char* dst, size_t dst_size, const char* _Source);
template<size_t size> char* json_escape_decode(char (&dst)[size], const char* src) { return json_escape_decode(dst, size, src); }

// _____________________________________________________________________________
// Path Parsing

// If zero_buffer is true, all extra chars in the destination will be set to 
// zero such that a memcmp of two cleaned buffers would be reliable.
char* clean_path(char* dst, size_t dst_size, const char* src_path, char separator = '/', bool zero_buffer = false);
template<size_t size> char* clean_path(char (&dst)[size], const char* src_path, char separator = '/', bool zero_buffer = false) { return clean_path(dst, size, src_path, separator, zero_buffer); }

// Fills dst with a version of FullPath that has all the common 
// parts between base_path and full_path removed and additionally has ../ 
// relative paths inserted until it is relative to the base path.
char* relativize_path(char* dst, size_t dst_size, const char* base_path, const char* full_path);
template<size_t size> char* relativize_path(char (&dst)[size], const char* base_path, const char* full_path) { return relativize_path(dst, size, base_path, full_path); }

// Standard Unix/MS-DOS style wildcard matching
bool matches_wildcard(const char* wildcard, const char* path);

// Returns the number of characters common to both specified paths.
size_t cmnroot(const char* path1, const char* path2);
size_t wcmnroot(const wchar_t* path1, const wchar_t* path2);

// Fills pointers into the specified path where different components start. If
// the component value does not exists, the pointer is filled with nullptr. This 
// returns the length of path.
size_t split_path(const char* path
	, bool _Posix
	, const char** out_root
	, const char** out_path
	, const char** out_parent_path_end
	, const char** out_basename
	, const char** out_ext);

size_t wsplit_path(const wchar_t* path
	, bool _Posix
	, const wchar_t**		
	, const wchar_t** out_path
	, const wchar_t** out_parent_path_end
	, const wchar_t** out_basename
	, const wchar_t** out_ext);

template<typename charT>
size_t tsplit_path(const charT* path
	, bool _Posix
	, const charT** out_root
	, const charT** out_path
	, const charT** out_parent_path_end
	, const charT** out_basename
	, const charT** out_ext)
{ return split_path(path, _Posix, out_root, out_path, out_parent_path_end, out_basename, out_ext); }

template<>
inline size_t tsplit_path(const wchar_t* path
	, bool _Posix
	, const wchar_t** out_root
	, const wchar_t** out_path
	, const wchar_t** out_parent_path_end
	, const wchar_t** out_basename
	, const wchar_t** out_ext)
{ return wsplit_path(path, _Posix, out_root, out_path, out_parent_path_end, out_basename, out_ext); }

// search_paths is a semi-colon-delimited set of strings (like Window's PATH
// environment variable). dot_path is a path to append if relative_path begins
// with a '.', like "./myfile.txt". This function goes through each of 
// search_paths paths and dot_path and prepends it to relative_path. If the 
// result causes the path_exists function to return true the full path is copied 
// into dst and a pointer to dst is returned. If all
// paths are exhausted without passing path_exists nullptr is returned.
char* search_path(char* dst
	, size_t dst_size
	, const char* search_paths
	, const char* relative_path
	, const char* dot_path
	, const std::function<bool(const char* path)>& path_exists);

template<size_t size>
char* search_path(char (&dst)[size]
	, const char* search_paths
	, const char* relative_path
	, const char* dot_path
	, const std::function<bool(const char* path)>& path_exists)
{
	return search_path(dst, size, search_paths, relative_path, dot_path, path_exists);
}

// Parses a single string of typical command line parameters into an argv-style
// array of strings. This uses the specified allocator, or malloc if nullptr is 
// specified. If app_path is non-null, it will be copied into the 0th element of 
// the returned argv. If that isn't desired, or the app path is already in 
// command_line, pass nullptr for app_path. The number of arguments will be 
// returned in out_num_args. On Windows this can be used similarly to 
// CommandLineToArgvW and serves as an implementation of CommandLineToArgvA that
// Windows does not provide.
const char** argtok(void* (*allocate)(size_t), const char* app_path, const char* command_line, int* out_num_args);

// _____________________________________________________________________________
// standard container support

template<> struct less<const char*> { int operator()(const char* x, const char* y) const { return strcmp(x, y) < 0; } };
template<> struct less_i<const char*> { bool operator()(const char* x, const char* y) const { return _stricmp(x, y) < 0; } };

template<> struct same<const char*> { int operator()(const char* x, const char* y) const { return !strcmp(x, y); } };
template<> struct same_i<const char*> { bool operator()(const char* x, const char* y) const { return !_stricmp(x, y); } };

}

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
		oALIGNAS(16) static const unsigned char is_whitespace__[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, oZ16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, oZ16, oZ16_12 }; \
		oALIGNAS(16) static const unsigned char is_line_whitespace__[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, oZ16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, oZ16, oZ16_12 }; \
		oALIGNAS(16) static const unsigned char is_newline__[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, oZ16, oZ16, oZ16, oZ16_12 }; \
	} namespace ouro { \
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
