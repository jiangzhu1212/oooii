// $(header)
// Helper macros for encapsulating some of the boilerplate typing required when 
// defining a custom STL std::allocator.
#pragma once
#ifndef oStdAlloc_h
#define oStdAlloc_h

#define oDEFINE_STD_ALLOCATOR_BOILERPLATE(ClassName) \
	typedef size_t size_type; typedef ptrdiff_t difference_type; typedef T value_type; typedef T* pointer; typedef T const* const_pointer; typedef T& reference; typedef T const& const_reference; \
	template<typename U> struct rebind { typedef ClassName<U> other; }; \
	pointer address(reference p) const { return &p; } \
	const_pointer address(const_reference p) const { return &p; } \
	size_type max_size() const { return static_cast<size_type>(~0u) / sizeof(value_type); } \
	void construct(pointer p) { ::new (static_cast<void*>(p)) T(); } \
	void construct(pointer p, const T& val) { ::new (static_cast<void*>(p)) T(val); } \
	void destroy(pointer p) { p->~T(); }

#define oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(ClassName) \
	template <> class ClassName<void> \
	{ public: typedef void* pointer; typedef const void* const_pointer; typedef void value_type; \
		template <class U> struct rebind { typedef ClassName<U> other; }; \
	};

#define oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(ClassName) \
	template<typename T, typename U> bool operator!=(ClassName<T> const& a, ClassName<U> const& b) { return !(a == b); } \
	template<typename T, typename U> bool operator==(ClassName<T> const& a, ClassName<U> const& b)

// As an example of this boilerplate usage, and as a semi-useful class itself,
// here is a std::allocator that takes user functions of the same signature as 
// malloc and free to do allocations in STL containers. Although this is 
// somewhat naughty in that state (the two function pointers) is retained, all 
// common STL implementations passing things through, and all copy and equality 
// semantics are enforced, so this works properly in all but the most contrived 
// cases.
template<typename T> struct oStdUserCallbackAllocator
{
	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oStdUserCallbackAllocator)
	oStdUserCallbackAllocator(void* (*alloc)(size_t size), void (*dealloc)(void* p)) : Allocate(alloc), Deallocate(dealloc) {}
	template<typename U> oStdUserCallbackAllocator(oStdUserCallbackAllocator<U> const& other) : Allocate(other.Allocate), Deallocate(other.Deallocate) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>((*Allocate)(sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { (*Deallocate)(p); }
	inline const oStdUserCallbackAllocator& operator=(const oStdUserCallbackAllocator& other) { Allocate = other.Allocate; Deallocate = other.Deallocate; return *this; }
	void* (*Allocate)(size_t size);
	void (*Deallocate)(void* p);
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oStdUserCallbackAllocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oStdUserCallbackAllocator) { return a.Allocate == b.Allocate && a.Deallocate == b.Deallocate; }

#endif
