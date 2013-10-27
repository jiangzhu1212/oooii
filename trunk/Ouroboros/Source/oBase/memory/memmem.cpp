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
#include <oBase/assert.h>
#include <oBase/byte.h>

namespace ouro {

void* memmem(void* _pBuffer, size_t _SizeofBuffer, const void* _pFind, size_t _SizeofFind)
{
	// @tony: This could be parallel-for'ed, but you'd have to check any 
	// buffer that might straddle splits, including if splits are smaller than the 
	// sizeof find (where it straddles 3 or more splits).

	oASSERT(_SizeofFind >= 4, "a find buffer under 4 bytes is not yet implemented");
	void* pEnd = byte_add(_pBuffer, _SizeofBuffer);
	void* pFound = memchr(_pBuffer, *(const int*)_pFind, _SizeofBuffer);
	while (pFound)
	{
		if (size_t(byte_diff(pEnd, pFound)) < _SizeofFind)
			return nullptr;

		if (!memcmp(pFound, _pFind, _SizeofFind))
			return pFound;

		else
			pFound = memchr(byte_add(pFound, 4), *(const int*)_pFind, _SizeofBuffer);
	}

	return pFound;
}

} // namespace ouro
