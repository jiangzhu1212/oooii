// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_concurrent_object_pool_h
#define oBase_concurrent_object_pool_h

// provides typed simplification of some concurrent_pool APIs as well as 
// create/destroy for constructor/destructor semantics.

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
	concurrent_object_pool(concurrent_object_pool&& that) : concurrent_pool(std::move((concurrent_pool&&)that)) {}
	concurrent_object_pool(void* memory, size_type capacity) : concurrent_pool(memory, sizeof(T), capacity) {}
	~concurrent_object_pool() { ((concurrent_pool*)this)->~concurrent_pool(); }
	concurrent_object_pool& operator=(concurrent_object_pool&& that) { return (concurrent_object_pool&)concurrent_pool::operator=(std::move((concurrent_pool&&)that)); }
	size_type initialize(void* memory, size_type capacity) { return concurrent_pool::initialize(memory, sizeof(T), capacity); }


	// concurrent api

	// define destroy(T* p) which calls the dtor before deallocating
	// define create( ... ) which takes whatever ctor parameters type T provides
	oCONCURRENT_OBJECT_POOL_CREATE_DESTROY

	T* typed_pointer(index_type index) const { return (T*)concurrent_pool::pointer(index); }

private:
	concurrent_object_pool(const concurrent_object_pool&); /* = delete; */
	const concurrent_object_pool& operator=(const concurrent_object_pool&); /* = delete; */
};

}

#endif
