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
// Encapsulate a nul-terminated c array of chars when std::string is too much
// overhead (small string opt is 16 bytes).
#pragma once
#ifndef oStd_fixed_string_h
#define oStd_fixed_string_h

#include <oStd/opttok.h>
#include <oStd/string.h>
#include <oStd/string_traits.h>
#include <cstring>
#include <cwchar>
#include <stdexcept>

// shorthand for readability below
#define TSTR_PARAMS typename charT, size_t Capacity
#define TSTR template<TSTR_PARAMS>
#define STRT fixed_string<charT, Capacity>

#define oFIXED_STRING_THROW_LEN_ERR() throw std::length_error("destination is not large enough")

namespace oStd {

	template<typename charT, size_t capacity_, typename traitsT = string_traits<charT>>
	class fixed_string
	{
	public:
		// until we can make capacity() a constexpr expose the template arg
		static const size_t Capacity = capacity_;
		typedef traitsT traits;
		typedef typename traits::char_type char_type;
		typedef typename traits::size_type size_type;
		typedef char_type* iterator;
		typedef const char_type* const_iterator;
		typedef std::pair<const char_type*, const char_type*> string_piece_type;
		typedef char_type(&array_ref)[Capacity];
		typedef const char_type(&const_array_ref)[Capacity];

		fixed_string() { *s = 0; }
		fixed_string(const string_piece_type& _StringPiece) { assign(_StringPiece.first, _StringPiece.second); }
		fixed_string(const char_type* _Start, const char_type* _End) { assign(_Start, _End); }
		fixed_string(const char_type* _String) { traits::copy(s, _String, Capacity); }

		template<typename _CharU> fixed_string(const _CharU* _String) { operator=(_String); }
		template<typename _CharU, size_type _CapacityU> fixed_string(const fixed_string<_CharU, _CapacityU>& _That) { operator=(_That); }
		template<typename _CharU> const fixed_string& operator=(const _CharU* _That) { if (string_traits<_CharU>::copy(s, _That, Capacity) > Capacity) oFIXED_STRING_THROW_LEN_ERR(); return *this; }
		template<typename _CharU, size_type _CapacityU> const fixed_string& operator=(const fixed_string<_CharU, _CapacityU>& _That) { if (string_traits<_CharU>::copy(s, _That.c_str(), Capacity) > Capacity) oFIXED_STRING_THROW_LEN_ERR(); return *this; }

		iterator begin() { return s; }
		const_iterator begin() const { return s; }

		// NOTE: To keep class size to same-as-c-array, length is recalculated on
		// each call, so the typical for (auto it = begin(); it !+ end(); ++it) 
		// idiom may be costly. Factor out the end call if the compiler doesn't do 
		// automatically it.
		iterator end() { return s + length(); }
		const_iterator end() const { return s + length(); }

		void clear() { *s = 0; }
		bool empty() const { return *s == 0; }
		size_type size() const { return traits::length(s); }
		size_type length() const { return size(); }
		/* constexpr */ size_type capacity() const { return Capacity; }

		fixed_string& assign(const char_type* _Start, const char_type* _End)
		{
			ptrdiff_t size = std::distance(_Start, _End);
			if (size >= Capacity)
				oFIXED_STRING_THROW_LEN_ERR();
			traits::copy(s, _Start, Capacity);
			s[size] = 0;
			return *this;
		}

		fixed_string& append(const char_type* _String) { if (traits::cat(s, _String, Capacity) > Capacity) oFIXED_STRING_THROW_LEN_ERR(); return *this; }

		fixed_string& append(const char_type* _Start, const char_type* _End)
		{
			size_type size = length();
			char_type* start = s + size;
			size += std::distance(_Start, _End);
			if (size >= Capacity)
				oFIXED_STRING_THROW_LEN_ERR();
			traits::copy(start, _Start, Capacity);
			s[size] = 0;
			return *this;
		}

		fixed_string& append(const string_piece_type& _StringPiece) { return append(_StringPiece.first, _StringPiece.second); }

		fixed_string& append(char_type _Char) { char_type str[2] = { _Char, '\0'}; return append(str); }

		fixed_string& operator+=(const char_type* _String) { return append(_String); }
		fixed_string& operator+=(const string_piece_type& _StringPiece) { return append(_StringPiece); }
		fixed_string& operator+=(char_type _Char) { return append(_Char); }

		void copy_to(charT* _StrDestination, size_t _SizeofStrDestination) const
		{
			if (traits::copy(_StrDestination, s, _SizeofStrDestination) > _SizeofStrDestination)
				oFIXED_STRING_THROW_LEN_ERR();
		}

		array_ref c_str() { return s; }
		const_array_ref c_str() const { return s; }

		operator char_type*() { return s; }
		operator const char_type*() const { return s; }

		// @ooii-tony: FIXME this was supposed to make coding easier because 99% of
		// fixed strings returned from a const threadsafe/volatile API are 
		// immutable, but there's no guarantee. Fix this once other refactoring is 
		// done.
		operator const char_type* const() const volatile { return const_cast<const char_type* const>(s); }

		int compare(const char_type* _That) const { return traits::compare(s, _That); }
		int compare(const fixed_string& _That) const { return compare(_That.c_str()); }

		bool operator==(const char_type* _That) const { return !compare(_That); }
		bool operator==(const fixed_string& _That) const { return !compare(_That); }

		bool operator<(const fixed_string& _That) const { return compare(_That) < 0; }
		bool operator<(const char_type* _That) const { return compare(_That) < 0; }

	private:
		char_type s[Capacity];
	};

	typedef fixed_string<char, 64> sstring;
	typedef fixed_string<char, 128> mstring;
	typedef fixed_string<char, 512> lstring;
	typedef fixed_string<char, 2048> xlstring;
	typedef fixed_string<char, 4096> xxlstring;
	typedef lstring path_string;
	typedef lstring uri_string;

	typedef fixed_string<wchar_t, 64> swstring;
	typedef fixed_string<wchar_t, 128> mwstring;
	typedef fixed_string<wchar_t, 512> lwstring;
	typedef fixed_string<wchar_t, 2048> xlwstring;
	typedef fixed_string<wchar_t, 4096> xxlwstring;
	typedef lwstring path_wstring;
	typedef lwstring uri_wstring;

	template<typename T> struct equal_to {};
	template<typename T> struct equal_to_case_insensitive {};
	template<typename T> struct less {};
	template<typename T> struct less_case_insensitive {};

	TSTR int format_duration(STRT& _StrDestination, double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true) { return format_duration(_StrDestination, Capacity, _TimeInSeconds, _Abbreviated, _IncludeMS); }
	TSTR int format_bytes(STRT& _StrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits) { return format_bytes(_StrDestination, _StrDestination.capacity(), _NumBytes, _NumPrecisionDigits); }
	TSTR char* format_commas(STRT& _StrDestination, int _Number) { return format_commas(_StrDestination, _StrDestination.capacity(), _Number); }
	TSTR char* format_commas(STRT& _StrDestination, unsigned int _Number) { return format_commas(_StrDestination, _StrDestination.capacity(), _Number); }

	TSTR struct equal_to<STRT> { bool operator()(const oStd::STRT& x, const STRT& y) const { return !strcmp(x, y); } };
	TSTR struct equal_to_case_insensitive<STRT> { bool operator()(const oStd::STRT& x, const STRT& y) const { return !_stricmp(x, y); } };

	TSTR struct less<STRT> { bool operator()(const STRT& x, const STRT& y) const { return strcmp(x.c_str(), y.c_str()) < 0; } };
	TSTR struct less_case_insensitive<STRT> { bool operator()(const STRT& x, const STRT& y) const { return _stricmp(x, y) < 0; } };

	template<size_t capacity> char* ellipsize(fixed_string<char, capacity>& _StrDestination) { return ellipsize(_StrDestination.c_str(), _StrDestination.capacity()); }
	template<size_t capacity> wchar_t* ellipsize(fixed_string<wchar_t, capacity>& _StrDestination) { return wcsellipsize(_StrDestination.c_str(), _StrDestination.capacity()); }

	TSTR char* trim_left(STRT& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_left(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
	TSTR char* trim_right(STRT& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_right(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
	TSTR char* trim(STRT& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
	TSTR char* clean_whitespace(STRT& _StrDestination, const char* _StrSource, char _Replacement = ' ', const char* _ToPrune = oWHITESPACE) { return clean_whitespace(_StrDestination, _StrDestination.capacity(), _StrSource, _Replacement, _ToPrune); }

	TSTR char* insert(STRT& _StrSource, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion)  { return insert(_StrSource, _StrSource.capacity(), _InsertionPoint, _ReplacementLength, _Insertion); }
	TSTR errno_t replace(STRT& _StrResult, const char* oRESTRICT _StrSource, const char* _StrFind, const char* _StrReplace) { return replace(_StrResult, _StrResult.capacity(), _StrSource, _StrFind, _StrReplace); }
	TSTR errno_t replace(STRT& _StrResult, const char* oRESTRICT _StrSource, char _ChrFind, char _ChrReplace) { return replace(_StrResult, _StrResult.capacity(), _StrSource, _ChrFind, _ChrReplace); }

	TSTR char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const STRT& _Value)
	{
		try { _Value.copy_to(_StrDestination, _SizeofStrDestination); }
		catch (std::exception&) { return nullptr; }
		return _StrDestination;
	}

	TSTR bool from_string(STRT* _pValue, const char* _StrSource)
	{
		try { *_pValue = _StrSource; }
		catch (std::exception&) { return false; }
		return true;
	}

	template<TSTR_PARAMS, typename T> char* to_string(STRT& _StrDestination, const T& _Value) { return to_string(_StrDestination.c_str(), _StrDestination.capacity(), _Value); }

	TSTR void tolower(STRT& _String) { tolower<STRT::char_type>(_String.c_str()); }
	TSTR void toupper(STRT& _String) { toupper<STRT::char_type>(_String.c_str()); }
	
	TSTR int vsncatf(STRT& _StrDestination, const char* _Format, va_list _Args) { return vsncatf(_StrDestination, _StrDestination.capacity(), _Format, _Args); }
	TSTR int sncatf(STRT& _StrDestination, const char* _Format, ...)
	{
		va_list args; va_start(args, _Format);
		int l = vsncatf(_StrDestination, _Format, args);
		va_end(args);
		return l;
	}

	TSTR char* percent_encode(STRT& _StrDestination, const char* _StrSource, const char* _StrReservedChars = oRESERVED_URI_CHARS) { return percent_encode(_StrDestination, _StrDestination.capacity(), _StrSource, _StrReservedChars); }
	TSTR char* percent_decode(STRT& _StrDestination, const char* _StrSource) { return percent_decode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* percent_to_lower(STRT& _StrDestination, const char* _StrSource) { return percent_to_lower(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* ampersand_encode(STRT& _StrDestination, const char* _StrSource) { return ampersand_encode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* ampersand_decode(STRT& _StrDestination, const char* _StrSource) { return ampersand_decode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* json_escape_encode(STRT& _StrDestination, const char* _StrSource) { return json_escape_encode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* json_escape_decode(STRT& _StrDestination, const char* _StrSource) { return json_escape_decode(_StrDestination, _StrDestination.capacity(), _StrSource); }

	TSTR char* clean_path(STRT& _StrDestination, const char* _SourcePath, char _FileSeparator = '/', bool _ZeroBuffer = false) { return clean_path(_StrDestination, _StrDestination.capacity(), _SourcePath, _FileSeparator, _ZeroBuffer); }
	TSTR char* relativize_path(STRT& _StrDestination, const char* _BasePath, const char* _FullPath) { return relativize_path(_StrDestination, _StrDestination.capacity(), _BasePath, _FullPath); }

	TSTR int vsnprintf(STRT& _StrDestination, const char* _Format, va_list _Args) { return vsnprintf(_StrDestination, Capacity, _Format, _Args); }
	TSTR char* strncpy(STRT& _StrDestination, const char* _StrSource, size_t _NumChars) { return strncpy(_StrDestination, _StrDestination.capacity(), _StrSource, _NumChars); }
	TSTR char* wcsncpy(STRT& _StrDestination, const wchar_t* _StrSource, size_t _NumChars) { return wcsncpy(_StrDestination, _StrDestination.capacity(), _StrSource, _NumChars); }

	TSTR char* optdoc(STRT& _StrDestination, const char* _AppName, const option* _pOptions, size_t _NumOptions, const char* _LooseParameters = "") { return optdoc(_StrDestination, _StrDestination.capacity(), _AppName, _pOptions, _NumOptions, _LooseParameters); }
	template<typename charT, size_t Capacity, size_t size> char* optdoc(STRT& _StrDestination, const char* _AppName, const option (&_pOptions)[size], const char* _LooseParameters = "") { return optdoc(_StrDestination, _StrDestination.capacity(), _AppName, _pOptions, size, _LooseParameters); }
} // namespace oStd

TSTR int snprintf(oStd::STRT& _StrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); int l = oStd::vsnprintf(_StrDestination, _Format, args); va_end(args); return l; }
TSTR size_t strlcat(oStd::STRT& _StrDestination, const char* _StrSource) { return strlcat(_StrDestination, _StrSource, _StrDestination.capacity()); }
TSTR size_t strlcpy(oStd::STRT& _StrDestination, const char* _StrSource) { return strlcpy(_StrDestination, _StrSource, _StrDestination.capacity()); }
TSTR size_t wcslcat(oStd::STRT& _StrDestination, const wchar_t* _StrSource) { return wcslcat(_StrDestination, _StrSource, _StrDestination.capacity()); }
TSTR size_t wcslcpy(oStd::STRT& _StrDestination, const wchar_t* _StrSource) { return wcslcpy(_StrDestination, _StrSource, _StrDestination.capacity()); }
TSTR size_t mbsltowsc(oStd::STRT& _StrDestination, const char* _StrSource) { return mbsltowsc(_StrDestination, _StrSource, _StrDestination.capacity()); }
TSTR size_t wcsltombs(oStd::STRT& _StrDestination, const wchar_t* _StrSource) { return wcsltombs(_StrDestination, _StrSource, _StrDestination.capacity()); }

#undef STRT
#undef TSTR

#endif
