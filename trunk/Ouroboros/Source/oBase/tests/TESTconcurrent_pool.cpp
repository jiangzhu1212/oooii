// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/concurrent_pool.h>
#include <oBase/concurrent_object_pool.h>
#include <oBase/concurrency.h>
#include <oBase/macros.h>
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
	IndexPoolT a(buffer.data(), sizeof(concurrent_pool::index_type), CAPACITY);

	oCHECK(a.full(), "index_allocator did not initialize correctly.");
	oCHECK(a.capacity() == CAPACITY, "Capacity mismatch.");

	unsigned int index[4];
	for (auto& i : index)
		i = a.allocate();

	oCHECK(a.nullidx == a.allocate(), "allocate succeed past allocator capacity");

	for (unsigned int i = 0; i < oCOUNTOF(index); i++)
		oCHECK(index[i] == static_cast<unsigned int>(i), "Allocation mismatch %u.", i);

	a.deallocate(index[1]);
	a.deallocate(index[0]);
	a.deallocate(index[2]);
	a.deallocate(index[3]);

	oCHECK(a.full(), "A deallocate failed.");
}

template<typename test_block_poolT>
static void test_allocate()
{
	static const size_t NumBlocks = 20;
	static const size_t BlockSize = sizeof(test_obj);
	std::vector<char> scopedArena(BlockSize * NumBlocks);
	
	test_block_poolT Allocator(scopedArena.data(), BlockSize, NumBlocks);
	oCHECK(NumBlocks == Allocator.count_free(), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.allocate_pointer();
		oCHECK(tests[i], "test_obj %u should have been allocated", i);
	}

	void* shouldBeNull = Allocator.allocate_pointer();
	oCHECK(!shouldBeNull, "Allocation should have failed");

	oCHECK(0 == Allocator.count_free(), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		Allocator.deallocate(tests[i]);

	oCHECK(NumBlocks == Allocator.count_free(), "There should be %u available blocks (after deallocate)", NumBlocks);
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

	oCHECK(NumBlocks == Allocator.count_free(), "There should be %u available blocks (after deallocate)", NumBlocks);
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

	oCHECK((NumBlocks/2) == Allocator.count_free(), "Allocation/Destroys did not occur correctly");

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

	oCHECK(Allocator.full(), "allocator should be full");

	Allocator.deinitialize();
}

void TESTconcurrent_pool()
{
	test_index_pool<concurrent_pool>();
	test_allocate<concurrent_pool>();
	test_create<concurrent_object_pool<test_obj>>();
	test_concurrency();
}

	} // namespace tests
} // namespace ouro
