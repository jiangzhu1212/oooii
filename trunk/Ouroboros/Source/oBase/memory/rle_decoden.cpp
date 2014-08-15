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
#include <oBase/byte.h>
#include <oBase/macros.h>
#include <oBase/memory.h>
#include <oBase/throw.h>

namespace ouro {

const void* rle_decoden(void* oRESTRICT _pDestination, size_t _SizeofDestination, 
	size_t _ElementStride, size_t _RleElementSize, const void* oRESTRICT _pSource)
{
	int8_t* d = (int8_t*)_pDestination;
	const int8_t* oRESTRICT end = byte_add(d, _SizeofDestination);
	int8_t* oRESTRICT s = (int8_t*)_pSource;
	size_t dstep = _ElementStride - _RleElementSize;

	while (d < end)
	{
		int8_t count = *s++;
		if (count >= 0)
		{
			count = 1 + count;
			while (count--)
			{
				size_t bytes = _RleElementSize;
				while (bytes--)
					*d++ = *s++;
				d += dstep;
			}
		}

		else
		{
			count = 1 - count;
			while (count--)
			{
				for (size_t byte = 0; byte < _RleElementSize; byte++)
					*d++ = *(s + byte);
				d += _ElementStride;
			}

			s += _RleElementSize;
		}
	}

	return s;
}

} // namespace ouro
