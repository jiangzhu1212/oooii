// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/filter_chain.h>
#include <oBase/throw.h>

using namespace ouro;

namespace ouro {
	namespace tests {

void TESTfilter_chain()
{
	filter_chain::filter filters[] =
	{
		{ ".*", filter_chain::include1 },
		{ "a+.*", filter_chain::exclude1 },
		{ "(ab)+.*", filter_chain::include1 },
		{ "aabc", filter_chain::include1 },
	};

	const char* symbols[] =
	{
		"test to succeed",
		"a test to fail",
		"aaa test to fail",
		"abab test to succeed",
		"aabc",
	};

	bool expected[] =
	{
		true,
		false,
		false,
		true,
		true,
	};

	char err[1024];
	filter_chain FilterChain(filters, err);
	oFORI(i, symbols)
		if (FilterChain.passes(symbols[i]) != expected[i])
			oTHROW(protocol_error, "Failed filter on %d%s symbol", i, ordinal(i));
}

	} // namespace tests
} // namespace ouro
