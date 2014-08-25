// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <memory.h>
#include <oBase/macros.h>

namespace ouro {

char* next_matching(char* p_open_brace, const char* open_brace, const char* close_brace);

char* zero_block_comments(char* str, const char* open_brace, const char* close_brace, char replacement)
{
	char* cur = str;
	while (*cur)
	{
		cur = strstr(str, open_brace);
		if (!cur)
			break;

		char* close = next_matching(cur, open_brace, close_brace);
		if (!close)
			return nullptr;

		close += strlen(close_brace);

		// preserve newlines since they might be used to report line numbers in compile errors
		while (cur < close)
		{
			size_t offset = __min(strcspn(cur, oNEWLINE), static_cast<size_t>(close-cur));
			memset(cur, replacement, offset);
			cur += offset;
			cur += strspn(cur, oNEWLINE);
		}
	}

	return str;
}

}
