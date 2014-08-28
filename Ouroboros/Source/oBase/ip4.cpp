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
#include <oBase/ip4.h>
#include <oString/string.h>

namespace ouro {

char* to_string(char* dst, size_t dst_size, const ip4& addr)
{
	int offset = snprintf(dst, dst_size, "%u.%u.%u.%u", 
		addr.octet(0), addr.octet(1), addr.octet(2), addr.octet(3));
	if (offset < 0)
		return nullptr;
	if (addr.port())
		offset = snprintf(dst + offset, dst_size - offset, ":%u", addr.port());
	return offset >= 0 ? dst : nullptr;
}

bool from_string(ip4* out_addr, const char* src)
{
	unsigned int x,y,z,w,p = 0;
	if (4 > sscanf_s(src, "%u.%u.%u.%u:%u", &x, &y, &z, &w, &p)
		&& ((x & 0xffffff00) || (y & 0xffffff00) || (z & 0xffffff00) || (w & 0xffffff00) || (p & 0xffff0000)))
		return false;

	*out_addr = ip4((unsigned char)x, (unsigned char)y, (unsigned char)z, (unsigned char)w, (unsigned short)p);
	return true;
}

}
