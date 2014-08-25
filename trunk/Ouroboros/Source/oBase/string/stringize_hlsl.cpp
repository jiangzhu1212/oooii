// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// This cpp contains implemenations of to_string and from_string for intrinsic
// types as well as ouro types.

#include <oHLSL/oHLSLMath.h>
#include <oBase/stringize.h>

namespace ouro {

bool from_string(float2* _pValue, const char* src)
{
	return from_string_float_array((float*)_pValue, 2, src);
}

bool from_string(float3* _pValue, const char* src)
{
	return from_string_float_array((float*)_pValue, 3, src);
}

bool from_string(float4* _pValue, const char* src)
{
	return from_string_float_array((float*)_pValue, 4, src);
}

bool from_string(float4x4* _pValue, const char* src)
{
	// Read in-order, then transpose
	bool result = from_string_float_array((float*)_pValue, 16, src);
	if (result)
		transpose(*_pValue);
	return result;
}

bool from_string(double4x4* _pValue, const char* src)
{
	// Read in-order, then transpose
	bool result = from_string_double_array((double*)_pValue, 16, src);
	if (result)
		transpose(*_pValue);
	return result;
}

#define CHK_MV() do \
	{	if (!_pValue || !src) return false; \
		src += strcspn(src, oDIGIT_SIGNED); \
		if (!*src) return false; \
	} while (false)

#define CHK_MV_U() do \
	{	if (!_pValue || !src) return false; \
		src += strcspn(src, oDIGIT_UNSIGNED); \
		if (!*src) return false; \
	} while (false)

bool from_string(int2* _pValue, const char* src) { CHK_MV(); return 2 == sscanf_s(src, "%d %d", &_pValue->x, &_pValue->y); }
bool from_string(int3* _pValue, const char* src) { CHK_MV(); return 3 == sscanf_s(src, "%d %d %d", &_pValue->x, &_pValue->y, &_pValue->z); }
bool from_string(int4* _pValue, const char* src) { CHK_MV(); return 4 == sscanf_s(src, "%d %d %d %d", &_pValue->x, &_pValue->y, &_pValue->z, &_pValue->w); }
bool from_string(uint2* _pValue, const char* src) { CHK_MV_U(); return 2 == sscanf_s(src, "%u %u", &_pValue->x, &_pValue->y); }
bool from_string(uint3* _pValue, const char* src) { CHK_MV_U(); return 3 == sscanf_s(src, "%u %u %u", &_pValue->x, &_pValue->y, &_pValue->z); }
bool from_string(uint4* _pValue, const char* src) { CHK_MV(); return 4 == sscanf_s(src, "%u %u %u %u", &_pValue->x, &_pValue->y, &_pValue->z, &_pValue->w); }

char* to_string(char* dst, size_t dst_size, const float2& value) { return -1 != snprintf(dst, dst_size, "%f %f", value.x, value.y) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const float3& value) { return -1 != snprintf(dst, dst_size, "%f %f %f", value.x, value.y, value.z) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const float4& value) { return -1 != snprintf(dst, dst_size, "%f %f %f %f", value.x, value.y, value.z, value.w) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const double2& value) { return -1 != snprintf(dst, dst_size, "%f %f", value.x, value.y) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const double3& value) { return -1 != snprintf(dst, dst_size, "%f %f %f", value.x, value.y, value.z) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const double4& value) { return -1 != snprintf(dst, dst_size, "%f %f %f %f", value.x, value.y, value.z, value.w) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const int2& value) { return -1 != snprintf(dst, dst_size, "%d %d", value.x, value.y) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const int3& value) { return -1 != snprintf(dst, dst_size, "%d %d %d", value.x, value.y, value.z) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const int4& value) { return -1 != snprintf(dst, dst_size, "%d %d %d %d", value.x, value.y, value.z, value.w) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const uint2& value) { return -1 != snprintf(dst, dst_size, "%u %u", value.x, value.y) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const uint3& value) { return -1 != snprintf(dst, dst_size, "%u %u %u", value.x, value.y, value.z) ? dst : nullptr; }
char* to_string(char* dst, size_t dst_size, const uint4& value) { return -1 != snprintf(dst, dst_size, "%u %u %u %u", value.x, value.y, value.z, value.w) ? dst : nullptr; }

template<typename T> char* to_stringT(char* dst, size_t dst_size, const TMAT4<T>& value)
{
	return -1 != snprintf(dst, dst_size, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"
		, value.Column0.x, value.Column1.x, value.Column2.x, value.Column3.x
		, value.Column0.y, value.Column1.y, value.Column2.y, value.Column3.y
		, value.Column0.z, value.Column1.z, value.Column2.z, value.Column3.z
		, value.Column0.w, value.Column1.w, value.Column2.w, value.Column3.w) ? dst : nullptr;
}

char* to_string(char* dst, size_t dst_size, const float4x4& value) { return to_stringT(dst, dst_size, value); }
char* to_string(char* dst, size_t dst_size, const double4x4& value) { return to_stringT(dst, dst_size, value); }

}
