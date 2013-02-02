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
#include <oooii/oMemory.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oSwizzle.h>
#include <emmintrin.h>

void oMemset4(void* _pDestination, long _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	char* pPrefix = (char*)_pDestination;
	long* p = (long*)oByteAlign(_pDestination, sizeof(long));
	size_t nPrefixBytes = oByteDiff(p, _pDestination);
	long* pEnd = oByteAdd(p, _NumBytes - nPrefixBytes);
	char* pPostfix = (char*)oByteAlignDown(pEnd, sizeof(long));
	size_t nPostfixBytes = oByteDiff(pEnd, pPostfix);

	oASSERT(oByteAdd(_pDestination, _NumBytes) == pEnd, "");
	oASSERT(oByteAdd(_pDestination, _NumBytes) == oByteAdd(pPostfix, nPostfixBytes), "");

	oByteSwizzle32 s;
	s.AsInt = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
		case 3: *pPrefix++ = s.AsChar[3];
		case 2: *pPrefix++ = s.AsChar[2];
		case 1: *pPrefix++ = s.AsChar[1];
		case 0: break;
		default: oASSUME(0);
	}

	// Do aligned assignment
	while (p < (long*)pPostfix)
		*p++ = _Value;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 3: *pPostfix++ = s.AsChar[3];
		case 2: *pPostfix++ = s.AsChar[2];
		case 1: *pPostfix++ = s.AsChar[1];
		case 0: break;
		default: oASSUME(0);
	}
}

void oMemcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	const void* end = oByteAdd(_pDestination, _DestinationPitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), _pSource = oByteAdd(_pSource, _SourcePitch))
		memcpy(_pDestination, _pSource, _SourceRowSize);
}

void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows)
{
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		memset(_pDestination, _Value, _SetPitch);
}

void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows)
{
	oASSERT((_SetPitch % sizeof(long)) == 0, "");
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		oMemset4(_pDestination, _Value, _SetPitch);
}

void oMemcpyAsym(void* oRESTRICT _pDestination, size_t _DestinationStride, const void* oRESTRICT _pSource, size_t _SourceStride, size_t _NumElements)
{
	const void* end = oByteAdd(_pDestination, _DestinationStride, _NumElements);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationStride), _pSource = oByteAdd(_pSource, _SourceStride))
		memcpy(_pDestination, _pSource, _SourceStride);
}

void oMemcpyToUshort(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements)
{
	const unsigned int* end = &_pSource[_NumElements];
	while (_pSource < end)
	{
		oASSERT(*_pSource <= 65535, "Truncating an unsigned int (%d) to a short in a way that will change its value.", *_pSource);
		*_pDestination++ = (*_pSource++) & 0xff;
	}
}

void oMemcpyToUint(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements)
{
	const unsigned short* end = &_pSource[_NumElements];
	while (_pSource < end)
		*_pDestination++ = *_pSource++;
}

//Could pull this from cpuid instead. All currently manufactured x86 processors(both intel and amd) use a 64 byte cacheline however.
static const unsigned int CACHE_LINE_SIZE = 64;

//this code assumes dest and source pointers are 16 byte aligned
void oMemcpyStream(void *_pDestination, const void *_pSource, size_t _NumBytes)
{
	oASSERT((((size_t)_pDestination)&(sizeof(__m128i)-1)) == 0 && (((size_t)_pSource)&(sizeof(__m128i)-1)) == 0, "oMemcpyStream requires 16 byte alignment");
	
	const __m128i *sseSource = (const __m128i*)_pSource;
	size_t numWords = _NumBytes/sizeof(__m128i);
	size_t extraBytes = _NumBytes - (numWords*sizeof(__m128i));
	
	__m128i* pPrefix = (__m128i*)_pDestination;
	__m128i* p = (__m128i*)oByteAlign(_pDestination, CACHE_LINE_SIZE);
	size_t nPrefixWords = oByteDiff(p, _pDestination) / sizeof(__m128i);
	size_t nLines = (numWords - nPrefixWords) / (CACHE_LINE_SIZE/sizeof(__m128i));
	
	char* pEnd = oByteAdd((char*)p, (numWords - nPrefixWords) * sizeof(__m128i) + extraBytes);
	char* pPostfix = (char*)oByteAlignDown(pEnd, CACHE_LINE_SIZE);
	size_t nPostfixBytes = oByteDiff(pEnd, pPostfix);

	oASSERT(oByteAdd(_pDestination, _NumBytes) == pEnd, "");
	oASSERT(oByteAdd(_pDestination, _NumBytes) == oByteAdd(pPostfix, nPostfixBytes), "");
	oASSERT(nPrefixWords*sizeof(__m128i) + nLines * CACHE_LINE_SIZE + nPostfixBytes == _NumBytes, "");
	
	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixWords)
	{
	case 3: *pPrefix++ = *sseSource++;
	case 2: *pPrefix++ = *sseSource++;
	case 1: *pPrefix++ = *sseSource++;
	case 0: break;
	default: oASSUME(0);
	}

	// full cache lines
	for(size_t i = 0;i < nLines; ++i)
	{
		_mm_stream_si128(p++ , *sseSource++);
		_mm_stream_si128(p++ , *sseSource++);
		_mm_stream_si128(p++ , *sseSource++);
		_mm_stream_si128(p++ , *sseSource++);
	}

	const char *postSource = (const char*)sseSource;

	//end bytes, could be up to 63 of them.
	for(size_t i = 0;i < nPostfixBytes; ++i)
	{
		*pPostfix++ = *postSource++;
	}
}

void oMemcpy2dStream(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	const void* end = oByteAdd(_pDestination, _DestinationPitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), _pSource = oByteAdd(_pSource, _SourcePitch))
		oMemcpyStream(_pDestination, _pSource, _SourceRowSize);
}