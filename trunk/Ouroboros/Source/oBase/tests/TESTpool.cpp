// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/pool.h>
#include <oBase/macros.h>
#include <oBase/object_pool.h>
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

	IndexPoolT a(buffer.data(), sizeof(unsigned int), CAPACITY);

	oCHECK(a.full(), "index_allocator did not initialize correctly.");
	oCHECK(a.capacity() == CAPACITY, "Capacity mismatch.");

	unsigned int index[4];
	for (auto i = 0; i < oCOUNTOF(index); i++)
		index[i] = a.allocate();

	oCHECK(a.nullidx == a.allocate(), "allocate succeed past allocator capacity");

	for (auto i = 0; i < oCOUNTOF(index); i++)
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
	
	test_block_poolT Allocator(scopedArena.data(), NumBlocks);
	oCHECK(NumBlocks == Allocator.size_free(), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = Allocator.allocate_pointer();
		oCHECK(tests[i], "test_obj %u should have been allocated", i);
	}

	void* shouldBeNull = Allocator.allocate_pointer();
	oCHECK(!shouldBeNull, "Allocation should have failed");

	oCHECK(0 == Allocator.size_free(), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		Allocator.deallocate(tests[i]);

	oCHECK(NumBlocks == Allocator.size_free(), "There should be %u available blocks (after deallocate)", NumBlocks);
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

	oCHECK(NumBlocks == Allocator.size_free(), "There should be %u available blocks (after deallocate)", NumBlocks);
}

void TESTpool()
{
	test_index_pool<pool>();
	test_allocate<object_pool<test_obj>>();
	test_create<object_pool<test_obj>>();
}

	} // namespace tests
} // namespace ouro
