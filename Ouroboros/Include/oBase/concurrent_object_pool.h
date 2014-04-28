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
// provides typed simplification of some concurrent_pool APIs as well as 
// create/destroy for constructor/destructor semantics.
#pragma once
#ifndef oBase_concurrent_object_pool_h
#define oBase_concurrent_object_pool_h

#include <oBase/callable.h>
#include <oBase/concurrent_pool.h>

// emulate variadic macros for passing various parameters to create()
#define oCONCURRENT_OBJECT_POOL_CREATE_N(n) \
	template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
	T* create(oCALLABLE_CONCAT(oARG_DECL,n)) { void* p = allocate_pointer(); return p ? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) : nullptr; }
#define oCONCURRENT_OBJECT_POOL_CREATE_DESTROY \
	oCALLABLE_PROPAGATE_SKIP0(oCONCURRENT_OBJECT_POOL_CREATE_N) \
	T* create() { void* p = allocate_pointer(); return p ? new (p) T() : nullptr; } \
	void destroy(T* _Pointer) { _Pointer->T::~T(); deallocate(_Pointer); }

namespace ouro {

template<typename T>
class concurrent_object_pool : public concurrent_pool
{
public:
	typedef concurrent_pool::index_type index_type;
	typedef concurrent_pool::size_type size_type;
	typedef T value_type;

	
	// non-concurrent api

	concurrent_object_pool() {}
	concurrent_object_pool(concurrent_object_pool&& _That) : concurrent_pool(std::move((concurrent_pool&&)_That)) {}
	concurrent_object_pool(void* memory, size_type capacity) : concurrent_pool(memory, sizeof(T), capacity) {}
	concurrent_object_pool(size_type capacity) : concurrent_pool(sizeof(T), capacity) {}
	~concurrent_object_pool() { ((concurrent_pool*)this)->~concurrent_pool(); }
	concurrent_object_pool& operator=(concurrent_object_pool&& _That) { return (concurrent_object_pool&)concurrent_pool::operator=(std::move((concurrent_pool&&)_That)); }
	index_type initialize(void* memory, size_type capacity) { return concurrent_pool::initialize(memory, sizeof(T), capacity); }
	size_type initialize(size_type capacity) { return concurrent_pool::initialize(sizeof(T), capacity); }

	// concurrent api

	// define destroy(T* p) which calls the dtor before deallocating
	// define create( ... ) which takes whatever ctor parameters type T provides
	oCONCURRENT_OBJECT_POOL_CREATE_DESTROY

	T* typed_pointer(index_type index) const { return (T*)concurrent_pool::pointer(index); }

private:
	concurrent_object_pool(const concurrent_object_pool&); /* = delete; */
	const concurrent_object_pool& operator=(const concurrent_object_pool&); /* = delete; */
};

} // namespace ouro

#endif
