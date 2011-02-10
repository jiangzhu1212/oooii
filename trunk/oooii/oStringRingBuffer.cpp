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
#include "pch.h"
#include <oooii/oStringRingBuffer.h>

template<> size_t oStringRingBuffer<char>::templated_strlen(const char* _String) { return strlen(_String); }
template<> size_t oStringRingBuffer<wchar_t>::templated_strlen(const wchar_t* _String) { return wcslen(_String); }

template<> int oStringRingBuffer<char>::templated_strcpy_s(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource) { return strcpy_s(_StrDestination, _SizeofStrDestination, _StrSource); }
template<> int oStringRingBuffer<wchar_t>::templated_strcpy_s(wchar_t* _StrDestination, size_t _SizeofStrDestination, const wchar_t* _StrSource) { return wcscpy_s(_StrDestination, _SizeofStrDestination, _StrSource); }

template<> int oStringRingBuffer<char>::templated_vsprintf_s(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args) { return vsprintf_s(_StrDestination, _SizeofStrDestination, _Format, _Args); }
template<> int oStringRingBuffer<wchar_t>::templated_vsprintf_s(wchar_t* _StrDestination, size_t _SizeofStrDestination, const wchar_t* _Format, va_list _Args) { return vswprintf_s(_StrDestination, _SizeofStrDestination, _Format, _Args); }
