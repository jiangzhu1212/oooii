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
#include <stdlib.h>
#include <oBase/string.h>

namespace ouro {

char* format_commas(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _Number)
{
	#ifdef _MSC_VER
		_itoa_s(_Number, _StrDestination, _SizeofStrDestination, 10);
	#else
		itoa(_Number, _StrDestination, 10);
	#endif
	size_t len = strlen(_StrDestination);

	size_t w = len % 3;
	if (!w)
		w = 3;

	while (w < len)
	{
		if (!insert(_StrDestination, _SizeofStrDestination, _StrDestination + w, 0, ","))
			return nullptr;
		w += 4;
	}

	return _StrDestination;
}

char* format_commas(char* _StrDestination, size_t _SizeofStrDestination, int _Number)
{
	if (_Number < 0)
	{
		if (_SizeofStrDestination < 1)
			return nullptr;

		_Number = -_Number;
		*_StrDestination++ = '-';
		_SizeofStrDestination--;
	}

	return format_commas(_StrDestination, _SizeofStrDestination, static_cast<unsigned int>(_Number));
}

}
