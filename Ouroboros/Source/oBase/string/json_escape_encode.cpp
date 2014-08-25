// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oBase/macros.h>
#include <oBase/string.h>
#include <oBase/throw.h>

#define oHEXDIGIT(d) ("0123456789abcdef"[(d)&0xf])

namespace ouro {

char* json_escape_encode(char* dst, size_t dst_size, const char* _Source)
{
	sncatf(dst, dst_size, "\"");
	while (*_Source)
	{
		if ((*_Source & 0x80) != 0)
		{
			// TODO: Support UTF-8 
			oTHROW(function_not_supported, "UTF-8 not yet supported");
			//sncatf(dst, dst_size, "_");
			//int unicode = oUTF8Decode(_Source);
			//sncatf(dst, dst_size, "\\u%c%c%c%c", oHEXDIGIT((unicode >> 12) & 0xf), oHEXDIGIT((unicode >> 8) & 0xf), oHEXDIGIT((unicode >> 4) & 0xf), oHEXDIGIT(unicode & 0xf));
		}
		if (*_Source == '\\' || *_Source == '\"')
		{
			sncatf(dst, dst_size, "\\%c", *_Source);
		}
		else if (*_Source <= 0x1f)
		{
			switch (*_Source)
			{
				case '\b': sncatf(dst, dst_size, "\\b"); break;
				case '\f': sncatf(dst, dst_size, "\\f"); break;
				case '\n': sncatf(dst, dst_size, "\\n"); break;
				case '\r': sncatf(dst, dst_size, "\\r"); break;
				case '\t': sncatf(dst, dst_size, "\\t"); break;
				default: sncatf(dst, dst_size, "\\u00%c%c", oHEXDIGIT(*_Source >> 4), oHEXDIGIT(*_Source)); break;
			}
		}
		else
			sncatf(dst, dst_size, "%c", *_Source);

		_Source++;
	}
	sncatf(dst, dst_size, "\"");
	return dst;
}

}
