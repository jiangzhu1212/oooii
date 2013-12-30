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
// -1 or 0xfff..ff is commonly used as a invalid value when 0 is a valid value.
// however different types can have issues:~0u != ~0ull and size_t is thus a 
// PITA. This object abstracts an invalid index value and also makes the code
// more self-documenting.
#pragma once
#ifndef oBase_invalid_h
#define oBase_invalid_h

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

template<typename T> bool operator==(const T _Value, const invalid_t& _Invalid) { return _Invalid == _Value; }
template<typename T> bool operator!=(const T _Value, const invalid_t& _Invalid) { return _Invalid != _Value; }

	} // namespace detail

static const detail::invalid_t invalid;
static const detail::invalid_t infinite;

} // namespace ouro

#endif
