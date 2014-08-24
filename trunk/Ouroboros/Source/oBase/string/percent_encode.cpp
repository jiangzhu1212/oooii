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
#include <oBase/macros.h>
#include <oBase/throw.h>

namespace ouro {

char* percent_encode(char* oRESTRICT _StrDestination, size_t _SizeofStrDestination, const char* oRESTRICT _StrSource, const char* oRESTRICT _StrReservedChars)
{
	*_StrDestination = 0;
	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	const char* s = _StrSource;
	while (*s)
	{
		if ((*s & 0x80) != 0)
		{
			// TODO: Support UTF-8 
			oTHROW(function_not_supported, "UTF-8 not yet supported");
		}

		if (strchr(_StrReservedChars, *s))
		{
			if ((d+3) > end)
				oTHROW0(no_buffer_space);
			*d++ = '%';
			snprintf(d, std::distance(d, end), "%02x", *s++); // use lower-case escaping http://www.textuality.com/tag/uri-comp-2.html
			d += 2;
		}

		else if (d >= end)
			oTHROW0(no_buffer_space);
		else
			*d++ = *s++;
	}

	*d = 0;
	return _StrDestination;
}

}
