// $(header)
#include <oBasis/oEightCC.h>
#include <oBasis/oByte.h>
#include <oBasis/oByteSwizzle.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oEightCC& _Value)
{
	if (_SizeofStrDestination < 9)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "String buffer not large enough");
		return nullptr;
	}

	#ifdef oLITTLEENDIAN
		oByteSwizzle64 sw; sw.AsUnsignedLongLong = (unsigned long long)_Value;
		unsigned long long fcc = ((unsigned long long)oByteSwap(sw.AsUnsignedInt[0]) << 32) | oByteSwap(sw.AsUnsignedInt[1]);
	#else
		unsigned long long fcc = _Value;
	#endif

	memcpy(_StrDestination, &fcc, sizeof(unsigned long long));
	_StrDestination[8] = 0;
	return _StrDestination;
}

bool oFromString(oEightCC* _pValue, const char* _StrSource)
{
	if (!oSTRVALID(_StrSource))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	#ifdef oLITTLEENDIAN
		*_pValue = oByteSwap(*(unsigned long long*)_StrSource);
	#else
		*_pValue = *(unsigned long long *)_StrSource;
	#endif
	return true;
}
