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
