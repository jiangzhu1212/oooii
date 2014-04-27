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
#ifndef oBase_concurrent_growable_object_pool_h
#define oBase_concurrent_growable_object_pool_h

#include <oBase/callable.h>
#include <oBase/concurrent_growable_pool.h>

// emulate variadic macros for passing various parameters to create()
#define oCONCURRENT_GROWABLE_OBJECT_POOL_CREATE_N(n) \
	template<oCALLABLE_CONCAT(oARG_TYPENAMES,n)> \
	T* create(oCALLABLE_CONCAT(oARG_DECL,n)) { void* p = allocate_pointer(); return p ? new (p) T(oCALLABLE_CONCAT(oARG_PASS,n)) : nullptr; }
#define oCONCURRENT_GROWABLE_OBJECT_POOL_CREATE_DESTROY \
	oCALLABLE_PROPAGATE_SKIP0(oCONCURRENT_GROWABLE_OBJECT_POOL_CREATE_N) \
	T* create() { void* p = allocate_pointer(); return p ? new (p) T() : nullptr; } \
	void destroy(T* _Pointer) { _Pointer->T::~T(); deallocate(_Pointer); }

namespace ouro {

template<typename T>
class concurrent_growable_object_pool : public concurrent_growable_pool
{
public:
	typedef concurrent_pool::size_type size_type;
	typedef T value_type;


	// non-concurrent api

	concurrent_growable_object_pool() {}
	concurrent_growable_object_pool(concurrent_growable_object_pool&& _That) : concurrent_growable_pool(std::move((concurrent_growable_pool&&)_That)) {}
	concurrent_growable_object_pool(size_type capacity) : concurrent_growable_pool(sizeof(T), capacity, oALIGNOF(value_type)) {}
	concurrent_growable_object_pool(size_type capacity, size_type alignment) : concurrent_growable_pool(sizeof(T), capacity, alignment) {}
	~concurrent_growable_object_pool() {}
	concurrent_growable_object_pool& operator=(concurrent_growable_object_pool&& _That) { return (concurrent_growable_object_pool&)concurrent_growable_pool::operator=(std::move((concurrent_growable_pool&&)_That)); }

	bool initialize(size_type capacity_per_chunk) { return concurrent_growable_pool::initialize(sizeof(T), capacity_per_chunk); }


	// concurrent api

	oCONCURRENT_GROWABLE_OBJECT_POOL_CREATE_DESTROY

private:
	concurrent_growable_object_pool(const concurrent_growable_object_pool&); /* = delete; */
	const concurrent_growable_object_pool& operator=(const concurrent_growable_object_pool&); /* = delete; */
};

} // namespace ouro

#endif
