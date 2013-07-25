/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#pragma once
#ifndef oUint128_h
#define oUint128_h

#include <oStd/operators.h>

namespace oStd {

class uint128 : public oComparable<uint128>
{
public:
	uint128() {}
	uint128(const unsigned long long& x) : DataMS(0), DataLS(x) {}
	uint128(const unsigned long long& _DataMS, const unsigned long long& _DataLS) : DataMS(_DataMS), DataLS(_DataLS) {}
	uint128(const uint128& _That) { operator=(_That); }

	const uint128& operator=(const unsigned long long& _That) { DataMS = 0; DataLS = _That; return *this; }
	const uint128& operator=(const uint128& _That) { DataMS = _That.DataMS; DataLS = _That.DataLS; return *this; }

	operator unsigned char() const { return (unsigned char)DataLS; }
	operator unsigned short() const { return (unsigned short)DataLS; }
	operator unsigned int() const { return (unsigned int)DataLS; }
	operator unsigned long long() const { return DataLS; }

	bool operator<(const uint128& _That) const { return (DataMS == _That.DataMS) ? DataLS < _That.DataLS : DataMS < _That.DataMS; }
	bool operator==(const uint128& _That) const { return DataMS == _That.DataMS && DataLS == _That.DataLS; }

	uint128& operator++() { if (0 == ++DataLS) DataMS++; return *this; }
	uint128& operator--() { if (0 == DataLS--) DataMS--; return *this; }

	uint128& operator|=(const uint128& _That) { DataMS |= _That.DataMS; DataLS |= _That.DataLS; return *this; }
	uint128& operator&=(const uint128& _That) { DataMS &= _That.DataMS; DataLS &= _That.DataLS; return *this; }
	uint128& operator^=(const uint128& _That) { DataMS ^= _That.DataMS; DataLS ^= _That.DataLS; return *this; }

	uint128 operator~() const { uint128 tmp(*this); tmp.DataMS = ~tmp.DataMS; tmp.DataLS = ~tmp.DataLS; return *this; }
	uint128 operator-() const { return ~(*this) + uint128(1); }

	uint128& operator+=(const uint128& _That) { DataMS += _That.DataMS; unsigned long long tmp = DataLS; DataLS += _That.DataLS; if (DataLS < tmp) DataMS++; }
	uint128& operator-=(const uint128& _That) { operator+=(-_That); }

	uint128& operator<<=(const unsigned int& x)
	{
		if (x >= 64) { DataMS = DataLS; DataLS = 0; return operator<<=(x-64); }
		DataMS = (DataMS << x) | (DataLS >> (64-x)); DataLS = (DataLS << x);
		return *this;
	}

	uint128& operator>>=(const unsigned int& x)
	{
		if (x >= 64) { DataLS = DataMS; DataMS = 0; return operator>>=(x-64); }
		DataLS = (DataMS << (64-x)) | (DataLS >> x); DataMS = (DataMS >> x);
		return *this;
	}

	// These are required to define oArithmetic<uint128>
	//uint128& operator*=(const uint128& _That);
	//uint128& operator/=(const uint128& _That);
	//uint128& operator%=(const uint128& _That);

	// These go away with oArithmetic<uint128> defined
	oOPERATORS_INCREMENTABLE(uint128)
	oOPERATORS_LOGICAL(uint128)
	oOPERATORS_SHIFT(uint128)
	oOPERATORS_DERIVED(uint128, +) oOPERATORS_DERIVED(uint128, -)

	unsigned long long DataMS; // most significant 8 bytes
	unsigned long long DataLS; // least significant 8 bytes
};

} // namespace oStd

// Leave it consistent with other basic types
using oStd::uint128;

#endif
