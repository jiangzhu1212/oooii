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
// Policy header that defines a common template interface for converting types 
// to and from string. This can be extended by client code by defining the
// functions for a new type according to the rules documented below.
// By default intrinsic types are defined.
#pragma once
#ifndef oStringize_h
#define oStringize_h

#include <oBasis/oFunction.h>
#include <oBasis/oString.h>

// Returns _StrDestination if successful, or nullptr if the parameters are 
// invalid or too small.
template<typename T> char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const T& _Value);
template<size_t size, typename T> char* oToString(char (&_StrDestination)[size], const T& _Value) { return oToString<T>(_StrDestination, size, _Value); }

// Returns true if successful or false of the specified string is malformed
// for the type, or if the parameters are otherwise invalid.
template<typename T> bool oFromString(T* _pValue, const char* _StrSource);

bool oFromString(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> bool oFromString(char (&_pValue)[size], const char* _StrSource) { return oFromString(_pValue, SIZE, _StrSource); }

// pass thru for const char* types
inline bool oFromString(const char** _pPointerToString, const char* _StrSource) { *_pPointerToString = _StrSource; return true; }

// Returns a const string representation of the specified value. This is most
// useful for enums when the object's value never changes.
template<typename T> const char* oAsString(T _Value);

// Fills the specified buffer with the flags converted to strings using the 
// specified function to convert each flag to a string.
char* oAsStringFlags(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _Flags, const char* _AllZerosValue, oFUNCTION<const char*(unsigned int _SingleFlag)> _AsString);
template<size_t size> char* oAsStringFlags(char (&_StrDestination)[size], unsigned int _Flags, const char* _AllZerosValue, oFUNCTION<const char*(unsigned int _SingleFlag)> _AsString) { return oAsStringFlags(_StrDestination, size, _Flags, _AllZerosValue, _AsString); }

// Utility function for using an array of strings - find the specified source
// in the array and return its index.
bool oIndexFromString(int* _pValue, const char* _StrTestSource, const char** _StrSourceArray, size_t _NumStrSources);

// For implementing oFromString for enum types quickly by using a string array
template<size_t _NumEnumValues, typename _EnumType, size_t _ArraySize> 
bool oEnumFromString(_EnumType* _pValue, const char* _StrTestSource, const char* (&_StrSourceArray)[_ArraySize])
{
	static_assert(_NumEnumValues == _ArraySize, "Enum value/string count mismatch");
	return oIndexFromString((int*)_pValue, _StrTestSource, _StrSourceArray, _ArraySize);
}

template<size_t _NumEnumValues, typename _EnumType> bool oEnumFromString(_EnumType* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i < _NumEnumValues; i++)
	{
		if (0 == oStricmp(oAsString((_EnumType)i), _StrSource))
		{
			*_pValue = (_EnumType)i;
			return true;
		}
	}
	return false;
}

// Utility function for failing on out-of-range and copying a value out of the 
// specified array into the specified destination.
char* oIndexToString(char* _StrDestination, size_t _SizeofStrDestination, int _Index, const char** _StrSourceArray, size_t _NumStrSources);

template<typename _EnumType, size_t _ArraySize> 
char* oEnumToString(char* _StrDestination, size_t _SizeofStrDestination, const _EnumType& _Value, const char* (&_StrSourceArray)[_ArraySize])
{
	return oIndexToString(_StrDestination, _SizeofStrDestination, (int)_Value, _StrSourceArray, _ArraySize);
}

#endif
