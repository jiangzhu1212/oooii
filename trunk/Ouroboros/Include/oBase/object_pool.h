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
#ifndef oBase_object_pool_h
#define oBase_object_pool_h

#include <oBase/byte.h>
#include <oBase/config.h>

namespace ouro {

template<typename IndexT, typename ObjectT, typename FreelistT, size_t IndexMask>
class object_pool_base
{
	object_pool_base(const object_pool_base&); /* = delete */
	const object_pool_base& operator=(const object_pool_base&); /* = delete */

public:
	typedef IndexT index_type;
	typedef ObjectT object_type;
	typedef FreelistT freelist_type;
	typedef unsigned int size_type;
	static const size_type index_size = sizeof(index_type);
	static const size_type object_size = sizeof(object_type);
	static const index_type index_mask = IndexMask & index_type(-1);
	static const size_type default_alignment = __max(sizeof(void*), oDEFAULT_MEMORY_ALIGNMENT);

	object_pool_base() : Objects(nullptr), NumObjects(0) { Freelist = 0; }
	object_pool_base(object_pool_base&& _That) { operator=(std::move(_That)); }
	object_pool_base& operator=(object_pool_base&& _That)
	{
		if (this != &_That)
		{
			Freelist = (index_type)_That.Freelist; _That.Freelist = 0;
			Objects = _That.Objects; _That.Objects = nullptr;
			NumObjects = _That.NumObjects; _That.NumObjects = 0;
		}

		return *this;
	}
	object_pool_base(void* _pObjects, size_type _NumObjects)
		: Objects(_pObjects)
		, NumObjects(_NumObjects)
	{
		if (object_size < sizeof(index_type))
			throw std::invalid_argument("block size must be at least the size of the index type");
		if (_NumObjects >= invalid_index)
			throw std::invalid_argument("too many Objects specified for the index type used");
		if (!byte_aligned(Objects, default_alignment))
			throw std::invalid_argument("the specified allocation of Objects must be properly aligned");
		clear();
	}

	// hard-resets the pool, leaving any outstanding allocation dangling. Not threadsafe.
	void clear()
	{
		Freelist = 0;
		index_type i = 0;
		for (index_type n = NumObjects - 1; i < n; i++)
			*at(i) = i+1;
		*at(i) = invalid_index;
	}

	// returns if this allocator has been initialized
	bool valid() const { return !!Objects; }

	// returns true if the specified index was allocated from this allocator
	bool valid(index_type _Index) const
	{
		return _Index < NumObjects;
	}

	// returns true if the specified pointer was allocated from this allocator
	bool valid(void* _Pointer) const
	{
		return in_range(_Pointer, Objects, NumObjects * object_size) && ((byte_diff(_Pointer, Objects) % object_size) == 0);
	}

	// SLOW! This walks the free list counting how many members there are. This is
	// designed for out-of-performance-critical code such as garbage collecting to
	// determine if all possible Objects are available and thus there are no 
	// outstanding allocations.
	size_type count_available() const
	{
		size_type n = 0;
		index_type i = Freelist & index_mask;
		while (i != invalid_index)
		{
			n++;
			if (n > capacity())
				throw std::length_error("count of available Objects has exceeded the maximum allowable");
			i = *at(i);
		}
		return n;
	}

	// number of objects allocated (SLOW! this loops through entire freelist 
	// each call)
	size_type size() const
	{
		size_type nFree = 0;
		if (NumObjects)
			nFree = count_available();
		return NumObjects - nFree;
	}

	// max number of objects to be allocated
	size_type capacity() const { return NumObjects; }

	// (SLOW! see size())
	bool empty() const { return size() == 0; }

	// returns true if all Objects have been allocated
	bool full() const { return (Freelist & index_mask) == invalid_index; }

	// retrieves the pointer used during construction so it can be 
	// be freed if no other reference is available.
	void* const get_objects_pointer() const { return Objects; }

	// return a pointer to the nth entry
	index_type* at(size_type _Index) { return static_cast<index_type*>(byte_add(Objects, object_size, _Index)); }
	const index_type* at(size_type _Index) const { return static_cast<index_type*>(byte_add(Objects, object_size, _Index)); }

protected:
	static const index_type invalid_index = index_mask;

	void* Objects;
	FreelistT Freelist;
	index_type NumObjects;
};

template<typename IndexT, typename ObjectT>
class object_pool : public object_pool_base<IndexT, ObjectT, IndexT, size_t(-1)>
{
	typedef object_pool_base<IndexT, ObjectT, IndexT, size_t(-1)> base_t;
	typedef object_pool<IndexT, ObjectT> self_t;

public:
	typedef typename base_t::index_type index_type;
	typedef typename base_t::object_type object_type;
	typedef typename base_t::size_type size_type;

	object_pool() {}
	object_pool(object_pool&& _That) { operator=(std::move(_That)); }
	object_pool& operator=(object_pool&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	object_pool(void* _pObjects, size_type _NumObjects) : base_t(_pObjects, _NumObjects) {}

	// allocate an index that could be used with at()
	index_type allocate_index()
	{
		index_type i;
		allocate(&i);
		return i;
	}

	// allocate a block or return nullptr if no available room and also return the new index
	object_type* allocate(index_type* _pIndex)
	{
		*_pIndex = Freelist;
		if (Freelist == invalid_index)
			return nullptr;
		index_type* p = at(Freelist);
		Freelist = *p;
		return new (p) object_type();
	}

	// allocate a block or return nullptr if no available room
	object_type* allocate()
	{
		index_type i;
		return allocate(&i);
	}

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
		*(index_type*)_pObject = Freelist;
		Freelist = (index_type)index_of(_pObject, Objects, object_size);
	}
};

} // namespace ouro

#endif
