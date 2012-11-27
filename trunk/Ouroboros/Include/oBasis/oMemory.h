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
#include <oBasis/oPlatformFeatures.h>
#include <malloc.h>

enum oUTF_TYPE
{
	oUNKNOWN_UTF_TYPE,
	oBINARY,
	oASCII,
	oUTF8,
	oUTF16BE,
	oUTF16LE,
	oUTF32BE,
	oUTF32LE,
};

// Allocate (un)aligned memory from the stack. These are implemented as macros
// because of course if they were functions the allocation would be invalid 
// after the function returns. oStackAlloc is 16-byte aligned by default.
#define oStackAlloc(_Size) _alloca(_Size)
#define oStackAllocAligned(_Size, _Alignment) oByteAlign(oStackAlloc(oByteAlign(_Size, _Alignment)), _Alignment)

// Sets an int value at a time. This is probably slower than
// c's memset, but this sets a full int value rather than
// a char value. This is useful for setting 0xdeadbeef or 
// 0xfeeefeee to memory rather than memset's dede or fefefe.
void oMemset2(void* _pDestination, short _Value, size_t _NumBytes);
void oMemset4(void* _pDestination, long _Value, size_t _NumBytes);

// 2D copies for copying image data, stuff that's easier to 
// conceptualize as 2D rather than 1D.

// _pDestination: pointer to the start of the 2D box to write
// _DestinationPitch: # bytes to get to the point just below _pDestination
// _pSource: pointer to the start of the 2D box to read
// _SourcePitch: # bytes to get to the point just below _pSource
// _SourceRowSize: # bytes in a scanline to copy into _pDestination
// _NumRows: Number of scanline copies to perform.

// NOTE: _SourceRowSize is different from _SourcePitch or _DestinationPitch 
// because this function can be used for copying a subregion out of the middle
// of a larger box. Thus the pitch to get to the next line might be 
// significantly larger than the valid data to copy. Likewise the destination
// could be in the middle of a larger container, so its pitch too might be 
// different than the data to be moved. In the case where an entire image or 
// surface is copied, _SourcePitch will likely be very similar or identical to
// _SourceRowSize.
// Besides copying AOS to AOS you can also use this function to copy asymmetrical.
// Set _SourceRowSize to the same value as _SourcePitch (SOA to AOS)
// or set _SourceRowSize to the same value as _DestinationPitch (AOS to SOA).
// If copying SOA-style data to AOS-style data, ensure _pDestination is pointing
// at the first field in the struct to write, not always the start of the struct itself.
inline void oMemcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	if (_DestinationPitch == _SourcePitch && _SourcePitch == _SourceRowSize)
		memcpy(_pDestination, _pSource, _SourcePitch * _NumRows);
	else
	{
		const void* end = oByteAdd(_pDestination, _DestinationPitch, _NumRows);
		for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), _pSource = oByteAdd(_pSource, _SourcePitch))
			memcpy(_pDestination, _pSource, _SourceRowSize);
	}
}

// Just like oMemcpy2d, but copies from the last source scanline to the first,
// thus flipping the image vertically for systems whose coordinate system is 
// flipped V from the destination coordinate system.
inline void oMemcpy2dVFlip(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	const void* pFlippedSource = oByteAdd(_pSource, _SourcePitch, _NumRows - 1);
	const void* end = oByteAdd(_pDestination, _DestinationPitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), pFlippedSource = oByteSub(pFlippedSource, _SourcePitch))
		memcpy(_pDestination, pFlippedSource, _SourceRowSize);
}

// A version that optionally chooses between the two above functions. This was 
// added to centralize these nasty parameters rather than having a bunch of 
// if's in client code.
inline void oMemcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows, bool _FlipVertically)
{
	if (_FlipVertically)
		oMemcpy2dVFlip(_pDestination, _DestinationPitch, _pSource, _SourcePitch, _SourceRowSize, _NumRows);
	else
		oMemcpy2d(_pDestination, _DestinationPitch, _pSource, _SourcePitch, _SourceRowSize, _NumRows);
}

void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows);

// sets words at a time.
void oMemset2d2(void* _pDestination, size_t _Pitch, short _Value, size_t _SetPitch, size_t _NumRows);
void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows);

// Copies 32-bit values to 16-bit values (sometimes useful when working with 
// graphics index buffers). Remember, _NumElements is count of unsigned ints 
// in _pSource and _pDestination has been pre-allocated to take at least the 
// same number of unsigned shorts.
void oMemcpyToUshort(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements);

// Reverse of oMemcpyToUshort. Remember, _NumElements is count of unsigned shorts
// in _pSource and _pDestination has been pre-allocated to take at least the
// same number of unsigned ints.
void oMemcpyToUint(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements);

// Searchs the sepcified buffer for a matching binary buffer specified by 
// _pFind. This function is based on the linux function memmem which is similar
// to strstr.
void* oMemmem(void* _pBuffer, size_t _SizeofBuffer, const void* _pFind, size_t _SizeofFind);
inline const void* oMemmem(const void* _pBuffer, size_t _SizeofBuffer, const void* _pFind, size_t _SizeofFind) { return oMemmem(const_cast<void*>(_pBuffer), _SizeofBuffer, _pFind, _SizeofFind); }

// Returns true by the same rules as Perl's -T flag... i.e. if there's more than
// 30% of the specified buffer that is either 0 or non-ascii, then this will
// return false.
bool oMemIsText(const void* _pBuffer, size_t _SizeofBuffer);
inline bool oMemIsBinary(const void* _pBuffer, size_t _SizeofBuffer) { return !oMemIsText(_pBuffer, _SizeofBuffer); }

// Analyzes the buffer to determine its UTF type. If a UTF header is not found,
// this uses oMemIsText to differ between ASCII and binary.
oUTF_TYPE oMemGetUTFType(const void* _pBuffer, size_t _SizeofBuffer);

#endif
