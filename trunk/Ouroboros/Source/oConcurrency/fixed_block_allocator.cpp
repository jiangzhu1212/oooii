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
#include <oConcurrency/fixed_block_allocator.h>
#include <oStd/byte.h>
#include <oStd/macros.h>
#include <oStd/atomic.h>
#include <oStd/config.h>

using namespace oConcurrency;

// helpers
typedef unsigned short index_type;
#define INDEX_TYPE_SIZE 2
static /*constexpr*/ size_t index_mask() { return std::numeric_limits<index_type>::max(); }
static /*constexpr*/ index_type invalid_index() { return static_cast<index_type>(index_mask()); }

static inline index_type* begin(const threadsafe fixed_block_allocator* _pThis) { return (index_type*)oStd::byte_align(_pThis+1, fixed_block_allocator::default_alignment); }
static inline index_type* at(const threadsafe fixed_block_allocator* _pThis, size_t _BlockSize, size_t _Index) { return oStd::byte_add(begin(_pThis), _BlockSize, _Index); }

size_t fixed_block_allocator::calc_required_size(size_t _BlockSize, size_t _NumBlocks)
{
	return (_BlockSize * _NumBlocks) + oStd::byte_align(sizeof(fixed_block_allocator), default_alignment);
}

bool fixed_block_allocator::valid(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) const threadsafe
{
	const index_type* pBegin = begin(this);
	ptrdiff_t diff = oStd::byte_diff(_Pointer, pBegin);
	return oStd::in_range(_Pointer, pBegin, _NumBlocks * _BlockSize) && oStd::byte_aligned(_Pointer, sizeof(void*)) && ((diff % _BlockSize) == 0);
}

fixed_block_allocator::fixed_block_allocator(size_t _BlockSize, size_t _NumBlocks)
{
	// Point to the first block and then in each block of uninit'ed memory store
	// an index to the next free block, like a linked list. At init time, this
	// means store an index to the very next block.
	NextAvailable.All = 0;
	if (_BlockSize < sizeof(index_type))
		throw std::invalid_argument("block size too small, must be at least " oSTRINGIZE(INDEX_TYPE_SIZE) " bytes");

	if (_NumBlocks >= invalid_index())
		throw std::invalid_argument("too many blocks specified");

	index_type i = 0;
	index_type* p = begin(this);
	for (; i < (_NumBlocks-1); i++)
		*at(this, _BlockSize, i) = i+1;
	*at(this, _BlockSize, i) = invalid_index();
}

void* fixed_block_allocator::allocate(size_t _BlockSize) threadsafe
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
	//oTRACE("alloc %p index %u (then %u)", pFreeIndex, Old.Index, New.Index);
	return reinterpret_cast<void*>(pFreeIndex);
}

void fixed_block_allocator::deallocate(size_t _BlockSize, size_t _NumBlocks, void* _Pointer) threadsafe
{
	if (!valid(_BlockSize, _NumBlocks, _Pointer))
		throw std::out_of_range("the specified pointer was not allocated from this allocator");
	tagged_index_t New, Old;
	do
	{
		Old.All = NextAvailable.All;
		*reinterpret_cast<index_type*>(_Pointer) = static_cast<index_type>(Old.Index & index_mask());
		New.Index = oStd::index_of(_Pointer, begin(this), _BlockSize);
		New.Tag = Old.Tag + 1;
	} while (!oStd::atomic_compare_exchange(&NextAvailable.All, New.All, Old.All));
	//oTRACE("dealloc %p index %u (then %u)", _Pointer, New.Index, *reinterpret_cast<index_type*>(_Pointer));
}

size_t fixed_block_allocator::count_available(size_t _BlockSize) const
{
	size_t n = 0;
	index_type i = static_cast<index_type>(NextAvailable.Index);
	while (i != invalid_index())
	{
		n++;
		if (n > max_num_blocks)
			throw std::length_error("count of available blocks has exceeded the maximum allowable");

		i = *at(this, _BlockSize, i);
	}
	return n;
}
