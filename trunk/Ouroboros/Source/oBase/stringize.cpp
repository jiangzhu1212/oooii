// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oBase/stringize.h>
#include <oMemory/atof.h>
#include <oBase/fourcc.h>
#include <oBase/guid.h>
#include <oString/string.h>
#include <oString/string_fast_scan.h>
#include <oBase/date.h>

oDEFINE_WHITESPACE_PARSING();
namespace ouro {

const char* as_string(const bool& value) { return value ? "true" : "false"; }

char* to_string(char* dst, size_t dst_size, const char* const & value)
{
	return strlcpy(dst, oSAFESTRN(value), dst_size) < dst_size ? dst : nullptr;
}

char* to_string(char* dst, size_t dst_size, const char& value)
{
	if (dst_size < 2) return nullptr;
	dst[0] = value;
	dst[1] = 0;
	return dst;
}

char* to_string(char* dst, size_t dst_size, const unsigned char& value)
{
	if (dst_size < 2) return nullptr;
	dst[0] = *(signed char*)&value;
	dst[1] = 0;
	return dst;
}

char* to_string(char* dst, size_t dst_size, const bool& value) { return strlcpy(dst, value ? "true" : "false", dst_size) < dst_size ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const short & value) { return 0 == _itoa_s(value, dst, dst_size, 10) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const unsigned short& value) { return -1 != snprintf(dst, dst_size, "%hu", value) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const int& value) { return 0 == _itoa_s(value, dst, dst_size, 10) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const unsigned int& value) { return -1 != snprintf(dst, dst_size, "%u", value) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const long& value) { return 0 == _itoa_s(value, dst, dst_size, 10) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const unsigned long& value) { return -1 != snprintf(dst, dst_size, "%u", value) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const long long& value) { return 0 == _i64toa_s(value, dst, dst_size, 10) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const uint64_t& value) { return 0 == _ui64toa_s(uint64_t(value), dst, dst_size, 10) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const float& value) { if (-1 != snprintf(dst, dst_size, "%f", value)) { trim_right(dst, dst_size, dst, "0"); return dst; } return nullptr; }
char* to_string(char* dst, size_t dst_size, const double& value) { if (-1 != snprintf(dst, dst_size, "%lf", value)) { trim_right(dst, dst_size, dst, "0"); return dst; } return nullptr; }
char* to_string(char* dst, size_t dst_size, const std::string& value) { return strlcpy(dst, value.c_str(), dst_size) < dst_size ? dst : nullptr; }

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

#define oCHK do { if (!out_value || !src) return false; } while(false)
bool from_string(bool* out_value, const char* src)
{
	oCHK;
	if (!_stricmp("true", src) || !_stricmp("t", src) || !_stricmp("yes", src) || !_stricmp("y", src))
		*out_value = true;
	else 
		*out_value = atoi(src) != 0;
	return true;
}

bool from_string(char* dst, size_t dst_size, const char* src)
{
	return strlcpy(dst, src, dst_size) < dst_size;
}

bool from_string(char** out_value, const char* src) { oCHK; return strlcpy(*out_value, src, SIZE_MAX) < SIZE_MAX; }
bool from_string(char* out_value, const char* src) { oCHK; *out_value = *src; return true; }
bool from_string(unsigned char* out_value, const char* src) { oCHK; *out_value = *(const unsigned char*)src; return true; }
template<typename T> inline bool _from_stringing(T* out_value, const char* fmt, const char* src) { oCHK; return 1 == sscanf_s(src, fmt, out_value); }
bool from_string(short* out_value, const char* src) { return _from_stringing(out_value, "%hd", src); }
bool from_string(unsigned short* out_value, const char* src) { return _from_stringing(out_value, "%hu", src); }
bool from_string(int* out_value, const char* src) { return _from_stringing(out_value, "%d", src); }
bool from_string(unsigned int* out_value, const char* src) { return _from_stringing(out_value, "%u", src); }
bool from_string(long* out_value, const char* src) { return _from_stringing(out_value, "%d", src); }
bool from_string(unsigned long* out_value, const char* src) { return _from_stringing(out_value, "%u", src); }
bool from_string(long long* out_value, const char* src) { return _from_stringing(out_value, "%lld", src); }
bool from_string(uint64_t* out_value, const char* src) { return _from_stringing(out_value, "%llu", src); }
bool from_string(float* out_value, const char* src) { oCHK; return atof(src, out_value); }
bool from_string(double* out_value, const char* src) { return _from_stringing(out_value, "%lf", src); }
bool from_string(fourcc* out_value, const char* src) { oCHK; *out_value = fourcc(src); return true; }
bool from_string(guid* out_value, const char* src) { oCHK; return 11 == sscanf_s(src, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", &out_value->Data1, &out_value->Data2, &out_value->Data3, &out_value->Data4[0], &out_value->Data4[1], &out_value->Data4[2], &out_value->Data4[3], &out_value->Data4[4], &out_value->Data4[5], &out_value->Data4[6], &out_value->Data4[7]); }

bool from_string_float_array(float* out_value, size_t num_values, const char* src)
{
	if (!out_value || !src) return false;
	move_past_line_whitespace(&src);
	while (num_values--)
	{
		if (!*src) return false;
		if (!atof(&src, out_value++)) return false;
	}
	return true;
}

bool from_string_double_array(double* out_value, size_t num_values, const char* src)
{
	if (!out_value || !src) return false;
	while (num_values--)
	{
		move_past_line_whitespace(&src);
		if (!*src) return false;
		if (1 != sscanf_s(src, "%f", out_value)) return false;
		move_to_whitespace(&src);
	}
	return true;
}

}
