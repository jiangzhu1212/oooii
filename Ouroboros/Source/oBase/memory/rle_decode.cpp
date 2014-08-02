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

void rle_decode(void* oRESTRICT _pDestination, size_t _SizeofDestination, size_t _ElementSize, const void* oRESTRICT _pSource)
{
	void* oRESTRICT d = _pDestination;
	const void* oRESTRICT end = byte_add(d, _SizeofDestination);
	int8_t* oRESTRICT s = (int8_t*)_pSource;

	while (d < end)
	{
    int8_t count = *s++;
    if (count >= 0)
    {
			size_t bytes = (1 + count) * _ElementSize;
			void* oRESTRICT new_d = byte_add(d, bytes);
      oCHECK(new_d <= end, "buffer overrun");
			memcpy(d, s, count);
			d = new_d;
			s = byte_add(s, bytes);
    }
    
    else
    {
			size_t bytes = (1 - count) * _ElementSize;
			void* oRESTRICT new_d = byte_add(d, bytes);
			oCHECK(new_d <= end, "buffer overrun");
			memnset(d, s, _ElementSize, bytes);
			d = new_d;
			s = byte_add(s, _ElementSize);
    }
  }
}

} // namespace ouro
