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
#include <oBasis/oBlockAllocatorGrowable.h>

oBlockAllocatorGrowable::oBlockAllocatorGrowable(size_t _BlockSize, size_t _NumBlocksPerChunk, oFUNCTION<void*(size_t _Size)> _PlatformAllocate, oFUNCTION<void(void* _Pointer)> _PlatformDeallocate)
	: BlockSize(_BlockSize)
	, NumBlocksPerChunk(_NumBlocksPerChunk)
	, ChunkSize(sizeof(chunk_t) + oBlockAllocatorFixed::CalculateRequiredSize(_BlockSize, _NumBlocksPerChunk))
	, pLastAlloc(nullptr)
	, pLastDealloc(nullptr)
	, PlatformAllocate(_PlatformAllocate)
	, PlatformDeallocate(_PlatformDeallocate)
{
}

oBlockAllocatorGrowable::~oBlockAllocatorGrowable()
{
	Shrink();
}

void* oBlockAllocatorGrowable::Allocate() threadsafe
{
	void* p = nullptr;

	// This read is a bit racy, but it's only a hint, so use it without concern
	// for its concurrent changing.
	threadsafe oBlockAllocatorFixed* pAllocator = pLastAlloc;
	if (pAllocator)
		p = pAllocator->Allocate(BlockSize);

	// search for another chunk
	if (!p)
	{
		// iterating over the list inside the stack is safe because we're only
		// ever going to allow growth of the list, which since it occurs on top
		// of any prior head means the rest of the list remains valid for traversal
		chunk_t* c = Chunks.peek();
		while (c)
		{
			p = c->pAllocator->Allocate(BlockSize);
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
			chunk_t* newHead = AllocateChunk();
			p = newHead->pAllocator->Allocate(BlockSize);

			// this assignment to pLastAlloc is a bit racy, but it's only a 
			// caching/hint, so elsewhere where this is used, just use whatever is 
			// there at the time
			Chunks.push(newHead);
			pLastAlloc = newHead->pAllocator;
		}
	}

	return p;
}

void oBlockAllocatorGrowable::Deallocate(void* _Pointer) threadsafe
{
	threadsafe oBlockAllocatorFixed* pAllocator = FindChunkAllocator(_Pointer);
	oASSERT(pAllocator, "dangling pointer from a chunk that has probably been Shrink()'ed");
	// this assignment to pLastDealloc is a bit racy, but it's only a 
	// caching/hint, so elsewhere where this is used, just use whatever is there 
	// at the time
	oStd::atomic_exchange(&pLastDealloc, pAllocator);
	pAllocator->Deallocate(BlockSize, NumBlocksPerChunk, _Pointer);
}

oBlockAllocatorGrowable::chunk_t* oBlockAllocatorGrowable::AllocateChunk() threadsafe
{
	chunk_t* c = reinterpret_cast<chunk_t*>(thread_cast<oBlockAllocatorGrowable*>(this)->PlatformAllocate(ChunkSize));
	c->pAllocator = reinterpret_cast<oBlockAllocatorFixed*>(c+1);
	c->pAllocator->Initialize(BlockSize, NumBlocksPerChunk);
	c->pNext = nullptr;
	return c;
}

threadsafe oBlockAllocatorFixed* oBlockAllocatorGrowable::FindChunkAllocator(void* _Pointer) const threadsafe
{
	// This read is a bit racy, but it's only a hint, so use it without concern
	// for its concurrent changing.
	threadsafe oBlockAllocatorFixed* pChunkAllocator = pLastDealloc;

	// Check a cached version to avoid a linear lookup
	if (pChunkAllocator && oInRange(_Pointer, (const void*)pChunkAllocator, ChunkSize))
		return pChunkAllocator;

	// There is no threadsafe condition where the Chunks list shrinks, and 
	// because all insertion happens as creating a new head, any sampling of
	// the list will be valid because its topology won't change.
	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (oInRange(_Pointer, c->pAllocator, ChunkSize))
			return c->pAllocator;
		c = c->pNext;
	}

	return nullptr;
}

void oBlockAllocatorGrowable::Grow(size_t _NumChunks)
{
	size_t currentSize = Chunks.size();
	for (size_t i = currentSize; i < _NumChunks; i++)
	{
		chunk_t* newHead = AllocateChunk();

		// this assignment to pLastAlloc is a bit racy, but it's only a 
		// caching/hint, so elsewhere where this is used, just use whatever is 
		// there at the time
		oStd::atomic_exchange(&pLastAlloc, newHead->pAllocator);
		Chunks.push(newHead);
	}
}

bool oBlockAllocatorGrowable::IsValid(void* _Pointer) const
{
	if (!_Pointer)
		return false;

	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (c->pAllocator->IsValid(BlockSize, NumBlocksPerChunk, _Pointer))
			return true;
		c = c->pNext;
	}
	return false;
}

void oBlockAllocatorGrowable::Shrink(size_t _KeepCount)
{
	chunk_t* c = Chunks.pop_all();
	chunk_t* toFree = nullptr;
	while (c)
	{
		chunk_t* tmp = c;
		c = c->pNext;

		if (tmp->pAllocator->CountAvailable(BlockSize) != NumBlocksPerChunk)
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

bool oBlockAllocatorGrowable::HasOutstandingAllocations() const
{
	chunk_t* c = Chunks.peek();
	while (c)
	{
		if (c->pAllocator->CountAvailable(BlockSize) != NumBlocksPerChunk)
			return true;
		c = c->pNext;
	}
	return false;
}

void oBlockAllocatorGrowable::Reset()
{
	chunk_t* c = Chunks.peek();
	while (c)
	{
		c->pAllocator->Initialize(BlockSize, NumBlocksPerChunk);
		c = c->pNext;
	}
}
