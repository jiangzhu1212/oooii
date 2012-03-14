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
#include <oBasis/oMemory.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oBasis/oByteSwizzle.h>
#include <cstdio>
#include <memory.h>

void oMemset4(void* _pDestination, long _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	char* pPrefix = (char*)_pDestination;
	long* p = (long*)oByteAlign(_pDestination, sizeof(long));
	size_t nPrefixBytes = oByteDiff(pPrefix, _pDestination);
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
		default: oASSERT_NOEXECUTION;
	}

	// Do aligned assignment
	while (p < (long*)pPostfix)
		*p++ = _Value;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 3: *pPrefix++ = s.AsChar[3];
		case 2: *pPrefix++ = s.AsChar[2];
		case 1: *pPrefix++ = s.AsChar[1];
		case 0: break;
		default: oASSERT_NOEXECUTION;
	}
}

void oMemset2(void* _pDestination, short _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	char* pPrefix = (char*)_pDestination;
	short* p = (short*)oByteAlign(_pDestination, sizeof(_Value));
	size_t nPrefixBytes = oByteDiff(pPrefix, _pDestination);
	short* pEnd = oByteAdd(p, _NumBytes - nPrefixBytes);
	char* pPostfix = (char*)oByteAlignDown(pEnd, sizeof(_Value));
	size_t nPostfixBytes = oByteDiff(pEnd, pPostfix);

	oASSERT(oByteAdd(_pDestination, _NumBytes) == pEnd, "");
	oASSERT(oByteAdd(_pDestination, _NumBytes) == oByteAdd(pPostfix, nPostfixBytes), "");

	oByteSwizzle16 s;
	s.AsShort = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
	case 1: *pPrefix++ = s.AsChar[1];
	case 0: break;
	default: oASSERT_NOEXECUTION;
	}

	// Do aligned assignment
	while (p < (short*)pPostfix)
		*p++ = _Value;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
	case 1: *pPrefix++ = s.AsChar[1];
	case 0: break;
	default: oASSERT_NOEXECUTION;
	}
}

void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows)
{
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		memset(_pDestination, _Value, _SetPitch);
}

void oMemset2d2(void* _pDestination, size_t _Pitch, short _Value, size_t _SetPitch, size_t _NumRows)
{
	oASSERT((_SetPitch % sizeof(_Value)) == 0, "");
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		oMemset2(_pDestination, _Value, _SetPitch);
}

void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows)
{
	oASSERT((_SetPitch % sizeof(long)) == 0, "");
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		oMemset4(_pDestination, _Value, _SetPitch);
}

void oMemcpyToUshort( unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements )
{
	const unsigned int* end = &_pSource[_NumElements];
	while (_pSource < end)
	{
		oASSERT(*_pSource <= 65535, "Truncating an unsigned int (%d) to a short in a way that will change its value.", *_pSource);
		*_pDestination++ = (*_pSource++) & 0xff;
	}
}

void oMemcpyToUint( unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements )
{
	const unsigned short* end = &_pSource[_NumElements];
	while (_pSource < end)
		*_pDestination++ = *_pSource++;
}
