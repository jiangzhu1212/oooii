// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oBase/macros.h>

namespace ouro {

char* ampersand_decode(char* dst, size_t dst_size, const char* src)
{
	if (!dst || !dst_size)
		return nullptr;

	char* d = dst;
	char* end = d + dst_size - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	struct SYM { const char* res; unsigned short len; char c; };
	static SYM reserved[] = { { "&lt;", 4, '<' }, { "&gt;", 4, '>' }, { "&amp;", 5, '&' }, { "&apos;", 6, '\'' }, { "&quot;", 6, '\"' }, };

	const char* s = src;
	while (*s)
	{
		if (d >= end)
			return nullptr;

		else if (*s == '&')
		{
			bool found = false;
			for (size_t i = 0; i < oCOUNTOF(reserved); i++)
			{
				if (0 == strcmp(reserved[i].res, s))
				{
					*d++ = reserved[i].c;
					s += reserved[i].len;
					found = true;
					break;
				}
			}

			if (!found)
				*d++ = *s++;
		}

		else
			*d++ = *s++;
	}

	*d = 0;
	return dst;
}

}
