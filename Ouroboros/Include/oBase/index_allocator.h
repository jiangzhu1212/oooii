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
// Allocates indices which can be used for fixed-size pool management. The 
// allocated arena will contain a linked list of free indices so it must be 
// sized to contain the number of indices desired, so 
// NumIndices * sizeof(unsigned int). This class is not threadsafe. See 
// concurrent_index_allocator for a threadsafe implementation.
#pragma once
#ifndef oBase_index_allocator_h
#define oBase_index_allocator_h

#include <oBase/index_allocator_base.h>

namespace ouro {

struct index_allocator_traits
{
	typedef unsigned int index_type;

	static const unsigned int index_mask = 0xffffffff;
};

class index_allocator : public index_allocator_base<index_allocator_traits>
{
public:
	index_allocator() {}
	index_allocator(void* _pArena, size_t _SizeofArena);
	index_allocator(concurrent_index_allocator&& _That) { operator=(std::move(_That)); }
	index_allocator& operator=(index_allocator&& _That)
	{
		index_allocator_base::operator=(std::move((index_allocator_base&&)_That));
		return *this;
	}

	// return an index reserved until it is made available by deallocate
	unsigned int allocate();

	// make index available again
	void deallocate(unsigned int _Index);
};

inline index_allocator::index_allocator(void* _pArena, size_t _SizeofArena) 
	: index_allocator_base(_pArena, _SizeofArena) 
{
}

inline unsigned int index_allocator::allocate()
{
	if (Freelist == invalid_index)
		return invalid_index;
	unsigned int allocatedIndex = Freelist;
	Freelist = static_cast<unsigned int*>(pArena)[Freelist];
	return allocatedIndex;
}

inline void index_allocator::deallocate(unsigned int _Index)
{
	static_cast<unsigned int*>(pArena)[_Index] = Freelist;
	Freelist = _Index;
}

} // namespace ouro

#endif
