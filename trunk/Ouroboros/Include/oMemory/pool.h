// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oMemory_pool_h
#define oMemory_pool_h

// allocate in O(1) time from a preallocated array of blocks (fixed block
// allocator).

#include <cstdint>

namespace ouro
{

class pool
{
public:
	typedef uint32_t index_type;
	typedef uint32_t size_type;

	// if at capacity allocate() will return this value
	static const index_type nullidx = 0xffffffff;

	// the largest index issued by this pool, also serving as a static max_capacity()
	static const size_type max_index = nullidx - 1;

	// the maximum number of items a pool can hold
	static size_type max_capacity() { return max_index; }

	// ctor creates as empty
	pool();

	// ctor that moves an existing pool into this one
	pool(pool&& that);

	// ctor creates as a valid pool using external memory
	pool(void* memory, size_type block_size, size_type capacity);

	// dtor
	~pool();

	// calls deinit on this, moves that's memory under the same config
	pool& operator=(pool&& that);

	// Returns bytes required for memory; pass nullptr to obtain size, allocate
	// and then pass that to memory in a second call to initialize the class.
	size_type initialize(void* memory, size_type block_size, size_type capacity);

	// deinitializes the pool and returns the memory passed to initialize()
	void* deinitialize();

	// returns the number of items available
	inline size_type size_free() const { return nfree; }

	// returns the number of allocated elements
	inline size_type size() const { return capacity() - size_free(); }

	// returns true of there are no outstanding allocations
	inline bool full() const { return capacity() == size_free(); }

	// returns the max number of items that can be allocated from this pool
	inline size_type capacity() const { return nblocks; }

	// returns true if all items have been allocated
	inline bool empty() const { return head == nullidx; }

	// allocate/deallocate an index into the pool. If empty out of resources nullidx
	// will be returned.
	index_type allocate();
	void deallocate(index_type index);

	// Wrappers for treating the block as a pointer to block_size memory.
	inline void* allocate_pointer() { index_type i = allocate(); return pointer(i); }
	inline void deallocate(void* pointer) { deallocate(index(pointer)); }

	// convert between allocated index and pointer values
	void* pointer(index_type index) const;
	index_type index(void* pointer) const;

	// simple range check that returns true if this index/pointer could have been allocated from this pool
	bool owns(index_type index) const { return index < nblocks; }
	bool owns(void* pointer) const { return owns(index(pointer)); }

private:
	pool* next; // reserved for use with linked-lists
	uint8_t* blocks;
	size_type stride;
	size_type nblocks;
	size_type nfree;
	uint32_t head;

	pool(const pool&); /* = delete; */
	const pool& operator=(const pool&); /* = delete; */
};

}

#endif
