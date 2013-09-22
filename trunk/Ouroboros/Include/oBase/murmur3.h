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
// murmur3 hash.
#pragma once
#ifndef oBase_murmur3_h
#define oBase_murmur3_h

#include <oBase/assert.h>
#include <oBase/uint128.h>

// Prefer ouro::murmur3 for codebase consistency
void MurmurHash3_x64_128(const void* key, int len, unsigned long seed, void* out);

namespace ouro {

inline uint128 murmur3(const void* _pBuffer, size_t _SizeofBuffer)
{
	oASSERT(size_t(int(_SizeofBuffer)) == _SizeofBuffer, "size is capped at signed int");
	uint128 h;
	MurmurHash3_x64_128(_pBuffer, static_cast<int>(_SizeofBuffer), 0, &h);
	return h;
}

} // namespace ouro

#endif
