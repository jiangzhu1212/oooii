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
#include <oBase/memory.h>

namespace ouro {

void memnset(void* oRESTRICT _pDestination, const void* oRESTRICT _pSource, size_t _SourceSize, size_t _CopySize)
{
	size_t alignedSize = _CopySize / _SourceSize;
	size_t unalignedSize = _CopySize - (alignedSize * _SourceSize);

	switch (_SourceSize)
	{
		case 8: memset8(_pDestination, *(const long long*)_pSource, _CopySize); break;
		case 4: memset4(_pDestination, *(const long*)_pSource, _CopySize); break;
		case 2: memset2(_pDestination, *(const short*)_pSource, _CopySize); break;
		case 1: memset(_pDestination, *(const char*)_pSource, _CopySize); break;
		default:
		{
			if (_SourceSize & 0xf)
			{
				{
					uint64_t* oRESTRICT d = (uint64_t*)_pDestination;
					const uint64_t* oRESTRICT s = (uint64_t*)_pSource;

					while (alignedSize--)
					{
						*d++ = *s++;
						*d++ = *s++;
					}
				}

				uint8_t* oRESTRICT d = (uint8_t*)_pDestination;
				const uint8_t* oRESTRICT s = (uint8_t*)_pSource;
				while (unalignedSize)
					*d++ = *s++;
			}

			else
			{
				while (_CopySize >= _SourceSize)
				{
					memcpy(_pDestination, _pSource, _SourceSize);
					_pDestination = byte_add(_pDestination, _SourceSize);
					_CopySize -= _SourceSize;
				}

				if (_CopySize)
					memcpy(_pDestination, _pSource, _CopySize);
			}
			break;
		}
	}
}

} // namespace ouro
