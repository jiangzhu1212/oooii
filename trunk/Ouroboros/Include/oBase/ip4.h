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
#pragma once
#ifndef oNet_ip4_h
#define oNet_ip4_h

namespace ouro {

class ip4
{
public:
	// remember endianness. The 32-bit version of the address is expected
	// in big-endian order, meaning the leftmost byte represents the 
	// leftmost address octet.
	ip4(unsigned char x, unsigned char y, unsigned char z, unsigned char w, unsigned short port = 0) : port_(port) { octet_[0] = x; octet_[1] = y; octet_[2] = z; octet_[3] = w; }
	ip4(unsigned int address, unsigned short port = 0) : address_(address), port_(port) {}
		
	inline void address(unsigned int a) { address_ = a; }
	inline unsigned int address() const { return address_; }

	inline unsigned char octet(size_t index) const { return octet_[index]; }
	inline void octet(size_t index, unsigned char x) { octet_[index] = x; }

	inline void port(unsigned short p) { port_ = p; }
	inline unsigned short port() const { return port_; }

	inline bool local() const { return address_ == localhost_address; }

private:
	static const unsigned int localhost_address = 0x0100007f; // 127.0.0.1
	union { unsigned int address_; unsigned char octet_[4]; };
	unsigned short port_;
};

}

#endif