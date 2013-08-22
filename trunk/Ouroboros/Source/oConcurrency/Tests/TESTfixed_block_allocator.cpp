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
#include <oConcurrency/fixed_block_allocator.h>
#include <oConcurrency/oConcurrency.h>
#include <oStd/throw.h>
#include <vector>

namespace oConcurrency {
	namespace tests {

static const unsigned int kMagicValue = 0xc001c0de;

struct TestObj
{
	TestObj(bool* _pDestroyed)
		: Value(kMagicValue)
		, pDestroyed(_pDestroyed)
	{
		*pDestroyed = false;
	}

	~TestObj()
	{
		*pDestroyed = true;
	}

	size_t Value;
	bool* pDestroyed;
};

static void test_fba_allocate()
{
	static const size_t NumBlocks = 20;
	static const size_t BlockSize = sizeof(TestObj);
	std::vector<char> scopedArena(fixed_block_allocator::calc_required_size(BlockSize, NumBlocks));
	fixed_block_allocator* pAllocator = ::new (&scopedArena[0]) fixed_block_allocator(BlockSize, NumBlocks);
	oCHECK(NumBlocks == pAllocator->count_available(BlockSize), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = pAllocator->allocate(BlockSize);
		oCHECK(tests[i], "TestObj %u should have been allocated", i);
	}

	void* shouldBeNull = pAllocator->allocate(BlockSize);
	oCHECK(!shouldBeNull, "Allocation should have failed");

	oCHECK(0 == pAllocator->count_available(BlockSize), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		pAllocator->deallocate(BlockSize, NumBlocks, tests[i]);

	oCHECK(NumBlocks == pAllocator->count_available(BlockSize), "There should be %u available blocks (after deallocate)", NumBlocks);
}

static void test_fba_create()
{
	static const size_t NumBlocks = 20;
	std::vector<char> scopedArena(fixed_block_allocator_t<TestObj>::calc_required_size(NumBlocks));
	fixed_block_allocator_t<TestObj>* pAllocator = ::new (&scopedArena[0]) fixed_block_allocator_t<TestObj>(NumBlocks);
		
	bool testdestroyed[NumBlocks];
	TestObj* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = pAllocator->construct(&testdestroyed[i]);
		oCHECK(tests[i], "TestObj %u should have been allocated", i);
		oCHECK(tests[i]->Value == kMagicValue, "TestObj %u should have been constructed", i);
		oCHECK(tests[i]->pDestroyed && false == *tests[i]->pDestroyed, "TestObj %u should have been constructed", i);
	}

	for (size_t i = 0; i < NumBlocks; i++)
	{
		pAllocator->destroy(NumBlocks, tests[i]);
		oCHECK(testdestroyed[i] == true, "TestObj %u should have been destroyed", i);
	}

	oCHECK(NumBlocks == pAllocator->count_available(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

static void test_fba_concurrency()
{
	static const int NumBlocks = 10000;
	std::vector<char> scopedArena(fixed_block_allocator_t<TestObj>::calc_required_size(NumBlocks));
	fixed_block_allocator_t<TestObj>* pAllocator = ::new (&scopedArena[0]) fixed_block_allocator_t<TestObj>(NumBlocks);

	bool destroyed[NumBlocks];
	memset(destroyed, 0, sizeof(destroyed));

	TestObj* tests[NumBlocks];
	memset(tests, 0xaa, sizeof(tests));

	parallel_for(0, NumBlocks, [&](size_t _Index)
	{
		tests[_Index] = pAllocator->construct(&destroyed[_Index]);
		tests[_Index]->Value = _Index;
		if (_Index & 0x1)
			pAllocator->destroy(NumBlocks, tests[_Index]);
	});

	oCHECK((NumBlocks/2) == pAllocator->count_available(), "Allocation/Destroys did not occur correctly");

	for (size_t i = 0; i < NumBlocks; i++)
	{
		if (i & 0x1)
			oCHECK(destroyed[i], "Destruction did not occur for allocation %d", i);
		else
			oCHECK(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
	}
	
}

void TESTfixed_block_allocator()
{
	test_fba_allocate();
	test_fba_create();
	test_fba_concurrency();
}

	} // namespace tests
} // namespace oConcurrency
