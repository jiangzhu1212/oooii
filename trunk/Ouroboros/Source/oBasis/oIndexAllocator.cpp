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
#include "oIndexAllocatorInternal.h"
#include <oBasis/oAssert.h>

// for the threadlocal version this limits the implementation, but that's
// good: both the concurrent and local should behave the same
const unsigned int oIndexAllocatorBase::InvalidIndex = TAGGED_INVALIDINDEX;

oIndexAllocatorBase::oIndexAllocatorBase()
	: Arena(0)
	, ArenaBytes(0)
	, Freelist(TAGGED_INVALIDINDEX)
{
}

oIndexAllocatorBase::oIndexAllocatorBase(void* _pArena, size_t _SizeofArena)
	: Arena(0)
	, ArenaBytes(0)
	, Freelist(InvalidIndex)
{
	Initialize(_pArena, _SizeofArena);
}

oIndexAllocator::oIndexAllocator() : oIndexAllocatorBase() {}
oIndexAllocator::oIndexAllocator(void* _pArena, size_t _SizeofArena) : oIndexAllocatorBase(_pArena, _SizeofArena) {}

oIndexAllocatorBase::~oIndexAllocatorBase()
{
	if (IsValid())
	{
		// @oooii-kevin: Turning off trace as this spams the system too much
		//oTRACE("Destroying index allocator implicitly: the user-defined arena may not be freed");
		Deinitialize();
	}
}

size_t oIndexAllocatorBase::GetCapacity() const threadsafe
{
	return ArenaBytes / SizeOfIndex;
}

void oIndexAllocatorBase::InternalReset()
{
	// Seed list with next free index (like a next pointer in an slist)
	unsigned int* indices = static_cast<unsigned int*>(Arena);
	const size_t cap = GetCapacity();
	for (unsigned int i = 0; i < cap; i++)
		indices[i] = i+1;
	indices[cap-1] = InvalidIndex; // last node has no next
	Freelist = 0;
}

void oIndexAllocatorBase::Reset()
{
	oASSERT(IsValid(), "Index allocator not valid on call to reset");
	InternalReset();
}

void oIndexAllocatorBase::Initialize(void* _pArena, size_t arenaBytes)
{
	oASSERT(!IsValid(), "Index allocator already initialized");
	Arena = _pArena;
	ArenaBytes = arenaBytes;
	oASSERT(GetCapacity() <= TAGGED_MAXINDEX, "A capacity of %u is too large because %u bits are reserved for thread-safety. Max capacity is current %u", GetCapacity(), TAG_BITS, TAGGED_MAXINDEX);
	InternalReset();
}
	
void* oIndexAllocatorBase::Deinitialize()
{
	oASSERT(IsValid(), "Index allocator already deinitialized");
	if (!IsEmpty()) oTRACE("Index allocator has outstanding allocations");
	Freelist = InvalidIndex;
	void* p = Arena;
	Arena = 0;
	ArenaBytes = 0;
	return p;
}

size_t oIndexAllocatorBase::GetSize() const
{
	// Calculate the number free since the freelist is available
	// and return the difference between that the capacity. The
	// decision here is to calculate this value on demand since 
	// GetSize() and IsEmpty() are not a critical-path functions, 
	// and keeping a count on the number allocated would incur 
	// another atomic operation in allocate/deallocate, so don't 
	// do that just for debugging.
	size_t nFree = 0;
	if (GetCapacity())
	{
		unsigned int n = Freelist >> TAG_BITS;
		while (n != TAGGED_INVALIDINDEX)
		{
			nFree++;
			oASSERT(nFree <= GetCapacity(), "Num free is more than the capacity of %u", GetCapacity());
			oASSERT(n < GetCapacity(), "while following the freelist, an index is present that is greater than capacity %u", GetCapacity());
			n = static_cast<unsigned int*>(Arena)[n];
		}
	}
	return GetCapacity() - nFree;
}

size_t oIndexAllocator::GetSize() const
{
	size_t nFree = 0;
	unsigned int n = Freelist;
	while (n != InvalidIndex)
	{
		nFree++;
		oASSERT(nFree <= GetCapacity(), "Num free is more than the capacity of %u", GetCapacity());
		oASSERT(n < GetCapacity(), "while following the freelist, an index is present that is greater than capacity %u", GetCapacity());
		n = static_cast<unsigned int*>(Arena)[n];
	}

	return GetCapacity() - nFree;
}

oIndexAllocator::~oIndexAllocator()
{
	if (IsValid())
	{
		oASSERT(IsEmpty(), "Index allocator not empty on destruction");
	}
}