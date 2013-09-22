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
#include <oBase/macros.h>
#include <oBase/throw.h>

namespace ouro {

char* percent_to_lower(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	const char* s = _StrSource;
	while (*s)
	{
		bool do_lower = *s == '%';
		*d++ = *s++;
		if (do_lower)
		{
			*d++ = (char)tolower(*s++);
			*d++ = (char)tolower(*s++);
		}
	}

	*d = 0;
	return _StrDestination;
}

char* percent_decode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	const char* s = _StrSource;
	while (*s)
	{
		if (*s == '%')
		{
			char hex[3] = { *(s+1), *(s+2), 0 };
			char c = strtoul(hex, nullptr, 16) & 0xff;
			*d++ = c;
			s += 3;
		}

		else if (d >= end)
			oTHROW0(no_buffer_space);
		else
			*d++ = *s++;
	}

	*d = 0;
	return _StrDestination;
}

} // namespace ouro
