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
#include <oBase/byte.h>
#include <oBase/macros.h>
#include "memduff.h"

namespace ouro {

void memset8(void* _pDestination, long long _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	long long* pBody;
	char* pPrefix, *pPostfix;
	size_t nPrefixBytes, nPostfixBytes;
	detail::init_duffs_device_pointers(_pDestination, _NumBytes, &pPrefix, &nPrefixBytes, &pBody, &pPostfix, &nPostfixBytes);

	byte_swizzle64 s;
	s.as_long_long = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
		case 7: *pPrefix++ = s.as_char[7];
		case 6: *pPrefix++ = s.as_char[6];
		case 5: *pPrefix++ = s.as_char[5];
		case 4: *pPrefix++ = s.as_char[4];
		case 3: *pPrefix++ = s.as_char[3];
		case 2: *pPrefix++ = s.as_char[2];
		case 1: *pPrefix++ = s.as_char[1];
		case 0: break;
		oNODEFAULT;
	}

	// Do aligned assignment
	while (pBody < (long long*)pPostfix)
		*pBody++ = _Value;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 7: *pPostfix++ = s.as_char[7];
		case 6: *pPostfix++ = s.as_char[6];
		case 5: *pPostfix++ = s.as_char[5];
		case 4: *pPostfix++ = s.as_char[4];
		case 3: *pPostfix++ = s.as_char[3];
		case 2: *pPostfix++ = s.as_char[2];
		case 1: *pPostfix++ = s.as_char[1];
		case 0: break;
		oNODEFAULT;
	}
}

} // namespace ouro
