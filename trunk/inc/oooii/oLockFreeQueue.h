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
#pragma once
#ifndef oLockFreeQueue_h
#define oLockFreeQueue_h

#include <oooii/oStddef.h>
#include <oooii/oAtomic.h>

template<typename T, typename Alloc = std::allocator<T> >
class oLockFreeQueue
{
	// single reader, single writer queue using memory barriers to guarantee thread safety
	// (no locks or atomics)
	
	T* ElementPool;
	size_t NumElements;
	volatile size_t Read;
	volatile size_t Write;
	Alloc Allocator;

	oFORCEINLINE void RWB() threadsafe { oReadWriteBarrier(); } // only real native part

public:
	typedef Alloc AllocatorType;
	typedef T ElementType;

	inline oLockFreeQueue(size_t capacity = 100000, Alloc allocator = Alloc())
		: Allocator(allocator)
		, ElementPool(allocator.allocate(capacity))
		, NumElements(capacity)
		, Read(0)
		, Write(0)
	{
		for( size_t i =0; i < NumElements; ++i )
			new( ElementPool + i )T();
	}

	~oLockFreeQueue()
	{
		oASSERT(IsEmpty(), "queue not empty");
		Read = Write = 0;
		Allocator.deallocate(ElementPool, NumElements);
		ElementPool = 0;
		NumElements = 0;
	}

	inline bool TryPush(const T& e) threadsafe
	{
		bool pushed = false;
		size_t r = Read;
		size_t w = Write;

		if (((w+1) % NumElements) != r)
		{
			RWB();
			ElementPool[w++] = e;
			RWB();
			Write = w % NumElements;
			pushed = true;
		}
	
		return pushed;
	}

	inline bool TryPop(T& e) threadsafe
	{
		bool popped = false;
		size_t r = Read;
		size_t w = Write;

		if (r != w)
		{
			RWB();
			e = ElementPool[r];
			ElementPool[r++].~T();
			RWB();
			Read = r % NumElements;
			popped = true;
		}

		return popped;
	}

	inline bool IsValid() const threadsafe { return !!ElementPool; }
	inline bool IsEmpty() const { return Read == Write; }
	inline size_t Capacity() const threadsafe { return NumElements; }
	inline size_t Size() const { return ((Write + NumElements) - Read) % NumElements; }
	inline void Push(const T& e) threadsafe { while (!TryPush(e)); }
	inline void Pop(T& e) threadsafe { while (!TryPop(e)); }
};

#endif
