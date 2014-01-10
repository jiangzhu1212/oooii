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
// implementation of index_allocator using atomics.
#pragma once
#ifndef oBase_concurrent_index_allocator_h
#define oBase_concurrent_index_allocator_h

#include <oBase/index_allocator_base.h>
#include <atomic>

namespace ouro {

struct concurrent_index_allocator_traits
{
	typedef std::atomic<unsigned int> index_type;

	static const unsigned int index_mask = 0x00ffffff;
};

class concurrent_index_allocator : public index_allocator_base<concurrent_index_allocator_traits>
{
	static const unsigned int tag_bits = 8;
	static const unsigned int tag_one = (1u << 31) >> (tag_bits - 1);
	static const unsigned int tag_mask = ~traits_type::index_mask;
public:
	// It is client code's responsibility to free _pArena after this class has
	// been destroyed.
	concurrent_index_allocator() {}
	concurrent_index_allocator(void* _pArena, size_t _SizeofArena);
	concurrent_index_allocator(concurrent_index_allocator&& _That) { operator=(std::move(_That)); }
	concurrent_index_allocator& operator=(concurrent_index_allocator&& _That)
	{
		index_allocator_base::operator=(std::move((index_allocator_base&&)_That));
		return *this;
	}

	// return an index reserved until it is made available by deallocate
	unsigned int allocate();
	
	// make index available again
	void deallocate(unsigned int _Index);
};

inline concurrent_index_allocator::concurrent_index_allocator(void* _pArena, size_t _SizeofArena) 
	: index_allocator_base(_pArena, _SizeofArena)
{
}

inline unsigned int concurrent_index_allocator::allocate()
{
	unsigned int oldI, newI, allocatedIndex;
	oldI = Freelist;
	do
	{
		allocatedIndex = oldI & traits_type::index_mask;
		if (allocatedIndex == invalid_index)
			return invalid_index;
		newI = (static_cast<unsigned int*>(pArena)[allocatedIndex]) | ((oldI + tag_one) & tag_mask);

	} while (!Freelist.compare_exchange_strong(oldI, newI));

	return allocatedIndex;
}

inline void concurrent_index_allocator::deallocate(unsigned int _Index)
{
	unsigned int oldI, newI;
	oldI = Freelist;
	do
	{
		static_cast<unsigned int*>(pArena)[_Index] = oldI & ~tag_mask;
		newI = _Index | ((oldI + tag_one) & tag_mask);
	} while (!Freelist.compare_exchange_strong(oldI, newI));
}

} // namespace ouro

#endif
