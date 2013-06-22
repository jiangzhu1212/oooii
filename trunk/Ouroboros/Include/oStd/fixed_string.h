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
// Encapsulate a nul-terminated c array of chars when std::string is too much
// overhead (small string opt is 16 bytes).
#pragma once
#ifndef oStd_fixed_string_h
#define oStd_fixed_string_h

#include <oStd/string.h>
#include <cstring>
#include <cwchar>
#include <stdexcept>

// shorthand for readability below
#define TSTR_PARAMS typename charT, size_t capacity_
#define TSTR template<TSTR_PARAMS>
#define STRT fixed_string<charT, capacity_>

namespace oStd {

	// note: there's no good way to return an error value from an assignment other
	// than throwing, so favor throwing. Secure CRT gets in the way of this by 
	// asserting with no override of the behavior, so avoid it where possible.

	template<typename charT> struct fixed_string_traits
	{
		static_assert(sizeof(charT) == 1, "bad char traits");
		typedef char char_type;
		typedef size_t size_type;
		static const char_type* safe(const char_type* _String) { return _String ? _String : ""; }
		static void copy(char_type* _StrDestination, size_type _SizeofStrDestination, const char_type* _StrSource)
		{
			size_t s = strlcpy(_StrDestination, safe(_StrSource), _SizeofStrDestination);
			if (s >= _SizeofStrDestination)
				throw std::length_error("destination is not large enough");
		}
		
		static void copy(char_type* _StrDestination, size_type _SizeofStrDestination, const wchar_t* _StrSource)
		{
			#pragma warning(disable:4996) // use wcsrtombs_s instead
			std::mbstate_t state = std::mbstate_t();
			const wchar_t* src = _StrSource ? _StrSource : L"";
			size_type len = 1 + std::wcsrtombs(nullptr, &src, 0, &state);
			const size_t CopyLen = __min(len, _SizeofStrDestination-1);
			std::wcsrtombs(_StrDestination, &src, CopyLen, &state);
			if (len >= _SizeofStrDestination)
				throw std::length_error("destination is not large enough");
			#pragma warning(default:4996)
		}

		static size_type len(const char_type* _String)
		{
			return strlen(safe(_String));
		}
	};

	template<> struct fixed_string_traits<wchar_t>
	{
		typedef wchar_t char_type;
		typedef size_t size_type;
		static const char_type* safe(const char_type* _String) { static const wchar_t* p = L""; return _String ? _String : p; }
		static void copy(char_type* _StrDestination, size_type _SizeofStrDestination, const char_type* _StrSource)
		{
			size_t s = wcslcpy(_StrDestination, safe(_StrSource), _SizeofStrDestination);
			if (s >= _SizeofStrDestination)
				throw std::length_error("destination is not large enough");
		}
		
		static void copy(char_type* _StrDestination, size_type _SizeofStrDestination, const char* _StrSource) 
		{
			#pragma warning(disable:4996) // use mbsrtowcs_s instead
			std::mbstate_t state = std::mbstate_t();
			const char* src = _StrSource ? _StrSource : "";
			size_type len = 1 + std::mbsrtowcs(nullptr, &src, 0, &state);
			const size_t CopyLen = __min(len, _SizeofStrDestination-1);
			std::mbsrtowcs(_StrDestination, &src, CopyLen, &state);
			if (len >= _SizeofStrDestination)
				throw std::length_error("destination is not large enough");
			#pragma warning(default:4996)
		}
		
		static size_type len(const char_type* _String)
		{
			return wcslen(safe(_String));
		}
	};

	template<typename charT, size_t capacity_, typename _Traits = fixed_string_traits<charT>>
	class fixed_string
	{
	public:
		// until we can make capacity() a constexpr expose the template arg
		static const size_t Capacity = capacity_;
		typedef typename _Traits::char_type char_type;
		typedef typename _Traits::size_type size_type;
		typedef char_type(&array_ref)[capacity_];
		typedef const char_type(&const_array_ref)[capacity_];

		fixed_string() { *s = 0; }
		fixed_string(const char_type* _Start, const char_type* _End) { assign(_Start, _End); }
		fixed_string(const char_type* _String) { _Traits::copy(s, capacity_, _String); }

		template<typename _CharU> fixed_string(const _CharU* _String) { operator=(_String); }
		template<typename _CharU, size_type _CapacityU> fixed_string(const fixed_string<_CharU, _CapacityU>& _That) { operator=(_That); }
		template<typename _CharU> const fixed_string& operator=(const _CharU* _That) { _Traits::copy(s, capacity_, _That); return *this; }
		template<typename _CharU, size_type _CapacityU> const fixed_string& operator=(const fixed_string<_CharU, _CapacityU>& _That) { _Traits::copy(s, capacity_, _That.c_str()); return *this; }

		void clear() { *s = 0; }
		bool empty() const { return *s == 0; }
		size_type size() const { return _Traits::len(s); }
		size_type length() const { return size(); }
		/* constexpr */ size_type capacity() const { return capacity_; }

		void assign(const char_type* _Start, const char_type* _End)
		{
			ptrdiff_t size = std::distance(_Start, _End);
			if (std::distance(_Start, _End) > capacity_)
				throw std::length_error("source string too long for destination");
			std::copy(_Start, _End, s);
			s[size] = 0;
		}

		void copy_to(charT* _StrDestination, size_t _SizeofStrDestination) const
		{
			_Traits::copy(_StrDestination, _SizeofStrDestination, s);
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

	private:
		char_type s[capacity_];

		// do not support direct comparisons
		bool operator==(const fixed_string& _That) const { return false; }
		bool operator==(const char_type* _That) const { return false; }
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

	TSTR int format_duration(STRT& _StrDestination, double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true) { return format_duration(_StrDestination, capacity_, _TimeInSeconds, _Abbreviated, _IncludeMS); }

	TSTR struct equal_to<STRT> { bool operator()(const oStd::STRT& x, const STRT& y) const { return !strcmp(x, y); } };
	TSTR struct equal_to_case_insensitive<STRT> { bool operator()(const oStd::STRT& x, const STRT& y) const { return !_stricmp(x, y); } };

	TSTR struct less<STRT> { bool operator()(const STRT& x, const STRT& y) const { return strcmp(x.c_str(), y.c_str()) < 0; } };
	TSTR struct less_case_insensitive<STRT> { bool operator()(const STRT& x, const STRT& y) const { return _stricmp(x, y) < 0; } };

	TSTR char* trim_left(STRT& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_left(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
	TSTR char* trim_right(STRT& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim_right(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
	TSTR char* trim(STRT& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return trim(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }

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
	TSTR char* ampersand_encode(STRT& _StrDestination, const char* _StrSource) { return ampersand_encode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* ampersand_decode(STRT& _StrDestination, const char* _StrSource) { return ampersand_decode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* json_escape_encode(STRT& _StrDestination, const char* _StrSource) { return json_escape_encode(_StrDestination, _StrDestination.capacity(), _StrSource); }
	TSTR char* json_escape_decode(STRT& _StrDestination, const char* _StrSource) { return json_escape_decode(_StrDestination, _StrDestination.capacity(), _StrSource); }

} // namespace oStd

TSTR int vsnprintf(oStd::STRT& _StrDestination, const char* _Format, va_list _Args)
{
	#pragma warning(disable:4996) // secure CRT warning
	return vsnprintf(_StrDestination, capacity_, _Format, _Args);
	#pragma warning(default:4996)
}

TSTR int snprintf(oStd::STRT& _StrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); int l = vsnprintf(_StrDestination, _Format, args); va_end(args); return l; }

#undef STRT
#undef TSTR
#endif
