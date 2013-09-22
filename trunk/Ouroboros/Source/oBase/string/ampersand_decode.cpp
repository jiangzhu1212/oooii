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

namespace ouro {

char* ampersand_decode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	if (!_StrDestination || !_SizeofStrDestination)
		return nullptr;

	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	struct SYM { const char* res; unsigned short len; char c; };
	static SYM reserved[] = { { "&lt;", 4, '<' }, { "&gt;", 4, '>' }, { "&amp;", 5, '&' }, { "&apos;", 6, '\'' }, { "&quot;", 6, '\"' }, };

	const char* s = _StrSource;
	while (*s)
	{
		if (d >= end)
			return nullptr;

		else if (*s == '&')
		{
			bool found = false;
			for (size_t i = 0; i < oCOUNTOF(reserved); i++)
			{
				if (0 == strcmp(reserved[i].res, s))
				{
					*d++ = reserved[i].c;
					s += reserved[i].len;
					found = true;
					break;
				}
			}

			if (!found)
				*d++ = *s++;
		}

		else
			*d++ = *s++;
	}

	*d = 0;
	return _StrDestination;
}

} // namespace ouro
