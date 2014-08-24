/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
