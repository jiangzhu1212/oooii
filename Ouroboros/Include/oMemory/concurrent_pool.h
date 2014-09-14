// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// O(1) fine-grained concurrent block allocator: uses space inside free blocks to 
// maintain freelist.

#pragma once
#include <oCompiler.h>
#include <cstdint>
#include <atomic>

namespace ouro {

class concurrent_pool
{
public:
	typedef uint32_t index_type;
	typedef uint32_t size_type;

	// if at capacity allocate() will return this value
	// (upper bits reserved for atomic tagging)
	static const index_type nullidx = 0x00ffffff;

	// the largest index issued by this pool, also serving as a static max_capacity()
	static const size_type max_index = nullidx - 1;

	// the maximum number of items a concurrent_pool can hold
	static size_type max_capacity() { return max_index; }

	// returns the size the memory param must be in init methods below
	static size_type calc_size(size_type block_size, size_type capacity);


	// non-concurrent api

	// ctor creates as empty
	concurrent_pool();

	// ctor moves an existing pool into this one
	concurrent_pool(concurrent_pool&& _That);

	// ctor creates as a valid pool using external memory
	concurrent_pool(void* memory, size_type block_size, size_type capacity);

	// dtor
	~concurrent_pool();

	// calls deinitialize on this, moves that's memory under the same config
	concurrent_pool& operator=(concurrent_pool&& _That);

	// use calc_size() to determine memory size
	void initialize(void* memory, size_type block_size, size_type capacity);

	// deinitializes the pool and returns the memory passed to initialize()
	void* deinitialize();

	// SLOW! walks the free list and returns the count
	size_type count_free() const;

	// SLOW! returns the number of allocated elements
	inline size_type size() const { return capacity() - count_free(); }

	// SLOW! returns true of there are no outstanding allocations
	inline bool full() const { return capacity() == count_free(); }


	// concurrent api

	// returns the max number of items that can be allocated from this pool
	inline size_type capacity() const { return nblocks; }

	// returns true if all items have been allocated
	bool empty() const;

	// allocate/deallocate an index into the pool. If empty out of resources nullidx
	// will be returned.
	index_type allocate_index();
	void deallocate(index_type index);

	// Wrappers for treating the block as a pointer to block_size memory.
	inline void* allocate() { index_type i = allocate_index(); return pointer(i); }
	inline void deallocate(void* pointer) { deallocate(index(pointer)); }

	// convert between allocated index and pointer values
	void* pointer(index_type index) const;
	index_type index(void* ptr) const;

	// simple range check that returns true if this index/pointer could have been allocated from this pool
	bool owns(index_type index) const { return index < nblocks; }
	bool owns(void* ptr) const { return owns(index(ptr)); }

private:
	union
	{
		char cache_padding[oCACHE_LINE_SIZE];
		struct
		{
			concurrent_pool* next;
			uint8_t* blocks;
			size_type stride;
			size_type nblocks;
			std::atomic_uint head;
			bool owns_memory;
		};
	};

	concurrent_pool(const concurrent_pool&); /* = delete; */
	const concurrent_pool& operator=(const concurrent_pool&); /* = delete; */
};
static_assert(sizeof(concurrent_pool) == oCACHE_LINE_SIZE, "size mismatch");

}
