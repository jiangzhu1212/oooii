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
#include <oBase/string.h>

namespace ouro {

static void json_escape_decode_unicode(char* _StrDestination, size_t _SizeofStrDestination, const char* _Source)
{
	unsigned int UnicodeValue = 0;
	// Expects 4 digits from _Source and turns it into UTF-8
	for (int i = 0; i < 4; i++)
	{
		UnicodeValue <<= 4;
		char Digit = *_Source++;
		if (Digit >= '0' && Digit <= '9') UnicodeValue |= Digit - '0';
		else if (Digit >= 'a' && Digit <= 'f') UnicodeValue |= Digit - 'a' + 10;
		else if (Digit >= 'A' && Digit <= 'F') UnicodeValue |= Digit - 'A' + 10;
	}
	// TODO: Support UTF-8 and surrogate pair ranges
	if (!UnicodeValue || UnicodeValue > 255)
		sncatf(_StrDestination, _SizeofStrDestination, "_");
	else
		sncatf(_StrDestination, _SizeofStrDestination, "%c", (unsigned char)UnicodeValue);
}

char* json_escape_decode(char* _StrDestination, size_t _SizeofStrDestination, const char* _Source)
{
	if (-1 == snprintf(_StrDestination, _SizeofStrDestination, ""))
		return nullptr;

	if (*_Source++ != '\"')
		return nullptr;

	while (*_Source)
	{
		if (*_Source == '\\')
		{
			switch (*++_Source)
			{
				case '\\': sncatf(_StrDestination, _SizeofStrDestination, "\\"); break;
				case '\"': sncatf(_StrDestination, _SizeofStrDestination, "\""); break;
				case 'b': sncatf(_StrDestination, _SizeofStrDestination, "\b"); break;
				case 'f': sncatf(_StrDestination, _SizeofStrDestination, "\f"); break;
				case 'n': sncatf(_StrDestination, _SizeofStrDestination, "\n"); break;
				case 'r': sncatf(_StrDestination, _SizeofStrDestination, "\r"); break;
				case 't': sncatf(_StrDestination, _SizeofStrDestination, "\t"); break;
				case 'u': json_escape_decode_unicode(_StrDestination, _SizeofStrDestination, ++_Source); _Source += 3; break;
				default: return false;
			}
		}
		else if (*_Source != '\"')
			sncatf(_StrDestination, _SizeofStrDestination, "%c", *_Source);

		_Source++;
	}
	return _StrDestination;
}

} // namespace ouro
