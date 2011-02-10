/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
