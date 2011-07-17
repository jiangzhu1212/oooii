// $(header)
#include <oooii/oMemory.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oSwizzle.h>

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
		default: oASSUME(0);
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
