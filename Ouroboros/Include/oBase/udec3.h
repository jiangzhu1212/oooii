// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A compressed float3+ format that can store a normalized value[0,1]. This 
// also can store a 2-bit value for a 4th term.
#pragma once
#ifndef oBase_udec3_h
#define oBase_udec3_h

#include <oMemory/byte.h>
#include <oHLSL/oHLSLTypes.h>
#include <stdexcept>

namespace ouro {

class udec3
{
public:
	udec3() : n(0) {}
	udec3(unsigned int _udec3) : n(_udec3) {}
	udec3(const float3& _That) { operator=(_That); }
	udec3(const float4& _That) { operator=(_That); }

	const udec3& operator=(const unsigned int& _That) { n = _That; return *this; }
	const udec3& operator=(const float3& _That) { return operator=(float4(_That, 1.0f)); }
	const udec3& operator=(const float4& _That)
	{
		#ifdef _DEBUG
			if (_That.x < 0.0f || _That.x > 1.0f) throw std::out_of_range("value must be [0,1]");
			if (_That.y < 0.0f || _That.y > 1.0f) throw std::out_of_range("value must be [0,1]");
			if (_That.z < 0.0f || _That.z > 1.0f) throw std::out_of_range("value must be [0,1]");
			if (_That.w < 0.0f || _That.w > 1.0f) throw std::out_of_range("value must be [0,1]");
		#endif
		n = (f32ton10(_That.x) << 22) | (f32ton10(_That.y) << 12) | (f32ton10(_That.z) << 2) | f32ton2(_That.w);
		return *this;
	}

	operator unsigned int() const { return n; }
	operator float3() const { return float3(n10tof32(n>>22), n10tof32((n>>12)&0x3ff), n10tof32((n>>2)&0x3ff)); }
	operator float4() const { float3 v = (float3)*this; return float4(v, n10tof32(n&0x3)); }

private:
	unsigned int n;
};

}

#endif
