// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// 128-bit unsigned integer type.

#pragma once
#ifndef oUint128_h
#define oUint128_h

#include <cstdint>

namespace ouro {

class uint128
{
public:
	uint128() {}
	uint128(const uint64_t& x) : hi(0), lo(x) {}
	uint128(const uint64_t& _hi, const uint64_t& _lo) : hi(_hi), lo(_lo) {}
	uint128(const uint128& _that) { operator=(_that); }


	const uint128& operator=(const uint64_t& _that) { hi = 0; lo = _that; return *this; }
	const uint128& operator=(const uint128& _that) { hi = _that.hi; lo = _that.lo; return *this; }


	operator unsigned char() const { return (unsigned char)lo; }
	operator unsigned short() const { return (unsigned short)lo; }
	operator unsigned int() const { return (unsigned int)lo; }
	operator uint64_t() const { return lo; }

	
	uint128 operator~() const { uint128 x(*this); x.hi = ~x.hi; x.lo = ~x.lo; return *this; }
	uint128 operator-() const { return ~(*this) + uint128(1); }


	bool operator<(const uint128& _that) const { return (hi == _that.hi) ? lo < _that.lo : hi < _that.hi; }
	bool operator==(const uint128& _that) const { return hi == _that.hi && lo == _that.lo; }

	
	uint128& operator++() { if (0 == ++lo) hi++; return *this; }
	uint128& operator--() { if (0 == lo--) hi--; return *this; }
	
	friend uint128 operator++(uint128& _this, int) { uint128 x(_this); ++_this; return x; }
	friend uint128 operator--(uint128& _this, int) { uint128 x(_this); --_this; return x; }

	
	uint128& operator|=(const uint128& _that) { hi |= _that.hi; lo |= _that.lo; return *this; }
	uint128& operator&=(const uint128& _that) { hi &= _that.hi; lo &= _that.lo; return *this; }
	uint128& operator^=(const uint128& _that) { hi ^= _that.hi; lo ^= _that.lo; return *this; }

	friend uint128 operator|(const uint128& _this, const uint128& that) { uint128 x(_this); x |= that; return x; }
	friend uint128 operator&(const uint128& _this, const uint128& that) { uint128 x(_this); x &= that; return x; }
	friend uint128 operator^(const uint128& _this, const uint128& that) { uint128 x(_this); x ^= that; return x; }


	uint128& operator+=(const uint128& _that) { hi += _that.hi; uint64_t tmp = lo; lo += _that.lo; if (lo < tmp) hi++; }
	uint128& operator-=(const uint128& _that) { operator+=(-_that); }

	friend uint128 operator+(const uint128& _this, const uint128& that) { uint128 x(_this); x += that; return x; }
	friend uint128 operator-(const uint128& _this, const uint128& that) { uint128 x(_this); x -= that; return x; }


	uint128& operator<<=(const unsigned int& x)
	{
		if (x >= 64) { hi = lo; lo = 0; return operator<<=(x-64); }
		hi = (hi << x) | (lo >> (64-x)); lo = (lo << x);
		return *this;
	}

	uint128& operator>>=(const unsigned int& x)
	{
		if (x >= 64) { lo = hi; hi = 0; return operator>>=(x-64); }
		lo = (hi << (64-x)) | (lo >> x); hi = (hi >> x);
		return *this;
	}

	friend uint128 operator<<(const uint128& _this, const uint128& that) { uint128 x(_this); x <<= that; return x; }
	friend uint128 operator>>(const uint128& _this, const uint128& that) { uint128 x(_this); x >>= that; return x; }


	uint64_t hi;
	uint64_t lo;
};

}

// Leave _t type in the global namespace, consistent with other basic types
typedef ouro::uint128 uint128_t;

#endif
