// $(header)
#include <oooii/oFilterChain.h>
#include <oooii/oRef.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>

struct TESTFilterChain : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
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

		oRef<threadsafe oFilterChain> pFilterChain;
		oTESTB(oFilterChain::Create(filters, &pFilterChain), "Failed to create filter chain");

		for (int i = 0; i < oCOUNTOF(symbols); i++)
			oTESTB(pFilterChain->Passes(symbols[i], 0) == expected[i], "Failed filter on %d%s symbol", i, oOrdinal(i));

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTFilterChain);

