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
#include <oBasis/oBlockAllocatorFixed.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oBasis/oLimits.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oStdAtomic.h>

// helpers
typedef unsigned short index_type;
static /*constexpr*/ size_t index_mask() { return oNumericLimits<index_type>::GetMax(); }
static /*constexpr*/ index_type invalid_index() { return static_cast<index_type>(index_mask()); }

static inline index_type* begin(const threadsafe oBlockAllocatorFixed* _pThis) { return (index_type*)oByteAlign(_pThis+1, oDEFAULT_MEMORY_ALIGNMENT); }
static inline index_type* at(const threadsafe oBlockAllocatorFixed* _pThis, size_t _BlockSize, size_t _Index) { return oByteAdd(begin(_pThis), _BlockSize, _Index); }

size_t oBlockAllocatorFixed::CalculateRequiredSize(size_t _BlockSize, size_t _NumBlocks)
{
	return (_BlockSize * _NumBlocks) + oByteAlign(sizeof(oBlockAllocatorFixed), oDEFAULT_MEMORY_ALIGNMENT);
}

bool oBlockAllocatorFixed::IsValid(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) const threadsafe
{
	const index_type* pBegin = begin(this);
	ptrdiff_t diff = oByteDiff(_Pointer, pBegin);
	return oInRange(_Pointer, pBegin, _NumBlocks * _BlockSize) && oIsByteAligned(_Pointer, sizeof(void*)) && ((diff % _BlockSize) == 0);
}

void oBlockAllocatorFixed::Initialize(size_t _BlockSize, size_t _NumBlocks)
{
	// Point to the first block and then in each block of uninit'ed memory store
	// an index to the next free block, like a linked list. At init time, this
	// means store an index to the very next block.
	NextAvailable.All = 0;
	oASSERT(_BlockSize >= sizeof(index_type), "block size too small, must be at least %d bytes", sizeof(index_type));
	oASSERT(_NumBlocks < invalid_index(), "too many blocks");
	index_type i = 0;
	index_type* p = begin(this);
	for (; i < (_NumBlocks-1); i++)
		*at(this, _BlockSize, i) = i+1;
	*at(this, _BlockSize, i) = invalid_index();
}

void* oBlockAllocatorFixed::Allocate(size_t _BlockSize) threadsafe
{
	index_type* pFreeIndex = nullptr;
	tagged_index_t New, Old;
	do
	{
		Old.All = NextAvailable.All;
		if (Old.Index == invalid_index())
			return nullptr;
		pFreeIndex = at(this, _BlockSize, Old.Index);
		New.Index = *pFreeIndex;
		New.Tag = Old.Tag + 1;
	} while (!oStd::atomic_compare_exchange(&NextAvailable.All, New.All, Old.All));
	*pFreeIndex = static_cast<index_type>(Old.All); // it can be useful to have this for index allocators, so assign the index of this newly allocated pointer
	return reinterpret_cast<void*>(pFreeIndex);
}

void oBlockAllocatorFixed::Deallocate(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) threadsafe
{
	oASSERT(IsValid(_BlockSize, _NumBlocks, _Pointer), "invalid pointer");
	tagged_index_t New, Old;
	do
	{
		Old.All = NextAvailable.All;
		*reinterpret_cast<index_type*>(_Pointer) = static_cast<index_type>(Old.Index & index_mask());
		New.Index = oIndexOf(_Pointer, begin(this), _BlockSize);
		New.Tag = Old.Tag + 1;
	} while (!oStd::atomic_compare_exchange(&NextAvailable.All, New.All, Old.All));
}

size_t oBlockAllocatorFixed::CountAvailable(size_t _BlockSize) const
{
	size_t n = 0;
	index_type i = static_cast<index_type>(NextAvailable.Index);
	while (i != invalid_index())
	{
		n++;
		i = *at(this, _BlockSize, i);
	}
	return n;
}
