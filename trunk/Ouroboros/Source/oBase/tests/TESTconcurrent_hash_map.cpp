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
#include <oBase/concurrent_hash_map.h>
#include <oBase/fnv1a.h>
#include <oBase/throw.h>
#include <string>
#include <vector>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

static void generate_strings(test_services& _Services, std::vector<std::string>& _Strings, size_t _NumToGenerate)
{
	_Strings.reserve(_NumToGenerate);

	for (size_t i = 0; i < _NumToGenerate; i++)
	{
		size_t StringLength = _Services.rand();
		std::string s;
		s.reserve(StringLength);

		for (size_t i = 0; i < StringLength; i++)
			s += (char)_Services.rand();

		_Strings.push_back(s);
	}
}

void TEST_chm_basics(test_services& _Services)
{
	concurrent_hash_map h(12);
	oCHECK(h.capacity() == 31, "pow-of-two rounding failed");
	oCHECK(h.size() == 0, "should be empty");

	std::vector<std::string> Strings;
	generate_strings(_Services, Strings, 10);

	std::vector<concurrent_hash_map::key_type> Keys;
	Keys.resize(Strings.size());
	for (size_t i = 0; i < Strings.size(); i++)
		Keys[i] = fnv1a<concurrent_hash_map::key_type>(Strings[i].c_str());

	for (unsigned int i = 0; i < static_cast<unsigned int>(Strings.size()); i += 3)
		h.set(Keys[i], i);

	oCHECK(h.size() == ((Strings.size() / 3) + 1), "set failed");

	for (unsigned int i = 0; i < static_cast<unsigned int>(Strings.size()); i += 3)
		oCHECK(i == h.get(Keys[i]), "get failed");

	for (unsigned int i = 0; i < static_cast<unsigned int>(Strings.size()); i++)
		h.set(Keys[i], i);

	oCHECK(h.size() == Strings.size(), "set2 failed");

	for (unsigned int i = 0; i < static_cast<unsigned int>(Strings.size()); i += 3)
		h.remove(Keys[i]);

	oCHECK(h.size() == (Strings.size() - 4), "set2 failed");

	oCHECK(4 == h.reclaim(), "reclaim failed");

	h.clear();
	oCHECK(h.empty(), "clear failed");
}

void TESTconcurrent_hash_map(test_services& _Services)
{
	TEST_chm_basics(_Services);
}

	} // namespace tests
} // namespace ouro
