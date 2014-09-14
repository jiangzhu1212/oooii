// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// -1 or 0xfff..ff is commonly used as a invalid value when 0 is a valid value.
// however different types can have issues:~0u != ~0ull and size_t is thus a 
// PITA. This object abstracts an invalid index value and also makes the code
// more self-documenting.

#pragma once

namespace ouro {
	namespace detail {

class invalid_t
{
public:
	operator char() const { return -1; }
	operator unsigned char() const { return static_cast<unsigned char>(~0u & 0xff); }
	operator short() const { return -1; }
	operator unsigned short() const { return static_cast<unsigned short>(~0u & 0xffff); }
	operator int() const { return -1; }
	operator unsigned int() const { return ~0u; }
	operator long() const { return -1; }
	operator unsigned long() const { return ~0u; }
	operator long long() const { return -1; }
	operator unsigned long long() const { return ~0ull; }

	template<typename T> bool operator==(const T& _That) const { return (T)*this == _That; }
	template<typename T> bool operator!=(const T& _That) const { return !(*this == _That); }
};

template<typename T> bool operator==(const T value, const invalid_t& _Invalid) { return _Invalid == value; }
template<typename T> bool operator!=(const T value, const invalid_t& _Invalid) { return _Invalid != value; }

	} // namespace detail

static const detail::invalid_t invalid;
static const detail::invalid_t infinite;

}
