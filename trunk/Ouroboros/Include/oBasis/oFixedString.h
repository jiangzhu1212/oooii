/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// A wrapper for a c-style char array that makes it a bit more like STL and 
// provides niceties such as auto-construction and shorthand assignment.
// Underneath it behaves exactly like a char array and should be used as such.
#pragma once
#ifndef oFixedString_h
#define oFixedString_h

#include <oBasis/oFunction.h>
#include <oBasis/oPath.h>
#include <oBasis/oThreadsafe.h>
#include <oBasis/oString.h>
#include <stddef.h>

template<typename CHAR_T, size_t CAPACITY>
class oFixedString
{
	CHAR_T s[CAPACITY];
	bool operator==(const oFixedString& _That) const { oASSERT(false, "Operator == not supported"); return false; }
	bool operator==(const char* _That) const { oASSERT(false, "Operator == not supported"); return false; }
public:
	static const int Capacity = CAPACITY; //until we can make capacity a constexpr. exposing the template arg
	typedef CHAR_T char_t;

	// A simple string for use which std::string is either overkill, or uses
	// the heap when it is not desired.

	oFixedString() { *s = 0; }
	oFixedString(const CHAR_T* _Start, const CHAR_T* _End) { assign(_Start, _End); }
	oFixedString(const CHAR_T* _String) { oStrcpy(s, CAPACITY, _String); }
	template<typename CHAR2_T> oFixedString(const CHAR2_T* _String) { oStrcpy(s, CAPACITY, _String); }
	template<typename CHAR2_T, size_t CAPACITY2> oFixedString(const oFixedString<CHAR2_T, CAPACITY2>& _That) { oStrcpy(s, CAPACITY, _That.c_str()); }
	template<typename CHAR2_T> const oFixedString& operator=(const CHAR2_T* _That) { oStrcpy(s, CAPACITY, _That); return *this; }
	template<typename CHAR2_T, size_t CAPACITY2> const oFixedString& operator=(const oFixedString<CHAR2_T, CAPACITY2>& _That) { oStrcpy(s, CAPACITY, _That.c_str()); return *this; }

	void clear() { *s = 0; }
	bool empty() const { return *s == 0; }
	size_t size() const { return oStrlen(s); }
	size_t length() const { return size(); }
	size_t capacity() const { return CAPACITY; }
	size_t capacity() const threadsafe { return CAPACITY; }

	void assign(const CHAR_T* _Start, const CHAR_T* _End)
	{
		ptrdiff_t size = __min(std::distance(_Start, _End), (CAPACITY-1) * sizeof(CHAR_T));
		memcpy(s, _Start, size);
		s[size] = 0;
	}
	
	typedef CHAR_T(&array_ref) [CAPACITY];
	typedef const CHAR_T(&const_array_ref) [CAPACITY];

	array_ref c_str() { return s; }
	const_array_ref c_str() const { return s; }
	const_array_ref c_str() const threadsafe { return thread_cast<const_array_ref>(s); }

	operator CHAR_T*() { return s; }
	operator const CHAR_T*() const { return s; }
	operator const CHAR_T*() const threadsafe { return thread_cast<const CHAR_T*>(s); }
};

// Prefer one of these fixed types over defining your own to reduce the number
// of template instantiations.
typedef oFixedString<char, 64> oStringS;
typedef oFixedString<char, 128> oStringM;
typedef oFixedString<char, 512> oStringL;
typedef oFixedString<char, 2048> oStringXL;
typedef oFixedString<char, 4096> oStringXXL;

typedef oStringL oStringPath;
typedef oStringL oStringURI;

typedef oFixedString<wchar_t, 64> oWStringS;
typedef oFixedString<wchar_t, 128> oWStringM;
typedef oFixedString<wchar_t, 512> oWStringL;
typedef oFixedString<wchar_t, 2048> oWStringXL;
typedef oFixedString<wchar_t, 4096> oWStringXXL;

typedef oWStringL oWStringPath;
typedef oWStringL oWStringURI;

// _____________________________________________________________________________
// Support for other string-based API

// oString.h
template<typename CHAR1_T, typename CHAR2_T, size_t capacity> CHAR1_T* oStrcpy(oFixedString<CHAR1_T, capacity>& _StrDestination, const CHAR2_T* _StrSource, bool _ZeroBuffer = false) { return oStrcpy(_StrDestination.c_str(), _StrDestination.capacity(), _StrSource, _ZeroBuffer); }
template<typename CHAR1_T, typename CHAR2_T, size_t capacity> CHAR1_T* oStrncpy(oFixedString<CHAR1_T, capacity>& _StrDestination, const CHAR2_T* _StrSource, size_t _NumChars) { return oStrncpy(_StrDestination.c_str(), _StrDestination.capacity(), _StrSource, _NumChars); }
template<typename CHAR_T, size_t capacity> CHAR_T* oStrcat(oFixedString<CHAR_T, capacity>& _StrDestination, const CHAR_T* _StrSource) { return oStrcat(_StrDestination.c_str(), _StrDestination.capacity(), _StrSource); }
template<typename CHAR_T, size_t capacity> int oVPrintf(oFixedString<CHAR_T, capacity>& _StrDestination, const CHAR_T* _Format, va_list _Args) { return oVPrintf(_StrDestination.c_str(), _StrDestination.capacity(), _Format, _Args); }
template<typename CHAR_T, size_t capacity> int oPrintf(oFixedString<CHAR_T, capacity>& _StrDestination, const CHAR_T* _Format, ...) { va_list args; va_start(args, _Format); return oVPrintf(_StrDestination, _Format, args); }
template<typename T, size_t capacity> char* oToString(oFixedString<char, capacity>& _StrDestination, const T& _Value) { return oToString(_StrDestination.c_str(), _StrDestination.capacity(), _Value); }
template<typename CHAR_T, size_t capacity> bool oFromString(oFixedString<CHAR_T, capacity> *_pValue, const char* _StrSource) { *_pValue = _StrSource; return true; }
template<size_t capacity> bool oGetKeyValuePair(oFixedString<char, capacity>& _KeyDestination, char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, _KeyDestination.capacity(), _ValueDestination, _SizeofValueDestination, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t capacity> bool oGetKeyValuePair(char* _KeyDestination, size_t _SizeofKeyDestination, oFixedString<char, capacity>& _ValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, _SizeofKeyDestination, _ValueDestination, _ValueDestination.capacity(), _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t capacity> char* oTrimLeft(oFixedString<char, capacity>& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimLeft(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
template<size_t capacity> char* oTrimRight(oFixedString<char, capacity>& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrimRight(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
template<size_t capacity> char* oTrim(oFixedString<char, capacity>& _Trimmed, const char* _StrSource, const char* _ToTrim = oWHITESPACE) { return oTrim(_Trimmed, _Trimmed.capacity(), _StrSource, _ToTrim); }
template<size_t KEY_capacity, size_t VALUE_capacity> bool oGetKeyValuePair(oFixedString<char, KEY_capacity>& _KeyDestination, oFixedString<char, VALUE_capacity>& _ValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return oGetKeyValuePair(_KeyDestination, _KeyDestination.capacity(), _ValueDestination, _ValueDestination.capacity(), _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t capacity> errno_t oStrAppendf(oFixedString<char, capacity>& _StrDestination, const char* _Format, ...) { va_list args; va_start(args, _Format); errno_t err = oStrVAppendf(_StrDestination.c_str(), _StrDestination.capacity(), _Format, args); va_end(args); return err; }
template<size_t capacity> char* oFormatMemorySize(oFixedString<char, capacity>& _StrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits) { return oFormatMemorySize(_StrDestination, _StrDestination.capacity(), _NumBytes, _NumPrecisionDigits); }
template<size_t capacity> char* oFormatTimeSize(oFixedString<char, capacity>& _StrDestination, double _TimeInSeconds, bool _Abbreviated = false, bool _IncludeMS = true) { return oFormatTimeSize(_StrDestination, _StrDestination.capacity(), _TimeInSeconds, _Abbreviated, _IncludeMS); }
template<size_t capacity> char* oFormatCommas(oFixedString<char, capacity>& _StrDestination, int _Number) { return oFormatCommas(_StrDestination, _StrDestination.capacity(), _Number); }
template<size_t capacity> errno_t oReplace(oFixedString<char, capacity>& _StrDestination, const char* _StrSource, const char* _StrFind, const char* _StrReplace) { return oReplace(_StrDestination, _StrDestination.capacity(), _StrSource, _StrFind, _StrReplace); }
template<size_t capacity> char* oInsert(oFixedString<char, capacity>& _StrDestination, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion) { return oInsert(_StrDestination, _StrDestination.capacity(), _InsertionPoint, _ReplacementLength, _Insertion); }
template<typename CHAR_T, size_t capacity> CHAR_T* oAddTruncationElipse(oFixedString<CHAR_T, capacity>& _StrDestination) { return oAddTruncationElipse(_StrDestination.c_str(), _StrDestination.capacity()); }
template<size_t capacity> char* oOptDoc(oFixedString<char, capacity>& _StrDestination, const char* _AppName, const oOption* _pOptions) { return oOptDoc(_StrDestination, _StrDestination.capacity(), _AppName, _pOptions); }

// oPath.h
template<size_t capacity> char* oGetFilebase(oFixedString<char, capacity>& _StrDestination, const char* _Path) { return oGetFilebase(_StrDestination, _StrDestination.capacity(), _Path); }
template<size_t capacity> char* oReplaceFileExtension(oFixedString<char, capacity>& _Path, const char* _Extension) { return oReplaceFileExtension(_Path, _Path.capacity(), _Extension); }
template<size_t capacity> char* oReplaceFilename(oFixedString<char, capacity>& _Path, const char* _Filename) { return oReplaceFilename(_Path, _Path.capacity(), _Filename); }
template<size_t capacity> char* oEnsureSeparator(oFixedString<char, capacity>& _Path, char _FileSeparator = '/') { return oEnsureSeparator(_Path, _Path.capacity(), _FileSeparator); }
template<size_t capacity> char* oCleanPath(oFixedString<char, capacity>& _Path, const char* _SourcePath, char _FileSeparator = '/', bool _ZeroBuffer = false) { return oCleanPath(_Path, _Path.capacity(), _SourcePath, _FileSeparator, _ZeroBuffer); }
template<size_t capacity> char* oMakeRelativePath(oFixedString<char, capacity>& _Path, const char* _FullPath, const char* _BasePath, char _FileSeparator = '/') { return oMakeRelativePath(_StrDestination, _StrDestination.capacity(), _FullPath, _BasePath, _FileSeparator); }
template<size_t capacity> char* oFindInPath(oFixedString<char, capacity>& _Path, const char* _SearchPath, const char* _RelativePath, const char* _DotPath, oFUNCTION_PATH_EXISTS _PathExists) { return oFindInPath(_Path, _Path.capacity(), _SearchPath, _RelativePath, _DotPath, _PathExists); }
template<size_t capacity> char* oStrTokToSwitches(oFixedString<char, capacity>& _StrDestination, const char* _Switch, const char* _Tokens, const char* _Separator) { return oStrTokToSwitches(_StrDestination, _StrDestination.capacity(), _Switch, _Tokens, _Separator); }

#endif
