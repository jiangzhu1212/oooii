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
#include <oBase/lzma.h>
#include <oBase/byte.h>
#include <oBase/throw.h>
#include <snappy/snappy.h>

namespace ouro {

size_t snappy_compress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	size_t CompressedSize = snappy::MaxCompressedLength(_SizeofSource);
	if (_pDestination)
	{
		if (_pDestination && _SizeofDestination < CompressedSize)
			oTHROW0(no_buffer_space);
		snappy::RawCompress(static_cast<const char*>(_pSource), _SizeofSource, static_cast<char*>(_pDestination), &CompressedSize);
	}
	return CompressedSize;
}

size_t snappy_decompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	size_t UncompressedSize = 0;
	snappy::GetUncompressedLength(static_cast<const char*>(_pSource), _SizeofSource, &UncompressedSize);
		if (_pDestination && _SizeofDestination < UncompressedSize)
			oTHROW0(no_buffer_space);
	if (_pDestination && !snappy::RawUncompress(static_cast<const char*>(_pSource), _SizeofSource, static_cast<char*>(_pDestination)))
		oTHROW0(protocol_error);
	return UncompressedSize;
}

} // namespace ouro
