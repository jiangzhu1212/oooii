/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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

// Advanced memcpy intended to encapsulate some special-case behaviors
// in a generic way, such as AOS to SOA, 2D copies, full 32-bit memset,
// etc.
#pragma once
#ifndef oMemory_h
#define oMemory_h

#include <oBasis/oByte.h>
#include <oooii/oStddef.h>

// Allocate aligned memory from the stack 
#define oSTACK_ALLOC(size, alignment) oByteAlign(_alloca(oByteAlign(size, alignment)), alignment);

// Sets an int value at a time. This is probably slower than
// c's memset, but this sets a full int value rather than
// a char value. This is useful for setting 0xdeadbeef or 
// 0xfeeefeee to memory rather than memset's dede or fefefe.
void oMemset4(void* _pDestination, long _Value, size_t _NumBytes);

// 2D copies for copying image data, stuff that's easier to 
// conceptualize as 2D rather than 1D.
void oMemcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows);
void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows);

// sets words at a time.
void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows);

// Copies asymmetrical memory. This is most applicable when converting from AOS
// source to an SOA destination or vice versa. If copying SOA-style data to AOS-
// style data, ensure _pDestination is pointing at the first field in the struct 
// to write, not always the start of the struct itself.
void oMemcpyAsym(void* oRESTRICT _pDestination, size_t _DestinationStride, const void* oRESTRICT _pSource, size_t _SourceStride, size_t _NumElements);

// Copies 32-bit values to 16-bit values (sometimes useful when working with 
// graphics index buffers). Remember, _NumElements is count of unsigned ints 
// in _pSource and _pDestination has been pre-allocated to take at least the 
// same number of unsigned shorts.
void oMemcpyToUshort(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements);

// Reverse of oMemcpyToUshort. Remember, _NumElements is count of unsigned shorts
// in _pSource and _pDestination has been pre-allocated to take at least the
// same number of unsigned ints.
void oMemcpyToUint(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements);

void oMemcpy2dStream(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows);
void oMemcpyStream(void *_pDestination, const void *_pSource, size_t _NumBytes);

#endif
