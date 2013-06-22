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
#include <oStd/string.h>

namespace oStd {

char* strbitmask(char* _StrDestination, size_t _SizeofStrDestination, int _Flags, const char* _AllZerosValue, as_string_fn _AsString)
{
	if (!_Flags && strlcpy(_StrDestination, _AllZerosValue, _SizeofStrDestination) >= _SizeofStrDestination)
		return nullptr;

	*_StrDestination = 0;
	unsigned int Mask = 0x80000000;
	while (Mask)
	{
		int v = _Flags & Mask;
		if (v)
			sncatf(_StrDestination, _SizeofStrDestination, "%s%s", *_StrDestination == 0 ? "" : "|", _AsString(v));
		Mask >>= 1;
	}

	return _StrDestination;
}

} // namespace oStd
