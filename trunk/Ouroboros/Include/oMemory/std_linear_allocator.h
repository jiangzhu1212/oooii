// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMemory_std_linear_allocator_h
#define oMemory_std_linear_allocator_h

// Wraps a linear_allocator in std::allocator's API as well as falling back
// on the default std::allocator if the original arena is insufficient so
// this can gain efficiency if the size of allocations is known ahead of time
// but also won't fail if that estimate comes up short.

#include <oMemory/std_allocator.h>
#include <oMemory/linear_allocator.h>

namespace ouro {

template<typename T> class std_linear_allocator
{
public:
	oDEFINE_STD_ALLOCATOR_BOILERPLATE(std_linear_allocator)
	std_linear_allocator(linear_allocator* alloc_, size_t* out_highwater)
			: alloc(alloc_)
			, highwater(out_highwater)
	{
		if (highwater)
			*highwater = 0;
	}

	~std_linear_allocator() {}
	
	template<typename U> std_linear_allocator(std_linear_allocator<U> const& that)
		: alloc(that.alloc)
		, highwater(that.highwater)
	{}
	
	inline const std_linear_allocator& operator=(const std_linear_allocator& that)
	{
		alloc = that.alloc;
		fallback = that.fallback;
		highwater = that.highwater;
		return *this;
	}
	
	inline pointer allocate(size_type count, const_pointer hint = 0)
	{
		const size_t nBytes = sizeof(T) * count;
		void* p = alloc->allocate(nBytes);
		if (!p)
			p = fallback.allocate(nBytes, hint);
		if (p && highwater)
			*highwater = max(*highwater, nBytes);
		return static_cast<pointer>(p);
	}
	
	inline void deallocate(pointer p, size_type count)
	{
		if (!alloc->owns(p))
			fallback.deallocate(p, count);
	}
	
	inline void reset() { alloc->reset(); }
	
	inline size_t get_highwater_mark() const { return highwater ? *highwater : 0; }

private:
	template<typename> friend class std_linear_allocator;
	linear_allocator* alloc;
	size_t* highwater;
	std::allocator<T> fallback;
};

}

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(ouro::std_linear_allocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(ouro::std_linear_allocator) { return a.alloc == b.alloc; }

#endif
