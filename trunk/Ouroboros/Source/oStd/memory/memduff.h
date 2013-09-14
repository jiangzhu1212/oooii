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
#pragma once
#ifndef oStd_memduff_h
#define oStd_memduff_h

#include <oStd/assert.h>

namespace oStd {
	namespace detail {

// const void* version
template<typename T> void init_duffs_device_pointers_const(
	const void* _pMemory
	, size_t _NumBytes
	, const char** _ppPrefix
	, size_t* _pNumPrefixBytes
	, const T** _ppBody
	, const char** _ppPostfix
	, size_t* _pNumPostfixBytes)
{
	*_ppPrefix = (char*)_pMemory;
	*_ppBody = (T*)oStd::byte_align(_pMemory, sizeof(T));
	*_pNumPrefixBytes = oStd::byte_diff(*_ppPrefix, _pMemory);
	const T* pEnd = oStd::byte_add(*_ppBody, _NumBytes - *_pNumPrefixBytes);
	*_ppPostfix = (char*)oStd::byte_align_down(pEnd, sizeof(T));
	*_pNumPostfixBytes = oStd::byte_diff(pEnd, *_ppPostfix);

	oASSERT(oStd::byte_add(_pMemory, _NumBytes) == pEnd, "");
	oASSERT(oStd::byte_add(_pMemory, _NumBytes) == oStd::byte_add(*_ppPostfix, *_pNumPostfixBytes), "");
}
// (non-const) void* version
template<typename T> void init_duffs_device_pointers(
	void* _pMemory
	, size_t _NumBytes
	, char** _ppPrefix
	, size_t* _pNumPrefixBytes
	, T** _ppBody
	, char** _ppPostfix
	, size_t* _pNumPostfixBytes)
{
	init_duffs_device_pointers_const(_pMemory, _NumBytes, (const char**)_ppPrefix, _pNumPrefixBytes, (const T**)_ppBody, (const char**)_ppPostfix, _pNumPostfixBytes);
}

	} // namespace detail
} // namespace oStd

#endif
