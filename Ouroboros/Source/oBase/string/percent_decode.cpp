// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oCompiler.h>
#include <oBase/throw.h>

namespace ouro {

char* percent_to_lower(char* oRESTRICT dst, size_t dst_size, const char* oRESTRICT src)
{
	char* d = dst;
	char* end = d + dst_size - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	const char* s = src;
	while (*s)
	{
		bool do_lower = *s == '%';
		*d++ = *s++;
		if (do_lower)
		{
			*d++ = (char)tolower(*s++);
			*d++ = (char)tolower(*s++);
		}
	}

	*d = 0;
	return dst;
}

char* percent_decode(char* oRESTRICT dst, size_t dst_size, const char* oRESTRICT src)
{
	char* d = dst;
	char* end = d + dst_size - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	const char* s = src;
	while (*s)
	{
		if (*s == '%')
		{
			char hex[3] = { *(s+1), *(s+2), 0 };
			char c = strtoul(hex, nullptr, 16) & 0xff;
			*d++ = c;
			s += 3;
		}

		else if (d >= end)
			oTHROW0(no_buffer_space);
		else
			*d++ = *s++;
	}

	*d = 0;
	return dst;
}

}
