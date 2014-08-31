// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
