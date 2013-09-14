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
// Advanced memcpy intended to encapsulate some special-case behaviors in a 
// generic way, such as AOS to SOA, 2D copies, full 32-bit memset, etc.
#pragma once
#ifndef oStd_memory_h
#define oStd_memory_h

#include <oStd/byte.h>
#include <oStd/config.h>
#include <memory.h>

namespace oStd {

/*enum class*/ namespace utf_type { enum value { unknown, binary, ascii, utf8, utf16be, utf16le, utf32be, utf32le }; };

// Returns the UTF type of the specified buffer
utf_type::value utfcmp(const void* _pBuffer, size_t _SizeofBuffer);

// Returns true by rules similar to Perl's -T flag... i.e. if there's more than
// 10% of the specified buffer that is either 0 or non-ascii, then this will
// return false.
bool is_ascii(const void* _pBuffer, size_t _SizeofBuffer);
inline bool is_binary(const void* _pBuffer, size_t _SizeofBuffer) { return !is_ascii(_pBuffer, _SizeofBuffer); }

// Sets an int value at a time. This is probably slower than c's memset, but 
// this sets a full int value rather than a char value. This is useful for 
// setting 0xdeadbeef or // 0xfeeefeee to memory rather than memset's dede 
// or fefefe.
void memset2(void* _pDestination, short _Value, size_t _NumBytes);
void memset4(void* _pDestination, long _Value, size_t _NumBytes);

// Compares a run of memory to a fixed value. Memory unaligned to the size of 
// value (sizeof(long)) will compare to the aligned byte portion of value. This
// means the memory is expected to have been set with memset4 which will always 
// have its 4-byte value start on 4-byte-aligned addresses. This function is 
// mostly useful when setting sentinel memory to a known shred value and then
// using this to test for the integrity of the sentinel. Returns true if memory
// is fully the same, or false if even one byte differs.
bool memcmp4(const void* _pMemory, long _Value, size_t _NumBytes);

// locate a binary substring, like Linux's memmem function.
void* memmem(void* _pBuffer, size_t _SizeofBuffer, const void* _pFind, size_t _SizeofFind);
inline const void* memmem(const void* _pBuffer, size_t _SizeofBuffer, const void* _pFind, size_t _SizeofFind) { return memmem(const_cast<void*>(_pBuffer), _SizeofBuffer, _pFind, _SizeofFind); }

// Copies 32-bit values to 16-bit values (useful when working with graphics 
// index buffers). Remember, _NumElements is count of unsigned ints in _pSource 
// and _pDestination has been pre-allocated to take at least the same number of 
// unsigned shorts.
void memcpyuitous(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements);

// Reverse of oMemcpyToUshort. Remember, _NumElements is count of unsigned shorts
// in _pSource and _pDestination has been pre-allocated to take at least the
// same number of unsigned ints.
void memcpyustoui(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements);

// _____________________________________________________________________________
// 2D copies for copying image data, stuff that's easier to conceptualize as 2D 
// rather than 1D.

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
// Besides copying AOS to AOS you can also use this function to copy 
// asymmetrical. Set _SourceRowSize to the same value as _SourcePitch (SOA to 
// AOS) or set _SourceRowSize to the same value as _DestinationPitch (AOS to 
// SOA). If copying SOA-style data to AOS-style data, ensure _pDestination is 
// pointing at the first field in the struct to write, not always the start of 
// the struct itself.
inline void memcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	if (_DestinationPitch == _SourcePitch && _SourcePitch == _SourceRowSize)
		memcpy(_pDestination, _pSource, _SourcePitch * _NumRows);
	else
	{
		const void* end = byte_add(_pDestination, _DestinationPitch, _NumRows);
		for (; _pDestination < end; _pDestination = byte_add(_pDestination, _DestinationPitch), _pSource = byte_add(_pSource, _SourcePitch))
			memcpy(_pDestination, _pSource, _SourceRowSize);
	}
}

// Just like oStd::memcpy2d, but copies from the last source scanline to the first,
// thus flipping the image vertically for systems whose coordinate system is 
// flipped V from the destination coordinate system.
inline void memcpy2dvflip(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	const void* pFlippedSource = byte_add(_pSource, _SourcePitch, _NumRows - 1);
	const void* end = byte_add(_pDestination, _DestinationPitch, _NumRows);
	for (; _pDestination < end; _pDestination = byte_add(_pDestination, _DestinationPitch), pFlippedSource = byte_sub(pFlippedSource, _SourcePitch))
		memcpy(_pDestination, pFlippedSource, _SourceRowSize);
}

// A version that optionally chooses between the two above functions. This was 
// added to centralize these nasty parameters rather than having a bunch of 
// if's in client code.
inline void memcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows, bool _FlipVertically)
{
	if (_FlipVertically) memcpy2dvflip(_pDestination, _DestinationPitch, _pSource, _SourcePitch, _SourceRowSize, _NumRows);
	else memcpy2d(_pDestination, _DestinationPitch, _pSource, _SourcePitch, _SourceRowSize, _NumRows);
}

// Just like memcpy2d, but uses another pitch/size per element so you can copy
// only the red channel of a texture for example.
inline void memcpy2delements(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceNumElements, size_t _SourceElementPitch, size_t _SourceElementSize, size_t _NumRows)
{
	const void* end = byte_add(_pDestination, _DestinationPitch, _NumRows);
	for (; _pDestination < end; _pDestination = byte_add(_pDestination, _DestinationPitch), _pSource = byte_add(_pSource, _SourcePitch))
		for (size_t elem = 0; elem < _SourceNumElements; elem++)
			memcpy(byte_add(_pDestination, _SourceElementPitch, elem), byte_add(_pSource, _SourceElementPitch, elem), _SourceElementSize);
}

void memset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows);
void memset2d2(void* _pDestination, size_t _Pitch, short _Value, size_t _SetPitch, size_t _NumRows);
void memset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows);

// Visual Studio 2012 Win32 compiler's conversion (c-cast, static_cast) is 
// bugged, so here's a software version.
unsigned long long dtoull(double n);

} // namespace oStd

#endif
