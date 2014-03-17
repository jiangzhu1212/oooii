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
#include <oBase/fixed_block_allocator.h>
#include <oBase/throw.h>
#include <vector>

namespace ouro {
	namespace tests {

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
		tests[i] = Allocator.construct(&testdestroyed[i]);
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

void TESTfixed_block_allocator()
{
	test_cfba_allocate();
	test_cfba_create();
}

	} // namespace tests
} // namespace ouro
