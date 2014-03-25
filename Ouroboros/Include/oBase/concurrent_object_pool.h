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
#pragma once
#ifndef oBase_concurrent_object_pool
#define oBase_concurrent_object_pool

#include <oBase/object_pool.h>
#include <atomic>

namespace ouro {

template<typename ObjectT>
class concurrent_object_pool : public object_pool_base<unsigned int, ObjectT, std::atomic<unsigned int>, 0x00ffffff>
{
	typedef object_pool_base<unsigned int, ObjectT, std::atomic<unsigned int>, 0x00ffffff> base_t;
	typedef concurrent_object_pool<ObjectT> self_t;
	static const unsigned int tag_mask = ~index_mask;
	static const unsigned int tag_one = 0x01000000;

public:
	concurrent_object_pool() {}
	concurrent_object_pool(concurrent_object_pool&& _That) { operator=(std::move(_That)); }
	concurrent_object_pool& operator=(concurrent_object_pool&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	concurrent_object_pool(void* _pObjects, size_type _NumObjects) : base_t(_pObjects, _NumObjects) {}

	// allocate an index that could be used with at()
	index_type allocate_index()
	{
		index_type i, newI, oldI = Freelist;
		do
		{
			i = oldI & index_mask;
			if (i == invalid_index)
				return i;
			index_type* p = at(i);
			newI = *p | ((oldI + tag_one) & tag_mask);

		} while (!Freelist.compare_exchange_strong(oldI, newI));
		return i;
	}

	// allocate a block or return nullptr if no available room and also return the new index
	object_type* allocate(index_type* _pIndex)
	{
		index_type* p = nullptr;
		index_type newI, oldI = Freelist;
		do
		{
			*_pIndex = oldI & index_mask;
			if (*_pIndex == invalid_index)
				return nullptr;
			p = at(*_pIndex);
			newI = *p | ((oldI + tag_one) & tag_mask);

		} while (!Freelist.compare_exchange_strong(oldI, newI));
		return new (p) object_type();
	}

	// allocate a block or return nullptr if no available room
	object_type* allocate() { index_type i; return allocate(&i); }

	// mark a block as available by index
	void deallocate_index(index_type _Index)
	{
		if (!valid(_Index))
			throw std::out_of_range("the specified index was not allocated from this allocator");
		object_type* o = (object_type*)at(_Index);
		deallocate(o);
	}

	// mark a block as available by pointer
	void deallocate(object_type* _pObject)
	{
		if (!valid(_pObject))
			throw std::out_of_range("the specified pointer was not allocated from this allocator");
		_pObject->~object_type();
		index_type i = (index_type)index_of(_pObject, Objects, object_size);
		index_type newI, oldI = Freelist;
		do
		{
			*(index_type*)_pObject = oldI & index_mask;
			newI = i | ((oldI + tag_one) & tag_mask);
		} while (!Freelist.compare_exchange_strong(oldI, newI));
	}
};
		
} // namespace ouro
#endif
