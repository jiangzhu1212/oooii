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
// Allocates fixed-sized blocks in O(1) time. See fixed_block_allocator_base 
// for more details on the boiler-plate API.

#ifndef oBase_fixed_block_allocator_h
#define oBase_fixed_block_allocator_h

#include <oBase/fixed_block_allocator_base.h>

namespace ouro {

template<typename IndexT>
class fixed_block_allocator_base : public fixed_block_allocator_common_base<IndexT, IndexT, IndexT(-1)>
{
	typedef fixed_block_allocator_common_base<IndexT, IndexT, IndexT(-1)> base_t;
	typedef fixed_block_allocator_base<IndexT> self_t;

public:
	fixed_block_allocator_base() {}
	fixed_block_allocator_base(fixed_block_allocator_base&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_base& operator=(fixed_block_allocator_base&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	fixed_block_allocator_base(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks) : base_t(_pBlocks, _BlockSize, _NumBlocks) {}

	// Allocate a block or return nullptr if no available room
	void* allocate(size_type _BlockSize)
	{
		if (Freelist == invalid_index)
			return nullptr;
		index_type* p = at(_BlockSize, Freelist);
		Freelist = *p;
		return p;
	}

	// Mark a block as available
	void deallocate(size_type _BlockSize, size_type _NumBlocks, void* _Pointer)
	{
		if (!valid(_BlockSize, _NumBlocks, _Pointer))
			throw std::out_of_range("the specified pointer was not allocated from this allocator");
		*(index_type*)_Pointer = Freelist;
		Freelist = static_cast<index_type>(index_of(_Pointer, Blocks, _BlockSize));
	}
};

template<typename IndexT, size_t S>
class fixed_block_allocator_s : public fixed_block_allocator_common_base_s<fixed_block_allocator_base<IndexT>, S>
{
	typedef fixed_block_allocator_common_base_s<fixed_block_allocator_base<IndexT>, S> base_t;
	typedef fixed_block_allocator_s<IndexT, S> self_t;
public:
	fixed_block_allocator_s() {}
	fixed_block_allocator_s(fixed_block_allocator_s&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_s(void* _pBlocks, size_type _NumBlocks) : base_t(_pBlocks, _NumBlocks) {}
	fixed_block_allocator_s& operator=(fixed_block_allocator_s&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
};

template<typename IndexT, typename T>
class fixed_block_allocator_t : public fixed_block_allocator_common_base_t<fixed_block_allocator_base<IndexT>, T>
{
	typedef fixed_block_allocator_common_base_t<fixed_block_allocator_base<IndexT>, T> base_t;
	typedef fixed_block_allocator_t<IndexT, T> self_t;
public:
	typedef T value_type;
	fixed_block_allocator_t() {}
	fixed_block_allocator_t(fixed_block_allocator_t&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_t(void* _pBlocks, size_type _NumBlocks) : base_t(_pBlocks, _NumBlocks) {}
	fixed_block_allocator_t& operator=(fixed_block_allocator_t&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
};

template<typename IndexT, typename T, size_t Capacity>
class fixed_block_allocator_static : public fixed_block_allocator_static_base<fixed_block_allocator_base<IndexT>, T, Capacity>
{
	typedef fixed_block_allocator_static_base<fixed_block_allocator_base<IndexT>, T, Capacity> base_t;
	typedef fixed_block_allocator_static<IndexT, T, Capacity> self_t;
public:
	fixed_block_allocator_static() : base_t() {}
	fixed_block_allocator_static(fixed_block_allocator_static&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator_static& operator=(fixed_block_allocator_static&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
};

template<typename IndexT>
class fixed_block_allocator : public fixed_block_allocator_encapsulated_base<fixed_block_allocator_base<IndexT>>
{
	typedef fixed_block_allocator_encapsulated_base<fixed_block_allocator_base<IndexT>> base_t;
	typedef fixed_block_allocator<IndexT> self_t;
public:
	fixed_block_allocator() {}
	fixed_block_allocator(fixed_block_allocator&& _That) { operator=(std::move(_That)); }
	fixed_block_allocator(void* _pBlocks, size_type _BlockSize, size_type _NumBlocks) : base_t(_pBlocks, _BlockSize, _NumBlocks) {}
	fixed_block_allocator& operator=(fixed_block_allocator&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
};

} // namespace ouro

#endif
