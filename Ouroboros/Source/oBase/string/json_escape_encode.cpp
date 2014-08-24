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
#include <oBase/string.h>
#include <oBase/throw.h>

#define oHEXDIGIT(d) ("0123456789abcdef"[(d)&0xf])

namespace ouro {

char* json_escape_encode(char* _StrDestination, size_t _SizeofStrDestination, const char* _Source)
{
	sncatf(_StrDestination, _SizeofStrDestination, "\"");
	while (*_Source)
	{
		if ((*_Source & 0x80) != 0)
		{
			// TODO: Support UTF-8 
			oTHROW(function_not_supported, "UTF-8 not yet supported");
			//sncatf(_StrDestination, _SizeofStrDestination, "_");
			//int unicode = oUTF8Decode(_Source);
			//sncatf(_StrDestination, _SizeofStrDestination, "\\u%c%c%c%c", oHEXDIGIT((unicode >> 12) & 0xf), oHEXDIGIT((unicode >> 8) & 0xf), oHEXDIGIT((unicode >> 4) & 0xf), oHEXDIGIT(unicode & 0xf));
		}
		if (*_Source == '\\' || *_Source == '\"')
		{
			sncatf(_StrDestination, _SizeofStrDestination, "\\%c", *_Source);
		}
		else if (*_Source <= 0x1f)
		{
			switch (*_Source)
			{
				case '\b': sncatf(_StrDestination, _SizeofStrDestination, "\\b"); break;
				case '\f': sncatf(_StrDestination, _SizeofStrDestination, "\\f"); break;
				case '\n': sncatf(_StrDestination, _SizeofStrDestination, "\\n"); break;
				case '\r': sncatf(_StrDestination, _SizeofStrDestination, "\\r"); break;
				case '\t': sncatf(_StrDestination, _SizeofStrDestination, "\\t"); break;
				default: sncatf(_StrDestination, _SizeofStrDestination, "\\u00%c%c", oHEXDIGIT(*_Source >> 4), oHEXDIGIT(*_Source)); break;
			}
		}
		else
			sncatf(_StrDestination, _SizeofStrDestination, "%c", *_Source);

		_Source++;
	}
	sncatf(_StrDestination, _SizeofStrDestination, "\"");
	return _StrDestination;
}

}
