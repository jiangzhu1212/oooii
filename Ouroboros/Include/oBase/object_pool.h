/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// allocate in O(1) time from a preallocated array of blocks (fixed block
// allocator).
#ifndef oBase_object_pool_h
#define oBase_object_pool_h

#include <oBase/callable.h>
#include <oBase/pool.h>

// emulate variadic macros for passing various parameters to create()
#define oOBJECT_POOL_CREATE_N(n) \
	template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
	T* create(oCALLABLE_CONCAT(oARG_DECL,n)) { void* p = allocate_pointer(); return p ? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) : nullptr; }
#define oOBJECT_POOL_CREATE_DESTROY \
	oCALLABLE_PROPAGATE_SKIP0(oOBJECT_POOL_CREATE_N) \
	T* create() { void* p = allocate_pointer(); return p ? new (p) T() : nullptr; } \
	void destroy(T* _Pointer) { _Pointer->T::~T(); deallocate(_Pointer); }

namespace ouro {

template<typename T>
class object_pool : public pool
{
public:
	typedef pool::index_type index_type;
	typedef pool::size_type size_type;
	typedef T value_type;

	
	object_pool() {}
	object_pool(object_pool&& _That) : pool(std::move((pool&&)_That)) {}
	object_pool(void* memory, size_type capacity) : pool(memory, sizeof(T), capacity) {}
	object_pool(size_type capacity) : pool(sizeof(T), capacity) {}
	~object_pool() { ((pool*)this)->~pool(); }
	object_pool& operator=(object_pool&& _That) { return (object_pool&)pool::operator=(std::move((pool&&)_That)); }
	index_type initialize(void* memory, size_type capacity) { return pool::initialize(memory, sizeof(T), capacity); }
	size_type initialize(size_type capacity) { return pool::initialize(sizeof(T), capacity); }

	// define destroy(T* p) which calls the dtor before deallocating
	// define create( ... ) which takes whatever ctor parameters type T provides
	oOBJECT_POOL_CREATE_DESTROY

	T* typed_pointer(index_type index) const { return (T*)pool::pointer(index); }

private:
	object_pool(const object_pool&); /* = delete; */
	const object_pool& operator=(const object_pool&); /* = delete; */
};

}

#endif
