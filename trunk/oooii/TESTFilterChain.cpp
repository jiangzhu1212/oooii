/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
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

