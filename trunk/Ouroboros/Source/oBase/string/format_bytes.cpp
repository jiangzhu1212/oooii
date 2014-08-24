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
#include <oBase/string.h>
#include <oMemory/equal.h>
#include <oBase/macros.h>

int ouro::format_bytes(char* _StrDestination, size_t _SizeofStrDestination, unsigned long long bytes, size_t _NumPrecisionDigits)
{
	char fmt[16];
	int result = snprintf(fmt, "%%.0%uf %%s%%s", _NumPrecisionDigits);

	const char* Type = "";
	double Amount = 0.0;

	#define ELIF(_Label) else if (bytes > oCONCAT(o, _Label)(1)) { Type = #_Label; Amount = bytes / static_cast<double>(oCONCAT(o, _Label)(1)); }
		if (bytes < oKB(1)) { Type = "byte"; Amount = static_cast<double>(bytes); }
		ELIF(TB) ELIF(GB) ELIF(MB) ELIF(KB)
	#undef ELIF

	const char* Plural = ouro::equal(Amount, 1.0) ? "" : "s";
	return snprintf(_StrDestination, _SizeofStrDestination, fmt, Amount, Type, Plural);
}
