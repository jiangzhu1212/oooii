// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/rgb.h>
#include <oString/stringize.h>

namespace ouro {

bool from_string(rgbf* _pValue, const char* _StrSource)
{
	color c;
	// Valid forms are: 0xAABBGGRR, R G B [0,1], and an ouro::color
	if (*_StrSource == '0' && tolower(*_StrSource) == 'x')
	{
		unsigned int i;
		if (from_string(&i, _StrSource))
			*_pValue = *(color*)&i;
	}
	else if (from_string(&c, _StrSource))
		*_pValue = c;
	else if (!from_string((float3*)_pValue, _StrSource))
		return false;
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const rgbf& value) 
{
	if (!to_string(_StrDestination, _SizeofStrDestination, (color)value))
		if (!to_string(_StrDestination, _SizeofStrDestination, (const float3&)value))
			return nullptr;

	return _StrDestination;
}

}
