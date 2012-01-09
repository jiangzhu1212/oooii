// $(header)
#include <oBasis/oFilterChain.h>
#include <oBasis/oError.h>

bool oBasisTest_oFilterChain()
{
	oFilterChain::FILTER filters[] =
	{
		{ ".*", oFilterChain::INCLUDE1 },
		{ "a+.*", oFilterChain::EXCLUDE1 },
		{ "(ab)+.*", oFilterChain::INCLUDE1 },
		{ "aabc", oFilterChain::INCLUDE1 },
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
	bool success = false;
	oFilterChain FilterChain(filters, err, &success);
	if (!success)
		return oErrorSetLast(oERROR_GENERIC, "Regex compile error in oFilterChain");

	for (int i = 0; i < oCOUNTOF(symbols); i++)
		if (FilterChain.Passes(symbols[i]) != expected[i])
			return oErrorSetLast(oERROR_GENERIC, "Failed filter on %d%s symbol", i, oOrdinal(i));

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
