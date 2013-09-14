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
#include <oStd/byte.h>
#include <oStd/macros.h>
#include "memduff.h"

namespace oStd {

bool memcmp4(const void* _pMemory, long _Value, size_t _NumBytes)
{
	// Compares a run of memory against a constant value.
	// this compares a full int value rather than a char value.

	// First move _pMemory up to long alignment

	const long* pBody;
	const char* pPrefix, *pPostfix;
	size_t nPrefixBytes, nPostfixBytes;
	detail::init_duffs_device_pointers_const(_pMemory, _NumBytes, &pPrefix, &nPrefixBytes, &pBody, &pPostfix, &nPostfixBytes);

	byte_swizzle32 s;
	s.as_int = _Value;

	// Duff's device up to alignment
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPrefixBytes)
	{
		case 3: if (*pPrefix++ != s.as_char[3]) return false;
		case 2: if (*pPrefix++ != s.as_char[2]) return false;
		case 1: if (*pPrefix++ != s.as_char[1]) return false;
		case 0: break;
		oNODEFAULT;
	}

	// Do aligned assignment
	while (pBody < (long*)pPostfix)
		if (*pBody++ != _Value)
			return false;

	// Duff's device any remaining bytes
	// http://en.wikipedia.org/wiki/Duff's_device
	switch (nPostfixBytes)
	{
		case 3: if (*pPostfix++ != s.as_char[3]) return false;
		case 2: if (*pPostfix++ != s.as_char[2]) return false;
		case 1: if (*pPostfix++ != s.as_char[1]) return false;
		case 0: break;
		oNODEFAULT;
	}

	return true;
}

} // namespace oStd
