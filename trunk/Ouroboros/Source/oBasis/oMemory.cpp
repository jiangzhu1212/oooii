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
#include <oBasis/oMemory.h>
#include <oStd/assert.h>
#include <oStd/macros.h>
#include <oStd/byte.h>
#include <cstdio>
#include <memory.h>

// const void* version
template<typename T> void InitDuffsDeviceConstPointers(
	const void* _pMemory
	, size_t _NumBytes
	, const char** _ppPrefix
	, size_t* _pNumPrefixBytes
	, const T** _ppBody
	, const char** _ppPostfix
	, size_t* _pNumPostfixBytes)
{
	*_ppPrefix = (char*)_pMemory;
	*_ppBody = (T*)oStd::byte_align(_pMemory, sizeof(T));
	*_pNumPrefixBytes = oStd::byte_diff(*_ppPrefix, _pMemory);
	const T* pEnd = oStd::byte_add(*_ppBody, _NumBytes - *_pNumPrefixBytes);
	*_ppPostfix = (char*)oStd::byte_align_down(pEnd, sizeof(T));
	*_pNumPostfixBytes = oStd::byte_diff(pEnd, *_ppPostfix);

	oASSERT(oStd::byte_add(_pMemory, _NumBytes) == pEnd, "");
	oASSERT(oStd::byte_add(_pMemory, _NumBytes) == oStd::byte_add(*_ppPostfix, *_pNumPostfixBytes), "");
}
// (non-const) void* version
template<typename T> void InitDuffsDevicePointers(
	void* _pMemory
	, size_t _NumBytes
	, char** _ppPrefix
	, size_t* _pNumPrefixBytes
	, T** _ppBody
	, char** _ppPostfix
	, size_t* _pNumPostfixBytes)
{
	InitDuffsDeviceConstPointers(_pMemory, _NumBytes, (const char**)_ppPrefix, _pNumPrefixBytes, (const T**)_ppBody, (const char**)_ppPostfix, _pNumPostfixBytes);
}

void oMemset4(void* _pDestination, long _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	long* pBody;
	char* pPrefix, *pPostfix;
	size_t nPrefixBytes, nPostfixBytes;
	InitDuffsDevicePointers(_pDestination, _NumBytes, &pPrefix, &nPrefixBytes, &pBody, &pPostfix, &nPostfixBytes);

	oStd::byte_swizzle32 s;
	s.as_int = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
		case 3: *pPrefix++ = s.as_char[3];
		case 2: *pPrefix++ = s.as_char[2];
		case 1: *pPrefix++ = s.as_char[1];
		case 0: break;
		oNODEFAULT;
	}

	// Do aligned assignment
	while (pBody < (long*)pPostfix)
		*pBody++ = _Value;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 3: *pPostfix++ = s.as_char[3];
		case 2: *pPostfix++ = s.as_char[2];
		case 1: *pPostfix++ = s.as_char[1];
		case 0: break;
		oNODEFAULT;
	}
}

void oMemset2(void* _pDestination, short _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	short* pBody;
	char* pPrefix, *pPostfix;
	size_t nPrefixBytes, nPostfixBytes;
	InitDuffsDevicePointers(_pDestination, _NumBytes, &pPrefix, &nPrefixBytes, &pBody, &pPostfix, &nPostfixBytes);

	oStd::byte_swizzle16 s;
	s.as_short = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
		case 1: *pPrefix++ = s.as_char[1];
		case 0: break;
		oNODEFAULT;
	}

	// Do aligned assignment
	while (pBody < (short*)pPostfix)
		*pBody++ = _Value;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 1: *pPostfix++ = s.as_char[1];
		case 0: break;
		oNODEFAULT;
	}
}

void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows)
{
	const void* end = oStd::byte_add(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oStd::byte_add(_pDestination, _Pitch))
		memset(_pDestination, _Value, _SetPitch);
}

void oMemset2d2(void* _pDestination, size_t _Pitch, short _Value, size_t _SetPitch, size_t _NumRows)
{
	oASSERT((_SetPitch % sizeof(_Value)) == 0, "");
	const void* end = oStd::byte_add(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oStd::byte_add(_pDestination, _Pitch))
		oMemset2(_pDestination, _Value, _SetPitch);
}

void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows)
{
	oASSERT((_SetPitch % sizeof(long)) == 0, "");
	const void* end = oStd::byte_add(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oStd::byte_add(_pDestination, _Pitch))
		oMemset4(_pDestination, _Value, _SetPitch);
}

void oMemcpyToUshort(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements)
{
	const unsigned int* end = &_pSource[_NumElements];
	while (_pSource < end)
	{
		oASSERT(*_pSource <= 65535, "Truncating an unsigned int (%d) to a short in a way that will change its value.", *_pSource);
		*_pDestination++ = (*_pSource++) & 0xffff;
	}
}

void oMemcpyToUint(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements)
{
	const unsigned short* end = &_pSource[_NumElements];
	while (_pSource < end)
		*_pDestination++ = *_pSource++;
}

// locate a binary substring, like Linux's memmem function.
void* oMemmem(void* _pBuffer, size_t _SizeofBuffer, const void* _pFind, size_t _SizeofFind)
{
	// @oooii-tony: This could be parallel-for'ed, but you'd have to check any 
	// buffer that might straddle splits, including if splits are smaller than the 
	// sizeof find (where it straddles 3 or more splits).

	oASSERT(_SizeofFind >= 4, "a find buffer under 4 bytes is not yet implemented");
	void* pEnd = oStd::byte_add(_pBuffer, _SizeofBuffer);
	void* pFound = memchr(_pBuffer, *(const int*)_pFind, _SizeofBuffer);
	while (pFound)
	{
		if (size_t(oStd::byte_diff(pEnd, pFound)) < _SizeofFind)
			return nullptr;

		if (!memcmp(pFound, _pFind, _SizeofFind))
			return pFound;

		else
			pFound = memchr(oStd::byte_add(pFound, 4), *(const int*)_pFind, _SizeofBuffer);
	}

	return pFound;
}

bool oMemcmp4(const void* _pMemory, long _Value, size_t _NumBytes)
{
	// Compares a run of memory against a constant value.
	// this compares a full int value rather than a char value.

	// First move _pMemory up to long alignment

	const long* pBody;
	const char* pPrefix, *pPostfix;
	size_t nPrefixBytes, nPostfixBytes;
	InitDuffsDeviceConstPointers(_pMemory, _NumBytes, &pPrefix, &nPrefixBytes, &pBody, &pPostfix, &nPostfixBytes);

	oStd::byte_swizzle32 s;
	s.as_int = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
		case 3: if (*pPrefix++ != s.as_char[3]) return false;
		case 2: if (*pPrefix++ != s.as_char[2]) return false;
		case 1: if (*pPrefix++ != s.as_char[1]) return false;
		case 0: break;
		oNODEFAULT;
	}

	// Do aligned assignment
	while (pBody < (long*)pPostfix)
		if (*pBody++ != _Value)
			return false;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 3: if (*pPostfix++ != s.as_char[3]) return false;
		case 2: if (*pPostfix++ != s.as_char[2]) return false;
		case 1: if (*pPostfix++ != s.as_char[1]) return false;
		case 0: break;
		oNODEFAULT;
	}

	return true;
}

bool oMemIsText(const void* _pBuffer, size_t _SizeofBuffer)
{
	// http://code.activestate.com/recipes/173220-test-if-a-file-or-string-is-text-or-binary/
	// "The difference between text and binary is ill-defined, so this duplicates 
	// "the definition used by Perl's -T flag, which is: <br/> The first block 
	// "or so of the file is examined for odd characters such as strange control
	// "codes or characters with the high bit set. If too many strange characters 
	// (>30%) are found, it's a -B file, otherwise it's a -T file. Also, any file 
	// containing null in the first block is considered a binary file."
	static const float PERCENTAGE_THRESHOLD_TO_BE_BINARY = 0.10f; // 0.30f; // @oooii-tony: 30% seems too high to me.

	// Count non-text characters
	size_t nonTextCount = 0;
	const char* b = static_cast<const char*>(_pBuffer);
	for (size_t i = 0; i < _SizeofBuffer; i++)
		if (b[i] == 0 || (b[i] & 0x80))
			nonTextCount++;

	// Determine results
	float percentNonAscii = nonTextCount / static_cast<float>(_SizeofBuffer);
	return percentNonAscii < PERCENTAGE_THRESHOLD_TO_BE_BINARY;
}

oUTF_TYPE oMemGetUTFType(const void* _pBuffer, size_t _SizeofBuffer)
{
	const unsigned char* b = static_cast<const unsigned char*>(_pBuffer);
	if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) return oUTF8;
	if (b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) return oUTF32BE;
	if (b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) return oUTF32LE;
	if (b[0] == 0xFE && b[1] == 0xFF) return oUTF16BE;
	if (b[0] == 0xFF && b[1] == 0xFE) return oUTF16LE;
	return oMemIsText(_pBuffer, _SizeofBuffer) ? oASCII : oBINARY;
}
