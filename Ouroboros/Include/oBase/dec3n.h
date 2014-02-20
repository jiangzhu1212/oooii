/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
// A compressed float3+ format that can store a signed normalized value[-1,1].
// This also can store a 2-bit value for a 4th term. Sometimes this can be 
// useful such as when storing geometric tangents along with a winding order 
// value of either -1 or 1.
#pragma once
#ifndef oBase_dec3n_h
#define oBase_dec3n_h

#include <oBase/byte.h>
#include <oHLSL/oHLSLTypes.h>
#include <stdexcept>

namespace ouro {

class dec3n
{
public:
	dec3n() : n(0) {}
	dec3n(unsigned int _Dec3n) : n(_Dec3n) {}
	dec3n(const float3& _That) { operator=(_That); }
	dec3n(const float4& _That) { operator=(_That); }

	const dec3n& operator=(const unsigned int& _That) { n = _That; return *this; }
	const dec3n& operator=(const float3& _That) { return operator=(float4(_That, 1.0f)); }
	const dec3n& operator=(const float4& _That)
	{
		#ifdef _DEBUG
			if (_That.x < -1.0f || _That.x > 1.0f) throw std::out_of_range("value must be [-1,1]");
			if (_That.y < -1.0f || _That.y > 1.0f) throw std::out_of_range("value must be [-1,1]");
			if (_That.z < -1.0f || _That.z > 1.0f) throw std::out_of_range("value must be [-1,1]");
			if (_That.w < -1.0f || _That.w > 1.0f) throw std::out_of_range("value must be [-1,1]");
		#endif
		n = (f32ton10(sf32touf32(_That.x)) << 22) | (f32ton10(sf32touf32(_That.y)) << 12) | (f32ton10(sf32touf32(_That.z)) << 2) | f32ton2(sf32touf32(_That.w));
		return *this;
	}

	operator unsigned int() const { return n; }
	operator float3() const { return float3(uf32tosf32(n10tof32(n>>22)), uf32tosf32(n10tof32((n>>12)&0x3ff)), uf32tosf32(n10tof32((n>>2)&0x3ff))); }
	operator float4() const { float3 v = (float3)*this; return float4(v, uf32tosf32(n2tof32(n&0x3))); }

private:
	unsigned int n;
};

} // namespace ouro

#endif