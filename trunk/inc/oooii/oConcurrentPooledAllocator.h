// $(header)

// threadsafe O(1) allocation of fixed-size objects
#pragma once
#ifndef oConcurrentPooledAllocator_h
#define oConcurrentPooledAllocator_h

#include <oooii/oPooledAllocator.h>
#include <oooii/oConcurrentIndexAllocator.h>

template<typename T, typename Alloc = std::allocator<char> >
class oConcurrentPooledAllocator : public oTPooledAllocator<T, oConcurrentIndexAllocator, Alloc>
{
public:
	oConcurrentPooledAllocator(const char* _DebugName, PoolInitType _Type = InitBytes, size_t _Value = 0, const Alloc& _Allocator = Alloc())
		: oTPooledAllocator(_DebugName, _Type, _Value, _Allocator)
	{}
};

#define oDEFINE_CONCURRENT_POOLED_NEW_DELETE(PooledClass, PoolInstance, ReserveNumElements) oDEFINE_CONCURRENT_POOLED_NEW_DELETE__(oConcurrentPooledAllocator, PooledClass, PoolInstance, ReserveNumElements)

#endif
