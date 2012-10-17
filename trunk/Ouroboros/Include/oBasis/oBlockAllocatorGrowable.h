/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// A more generic ready-to-use version of oBlockAllocatorFixed that 
// automatically grows its heap from a system allocator as more memory is 
// needed.

// This uses oBlockAllocatorFixed, so has all the same concurrency and 
// performance features, but has the additional overhead of sometimes having to
// do a linear search through a list of those allocators to find one that has
// memory to allocate; likewise on deallocation. Additionally the size of the 
// reserve chunks can be dynamically altered using the Grow() and Shrink() 
// functions. Be careful! They are not threadsafe and assume the application is
// in a steady state.

#ifndef oBlockAllocatorGrowable_h
#define oBlockAllocatorGrowable_h

#include <oBasis/oAssert.h>
#include <oBasis/oBlockAllocatorFixed.h>
#include <oBasis/oByte.h>
#include <oBasis/oFunction.h>
#include <oBasis/oConcurrentStack.h>
#include <oBasis/oLimits.h>
#include <oBasis/oStdAtomic.h>

class oBlockAllocatorGrowable : oNoncopyable
{
	struct chunk_t
	{
		chunk_t* pNext;
		// In the runtime case (Alloc/Dealloc) this is a threadsafe-specified access
		// from threadsafe methods. However to support growable methods, we must 
		// operate on these sometimes from non-threadsafe methods. So this pointer 
		// is not enforced as threadsafe, but is ok to use in threadsafe methods 
		// because of oConcurrentBlockAllocator implementation guarantees.
		oBlockAllocatorFixed* pAllocator;
	};

public:
	// The specified Allocate and Deallocate functions must be thread safe
	oBlockAllocatorGrowable(size_t _BlockSize, size_t _NumBlocksPerChunk, oFUNCTION<void*(size_t _Size)> _PlatformAllocate = malloc, oFUNCTION<void(void* _Pointer)> _PlatformDeallocate = free);
	~oBlockAllocatorGrowable();

	static const size_t max_num_blocks_per_chunk = oBlockAllocatorFixed::max_num_blocks;

	/*constexpr*/ static size_t GetMaxNumBlocksPerChunk() { return oBlockAllocatorFixed::GetMaxNumBlocks(); }

	size_t GetNumBlocksPerChunk() const threadsafe { return NumBlocksPerChunk; }

	// Allocates a block. If there isn't enough memory, this will call the 
	// specified PlatformAllocate function to allocate more memory from the 
	// underlying platform. Such allocations occur in chunks. The memory is not 
	// yielded back to the platform automatically. Use Shrink() to explicitly 
	// deallocate chunks.
	void* Allocate() threadsafe;

	// Deallocates a pointer allocated from this allocator.
	void Deallocate(void* _Pointer) threadsafe;
	
	// Returns the current number of chunks in use.
	inline size_t GetNumChunks() const threadsafe { return Chunks.size(); }

	// Like STL reserve(), but in units of chunks.
	void Grow(size_t _NumChunks);
	
	// Like STL reserve() in number of elements.
	inline void GrowByElements(size_t _NumElements)
	{
		bool additional = ((_NumElements % NumBlocksPerChunk) == 0) ? 0 : 1;
		Grow(additional + (_NumElements / NumBlocksPerChunk));
	}

	// Returns true if the specified pointer was allocated by this allocator.
	bool IsValid(void* _Pointer) const;

	// Walks through chunks and free any chunk that has no allocations in the wild
	// back to the underlying allocator. This is not threadsafe and it is assumed
	// no allocations or deallocations will occur while this function is running.
	void Shrink(size_t _KeepCount = 0);

	// Returns true if even one allocation is valid, false if all chunks report
	// full availability. The default behavior of this allocator is to ignore 
	// leaks on its destruction so that expensive structures can be abandoned (not
	// cleaned up) in the cases where a reset of the memory doesn't leave dangling
	// resources. For example, some unordered_maps can get quite large, but 
	// contain only bookkeeping data/values, not pointers or handles to resources
	// so why bother deallocating the memory just to kill the arenas anyway when
	// using this allocator? However where cleanup is important oASSERT on this
	// condition at the end of your container's lifetime.
	bool HasOutstandingAllocations() const;

	// Re-initializes all chunks to be as if no allocations have occurred. This x 
	// will leave dangling pointers in client code, and should be used only when
	// it is known that the memory won't be used anymore. The benefit is that this
	// can be faster than individually calling Deallocate on each outstanding 
	// allocation.
	void Reset();

private:
	threadsafe oBlockAllocatorFixed* pLastAlloc;
	threadsafe oBlockAllocatorFixed* pLastDealloc;
	size_t BlockSize;
	size_t NumBlocksPerChunk;
	size_t ChunkSize;
	oConcurrentStack<chunk_t> Chunks;

	oFUNCTION<void*(size_t _Size)> PlatformAllocate;
	oFUNCTION<void(void* _Pointer)> PlatformDeallocate;

	// Allocates and initializes a new chunk. Use this when out of currently 
	// reserved memory.
	chunk_t* AllocateChunk() threadsafe;

	// Maps a pointer back to the chunk allocator it came from
	threadsafe oBlockAllocatorFixed* FindChunkAllocator(void* _Pointer) const threadsafe;
};

template<size_t S> class oBlockAllocatorGrowableS : public oBlockAllocatorGrowable
{
public:
	const static size_t block_size = S;

	oBlockAllocatorGrowableS(size_t _NumBlocksPerChunk = max_num_blocks_per_chunk, oFUNCTION<void*(size_t _Size)> _PlatformAllocate = malloc, oFUNCTION<void(void* _Pointer)> _PlatformDeallocate = free)
		: oBlockAllocatorGrowable(block_size, _NumBlocksPerChunk, _PlatformAllocate, _PlatformDeallocate)
	{}
};

template<typename T>
class oBlockAllocatorGrowableT : public oBlockAllocatorGrowableS<sizeof(T)>
{
public:
	typedef T value_type;
	oBlockAllocatorGrowableT(size_t _NumBlocksPerChunk = max_num_blocks_per_chunk, oFUNCTION<void*(size_t _Size)> _PlatformAllocate = malloc, oFUNCTION<void(void* _Pointer)> _PlatformDeallocate = free)
		: oBlockAllocatorGrowableS(_NumBlocksPerChunk, _PlatformAllocate, _PlatformDeallocate)
	{}

	T* Allocate() threadsafe { return static_cast<T*>(oBlockAllocatorGrowable::Allocate()); }
	void Deallocate(T* _Pointer) threadsafe { oBlockAllocatorGrowable::Deallocate(_Pointer); }

	oALLOCATOR_CREATE();
	oALLOCATOR_DESTROY();
};

#endif
