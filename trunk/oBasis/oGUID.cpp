// $(header)
#include <oBasis/oGUID.h>
#include <oBasis/oMacros.h>
#include <oBasis/oPlatformFeatures.h>
#include <cstring>
#include <cstdio>

bool oGUID::operator==(const oGUID& other) const
{
	return !memcmp(this, &other, sizeof(oGUID));
}

bool oGUID::operator<(const oGUID& other) const
{
	return memcmp(this, &other, sizeof(oGUID)) < 0;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oGUID& _Value)
{
	if (_SizeofStrDestination <= 38) return nullptr;
	return -1 != sprintf_s(_StrDestination, _SizeofStrDestination, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", _Value.Data1, _Value.Data2, _Value.Data3, _Value.Data4[0], _Value.Data4[1], _Value.Data4[2], _Value.Data4[3], _Value.Data4[4], _Value.Data4[5], _Value.Data4[6], _Value.Data4[7]) ? _StrDestination : nullptr;
}

bool oFromString(oGUID* _pValue, const char* _StrSource)
{
	if (!_pValue || !oSTRVALID(_StrSource)) return false;
	return 1 == sscanf_s(	_StrSource, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", &_pValue->Data1, &_pValue->Data2, &_pValue->Data3, &_pValue->Data4[0], &_pValue->Data4[1], &_pValue->Data4[2], &_pValue->Data4[3],&_pValue->Data4[4], &_pValue->Data4[5], &_pValue->Data4[6], &_pValue->Data4[7]);
}
