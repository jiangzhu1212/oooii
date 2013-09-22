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
// All compression wrappers should comply with this API definition. Compression
// wrappers are named for their algorithm in oBasis (i.e. oGZip.h or oSnappy.h).
#pragma once
#ifndef oCompression_h
#define oCompression_h

#include <oBase/config.h>

// If _pDestination is nullptr, this will return an estimation of the size 
// _pDestination should be. If _pDestination is valid, this will return the 
// actual compressed size. In either case if there is a failure, this will 
// return 0.
typedef size_t (*oCompressFn)(
	void* oRESTRICT _pDestination, size_t _SizeofDestination
	, const void* oRESTRICT _pSource, size_t _SizeofSource);

// Returns the uncompressed size of _pSource. Call this first with 
// _pDestination = nullptr to get the size value to properly allocate a 
// destination buffer, then pass that as _pDestination to finish the 
// decompression. NOTE: _pSource must have been a buffer produced with 
// oCompressFn because its implementation may have tacked on extra information
// to the buffer and thus only the paired implementation of decompress knows how 
// to access such a buffer correctly.
// (Implementation note: if an algorithm doesn't store the uncompressed size,
// then it should be tacked onto the compressed buffer and accessible by this
// function to comply with the API specification.)
typedef size_t (*oDecompressFn)(
	void* oRESTRICT _pDestination, size_t _SizeofDestination
	, const void* oRESTRICT _pSource, size_t _SizeofSource);

// Implementation helpers. Prefer these to keep implementations consistent.

// use this for checking either compress/decompress destination
#define oCOMPRESSION_CHECK_DEST(_DataSize) \
	if (_pDestination && _SizeofDestination < _DataSize) \
	{	oErrorSetLast(std::errc::no_buffer_space); \
		return 0; \
	}

#endif
