// $(header)
#include <oBasis/oFourCC.h>
#include <oBasis/oByte.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oFourCC& _Value)
{
	if (_SizeofStrDestination < 5)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "String buffer not large enough");
		return nullptr;
	}

	#ifdef oLITTLEENDIAN
		unsigned int fcc = oByteSwap((unsigned int)_Value);
	#else
		unsigned int fcc = _Value;
	#endif

	memcpy(_StrDestination, &fcc, sizeof(unsigned int));
	_StrDestination[4] = 0;
	return _StrDestination;
}

bool oFromString(oFourCC* _pValue, const char* _StrSource)
{
	if (!oSTRVALID(_StrSource))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	#ifdef oLITTLEENDIAN
		*_pValue = oByteSwap(*(unsigned int*)_StrSource);
	#else
		*_pValue = *(unsigned int *)_StrSource;
	#endif
	return true;
}