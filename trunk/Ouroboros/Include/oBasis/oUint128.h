/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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

struct uint128
{
	operator unsigned char() const { return (unsigned char)DataLS; }
	operator unsigned short() const { return (unsigned short)DataLS; }
	operator unsigned int() const { return (unsigned int)DataLS; }
	operator unsigned long long() const { return DataLS; }
	const uint128& operator=(const unsigned long long& _That) { DataMS = 0; DataLS = _That; return *this; }
	const uint128& operator=(const uint128& _That) { DataMS = _That.DataMS; DataLS = _That.DataLS; return *this; }
	bool operator==(const uint128& _That) const { return DataMS == _That.DataMS && DataLS == _That.DataLS; }
	bool operator!=(const uint128& _That) const { return !(*this == _That); }
	unsigned long long DataMS; // most significant 8 bytes
	unsigned long long DataLS; // least significant 8 bytes
};

#endif
