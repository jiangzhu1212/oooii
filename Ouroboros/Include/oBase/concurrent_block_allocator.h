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
// This behaves the same as a concurrent_fixed_block_allocator but this can
// grow beyond its initial size. It does this in slabs of local fba's so if
// one slab is full, it looks into others for allocation space. Allocation
// and deallocation time should be mostly O(1) with the occasional new slab
// allocation or iterating slabs to find free space. Atomics ensure 
// concurrency.

#ifndef oBase_block_allocator_h
#define oBase_block_allocator_h

#include <oBase/concurrent_stack.h>
#include <oBase/concurrent_fixed_block_allocator.h>
#include <atomic>
#include <cstdlib>

namespace ouro {

class concurrent_block_allocator
{
	typedef concurrent_fixed_block_allocator_base<unsigned short> fba_t;

public:
	typedef fba_t::size_type size_type;
	typedef std::function<void*(size_t _Size)> allocate_t;
	typedef std::function<void(void* _Pointer)> deallocate_t;

	static const size_type max_blocks_per_chunk = fba_t::max_num_blocks;

	// The specified allocate and deallocate functions must be thread safe
	concurrent_block_allocator(size_type _BlockSize
		, size_type _NumBlocksPerChunk = max_blocks_per_chunk
		, const allocate_t& _PlatformAllocate = malloc
		, const deallocate_t& _PlatformDeallocate = free);

	~concurrent_block_allocator();

	// Allocates a block. If there isn't enough memory, this will allocate more
	// memory from the underlying platform. Such allocations occur in chunks. The 
	// memory is not yielded back to the platform automatically. Use shrink() to 
	// explicitly deallocate chunks.
	void* allocate();

	// Deallocates a pointer allocated from this allocator.
	void deallocate(void* _Pointer);

	// Re-initializes all chunks to be as if no allocations have occurred. This 
	// will leave dangling pointers in client code and should be used only when it
	// is known that the memory will not be used anymore. The benefit is this can
	// be faster than individually calling deallocate on each outstanding 
	// allocation.
	void clear();

	// Reserves enough memory to allocate the specified number of elements. 
	// Allocation occurs in chunks, so this may allocate more memory than is 
	// needed. This will not release memory, only ensure there is at least enough
	// to meet the specified needs. This is not. Existing allocations
	// will be preserved but this should not be called when other threads might be 
	// allocating or deallocating memory.
	void reserve(size_type _NumElements);

	// Ensures the specified number of elements can be allocated and will release
	// all chunks not required to meet the specified needs. Only chunks with zero
	// outstanding allocations can be freed, so this will quietly skip any such
	// chunks. This is not. Existing allocations will be preserved but
	// this should not be called when other threads might be allocating or 
	// deallocating memory.
	void shrink(size_type _NumElements);

	// Returns true if the specified pointer was allocated by this allocator.
	bool valid(void* _Pointer) const;

	// Returns the number of blocks per chunk that was specified at construction
	// time.
	size_type num_blocks_per_chunk() const { return NumBlocksPerChunk; }

	// Returns the current number of chunks in use.
	inline size_type num_chunks() const { return Chunks.size(); }

	// Returns true if even one allocation is valid, false if all chunks report
	// full availability. The default behavior of this allocator is to ignore 
	// leaks on its destruction so expensive structures can be abandoned (not
	// cleaned up) in the cases where a reset of the memory doesn't leave dangling
	// resources. For example, some unordered_maps can get quite large, but 
	// contain only bookkeeping data/values, not pointers or handles to resources
	// so why bother deallocating the memory just to kill the arenas anyway when
	// using this allocator? However where cleanup is important assert on this
	// condition at the end of your container's lifetime.
	bool has_outstanding_allocations() const;

	// SLOW! Walks the freelists of all chunks and counts how many entries have
	// not yet been allocated.
	size_type count_available() const;

	inline size_type count_allocated() const { return (num_blocks_per_chunk() * num_chunks()) - count_available(); }

private:
	struct chunk_t
	{
		chunk_t* next;
		fba_t Allocator;
	};

	std::atomic<fba_t*> pLastAlloc;
	std::atomic<fba_t*> pLastDealloc;
	size_type BlockSize;
	size_type NumBlocksPerChunk;
	size_type ChunkSize;
	oCACHE_ALIGNED(concurrent_stack<chunk_t> Chunks);

	allocate_t PlatformAllocate;
	deallocate_t PlatformDeallocate;

	// Allocates and initializes a new chunk. Use this when out of currently 
	// reserved memory.
	chunk_t* allocate_chunk();

	// Maps a pointer back to the chunk allocator it came from
	fba_t* find_chunk_allocator(void* _Pointer) const;

	concurrent_block_allocator(const concurrent_block_allocator&); /* = delete */
	const concurrent_block_allocator& operator=(const concurrent_block_allocator&); /* = delete */

	concurrent_block_allocator(concurrent_block_allocator&&); /* = delete */
	concurrent_block_allocator& operator=(concurrent_block_allocator&&); /* = delete */
};

template<unsigned int S>
class concurrent_block_allocator_s : public concurrent_block_allocator
{
public:
	typedef concurrent_block_allocator::size_type size_type;
	const static size_type block_size = S;

	concurrent_block_allocator_s(size_type _NumBlocksPerChunk = max_blocks_per_chunk
		, const allocate_t& _PlatformAllocate = malloc
		, const deallocate_t& _PlatformDeallocate = free)
		: concurrent_block_allocator(block_size, _NumBlocksPerChunk, _PlatformAllocate, _PlatformDeallocate)
	{}
};

template<typename T>
class concurrent_block_allocator_t : public concurrent_block_allocator_s<sizeof(T)>
{
	typedef concurrent_block_allocator_s<sizeof(T)> base_t;
public:
	typedef base_t::size_type size_type;
	typedef T value_type;
	concurrent_block_allocator_t(size_type _NumBlocksPerChunk = max_blocks_per_chunk
		, const allocate_t& _PlatformAllocate = malloc
		, const deallocate_t& _PlatformDeallocate = free)
		: concurrent_block_allocator_s(_NumBlocksPerChunk, _PlatformAllocate, _PlatformDeallocate)
	{}

	T* allocate() { return static_cast<T*>(concurrent_block_allocator::allocate()); }
	void deallocate(T* _Pointer) { concurrent_block_allocator::deallocate(_Pointer); }

	oALLOCATOR_CONSTRUCT();
	oALLOCATOR_DESTROY();
};

} // namespace ouro

#endif
