// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/quat.h>
#include <oString/stringize.h>

namespace ouro {

bool from_string(quatf* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 4, _StrSource);
}

bool from_string(quatd* _pValue, const char* _StrSource)
{
	return from_string_double_array((double*)_pValue, 4, _StrSource);
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const quatf& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const quatd& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }

}
