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
// A more generic ready-to-use version of fixed_block_allocator that grows its
// heap from a system allocator as more memory is needed.

// This uses fixed_block_allocator, so it has all the same concurrency and 
// performance features but has the additional overhead of sometimes having to
// do a linear search through a list of those allocators to find one that has
// memory to allocate; likewise on deallocation. Additionally the size of the 
// reserve chunks can be dynamically altered using the grow() and shrink() 
// functions. Be careful! They are not and assume the application is
// in a steady state.

#ifndef oConcurrency_block_allocator_h
#define oConcurrency_block_allocator_h

#include <oConcurrency/concurrent_stack.h>
#include <oConcurrency/fixed_block_allocator.h>
#include <atomic>
#include <cstdlib>

namespace oConcurrency {

class block_allocator
{
public:
	typedef std::function<void*(size_t _Size)> allocate_t;
	typedef std::function<void(void* _Pointer)> deallocate_t;

	static const size_t max_blocks_per_chunk = fixed_block_allocator::max_num_blocks;

	// The specified allocate and deallocate functions must be thread safe
	block_allocator(size_t _BlockSize
		, size_t _NumBlocksPerChunk = max_blocks_per_chunk
		, const allocate_t& _PlatformAllocate = malloc
		, const deallocate_t& _PlatformDeallocate = free);

	~block_allocator();

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
	void reserve(size_t _NumElements);

	// Ensures the specified number of elements can be allocated and will release
	// all chunks not required to meet the specified needs. Only chunks with zero
	// outstanding allocations can be freed, so this will quietly skip any such
	// chunks. This is not. Existing allocations will be preserved but
	// this should not be called when other threads might be allocating or 
	// deallocating memory.
	void shrink(size_t _NumElements);

	// Returns true if the specified pointer was allocated by this allocator.
	bool valid(void* _Pointer) const;

	// Returns the number of blocks per chunk that was specified at construction
	// time.
	size_t num_blocks_per_chunk() const { return NumBlocksPerChunk; }

	// Returns the current number of chunks in use.
	inline size_t num_chunks() const { return Chunks.size(); }

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
	size_t count_available() const;

	inline size_t count_allocated() const { return (num_blocks_per_chunk() * num_chunks()) - count_available(); }

private:
	struct chunk_t
	{
		chunk_t* pNext;
		// In the runtime case (Alloc/Dealloc) this is a-specified access
		// from methods. However to support growable methods, we must 
		// operate on these sometimes from non-threadsafe methods. So this pointer 
		// is not enforced as, but is ok to use in methods 
		// because of oConcurrentBlockAllocator implementation guarantees.
		fixed_block_allocator* pAllocator;
	};

	std::atomic<fixed_block_allocator*> pLastAlloc;
	std::atomic<fixed_block_allocator*> pLastDealloc;
	size_t BlockSize;
	size_t NumBlocksPerChunk;
	size_t ChunkSize;
	oCACHE_ALIGNED(concurrent_stack<chunk_t> Chunks);

	allocate_t PlatformAllocate;
	deallocate_t PlatformDeallocate;

	// Allocates and initializes a new chunk. Use this when out of currently 
	// reserved memory.
	chunk_t* allocate_chunk();

	// Maps a pointer back to the chunk allocator it came from
	fixed_block_allocator* find_chunk_allocator(void* _Pointer) const;

	block_allocator(const block_allocator&); /* = delete */
	const block_allocator& operator=(const block_allocator&); /* = delete */

	block_allocator(block_allocator&&); /* = delete */
	block_allocator& operator=(block_allocator&&); /* = delete */
};

template<size_t S>
class block_allocator_s : public block_allocator
{
public:
	const static size_t block_size = S;

	block_allocator_s(size_t _NumBlocksPerChunk = max_blocks_per_chunk
		, const allocate_t& _PlatformAllocate = malloc
		, const deallocate_t& _PlatformDeallocate = free)
		: block_allocator(block_size, _NumBlocksPerChunk, _PlatformAllocate, _PlatformDeallocate)
	{}
};

template<typename T>
class block_allocator_t : public block_allocator_s<sizeof(T)>
{
public:
	typedef T value_type;
	block_allocator_t(size_t _NumBlocksPerChunk = max_blocks_per_chunk
		, const allocate_t& _PlatformAllocate = malloc
		, const deallocate_t& _PlatformDeallocate = free)
		: block_allocator_s(_NumBlocksPerChunk, _PlatformAllocate, _PlatformDeallocate)
	{}

	T* allocate() { return static_cast<T*>(block_allocator::allocate()); }
	void deallocate(T* _Pointer) { block_allocator::deallocate(_Pointer); }

	oALLOCATOR_CONSTRUCT();
	oALLOCATOR_DESTROY();
};

} // namespace oConcurrency

#endif
