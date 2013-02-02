/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Wrappers for various underlying compression algorithms.
#pragma once
#ifndef oCompression_h
#define oCompression_h

#include <oBasis/oPlatformFeatures.h>

enum oCOMPRESSION_TRADEOFF
{
	oFAVOR_DECOMPRESSION_SPEED,
	oFAVOR_MINIMAL_SIZE,
};

// Compress the contents of _pSource into the properly allocated _pDestination.
// The destination size should be the value returned from oCalcMaxCompressedSize.
// If desired, the actual sized used of the destination buffer can be returned
// in _pActualComressedSize.
oAPI bool oCompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource, oCOMPRESSION_TRADEOFF _Tradeoff, size_t* _pActualCompressedSize = nullptr);

// Decompress _pSource into _pDestination. _SizeofSource must be the value 
// returned from oCompress in _pActualCompressedSize because many decompression
// algorithms don't know when to stop - no header or null terminator. The 
// compressed buffer can be queried with oGetUncompressedSize() to get what 
// _SizeofDestination should be.
oAPI bool oUncompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource);

// Estimate the proper buffer to be a destination for oCompress.
oAPI size_t oCalcMaxCompressedSize(size_t _SizeofSource, oCOMPRESSION_TRADEOFF _Tradeoff);

// Given the result written into _pDestination in an oCompress call, query what
// size the data would be uncompressed. This is a simple data lookup, so is very 
// fast O(1).
oAPI size_t oGetUncompressedSize(const void* _pCompressedBuffer);

#endif
