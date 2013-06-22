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
// Simple allocator that has a fixed arena of memory and increments an index 
// with each allocation until all memory is used up. There is no Deallocate(), 
// but Reset() will set the index back to zero. This is useful for containers
// such as std::maps that are built up and searched and whose data is simple
// (i.e. no ref counting or handles/pointers that need to be cleaned up). Use of 
// this allocator can alleviate long destructor times in such containers where
// there's not a lot of use to the destruction because of the simple types.
//
// This uses the oversized-allocation pattern where the full arena is allocated
// and this class is overlaid on top of it to manage the buffer.
//
// Allocation is O(1) and a single CAS is used to enable concurrency.

#pragma once
#ifndef oLinearAllocator_h
#define oLinearAllocator_h

#include <oBasis/oPlatformFeatures.h>
#include <oConcurrency/concurrent_linear_allocator.h>
#include <oConcurrency/thread_safe.h>
#include <oStd/macros.h>

template<typename T> struct oStdLinearAllocator
{
	// Use an initial buffer or when that is exhausted fall back to system malloc.
	// Deallocate noops on the first allocations, but will free memory allocated
	// by the system heaps. This also keeps a pointer that records how many bytes
	// were allocated from platform Malloc so that the arena size can be adjusted.

	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oStdLinearAllocator)
	oStdLinearAllocator(void* _Arena
		, size_t _SizeofArena
		, size_t* _pPlatformBytesAllocated = nullptr
		, void* (*_PlatformMalloc)(size_t _Size) = malloc
		, void (*_PlatformFree)(void* _Pointer) = free)
		: pAllocator(reinterpret_cast<oConcurrency::concurrent_linear_allocator*>(_Arena))
		, PlatformMalloc(_PlatformMalloc)
		, PlatformFree(_PlatformFree)
		, pPlatformBytesAllocated(_pPlatformBytesAllocated)
	{
		pAllocator->initialize(_SizeofArena);
		if (pPlatformBytesAllocated)
			*pPlatformBytesAllocated = 0;
	}

	~oStdLinearAllocator()
	{
	}
	
	template<typename U> oStdLinearAllocator(oStdLinearAllocator<U> const& _That)
		: pAllocator(_That.pAllocator)
		, PlatformMalloc(_That.PlatformMalloc)
		, PlatformFree(_That.PlatformFree)
		, pPlatformBytesAllocated(_That.pPlatformBytesAllocated)
	{}
	
	inline pointer allocate(size_type count, const_pointer hint = 0)
	{
		const size_t nBytes = sizeof(T) * count;

		void* p = pAllocator->allocate(nBytes);
		if (!p)
		{
			p = PlatformMalloc(nBytes);
			if (p && pPlatformBytesAllocated)
				*pPlatformBytesAllocated += nBytes;
		}
		return static_cast<pointer>(p);
	}
	
	inline void deallocate(pointer p, size_type count)
	{
		if (!pAllocator->valid(p))
			PlatformFree(p);
	}
	
	inline void Reset() { pAllocator->Reset(); }
	inline const oStdLinearAllocator& operator=(const oStdLinearAllocator& _That)
	{
		pAllocator = _That.pAllocator;
		PlatformMalloc = _That.PlatformMalloc;
		PlatformFree = _That.PlatformFree;
		pPlatformBytesAllocated = _That.pPlatformBytesAllocated;
		return *this;
	}
	
	oConcurrency::concurrent_linear_allocator* pAllocator;
	void* (*PlatformMalloc)(size_t _Size);
	void (*PlatformFree)(void* _Pointer);
	size_t* pPlatformBytesAllocated;
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oStdLinearAllocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oStdLinearAllocator) { return a.pAllocator == b.pAllocator; }

#endif
