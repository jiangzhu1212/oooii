// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oBase_object_pool_h
#define oBase_object_pool_h

// allocate in O(1) time from a preallocated array of blocks (fixed block
// allocator).

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
	object_pool(object_pool&& that) : pool(std::move((pool&&)that)) {}
	object_pool(void* memory, size_type capacity) : pool(memory, sizeof(T), capacity) {}
	~object_pool() { ((pool*)this)->~pool(); }
	object_pool& operator=(object_pool&& that) { return (object_pool&)pool::operator=(std::move((pool&&)that)); }
	size_type initialize(void* memory, size_type capacity) { return pool::initialize(memory, sizeof(T), capacity); }

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
