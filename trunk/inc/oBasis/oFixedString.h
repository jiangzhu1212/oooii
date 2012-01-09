// $(header)
// A wrapper for a c-style char array that makes it a bit more like STL and 
// provides niceties such as auto-construction and shorthand assignment.
// Underneath it behaves exactly like a char array and should be used as such.


#pragma once
#ifndef oFixedString_h
#define oFixedString_h

#include <oBasis/oThreadsafe.h>
#include <stddef.h>

template<typename CHAR_T> size_t oFixedStringLength(const CHAR_T* _StrSource);

// If _StrSource is null, the empty string is copied
template<typename CHAR1_T, typename CHAR2_T> void oFixedStringCopy(CHAR1_T* _StrDestination, size_t _NumDestinationChars, const CHAR2_T* _StrSource);

template<typename CHAR_T, size_t CAPACITY>
class oFixedString
{
	CHAR_T s[CAPACITY];
	bool operator==(const oFixedString& _Other) const{ oASSERT(false, "Operator == not supported"); return false; }
public:
	// A simple string for use which std::string is either overkill, or uses
	// the heap when it is not desired.

	oFixedString() { *s = 0; }
	oFixedString(const CHAR_T* _String) { oFixedStringCopy(s, CAPACITY, _String); }
	template<typename CHAR2_T> oFixedString(const CHAR2_T* _String) { oFixedStringCopy(s, CAPACITY, _String); }
	template<typename CHAR2_T, size_t CAPACITY2> oFixedString(const oFixedString<CHAR2_T, CAPACITY2>& _That) { oFixedStringCopy(s, CAPACITY, _That.c_str()); }
	template<typename CHAR2_T> const oFixedString& operator=(const CHAR2_T* _That) { oFixedStringCopy(s, CAPACITY, _That); return *this; }
	template<typename CHAR2_T, size_t CAPACITY2> const oFixedString& operator=(const oFixedString<CHAR2_T, CAPACITY2>& _That) { oFixedStringCopy(s, CAPACITY, _That.c_str()); return *this; }

	void clear() { *s = 0; }
	bool empty() const { return *s == 0; }
	size_t size() const { return oFixedStringLength(s); }
	size_t length() const { return size(); }
	size_t capacity() const { return CAPACITY; }
	
	typedef CHAR_T(&array_ref) [CAPACITY];
	typedef const CHAR_T(&const_array_ref) [CAPACITY];

	array_ref c_str() { return s; }
	const_array_ref c_str() const { return s; }
	const_array_ref c_str() const volatile { return thread_cast<const_array_ref>(s); }

	operator CHAR_T*() { return s; }
	operator const CHAR_T*() const { return s; }
	operator const CHAR_T*() const volatile { return thread_cast<const CHAR_T*>(s); }
};

// Prefer one of these fixed types over defining your own to reduce the number
// of template instantiations.
typedef oFixedString<char, 64> oStringS;
typedef oFixedString<char, 512> oStringM;
typedef oFixedString<char, 2048> oStringL;
typedef oFixedString<char, 4096> oStringXL;

typedef oStringM oStringPath;
typedef oStringM oStringURI;

typedef oFixedString<wchar_t, 64> oWStringS;
typedef oFixedString<wchar_t, 512> oWStringM;
typedef oFixedString<wchar_t, 2048> oWStringL;
typedef oFixedString<wchar_t, 4096> oWStringXL;

typedef oWStringM oWStringPath;
typedef oWStringM oWStringURI;

// Add support for some super-generic functions, but how to move forward from 
// here? Do all APIs support oFixedString? Or does oFixedString add support for
// all APIs in this header?

template<typename T, size_t CAPACITY> char* oToString(oFixedString<char, CAPACITY>& _StrDestination, const T& _Value) { return oToString(_StrDestination.c_str(), _StrDestination.capacity(), _Value); }
template<size_t CAPACITY> errno_t oStrAppend(oFixedString<char, CAPACITY>& _StrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); return oVStrAppend(_StrDestination.c_str(), _StrDestination.capacity(), _Format, args); }
template<size_t CAPACITY> int vsprintf_s(oFixedString<char, CAPACITY>& _StrDestination, const char* _Format, va_list _Args) { return vsprintf_s(_StrDestination.c_str(), _StrDestination.capacity(), _Format, _Args); }
template<size_t CAPACITY> int sprintf_s(oFixedString<char, CAPACITY>& _StrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); return vsprintf_s(_StrDestination, _Format, args); }
template<size_t CAPACITY> int strcat_s(oFixedString<char, CAPACITY>& _StrDestination, const char* _Source) { return strcat_s(_StrDestination.c_str(), _StrDestination.capacity(), _Source); }
template<size_t CAPACITY> int strncpy_s(oFixedString<char, CAPACITY>& _StrDestination, const char* _Source, size_t _Count) { return strncpy_s(_StrDestination.c_str(), _StrDestination.capacity(), _Source, _Count); }

#endif
