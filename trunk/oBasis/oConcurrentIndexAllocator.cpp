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
#include <oBasis/oConcurrentIndexAllocator.h>
#include <oBasis/oAssert.h>
#include <oBasis/oStdAtomic.h>

oConcurrentIndexAllocator::oConcurrentIndexAllocator() : oIndexAllocatorBase() {}
oConcurrentIndexAllocator::oConcurrentIndexAllocator(void* _pArena, size_t _SizeofArena) : oIndexAllocatorBase(_pArena, _SizeofArena) {}

unsigned int oConcurrentIndexAllocator::Allocate() threadsafe
{
	unsigned int oldI, newI, allocatedIndex;
	do
	{	// Tagging guards against ABA by including a monotonically increasing version counter
		oldI = Freelist;
		allocatedIndex = oldI >> TAG_BITS;
		if (allocatedIndex == TAGGED_INVALIDINDEX)
			return InvalidIndex;
		newI = (static_cast<unsigned int*>(Arena)[allocatedIndex] << TAG_BITS) | (TAG_MASK&(oldI+1));
	} while (!oStd::atomic_compare_exchange(&Freelist, newI, oldI));

	return allocatedIndex;
}

void oConcurrentIndexAllocator::Deallocate(unsigned int _Index) threadsafe
{
	unsigned int oldI, newI;
	do
	{
		oldI = Freelist;
		static_cast<unsigned int*>(Arena)[_Index] = oldI >> TAG_BITS;
		newI = (_Index << TAG_BITS) | (TAG_MASK&(oldI+1));
	} while (!oStd::atomic_compare_exchange(&Freelist, newI, oldI));
}

oConcurrentIndexAllocator::~oConcurrentIndexAllocator()
{
	if (IsValid())
	{
		oASSERT(IsEmpty(), "Index allocator not empty on destruction");
	}
}