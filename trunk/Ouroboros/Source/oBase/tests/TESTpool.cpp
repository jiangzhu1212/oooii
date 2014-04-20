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
#include <oBase/pool.h>
#include <oBase/concurrency.h>
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

template<typename IndexPoolT>
static void test_index_pool()
{
	const size_t CAPACITY = 4;
	std::vector<unsigned int> buffer(256, 0xcccccccc);

	IndexPoolT a(buffer.data(), CAPACITY);

	oCHECK(a.empty(), "index_allocator did not initialize correctly.");
	oCHECK(a.capacity() == CAPACITY, "Capacity mismatch.");

	unsigned int index[4];
	oFORI(i, index)
		index[i] = a.allocate();

	oCHECK(a.invalid_index == a.allocate(), "allocate succeed past allocator capacity");

	oFORI(i, index)
		oCHECK(index[i] == static_cast<unsigned int>(i), "Allocation mismatch %u.", i);

	a.deallocate(index[1]);
	a.deallocate(index[0]);
	a.deallocate(index[2]);
	a.deallocate(index[3]);

	oCHECK(a.empty(), "A deallocate failed.");
}

template<typename test_block_poolT>
static void test_allocate()
{
	static const size_t NumBlocks = 20;
	static const size_t BlockSize = sizeof(test_obj);
	std::vector<char> scopedArena(BlockSize * NumBlocks);
	
	test_block_poolT Allocator(scopedArena.data(), NumBlocks);
	oCHECK(NumBlocks == Allocator.count_available(), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.allocate_ptr();
		oCHECK(tests[i], "test_obj %u should have been allocated", i);
	}

	void* shouldBeNull = Allocator.allocate_ptr();
	oCHECK(!shouldBeNull, "Allocation should have failed");

	oCHECK(0 == Allocator.count_available(), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		Allocator.deallocate(tests[i]);

	oCHECK(NumBlocks == Allocator.count_available(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

template<typename test_obj_poolT>
static void test_create()
{
	static const size_t NumBlocks = 20;
	std::vector<char> scopedArena(NumBlocks * sizeof(test_obj));
	test_obj_poolT Allocator(scopedArena.data(), NumBlocks); 
		
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
		Allocator.destroy(tests[i]);
		oCHECK(testdestroyed[i] == true, "test_obj %u should have been destroyed", i);
	}

	oCHECK(NumBlocks == Allocator.count_available(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

static void test_concurrency()
{
	static const size_t NumBlocks = 10000;
	std::vector<char> scopedArena(NumBlocks * sizeof(test_obj));
	concurrent_object_pool<test_obj> Allocator(scopedArena.data(), NumBlocks);

	bool destroyed[NumBlocks];
	memset(destroyed, 0, sizeof(destroyed));

	test_obj* tests[NumBlocks];
	memset(tests, 0xaa, sizeof(tests));

	ouro::parallel_for(0, NumBlocks, [&](size_t _Index)
	{
		tests[_Index] = Allocator.create(&destroyed[_Index]);
		tests[_Index]->Value = _Index;
		if (_Index & 0x1)
			Allocator.destroy(tests[_Index]);
	});

	oCHECK((NumBlocks/2) == Allocator.count_available(), "Allocation/Destroys did not occur correctly");

	for (size_t i = 0; i < NumBlocks; i++)
	{
		if (i & 0x1)
			oCHECK(destroyed[i], "Destruction did not occur for allocation %d", i);
		else
		{
			oCHECK(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
			Allocator.deallocate(tests[i]);
		}
	}

	oCHECK(Allocator.empty(), "allocator should be empty");

	Allocator.deinitialize();
}

void TESTpool()
{
	test_index_pool<pool<unsigned int>>();
	test_index_pool<concurrent_pool<unsigned int>>();

	test_allocate<pool<test_obj>>();
	test_create<object_pool<test_obj>>();
	test_allocate<concurrent_pool<test_obj>>();
	test_create<concurrent_object_pool<test_obj>>();
	test_concurrency();
}

	} // namespace tests
} // namespace ouro
