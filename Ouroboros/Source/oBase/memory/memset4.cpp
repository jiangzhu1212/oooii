// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include "memduff.h"

namespace ouro {

void memset4(void* dst, int32_t value, size_t bytes)
{
	// Probably slower than stdc's memset but sets a full int32_t at a time.

	// First move dst up to alignment
	int32_t* body;
	int8_t* prefix, *postfix;
	size_t prefix_nbytes, postfix_nbytes;
	detail::init_duffs_device_pointers(dst, bytes, &prefix, &prefix_nbytes, &body, &postfix, &postfix_nbytes);

	byte_swizzle32 s;
	s.as_int = value;

	// Duff's device up to alignment: http://en.wikipedia.org/wiki/Duff's_device
	switch (prefix_nbytes)
	{
		case 3: *prefix++ = s.as_char[3];
		case 2: *prefix++ = s.as_char[2];
		case 1: *prefix++ = s.as_char[1];
		default: break;
	}

	// Do aligned assignment
	while (body < (int32_t*)postfix)
		*body++ = value;

	// Duff's device final bytes: http://en.wikipedia.org/wiki/Duff's_device
	switch (postfix_nbytes)
	{
		case 3: *postfix++ = s.as_char[3];
		case 2: *postfix++ = s.as_char[2];
		case 1: *postfix++ = s.as_char[1];
		default: break;
	}
}

}
