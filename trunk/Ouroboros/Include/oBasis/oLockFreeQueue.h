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
// Single-reader, single-writer queue that uses no explicit mechanism for
// synchronization.
#pragma once
#ifndef oLockFreeQueue_h
#define oLockFreeQueue_h

#include <oBasis/oStdAtomic.h>
#include <oBasis/oThreadsafe.h>
#include <memory>

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

public:
	typedef Alloc AllocatorType;
	typedef T ElementType;

	inline oLockFreeQueue(size_t _Capacity = 100000, Alloc _Allocator = Alloc())
		: Allocator(_Allocator)
		, ElementPool(_Allocator.allocate(_Capacity))
		, NumElements(_Capacity)
		, Read(0)
		, Write(0)
	{
		for (size_t i = 0; i < NumElements; i++)
			new(ElementPool + i) T();
	}

	~oLockFreeQueue()
	{
		oASSERT(empty(), "queue not empty");
		Read = Write = 0;
		Allocator.deallocate(ElementPool, NumElements);
		ElementPool = 0;
		NumElements = 0;
	}

	inline bool try_push(const T& e) threadsafe
	{
		bool pushed = false;
		size_t r = Read;
		size_t w = Write;

		if (((w+1) % NumElements) != r)
		{
			oStd::atomic_thread_fence_read_write();
			ElementPool[w++] = e;
			oStd::atomic_thread_fence_read_write();
			Write = w % NumElements;
			pushed = true;
		}
	
		return pushed;
	}

	inline void push(const T& e) threadsafe { while (!try_push(e)); }

	inline bool try_pop(T& e) threadsafe
	{
		bool popped = false;
		size_t r = Read;
		size_t w = Write;

		if (r != w)
		{
			oStd::atomic_thread_fence_read_write();
			e = ElementPool[r];
			ElementPool[r++].~T();
			oStd::atomic_thread_fence_read_write();
			Read = r % NumElements;
			popped = true;
		}

		return popped;
	}

	inline bool valid() const threadsafe { return !!ElementPool; }
	inline bool empty() const threadsafe { return Read == Write; }
	inline size_t capacity() const threadsafe { return NumElements; }
	inline size_t unsafe_size() const { return ((Write + NumElements) - Read) % NumElements; }
};

#endif
