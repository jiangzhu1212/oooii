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
#include <ctype.h>

namespace ouro {

bool is_sep(wchar_t c) { return c == L'/' || c == L'\\'; }

size_t wcmnroot(const wchar_t* _Path1, const wchar_t* _Path2)
{
	size_t last_sep = 0;
	size_t len = 0;
	if (_Path1 && *_Path1 && _Path2 && *_Path2)
	{
		while (_Path1[len] && _Path2[len])
		{
			wchar_t a = towlower(_Path1[len]);
			wchar_t b = towlower(_Path2[len]);

			if (a != b || is_sep(a) != is_sep(b))
				break;

			if (is_sep(a) || is_sep(b))
				last_sep = len;
			
			len++;
		}
	}

	return last_sep;
}

}
