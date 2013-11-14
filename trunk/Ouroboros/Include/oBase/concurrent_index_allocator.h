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
#include <oStd/atomic.h>

namespace ouro {

class concurrent_index_allocator : public index_allocator_base
{
	static const unsigned int tag_one = (1u << 31) >> (tag_bits - 1);
public:
	// It is client code's responsibility to free _pArena after this class has
	// been destroyed.
	concurrent_index_allocator(void* _pArena, size_t _SizeofArena);

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
	do
	{
		oldI = Freelist;
		allocatedIndex = oldI & ~tag_mask;
		if (allocatedIndex == tagged_invalid_index)
			return invalid_index;
		newI = (static_cast<unsigned int*>(Arena)[allocatedIndex]) | ((oldI + tag_one) & tag_mask);
	} while (!oStd::atomic_compare_exchange(&Freelist, newI, oldI));

	return allocatedIndex;
}

inline void concurrent_index_allocator::deallocate(unsigned int _Index)
{
	unsigned int oldI, newI;
	do
	{
		oldI = Freelist;
		static_cast<unsigned int*>(Arena)[_Index] = oldI & ~tag_mask;
		newI = _Index | ((oldI + tag_one) & tag_mask);
	} while (!oStd::atomic_compare_exchange(&Freelist, newI, oldI));
}

} // namespace ouro

#endif
