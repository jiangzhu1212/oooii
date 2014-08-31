// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oString/stringize.h>
#include <oBase/fourcc.h>
#include <oBase/guid.h>
#include <oBase/date.h>

namespace ouro {

char* to_string(char* dst, size_t dst_size, const fourcc& value)
{
	if (dst_size < 5) return nullptr;
	unsigned int fcc = from_big_endian((unsigned int)value);
	memcpy(dst, &fcc, sizeof(unsigned int));
	dst[4] = 0;
	return dst;
}

char* to_string(char* dst, size_t dst_size, const guid& value)
{
	if (dst_size <= 38) return nullptr;
	return -1 != snprintf(dst, dst_size, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", value.Data1, value.Data2, value.Data3, value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6], value.Data4[7]) ? dst : nullptr;
}

bool from_string(fourcc* out_value, const char* src) { *out_value = fourcc(src); return true; }
bool from_string(guid* out_value, const char* src) { return 11 == sscanf_s(src, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", &out_value->Data1, &out_value->Data2, &out_value->Data3, &out_value->Data4[0], &out_value->Data4[1], &out_value->Data4[2], &out_value->Data4[3], &out_value->Data4[4], &out_value->Data4[5], &out_value->Data4[6], &out_value->Data4[7]); }

}
