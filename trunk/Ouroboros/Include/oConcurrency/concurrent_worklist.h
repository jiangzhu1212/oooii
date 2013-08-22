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
// A thread-safe queue-like structure that uses a mutex to ensure concurrency 
// but aggressively tries to avoid the lock. It also uses LIFO behavior for 
// threadsafe try_pop to take take advantage of cache locality, while using
// FIFO when stealing. This has been benchmarked up to 3x faster than MS-queue
// (oConcurrency::concurrent_queue) even in the non-try_pop_local usage case, but since the 
// non-specialized API is not FIFO, this is not called a queue.
#pragma once
#ifndef oConcurrency_concurrent_worklist_h
#define oConcurrency_concurrent_worklist_h

#include <oConcurrency/concurrent_queue_base.h>
#include <oConcurrency/mutex.h>
#include <memory>

namespace oConcurrency {

template<typename T, typename Alloc = std::allocator<T>>
class concurrent_worklist : public concurrent_queue_base<T, concurrent_worklist<T, Alloc>>
{
	/** <citation
		usage="Paper" 
		reason="Synchronized basic_threadpool is slow" 
		author="Joe Duffy"
		description="http://bluebytesoftware.com/blog/2008/08/12/BuildingACustomThreadPoolSeriesPart2AWorkStealingQueue.aspx"
		modifications="C++ version from the paper's C#"
	/>*/

public:
	typedef Alloc allocator_type;
	oDEFINE_CONCURRENT_QUEUE_TYPE(T, unsigned int);

	// This implementation auto-grows capacity in a threadsafe manner if the 
	// initial capacity is inadequate. Capacity must always be a power of two.
	concurrent_worklist(const allocator_type& _Allocator = allocator_type());
	~concurrent_worklist();

	// _____________________________________________________________________________
	// Compatibility/slow-path API. The goal of this queue is to avoid locks when
	// work is scheduled locally on a worker thread in a thread pool. Thus as a 
	// general-purpose queue this is not the greatest. For compatibility and 
	// testing, the same API as other queues is implemented here and really 
	// represents the yes-do-the-lock path of the more efficient *_local APIs, so
	// is still worth testing. Prefer using this queue only in the implementation
	// of a thread pool and then only as the thread-local queue.

	// Push an element into the queue.
	void push(const_reference _Element) threadsafe;
	void push(value_type&& _Element) threadsafe;

	// Returns false if the queue is empty. This is a LIFO operation, the most
	// recently pushed element will be returned.
	bool try_pop(reference _Element) threadsafe;

	// Returns true if no elements are in the queue
	bool empty() const threadsafe;

	// Returns the number of elements in the queue. This is an instantaneous 
	// sampling and thus might not be valid by the very next line of code. Client
	// code should not be reliant on this value and the API is included only for 
	// debugging and testing purposes.
	size_type size() const;

	// _____________________________________________________________________________
	// Efficient/thread-local API. The goal of these is to try to avoid locks, but
	// is only valid if all called from the same thread, thus the APIs are not 
	// marked threadsafe. Do not mix the standard API with this local API without
	// external synchronization.

	// This can only be called from the same thread as the one calling 
	// try_pop_local. If called from a different thread this will have undefined 
	// behavior. There are performance advantages for choosing this API over push 
	// because this tries to avoid locking.
	void push_local(const_reference _Element);
	void push_local(value_type&& _Element);

	// Returns false if the queue is empty. This is a LIFO operation, the most
	// recently pushed element will be returned. This should only be called from
	// the same thread as the one calling push_local
	bool try_pop_local(reference _Element);

	// Blocks until an element can be popped from the queue
	void pop_local(reference _Element);

	// _____________________________________________________________________________
	// Extended API - this is the specialized solution for an optimized thread 
	// pool: this is FIFO while try_pop_local (and try_pop) is LIFO. This is able
	// to interact in any push/pop behavior and thus is marked threadsafe.

	// Returns false if the queue is empty. This is a FIFO operation, the first
	// element to be pushed will be stolen through this call.
	bool try_steal(reference _Element) threadsafe;

	// Returns false if the queue is empty. This is a FIFO operation, the first
	// element to be pushed will be stolen through this call.
	template<typename Rep, typename Period>
	bool try_steal_for(reference _Element
		, const oStd::chrono::duration<Rep, Period>& _TimeoutDuration) threadsafe;

private:
	pointer pCircularBuffer;
	size_type Mask;
	volatile size_type Head;
	volatile size_type Tail;
	oConcurrency::timed_mutex Mutex;
	Alloc Allocator;

	static const size_t InitialCapacity = 1<<16; // must be pow-of-2

	void ensure_capacity(size_type& _Tail_InOut);
	void internal_push(const_reference _Element, size_type _Tail);
	void internal_push(value_type&& _Element, size_type _Tail);
	void internal_push_safe(const_reference _Element, size_type _Tail);
	void internal_push_safe(value_type&& _Element, size_type _Tail);
	bool try_pop_internal(reference _Element, size_type _Tail);
	bool try_steal_internal(reference _Element, bool _TryLockResult) threadsafe;

	void destroy_buffer();
};

template<typename T, typename Alloc>
struct is_fifo<concurrent_worklist<T, Alloc>> : std::false_type {};

template<typename T, typename Alloc>
concurrent_worklist<T, Alloc>::concurrent_worklist(const allocator_type& _Allocator)
	: Allocator(_Allocator)
	, pCircularBuffer(Allocator.allocate(InitialCapacity)) // must be pow-of-2
	, Mask(InitialCapacity - 1)
	, Head(0)
	, Tail(0)
{
	for (size_t i = 0; i < InitialCapacity; i++)
		::new (&pCircularBuffer[i]) value_type();
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::destroy_buffer()
{
	const size_type len = Mask + 1;
	pointer p = pCircularBuffer;
	pointer pEnd = p + len;
	while (p < pEnd)
		Allocator.destroy(p++);
	Allocator.deallocate(pCircularBuffer, len);
}

template<typename T, typename Alloc>
concurrent_worklist<T, Alloc>::~concurrent_worklist()
{
	if (!empty())
		throw oConcurrency::container_error(oConcurrency::container_errc::not_empty);
	destroy_buffer();
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::ensure_capacity(size_type& _Tail_InOut)
{
	size_type h = Head;
	size_type count = Tail - h;
	if (count >= Mask)
	{
		const size_type len = Mask + 1;
		const size_type new_len = len << 1;
		pointer pNewCircularBuffer = Allocator.allocate(new_len);

		for (size_type i = 0; i < len; i++)
			::new (&pNewCircularBuffer[i]) value_type(std::move(pCircularBuffer[(i + h) & Mask]));

		for (size_type i = len; i < (len << 1); i++)
			::new (&pNewCircularBuffer[i]) value_type();

		destroy_buffer();
		pCircularBuffer = pNewCircularBuffer;
		Head = 0;
		Tail = _Tail_InOut = count;
		Mask = (Mask << 1) | 1;
	}
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::internal_push(const_reference _Element, size_type _Tail)
{
	pCircularBuffer[_Tail & Mask] = _Element;
	Tail = _Tail + 1;
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::internal_push(value_type&& _Element, size_type _Tail)
{
	pCircularBuffer[_Tail & Mask] = std::move(_Element);
	Tail = _Tail + 1;
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::internal_push_safe(const_reference _Element, size_type _Tail)
{
	ensure_capacity(_Tail);
	internal_push(_Element, _Tail);
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::internal_push_safe(value_type&& _Element, size_type _Tail)
{
	ensure_capacity(_Tail);
	internal_push(std::move(_Element), _Tail);
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::push(const_reference _Element) threadsafe
{
	oConcurrency::unique_lock<oConcurrency::timed_mutex> Lock(Mutex);
	size_type t = Tail;
	oThreadsafe(this)->internal_push(_Element, t);
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::push(value_type&& _Element) threadsafe
{
	oConcurrency::unique_lock<oConcurrency::timed_mutex> Lock(Mutex);
	size_type t = Tail;
	oThreadsafe(this)->internal_push(std::move(_Element), t);
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::push_local(const_reference _Element)
{
	size_type t = Tail;
	if (t < Head + Mask)
		internal_push(_Element, t);
	else
	{
		oConcurrency::unique_lock<oConcurrency::timed_mutex> Lock(Mutex);
		internal_push_safe(_Element, t);
	}
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::push_local(value_type&& _Element)
{
	size_type t = Tail;
	if (t < Head + Mask)
		internal_push(std::move(_Element), t);
	else
	{
		oConcurrency::unique_lock<oConcurrency::timed_mutex> Lock(Mutex);
		internal_push_safe(std::move(_Element), t);
	}
}

template<typename T, typename Alloc>
bool concurrent_worklist<T, Alloc>::try_pop_internal(reference _Element, size_type _Tail)
{
	if (Head <= _Tail)
	{
		_Element = std::move(pCircularBuffer[_Tail & Mask]);
		return true;
	}

	Tail = _Tail + 1;
	return false;
}

template<typename T, typename Alloc>
bool concurrent_worklist<T, Alloc>::try_pop(reference _Element) threadsafe
{
	oConcurrency::unique_lock<oConcurrency::timed_mutex> Lock(Mutex);
	if (Head >= Tail)
		return false;
	return oThreadsafe(this)->try_pop_internal(_Element, --Tail);
}

template<typename T, typename Alloc>
bool concurrent_worklist<T, Alloc>::try_pop_local(reference _Element)
{
	size_type t = Tail;
	if (Head >= t)
		return false;

	t--;
	oStd::atomic_exchange(&Tail, t);

	if (Head <= t)
	{
		_Element = pCircularBuffer[t & Mask];
		return true;
	}

	oConcurrency::unique_lock<oConcurrency::timed_mutex> Lock(Mutex);
	return try_pop_internal(_Element, t);
}

template<typename T, typename Alloc>
void concurrent_worklist<T, Alloc>::pop_local(reference _Element)
{
	while (!try_pop_local(_Element));
}

template<typename T, typename Alloc>
bool concurrent_worklist<T, Alloc>::try_steal_internal(reference _Element, bool _TryLockResult) threadsafe
{
	bool stolen = _TryLockResult;
	if (stolen)
	{
		size_type h = Head;
		oStd::atomic_exchange(&Head, h + 1);
		if (h < Tail)
			_Element = pCircularBuffer[h & Mask];

		else
		{
			Head = h;
			stolen = false;
		}

		Mutex.unlock();
	}
	return stolen;
}

template<typename T, typename Alloc>
bool concurrent_worklist<T, Alloc>::try_steal(reference _Element) threadsafe
{
	return try_steal_internal(_Element, Mutex.try_lock());
}

template<typename T, typename Alloc>
template<typename Rep, typename Period>
bool concurrent_worklist<T, Alloc>::try_steal_for(reference _Element, const oStd::chrono::duration<Rep, Period>& _TimeoutDuration) threadsafe
{
	return try_steal_internal(_Element, Mutex.try_lock_for(_TimeoutDuration));
}

template<typename T, typename Alloc>
bool concurrent_worklist<T, Alloc>::empty() const threadsafe
{
	return Head >= Tail;
}

template<typename T, typename Alloc>
typename concurrent_worklist<T, Alloc>::size_type concurrent_worklist<T, Alloc>::size() const
{
	return Tail - Head;
}

} // namespace oConcurrency

#endif
