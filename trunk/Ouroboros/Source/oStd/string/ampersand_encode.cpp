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
#include <oStd/assert.h>
#include <oStd/macros.h>
#include <oStd/string.h>

namespace oStd {

char* ampersand_encode(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	*_StrDestination = 0;
	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	oASSERT(_StrSource < _StrDestination || _StrSource >= (_StrDestination + _SizeofStrDestination), "Overlapping buffers not allowed");

	const char* xml_reserved_chars = "<>&'\"";
	struct SYM { const char* res; unsigned short len; char c; };
	static SYM reserved[] = { { "&lt;", 4, '<' }, { "&gt;", 4, '>' }, { "&amp;", 5, '&' }, { "&apos;", 6, '\'' }, { "&quot;", 6, '\"' }, };

	const char* s = _StrSource;
	while (*s)
	{
		const char* pos = strchr(xml_reserved_chars, *s);
		if (pos)
		{
			size_t reserved_idx = std::distance(xml_reserved_chars, pos);

			if ((d+reserved[reserved_idx].len) > end)
				return nullptr;

			snprintf(d, std::distance(d, end), "%s", reserved[reserved_idx].res);
			d += reserved[reserved_idx].len;
			s++;
		}

		else if (d >= end)
			return nullptr;
		else
			*d++ = *s++;
	}

	*d = 0;
	return _StrDestination;
}

} // namespace oStd
