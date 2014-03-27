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

#ifndef oBase_concurrent_growable_object_pool_h
#define oBase_concurrent_growable_object_pool_h

#include <oBase/allocate.h>
#include <oBase/concurrent_stack.h>
#include <oBase/pool.h>
#include <cstdlib>

namespace ouro {

template<typename T>
class concurrent_growable_object_pool
{
	typedef concurrent_block_pool<sizeof(T)> chunk_alloc_t;

public:
	typedef typename chunk_alloc_t::size_type size_type;
	static const size_type max_blocks_per_chunk = chunk_alloc_t::max_blocks;

	// The specified allocate and deallocate functions must be thread safe
	concurrent_growable_object_pool(size_type _NumBlocksPerChunk = max_blocks_per_chunk, const allocator& _Allocator = default_allocator);

	~concurrent_growable_object_pool();

	oPOOL_CREATE();
	oPOOL_DESTROY();

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
	bool valid(T* _Pointer) const;

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
		chunk_alloc_t allocator;
		chunk_t* next;
	};

	std::atomic<chunk_alloc_t*> pLastAlloc;
	std::atomic<chunk_alloc_t*> pLastDealloc;
	size_type NumBlocksPerChunk;
	size_type ChunkSize;
	oCACHE_ALIGNED(concurrent_stack<chunk_t> Chunks);

	allocator Allocator;

	// Allocates and initializes a new chunk. Use this when out of currently 
	// reserved memory.
	chunk_t* allocate_chunk();

	// Maps a pointer back to the chunk allocator it came from
	chunk_alloc_t* find_chunk_allocator(void* _Pointer) const;

	concurrent_growable_object_pool(const concurrent_growable_object_pool&); /* = delete */
	const concurrent_growable_object_pool& operator=(const concurrent_growable_object_pool&); /* = delete */

	concurrent_growable_object_pool(concurrent_growable_object_pool&&); /* = delete */
	concurrent_growable_object_pool& operator=(concurrent_growable_object_pool&&); /* = delete */
};

template<typename T>
concurrent_growable_object_pool<T>::concurrent_growable_object_pool(size_type _NumBlocksPerChunk, const allocator& _Allocator)
	: NumBlocksPerChunk(_NumBlocksPerChunk)
	, ChunkSize(static_cast<size_type>(byte_align(sizeof(chunk_t) + sizeof(T) * _NumBlocksPerChunk, oDEFAULT_MEMORY_ALIGNMENT)))
	, pLastAlloc(nullptr)
	, pLastDealloc(nullptr)
	, Allocator(_Allocator)
{}

template<typename T>
concurrent_growable_object_pool<T>::~concurrent_growable_object_pool()
{
	shrink(0);
}

template<typename T>
void concurrent_growable_object_pool<T>::clear()
{
	chunk_t* c = Chunks.peek();
	while (c)
	{
		c->allocator.clear();
		c = c->next;
	}
}

template<typename T>
void concurrent_growable_object_pool<T>::reserve(size_type _NumElements)
{
	bool additional = ((_NumElements % NumBlocksPerChunk) == 0) ? 0 : 1;
	size_type NewNumChunks = additional + (_NumElements / NumBlocksPerChunk);

	size_type currentSize = Chunks.size();
	for (size_type i = currentSize; i < NewNumChunks; i++)
	{
		chunk_t* c = allocate_chunk();
		pLastAlloc = &c->allocator; // racy but only an optimization hint
		Chunks.push(c);
	}
}

template<typename T>
void concurrent_growable_object_pool<T>::shrink(size_type _KeepCount)
{
	chunk_t* c = Chunks.pop_all();
	chunk_t* toFree = nullptr;
	while (c)
	{
		chunk_t* tmp = c;
		c = c->next;

		if (!tmp->allocator.empty())
			Chunks.push(tmp);
		else
		{
			tmp->next = toFree;
			toFree = tmp;
		}
	}

	size_type nChunks = Chunks.size();
	c = toFree;

	while (c && nChunks < _KeepCount)
	{
		chunk_t* tmp = c;
		c = c->next;
		Chunks.push(tmp);
		nChunks++;
	}

	while (c)
	{
		chunk_t* tmp = c;
		c = c->next;
		Allocator.deallocate(tmp);
	}

	// reset out cached values
	pLastAlloc = pLastDealloc = nullptr;
}

template<typename T>
bool concurrent_growable_object_pool<T>::valid(T* _Pointer) const
{
	if (_Pointer)
	{
		chunk_t* c = Chunks.peek();
		while (c)
		{
			if (c->Allocator.valid(_Pointer))
				return true;
			c = c->next;
		}
	}
	return false;
}

template<typename T>
bool concurrent_growable_object_pool<T>::has_outstanding_allocations() const
{
	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (!c->allocator.empty())
			return true;
		c = c->next;
	}
	return false;
}

template<typename T>
typename concurrent_growable_object_pool<T>::size_type concurrent_growable_object_pool<T>::count_available() const
{
	size_type n = 0;
	chunk_t* c = Chunks.peek();
	while (c)
	{
		n += c->allocator.count_available();
		c = c->next;
	}
	return n;
}

template<typename T>
typename concurrent_growable_object_pool<T>::chunk_t* concurrent_growable_object_pool<T>::allocate_chunk()
{
	chunk_t* c = reinterpret_cast<chunk_t*>(Allocator.allocate(ChunkSize, memory_alignment::align_to_cache_line));
	c->allocator = std::move(chunk_alloc_t(byte_align(c+1, oDEFAULT_MEMORY_ALIGNMENT), NumBlocksPerChunk));
	c->next = nullptr;
	return c;
}

template<typename T>
void* concurrent_growable_object_pool<T>::allocate()
{
	void* p = nullptr;

	// check cached last-used chunk (racy but it's only an optimization hint)
	chunk_alloc_t* pAllocator = pLastAlloc;
	if (pAllocator)
		p = pAllocator->allocate();

	// search for another chunk
	if (!p)
	{
		// iterating over the list inside the stack is safe because we're only
		// ever going to allow growth of the list which since it occurs on top
		// of any prior head means the rest of the list remains valid for traversal
		chunk_t* c = Chunks.peek();
		while (c)
		{
			p = c->allocator.allocate();
			if (p)
			{
				pLastAlloc = &c->allocator; // racy but only an optimization hint
				break;
			}

			c = c->next;
		}

		// still no memory? add another chunk
		if (!p)
		{
			chunk_t* newHead = allocate_chunk();
			p = newHead->allocator.allocate();

			// this assignment to pLastAlloc is a bit racy, but it's only a 
			// caching/hint, so elsewhere where this is used, just use whatever is 
			// there at the time
			Chunks.push(newHead);
			pLastAlloc = &newHead->allocator;
		}
	}

	return p;
}

template<typename T>
void concurrent_growable_object_pool<T>::deallocate(void* _Pointer)
{
	chunk_alloc_t* pAllocator = find_chunk_allocator(_Pointer);
	if (!pAllocator)
		throw std::runtime_error("deallocate called on a dangling pointer from a chunk that has probably been shrink()'ed");
	pLastDealloc = pAllocator; // racy but only an optimization hint
	pAllocator->deallocate(_Pointer);
}

template<typename T>
typename concurrent_growable_object_pool<T>::chunk_alloc_t* concurrent_growable_object_pool<T>::find_chunk_allocator(void* _Pointer) const
{
	chunk_alloc_t* pChunkAllocator = pLastDealloc; // racy but only an optimization hint

	// Check the cached version to avoid a linear lookup
	if (pChunkAllocator && pChunkAllocator->valid(_Pointer))
		return pChunkAllocator;

	// iterating over the list inside the stack is safe because we're only
	// ever going to allow growth of the list which since it occurs on top
	// of any prior head means the rest of the list remains valid for traversal
	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (c->allocator.valid(_Pointer))
			return &c->allocator;
		c = c->next;
	}

	return nullptr;
}

} // namespace ouro

#endif
