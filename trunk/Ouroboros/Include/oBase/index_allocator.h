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
// index_allocator for a threadsafe implementation.
#pragma once
#ifndef oBase_index_allocator_h
#define oBase_index_allocator_h

#include <oBase/object_pool.h>

namespace ouro {

class index_allocator : private object_pool<unsigned int, unsigned int>
{
	typedef object_pool<unsigned int, unsigned int> base_t;
	typedef index_allocator self_t;
public:
	typedef base_t::index_type index_type;
	typedef base_t::size_type size_type;
	static const index_type invalid_index = base_t::invalid_index;

	index_allocator() {}
	index_allocator(index_allocator&& _That) { operator=(std::move(_That)); }
	index_allocator& operator=(index_allocator&& _That) { return (self_t&)base_t::operator=(std::move((base_t&&)_That)); }
	index_allocator(unsigned int* _pIndices, size_type _NumIndices) : base_t(_pIndices, _NumIndices) {}
	
	unsigned int allocate() { return base_t::allocate_index(); }
	void deallocate(unsigned int _Index) { base_t::deallocate_index(_Index); }
	void clear() { return base_t::clear(); }
	bool valid() const { return base_t::valid(); }
	bool valid(index_type _Index) const { return base_t::valid(_Index); }
	size_type count_available() const { return base_t::count_available(); }
	size_type size() const { return base_t::size(); }
	size_type capacity() const { return base_t::capacity(); }
	bool empty() const { return base_t::empty(); }
	bool full() const { return base_t::full(); }
	void* const get_objects_pointer() const { return base_t::get_objects_pointer(); }
};

} // namespace ouro

#endif
