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

namespace oStd {

char* ellipsize(char* _StrDestination, size_t _SizeofStrDestination)
{
	switch (_SizeofStrDestination - 4)
	{ // Duff's device (no breaks)
		default: _StrDestination[_SizeofStrDestination-4] = '.';
		case 3: _StrDestination[_SizeofStrDestination-3] = '.';
		case 2: _StrDestination[_SizeofStrDestination-2] = '.';
		case 1: _StrDestination[_SizeofStrDestination-1] = 0;
		case 0: break;
	}

	return _StrDestination;
}

} // namespace oStd
