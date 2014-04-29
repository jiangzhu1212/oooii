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
// allocate in O(1) time from a preallocated array of blocks (fixed block
// allocator).
#ifndef oBase_pool_h
#define oBase_pool_h

#include <oBase/compiler_config.h>

namespace ouro
{

class pool
{
public:
	typedef unsigned int index_type;
	typedef unsigned int size_type;

	// if at capacity allocate() will return this value
	// (upper bits reserved for atomic tagging)
	static const index_type nullidx = 0xffffffff;

	// the largest index issued by this pool, also serving as a static max_capacity()
	static const size_type max_index = nullidx - 1;

	// the maximum number of items a pool can hold
	static size_type max_capacity() { return max_index; }

	// ctor creates as empty
	pool();

	// ctor that moves an existing pool into this one
	pool(pool&& _That);

	// ctor creates as a valid pool using external memory
	pool(void* memory, size_type block_size, size_type capacity, size_type block_alignment = oDEFAULT_MEMORY_ALIGNMENT);

	// ctor creates as a valid pool using internally allocated memory
	pool(size_type block_size, size_type capacity, size_type block_alignment = oDEFAULT_MEMORY_ALIGNMENT);

	// dtor
	~pool();

	// calls deinit on this, moves that's memory under the same config
	pool& operator=(pool&& _That);

	// returns the bytes required for this class. If memory is not nullptr then the class
	// is initialized as a full pool of memory blocks.
	size_type initialize(void* memory, size_type block_size, size_type capacity, size_type block_alignment = oDEFAULT_MEMORY_ALIGNMENT);

	// self-allocates and manages memory used, otherwise this is like the other initialize()
	size_type initialize(size_type block_size, size_type capacity, size_type block_alignment = oDEFAULT_MEMORY_ALIGNMENT);

	// deinitializes the pool, returning it to a default-constructed state. This returns the
	// memory used in initialize or nullptr if self-allocated memory was used.
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
	void* blocks;
	size_type stride;
	size_type nblocks;
	size_type nfree;
	unsigned int head;
	bool owns_memory;

	pool(const pool&); /* = delete; */
	const pool& operator=(const pool&); /* = delete; */
};

} // namespace ouro

#endif
