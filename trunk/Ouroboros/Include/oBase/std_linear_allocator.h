// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Wraps a linear_allocator in std::allocator's API as well as falling back
// on the default std::allocator if the original arena is insufficient, so
// this can gain efficiency if the size of allocations is known ahead of time
// but also won't fail if that estimate comes up short.
#pragma once
#ifndef oBase_std_linear_allocator_h
#define oBase_std_linear_allocator_h

#include <oBasis/oPlatformFeatures.h>
#include <oBase/linear_allocator.h>

namespace ouro {

template<typename T> class std_linear_allocator
{
public:
	oDEFINE_STD_ALLOCATOR_BOILERPLATE(std_linear_allocator)
	std_linear_allocator(linear_allocator* _pAllocator, size_t* _pHighwaterMark)
			: pAllocator(_pAllocator)
			, pHighwaterMark(_pHighwaterMark)
	{
		if (pHighwaterMark)
			*pHighwaterMark = 0;
	}

	~std_linear_allocator() {}
	
	template<typename U> std_linear_allocator(std_linear_allocator<U> const& _That)
		: pAllocator(_That.pAllocator)
		, pHighwaterMark(_That.pHighwaterMark)
	{}
	
	inline const std_linear_allocator& operator=(const std_linear_allocator& _That)
	{
		pAllocator = _That.pAllocator;
		FallbackAllocator = _That.FallbackAllocator;
		pHighwaterMark = _That.pHighwaterMark;
		return *this;
	}
	
	inline pointer allocate(size_type count, const_pointer hint = 0)
	{
		const size_t nBytes = sizeof(T) * count;
		void* p = pAllocator->allocate(nBytes);
		if (!p)
			p = FallbackAllocator.allocate(nBytes, hint);
		if (p && pHighwaterMark)
			*pHighwaterMark = max(*pHighwaterMark, nBytes);
		return static_cast<pointer>(p);
	}
	
	inline void deallocate(pointer p, size_type count)
	{
		if (!pAllocator->valid(p))
			FallbackAllocator.deallocate(p, count);
	}
	
	inline void reset() { pAllocator->reset(); }
	
	inline size_t get_highwater_mark() const { return pHighwaterMark ? *pHighwaterMark : 0; }

private:
	template<typename> friend class std_linear_allocator;
	linear_allocator* pAllocator;
	size_t* pHighwaterMark;
	std::allocator<T> FallbackAllocator;
};

}

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(ouro::std_linear_allocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(ouro::std_linear_allocator) { return a.pAllocator == b.pAllocator; }

#endif
