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
#include <oBasis/oGUID.h>
#include <oBasis/oMacros.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oString.h>
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
	return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", _Value.Data1, _Value.Data2, _Value.Data3, _Value.Data4[0], _Value.Data4[1], _Value.Data4[2], _Value.Data4[3], _Value.Data4[4], _Value.Data4[5], _Value.Data4[6], _Value.Data4[7]) ? _StrDestination : nullptr;
}

bool oFromString(oGUID* _pValue, const char* _StrSource)
{
	if (!_pValue || !oSTRVALID(_StrSource)) return false;
	return 11 == sscanf_s(	_StrSource, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", &_pValue->Data1, &_pValue->Data2, &_pValue->Data3, &_pValue->Data4[0], &_pValue->Data4[1], &_pValue->Data4[2], &_pValue->Data4[3], &_pValue->Data4[4], &_pValue->Data4[5], &_pValue->Data4[6], &_pValue->Data4[7]);
}
