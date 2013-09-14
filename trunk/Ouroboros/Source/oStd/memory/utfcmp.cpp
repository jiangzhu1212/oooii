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
#include <oStd/memory.h>

namespace oStd {

utf_type::value utfcmp(const void* _pBuffer, size_t _SizeofBuffer)
{
	const unsigned char* b = static_cast<const unsigned char*>(_pBuffer);
	if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) return utf_type::utf8;
	if (b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) return utf_type::utf32be;
	if (b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) return utf_type::utf32le;
	if (b[0] == 0xFE && b[1] == 0xFF) return utf_type::utf16be;
	if (b[0] == 0xFF && b[1] == 0xFE) return utf_type::utf16le;
	return is_ascii(_pBuffer, _SizeofBuffer) ? utf_type::ascii : utf_type::binary;
}

} // namespace oStd
