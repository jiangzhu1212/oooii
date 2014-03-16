/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
#include <oBase/hash_map.h>
#include <oBase/fnv1a.h>
#include <oBase/throw.h>
#include <string>
#include <vector>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

#if 0

static const unsigned int kMagicValue = 0xc001c0de;

struct test_obj
{
	test_obj() : Value(0), pDestroyed(nullptr) {}

	test_obj(bool* _pDestroyed)
		: Value(kMagicValue)
		, pDestroyed(_pDestroyed)
	{
		*pDestroyed = false;
	}

	~test_obj()
	{
		*pDestroyed = true;
	}

	size_t Value;
	bool* pDestroyed;
};

static void test_cfba_allocate()
{
	static const size_t NumBlocks = 20;
	static const size_t BlockSize = sizeof(test_obj);
	std::vector<char> scopedArena(BlockSize * NumBlocks);
	
	fixed_block_allocator<unsigned char> Allocator(scopedArena.data(), BlockSize, NumBlocks);
	oCHECK(NumBlocks == Allocator.count_available(), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.allocate();
		oCHECK(tests[i], "test_obj %u should have been allocated", i);
	}

	void* shouldBeNull = Allocator.allocate();
	oCHECK(!shouldBeNull, "Allocation should have failed");

	oCHECK(0 == Allocator.count_available(), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		Allocator.deallocate(tests[i]);

	oCHECK(NumBlocks == Allocator.count_available(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

static void test_cfba_create()
{
	static const size_t NumBlocks = 20;
	std::vector<char> scopedArena(NumBlocks * sizeof(test_obj));
	fixed_block_allocator_t<unsigned short, test_obj> Allocator(scopedArena.data(), NumBlocks); 
		
	bool testdestroyed[NumBlocks];
	test_obj* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.create(&testdestroyed[i]);
		oCHECK(tests[i], "test_obj %u should have been allocated", i);
		oCHECK(tests[i]->Value == kMagicValue, "test_obj %u should have been constructed", i);
		oCHECK(tests[i]->pDestroyed && false == *tests[i]->pDestroyed, "test_obj %u should have been constructed", i);
	}

	for (size_t i = 0; i < NumBlocks; i++)
	{
		Allocator.destroy(NumBlocks, tests[i]);
		oCHECK(testdestroyed[i] == true, "test_obj %u should have been destroyed", i);
	}

	oCHECK(NumBlocks == Allocator.count_available(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

#endif

void generate_strings(test_services& _Services, std::vector<std::string>& _Strings, size_t _NumToGenerate)
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

void TESThash_map(test_services& _Services)
{
	typedef unsigned long long hash_t;

	typedef hash_map<hash_t, std::string> hash_map_t;
	static const unsigned int NumElements = 10;

	std::vector<std::string> Strings;
	generate_strings(_Services, Strings, NumElements);
	std::vector<hash_t> Hashes;
	Hashes.reserve(Strings.size());

	for (const auto& s : Strings)
		Hashes.push_back(fnv1a<hash_t>(s.c_str()));

	hash_map_t map2(NumElements);
	hash_map_t map1(std::move(map2));
	hash_map_t map = std::move(map1);

	for (size_t i = 0; i < Strings.size(); i++)
		oCHECK(map.add(Hashes[i], Strings[i]) || map.exists(Hashes[i]), "failed to add entry");

	oCHECK(map.size() == Strings.size(), "unexpected map size");
	int CountedEntries = 0;
	map.enumerate_const([&](const hash_t& _HashKey, const std::string& _Value)->bool
	{
		oCHECK(std::find(Hashes.begin(), Hashes.end(), _HashKey) != Hashes.end(), "traversing a hash that was not inserted");
		CountedEntries++;
		return true;
	});
	
	oCHECK(CountedEntries == Strings.size(), "not all entries were added");
	CountedEntries = 0;

	map.enumerate([&](const hash_t& _HashKey, std::string& _Value)->bool
	{
		oCHECK(std::find(Hashes.begin(), Hashes.end(), _HashKey) != Hashes.end(), "traversing a hash that was not inserted");
		CountedEntries++;
		return true;
	});
	
	oCHECK(CountedEntries == Strings.size(), "not all entries were added");
	CountedEntries = 0;

	map.resize(NumElements * 2);
	map.enumerate_const([&](const hash_t& _HashKey, const std::string& _Value)->bool
	{
		oCHECK(std::find(Hashes.begin(), Hashes.end(), _HashKey) != Hashes.end(), "traversing a hash that was not inserted");
		CountedEntries++;
		return true;
	});
	
	oCHECK(CountedEntries == Strings.size(), "resize failed");

	for (size_t i = 0; i < Strings.size(); i += 2)
		oCHECK(map.remove(Hashes[i]), "failed to remove an entry known to exist");

	oCHECK(map.size() == Strings.size() / 2, "unexpected map size");

	for (size_t i = 0; i < Strings.size(); i++)
	{
		if (i & 0x1)
			oCHECK(map.exists(Hashes[i]), "an entry that should be present was not found");
		else
			oCHECK(!map.exists(Hashes[i]), "found an entry that was just removed");
	}

	unsigned int ExpectedRemoval = 0;
	for (size_t i = 0; i < Strings.size(); i += 3)
		if (map.mark(Hashes[i], 0xdeadc0de))
			ExpectedRemoval++;

	unsigned int Removed = map.sweep(0xdeadc0de);
	oCHECK(ExpectedRemoval == Removed, "removed a different number of entries than marked");

	oCHECK(map.size() == (Strings.size() / 2) - Removed, "unexpected map size");

	map.clear();
	oCHECK(map.empty(), "unexpected map size");
}

	} // namespace tests
} // namespace ouro
