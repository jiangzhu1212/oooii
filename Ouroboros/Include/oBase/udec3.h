/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// A compressed float3+ format that can store a normalized value[0,1]. This 
// also can store a 2-bit value for a 4th term.
#pragma once
#ifndef oBase_udec3_h
#define oBase_udec3_h

#include <oBase/byte.h>
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
