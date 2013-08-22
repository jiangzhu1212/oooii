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
// Single-reader, single-writer queue that uses no explicit mechanism for
// synchronization.
#pragma once
#ifndef oConcurrency_lock_free_queue_h
#define oConcurrency_lock_free_queue_h

#include <oConcurrency/concurrent_queue_base.h>
#include <oConcurrency/thread_safe.h>
#include <oStd/oStdAtomic.h>
#include <memory>

namespace oConcurrency {

template<typename T, typename Alloc = std::allocator<T>>
class lock_free_queue : public concurrent_queue_base<T, lock_free_queue<T, Alloc>>
{
	// single reader, single writer queue using memory barriers to guarantee thread safety
	// (no locks or atomics)
public:
	typedef Alloc allocator_type;
	oDEFINE_CONCURRENT_QUEUE_TYPE(T, size_t);

	lock_free_queue(size_type _Capacity = 10000, const allocator_type& _Allocator = allocator_type());
	~lock_free_queue();

	// Push an element into the queue.
	void push(const_reference _Element) threadsafe;
	
	// Returns false if the queue is empty, otherwise _Element is a valid element
	// from the head of the queue.
	bool try_pop(reference _Element) threadsafe;

	// Returns true if no elements are in the queue
	bool empty() const threadsafe;
	
	// Returns the number of elements in the queue. Client code should not be 
	// reliant on this value and the API is included only for debugging and 
	// testing purposes. It is not threadsafe.
	size_type size() const;

private:
	value_type* ElementPool;
	size_type NumElements;
	volatile size_type Read;
	volatile size_type Write;
	Alloc Allocator;
};

template<typename T, typename Alloc>
lock_free_queue<T, Alloc>::lock_free_queue(size_type _Capacity, const allocator_type& _Allocator)
	: Allocator(_Allocator)
	, ElementPool(Allocator.allocate(_Capacity))
	, NumElements(_Capacity)
	, Read(0)
	, Write(0)
{
	for (size_type i = 0; i < NumElements; i++)
		new(ElementPool + i) T();
}

template<typename T, typename Alloc>
lock_free_queue<T, Alloc>::~lock_free_queue()
{
	if (!empty())
		throw container_error(container_errc::not_empty);

	Read = Write = 0;
	Allocator.deallocate(ElementPool, NumElements);
	ElementPool = 0;
	NumElements = 0;
}

template<typename T, typename Alloc>
void lock_free_queue<T, Alloc>::push(const_reference _Element) threadsafe
{
	size_type r = Read;
	size_type w = Write;

	if (((w+1) % NumElements) != r)
	{
		oStd::atomic_thread_fence_read_write();
		ElementPool[w++] = _Element;
		oStd::atomic_thread_fence_read_write();
		Write = w % NumElements;
	}

	else
		throw std::overflow_error("lock_free_queue cannot hold any more elements");
}

template<typename T, typename Alloc>
bool lock_free_queue<T, Alloc>::try_pop(reference _Element) threadsafe
{
	bool popped = false;
	size_type r = Read;
	size_type w = Write;

	if (r != w)
	{
		oStd::atomic_thread_fence_read_write();
		_Element = ElementPool[r];
		ElementPool[r++].~value_type();
		oStd::atomic_thread_fence_read_write();
		Read = r % NumElements;
		popped = true;
	}

	return popped;
}

template<typename T, typename Alloc>
bool lock_free_queue<T, Alloc>::empty() const threadsafe
{
	return Read == Write;
}

template<typename T, typename Alloc>
typename lock_free_queue<T, Alloc>::size_type lock_free_queue<T, Alloc>::size() const
{
	return ((Write + NumElements) - Read) % NumElements;
}

} // namespace oConcurrency

#endif
