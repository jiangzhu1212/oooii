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
#include <oBase/vendor.h>

namespace ouro {

const char* as_string(const vendor::value& _Vendor)
{
	switch (_Vendor)
	{
		case vendor::unknown: return "unknown";
		case vendor::amd: return "amd";
		case vendor::apple: return "Apple";
		case vendor::arm: return "ARM";
		case vendor::intel: return "Intel";
		case vendor::internal: return "internal";
		case vendor::lg: return "LG";
		case vendor::maxtor: return "Maxtor";
		case vendor::microsoft: return "Microsoft";
		case vendor::nintendo: return "Nintendo";
		case vendor::nvidia: return "NVIDIA";
		case vendor::sandisk: return "SanDisk";
		case vendor::samsung: return "Samsung";
		case vendor::sony: return "Sony";
		case vendor::vizio: return "Vizio";
		case vendor::western_digital: return "Western Digital";
		default: break;
	}
	return "?";
}

}
