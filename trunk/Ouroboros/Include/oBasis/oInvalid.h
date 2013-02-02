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
// A nothing class designed to control the difference between setting an integer
// type to all 0xff's based on type. Mainly this should be used as an initializer
// for unsigned int types or counting types.
#pragma once
#ifndef oInvalid_h
#define oInvalid_h

class oInvalid_t
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
};

const oInvalid_t oInvalid;

template<typename T> bool operator==(const oInvalid_t _Invalid, const T _Value) { return (T)_Invalid == _Value; }
template<typename T> bool operator==(const T _Value, const oInvalid_t _Invalid) { return _Invalid == _Value; }

template<typename T> bool operator!=(const oInvalid_t _Invalid, const T _Value) { return !((T)_Invalid == _Value); }
template<typename T> bool operator!=(const T _Value, const oInvalid_t _Invalid) { return !((T)_Invalid == _Value); }

// For timeout parameters
static const oInvalid_t oInfiniteWait = oInvalid;

#endif
