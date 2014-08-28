// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oString/string.h>
#include <oMemory/equal.h>
#include <oBase/macros.h>

int ouro::format_bytes(char* dst, size_t dst_size, uint64_t bytes, size_t num_precision_digits)
{
	char fmt[16];
	int result = snprintf(fmt, "%%.0%uf %%s%%s", num_precision_digits);

	const char* Type = "";
	double Amount = 0.0;

	#define ELIF(_Label) else if (bytes > oCONCAT(o, _Label)(1)) { Type = #_Label; Amount = bytes / static_cast<double>(oCONCAT(o, _Label)(1)); }
		if (bytes < oKB(1)) { Type = "byte"; Amount = static_cast<double>(bytes); }
		ELIF(TB) ELIF(GB) ELIF(MB) ELIF(KB)
	#undef ELIF

	const char* Plural = ouro::equal(Amount, 1.0) ? "" : "s";
	return snprintf(dst, dst_size, fmt, Amount, Type, Plural);
}
