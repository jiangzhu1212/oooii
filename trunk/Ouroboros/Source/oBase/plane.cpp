// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/plane.h>
#include <oString/stringize.h>

namespace ouro {

bool from_string(planef* _pValue, const char* _StrSource)
{
	float4* pTmp = (float4*)_pValue;
	return from_string(pTmp, _StrSource);
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const planef& value) { return to_string(_StrDestination, _SizeofStrDestination, (const float4&)value); }

}
