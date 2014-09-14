// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/concurrent_pool.h>
#include <oMemory/concurrent_object_pool.h>
#include <oConcurrency/concurrency.h>
#include <vector>
#include "../../test_services.h"

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
static void test_index_pool(test_services& services)
{
	const size_t CAPACITY = 4;
	std::vector<unsigned int> buffer(256, 0xcccccccc);
	IndexPoolT a(buffer.data(), sizeof(concurrent_pool::index_type), CAPACITY);

	oTEST(a.full(), "index_allocator did not initialize correctly.");
	oTEST(a.capacity() == CAPACITY, "Capacity mismatch.");

	unsigned int index[4];
	for (auto& i : index)
		i = a.allocate_index();

	oTEST(a.nullidx == a.allocate_index(), "allocate succeed past allocator capacity");

	for (unsigned int i = 0; i < oCOUNTOF(index); i++)
		oTEST(index[i] == static_cast<unsigned int>(i), "Allocation mismatch %u.", i);

	a.deallocate(index[1]);
	a.deallocate(index[0]);
	a.deallocate(index[2]);
	a.deallocate(index[3]);

	oTEST(a.full(), "A deallocate failed.");
}

template<typename test_block_poolT>
static void test_allocate(test_services& services)
{
	static const size_t NumBlocks = 20;
	static const size_t BlockSize = sizeof(test_obj);
	std::vector<char> scopedArena(BlockSize * NumBlocks);
	
	test_block_poolT Allocator(scopedArena.data(), BlockSize, NumBlocks);
	oTEST(NumBlocks == Allocator.count_free(), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.allocate();
		oTEST(tests[i], "test_obj %u should have been allocated", i);
	}

	void* shouldBeNull = Allocator.allocate();
	oTEST(!shouldBeNull, "Allocation should have failed");

	oTEST(0 == Allocator.count_free(), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		Allocator.deallocate(tests[i]);

	oTEST(NumBlocks == Allocator.count_free(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

template<typename test_obj_poolT>
static void test_create(test_services& services)
{
	static const size_t NumBlocks = 20;
	std::vector<char> scopedArena(NumBlocks * sizeof(test_obj));
	test_obj_poolT Allocator(scopedArena.data(), NumBlocks); 
		
	bool testdestroyed[NumBlocks];
	test_obj* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.create(&testdestroyed[i]);
		oTEST(tests[i], "test_obj %u should have been allocated", i);
		oTEST(tests[i]->Value == kMagicValue, "test_obj %u should have been constructed", i);
		oTEST(tests[i]->pDestroyed && false == *tests[i]->pDestroyed, "test_obj %u should have been constructed", i);
	}

	for (size_t i = 0; i < NumBlocks; i++)
	{
		Allocator.destroy(tests[i]);
		oTEST(testdestroyed[i] == true, "test_obj %u should have been destroyed", i);
	}

	oTEST(NumBlocks == Allocator.count_free(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

static void test_concurrency(test_services& services)
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

	oTEST((NumBlocks/2) == Allocator.count_free(), "Allocation/Destroys did not occur correctly");

	for (size_t i = 0; i < NumBlocks; i++)
	{
		if (i & 0x1)
			oTEST(destroyed[i], "Destruction did not occur for allocation %d", i);
		else
		{
			oTEST(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
			Allocator.deallocate(tests[i]);
		}
	}

	oTEST(Allocator.full(), "allocator should be full");

	Allocator.deinitialize();
}

void TESTconcurrent_pool(test_services& services)
{
	test_index_pool<concurrent_pool>(services);
	test_allocate<concurrent_pool>(services);
	test_create<concurrent_object_pool<test_obj>>(services);
	test_concurrency(services);
}

	} // namespace tests
} // namespace ouro
