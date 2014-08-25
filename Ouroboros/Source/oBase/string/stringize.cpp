// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oBase/stringize.h>
#include <oMemory/atof.h>
#include <oBase/fourcc.h>
#include <oBase/guid.h>
#include <oBase/string.h>
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

#define oCHK do { if (!_pValue || !src) return false; } while(false)
bool from_string(bool* _pValue, const char* src)
{
	oCHK;
	if (!_stricmp("true", src) || !_stricmp("t", src) || !_stricmp("yes", src) || !_stricmp("y", src))
		*_pValue = true;
	else 
		*_pValue = atoi(src) != 0;
	return true;
}

bool from_string(char* dst, size_t dst_size, const char* src)
{
	return strlcpy(dst, src, dst_size) < dst_size;
}

bool from_string(char** _pValue, const char* src) { oCHK; return strlcpy(*_pValue, src, SIZE_MAX) < SIZE_MAX; }
bool from_string(char* _pValue, const char* src) { oCHK; *_pValue = *src; return true; }
bool from_string(unsigned char* _pValue, const char* src) { oCHK; *_pValue = *(const unsigned char*)src; return true; }
template<typename T> inline bool _from_stringing(T* _pValue, const char* fmt, const char* src) { oCHK; return 1 == sscanf_s(src, fmt, _pValue); }
bool from_string(short* _pValue, const char* src) { return _from_stringing(_pValue, "%hd", src); }
bool from_string(unsigned short* _pValue, const char* src) { return _from_stringing(_pValue, "%hu", src); }
bool from_string(int* _pValue, const char* src) { return _from_stringing(_pValue, "%d", src); }
bool from_string(unsigned int* _pValue, const char* src) { return _from_stringing(_pValue, "%u", src); }
bool from_string(long* _pValue, const char* src) { return _from_stringing(_pValue, "%d", src); }
bool from_string(unsigned long* _pValue, const char* src) { return _from_stringing(_pValue, "%u", src); }
bool from_string(long long* _pValue, const char* src) { return _from_stringing(_pValue, "%lld", src); }
bool from_string(uint64_t* _pValue, const char* src) { return _from_stringing(_pValue, "%llu", src); }
bool from_string(float* _pValue, const char* src) { oCHK; return atof(src, _pValue); }
bool from_string(double* _pValue, const char* src) { return _from_stringing(_pValue, "%lf", src); }
bool from_string(fourcc* _pValue, const char* src) { oCHK; *_pValue = fourcc(src); return true; }
bool from_string(guid* _pValue, const char* src) { oCHK; return 11 == sscanf_s(src, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", &_pValue->Data1, &_pValue->Data2, &_pValue->Data3, &_pValue->Data4[0], &_pValue->Data4[1], &_pValue->Data4[2], &_pValue->Data4[3], &_pValue->Data4[4], &_pValue->Data4[5], &_pValue->Data4[6], &_pValue->Data4[7]); }

bool from_string_float_array(float* _pValue, size_t _NumValues, const char* src)
{
	if (!_pValue || !src) return false;
	move_past_line_whitespace(&src);
	while (_NumValues--)
	{
		if (!*src) return false;
		if (!atof(&src, _pValue++)) return false;
	}
	return true;
}

bool from_string_double_array(double* _pValue, size_t _NumValues, const char* src)
{
	if (!_pValue || !src) return false;
	while (_NumValues--)
	{
		move_past_line_whitespace(&src);
		if (!*src) return false;
		if (1 != sscanf_s(src, "%f", _pValue)) return false;
		move_to_whitespace(&src);
	}
	return true;
}

}
