// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oBase/macros.h>
#include <oBase/string.h>

namespace ouro {

static void json_escape_decode_unicode(char* dst, size_t dst_size, const char* _Source)
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
		sncatf(dst, dst_size, "_");
	else
		sncatf(dst, dst_size, "%c", (unsigned char)UnicodeValue);
}

char* json_escape_decode(char* dst, size_t dst_size, const char* _Source)
{
	if (-1 == snprintf(dst, dst_size, ""))
		return nullptr;

	if (*_Source++ != '\"')
		return nullptr;

	while (*_Source)
	{
		if (*_Source == '\\')
		{
			switch (*++_Source)
			{
				case '\\': sncatf(dst, dst_size, "\\"); break;
				case '\"': sncatf(dst, dst_size, "\""); break;
				case 'b': sncatf(dst, dst_size, "\b"); break;
				case 'f': sncatf(dst, dst_size, "\f"); break;
				case 'n': sncatf(dst, dst_size, "\n"); break;
				case 'r': sncatf(dst, dst_size, "\r"); break;
				case 't': sncatf(dst, dst_size, "\t"); break;
				case 'u': json_escape_decode_unicode(dst, dst_size, ++_Source); _Source += 3; break;
				default: return false;
			}
		}
		else if (*_Source != '\"')
			sncatf(dst, dst_size, "%c", *_Source);

		_Source++;
	}
	return dst;
}

}
