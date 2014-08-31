// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/fourcc.h>
#include <oBase/throw.h>
#include <oString/stringize.h>

namespace ouro {
	namespace tests {

void TESTfourcc()
{
	fourcc fcc('TEST');

	char str[16];

	if (!to_string(str, fcc))
		oTHROW(protocol_error, "to_string on fourcc failed 1");

	if (strcmp("TEST", str))
		oTHROW(protocol_error, "to_string on fourcc failed 2");

	const char* fccStr = "RGBA";
	if (!from_string(&fcc, fccStr))
		oTHROW(protocol_error, "from_string on fourcc failed 1");

	if (fourcc('RGBA') != fcc)
		oTHROW(protocol_error, "from_string on fourcc failed 2");
}

	} // namespace tests
} // namespace ouro
