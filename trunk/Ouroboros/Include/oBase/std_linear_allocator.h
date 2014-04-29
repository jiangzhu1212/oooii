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

} // namespace ouro

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(ouro::std_linear_allocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(ouro::std_linear_allocator) { return a.pAllocator == b.pAllocator; }

#endif