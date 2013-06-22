/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oConcurrency/block_allocator.h>
#include <oStd/byte.h>
#include <oStd/oStdAtomic.h>

using namespace oConcurrency;

block_allocator::block_allocator(size_t _BlockSize, size_t _NumBlocksPerChunk, const allocate_t& _Platformallocate, const deallocate_t& _Platformdeallocate)
	: BlockSize(_BlockSize)
	, NumBlocksPerChunk(_NumBlocksPerChunk)
	, ChunkSize(sizeof(chunk_t) + fixed_block_allocator::calc_required_size(_BlockSize, _NumBlocksPerChunk))
	, pLastAlloc(nullptr)
	, pLastDealloc(nullptr)
	, PlatformAllocate(_Platformallocate)
	, PlatformDeallocate(_Platformdeallocate)
{
}

block_allocator::~block_allocator()
{
	shrink(0);
}

void* block_allocator::allocate() threadsafe
{
	void* p = nullptr;

	// This read is a bit racy, but it's only a hint, so use it without concern
	// for its concurrent changing.
	threadsafe fixed_block_allocator* pAllocator = pLastAlloc;
	if (pAllocator)
		p = pAllocator->allocate(BlockSize);

	// search for another chunk
	if (!p)
	{
		// iterating over the list inside the stack is safe because we're only
		// ever going to allow growth of the list, which since it occurs on top
		// of any prior head means the rest of the list remains valid for traversal
		chunk_t* c = Chunks.peek();
		while (c)
		{
			p = c->pAllocator->allocate(BlockSize);
			if (p)
			{
				// this assignment to pLastAlloc is a bit racy, but it's only a 
				// caching/hint, so elsewhere where this is used, just use whatever is 
				// there at the time
				pLastAlloc = c->pAllocator;
				break;
			}

			c = c->pNext;
		}

		// still no memory? add another chunk
		if (!p)
		{
			chunk_t* newHead = allocate_chunk();
			p = newHead->pAllocator->allocate(BlockSize);

			// this assignment to pLastAlloc is a bit racy, but it's only a 
			// caching/hint, so elsewhere where this is used, just use whatever is 
			// there at the time
			Chunks.push(newHead);
			pLastAlloc = newHead->pAllocator;
		}
	}

	return p;
}

void block_allocator::deallocate(void* _Pointer) threadsafe
{
	threadsafe fixed_block_allocator* pAllocator = find_chunk_allocator(_Pointer);
	if (!pAllocator)
		throw std::runtime_error("deallocate called on a dangling pointer from a chunk that has probably been shrink()'ed");

	// this assignment to pLastDealloc is a bit racy, but it's only a 
	// caching/hint, so elsewhere where this is used, just use whatever is there 
	// at the time
	oStd::atomic_exchange(&pLastDealloc, pAllocator);
	pAllocator->deallocate(BlockSize, NumBlocksPerChunk, _Pointer);
}

block_allocator::chunk_t* block_allocator::allocate_chunk() threadsafe
{
	chunk_t* c = reinterpret_cast<chunk_t*>(thread_cast<block_allocator*>(this)->PlatformAllocate(ChunkSize));
	c->pAllocator = ::new (c+1) fixed_block_allocator(BlockSize, NumBlocksPerChunk);
	c->pNext = nullptr;
	return c;
}

threadsafe fixed_block_allocator* block_allocator::find_chunk_allocator(void* _Pointer) const threadsafe
{
	// This read is a bit racy, but it's only a hint, so use it without concern
	// for its concurrent changing.
	threadsafe fixed_block_allocator* pChunkAllocator = pLastDealloc;

	// Check a cached version to avoid a linear lookup
	if (pChunkAllocator && oStd::in_range(_Pointer, (const void*)pChunkAllocator, ChunkSize))
		return pChunkAllocator;

	// There is no threadsafe condition where the Chunks list shrinks, and 
	// because all insertion happens as creating a new head, any sampling of
	// the list will be valid because its topology won't change.
	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (oStd::in_range(_Pointer, c->pAllocator, ChunkSize))
			return c->pAllocator;
		c = c->pNext;
	}

	return nullptr;
}

void block_allocator::clear()
{
	chunk_t* c = Chunks.peek();
	while (c)
	{
		c->pAllocator = ::new (c->pAllocator) fixed_block_allocator(BlockSize, NumBlocksPerChunk);
		c = c->pNext;
	}
}

void block_allocator::reserve(size_t _NumElements)
{
	bool additional = ((_NumElements % NumBlocksPerChunk) == 0) ? 0 : 1;
	size_t NewNumChunks = additional + (_NumElements / NumBlocksPerChunk);

	size_t currentSize = Chunks.size();
	for (size_t i = currentSize; i < NewNumChunks; i++)
	{
		chunk_t* newHead = allocate_chunk();

		// this assignment to pLastAlloc is a bit racy, but it's only a 
		// caching/hint, so elsewhere where this is used, just use whatever is 
		// there at the time
		oStd::atomic_exchange(&pLastAlloc, newHead->pAllocator);
		Chunks.push(newHead);
	}
}

void block_allocator::shrink(size_t _KeepCount)
{
	chunk_t* c = Chunks.pop_all();
	chunk_t* toFree = nullptr;
	while (c)
	{
		chunk_t* tmp = c;
		c = c->pNext;

		if (tmp->pAllocator->count_available(BlockSize) != NumBlocksPerChunk)
			Chunks.push(tmp);
		else
		{
			tmp->pNext = toFree;
			toFree = tmp;
		}
	}

	size_t nChunks = Chunks.size();
	c = toFree;

	while (c && nChunks < _KeepCount)
	{
		chunk_t* tmp = c;
		c = c->pNext;
		Chunks.push(tmp);
		nChunks++;
	}

	while (c)
	{
		chunk_t* tmp = c;
		c = c->pNext;
		PlatformDeallocate(tmp);
	}

	// reset out cached values
	pLastAlloc = pLastDealloc = nullptr;
}

bool block_allocator::valid(void* _Pointer) const
{
	if (!_Pointer)
		return false;

	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (c->pAllocator->valid(BlockSize, NumBlocksPerChunk, _Pointer))
			return true;
		c = c->pNext;
	}
	return false;
}

bool block_allocator::has_outstanding_allocations() const
{
	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (c->pAllocator->count_available(BlockSize) != NumBlocksPerChunk)
			return true;
		c = c->pNext;
	}
	return false;
}

size_t block_allocator::count_available() const
{
	size_t n = 0;
	chunk_t* c = Chunks.peek();
	while (c)
	{
		n += c->pAllocator->count_available(BlockSize);
		c = c->pNext;
	}
	return n;
}
