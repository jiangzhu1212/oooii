// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/aabox.h>
#include <oString/stringize.h>

namespace ouro {

bool from_string(aaboxf* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 6, _StrSource);
}

bool from_string(rect* _pValue, const char* _StrSource)
{
	if (!_pValue) return false;
	int4 temp;
	if (!from_string(&temp, _StrSource))
		return false;
	_pValue->Min = int2(temp.x, temp.y);
	_pValue->Max = int2(temp.z, temp.w);
	return true;
}

bool from_string(rectf* _pValue, const char* _StrSource)
{
	if (!_pValue) return false;
	float4 temp;
	if (!from_string(&temp, _StrSource))
		return false;
	_pValue->Min = float2(temp.x, temp.y);
	_pValue->Max = float2(temp.z, temp.w);
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const aaboxf& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f %f %f", value.Min.x, value.Min.y, value.Min.z, value.Max.x, value.Max.y, value.Max.z) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const rect& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u %u %u %u", value.Min.x, value.Min.y, value.Max.x, value.Max.y) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const rectf& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.Min.x, value.Min.y, value.Max.x, value.Max.y) ? _StrDestination : nullptr; }

}
