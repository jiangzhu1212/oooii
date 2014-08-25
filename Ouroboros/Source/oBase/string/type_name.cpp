// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <stdlib.h>
#include <oBase/macros.h>

namespace ouro {

const char* type_name(const char* typeinfo_name)
{
	static struct { const char* s; size_t len; } sPrefixesToSkip[] = { { "enum ", 5 }, { "struct ", 7 }, { "class ", 6 }, { "interface ", 10 }, { "union ", 6 }, };
	const char* n = typeinfo_name;
	for (size_t i = 0; i < oCOUNTOF(sPrefixesToSkip); i++)
		if (!memcmp(n, sPrefixesToSkip[i].s, sPrefixesToSkip[i].len))
			return n + sPrefixesToSkip[i].len;
	return n;
}

}
