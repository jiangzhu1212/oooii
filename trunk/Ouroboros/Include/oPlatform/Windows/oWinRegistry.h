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
// Read / Write info to the registry
//
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724897%28v=vs.85%29.aspx
#pragma once
#ifndef oWinRegistry_h
#define oWinRegistry_h

#include <oStd/fixed_string.h>

enum oWIN_REGISTRY_ROOT
{
	oHKEY_CLASSES_ROOT,
	oHKEY_CURRENT_USER,
	oHKEY_LOCAL_MACHINE,
	oHKEY_USERS,
};

// Sets the specified key's value to _pValue as a string. If the key path does 
// not exist, it is created. If _pValueName is null or "", then the (default) 
// value key.
bool oWinRegistrySetValue(oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName, const char* _Value);

// Fills destination with the specified Key's path and returns _StrDestination. If the specified key does not exist, returns nullptr
char* oWinRegistryGetValue(char* _StrDestination, size_t _SizeofStrDestination, oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName);

bool oWinRegistryDeleteValue(oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName);
bool oWinRegistryDeleteKey(oWIN_REGISTRY_ROOT _Root, const char*_KeyPath, bool _Recursive = true);

template<size_t size> char* oWinRegistryGetValue(char (&_StrDestination)[size], oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName) { return oWinRegistryGetValue(_StrDestination, size, _Root, _KeyPath, _ValueName); }
template<size_t capacity> char* oWinRegistryGetValue(oStd::fixed_string<char, capacity>& _StrDestination, oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName) { return oWinRegistryGetValue(_StrDestination, _StrDestination.capacity(), _Root, _KeyPath, _ValueName); }

template<typename T> bool oWinRegistryGetValue(T* _pTypedValue, oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName)
{
	oStd::sstring buf;
	if (!oWinRegistryGetValue(buf, _Root, _KeyPath, _ValueName))
		return false;
	return oStd::from_string(_pTypedValue, buf);
}

#endif
