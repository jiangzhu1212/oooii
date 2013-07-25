/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// A thread-safe queue (FIFO) that uses atomics to ensure concurrency. This 
// implementation is based on The Maged Michael/Michael Scott paper cited below.
// I have benchmarked this against an implementation of the Optimistic FIFO by
// Edya Ladan-Mozes/Nir Shavit as well as tbb's concurrent_queue and concrt's
// concurrent_queue and found that in the types of situations I use queues, that
// the MS-queue remains the fastest. Typically my benchmark cases allow for
// an #ifdef exchange of queue types to test new implementations and benchmark,
// so it is as close to apples to apples as I can get. The main benchmark case
// I have is a single work queue thread pool, so contention on the list is high.
// It performs within 10% (sometimes slower, sometimes faster) of Window's
// thread pool, so I guess that one is implemented the same way and just 
// optimized a bit better. NOTE: This thread pool is for hammering a concurrent
// queue and for being a control case in benchmarks of queues and of other 
// systems. When a real thread pool is required our production code currently
// uses TBB which is a full 4x to 5x faster than Windows or the custom 
// thread pool.
#pragma once
#ifndef oConcurrency_concurrent_queue_h
#define oConcurrency_concurrent_queue_h

#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/block_allocator.h>
#include <oConcurrency/concurrent_queue_base.h>
#include <oConcurrency/tagged_pointer.h>

namespace oConcurrency {

template<typename T>
class concurrent_queue : public concurrent_queue_base<T, concurrent_queue<T>>
{
	/** <citation
		usage="Paper" 
		reason="The MS queue is often used as the benchmark for other concurrent queue algorithms, so here is an implementation to use to compare such claims." 
		author="Maged M. Michael and Michael L. Scott"
		description="http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html"
		modifications="Modified to support types with dtors."
	/>*/

public:
	oDEFINE_CONCURRENT_QUEUE_TYPE(T, size_t);

	// This implementation auto-grows capacity in a threadsafe manner if the 
	// initial capacity is inadequate.
	concurrent_queue();
	~concurrent_queue();

	// Push an element into the queue.
	void push(const_reference _Element) threadsafe;
	void push(value_type&& _Element) threadsafe;

	// Returns false if the queue is empty
	bool try_pop(reference _Element) threadsafe;

	// Returns true if no elements are in the queue
	bool empty() const threadsafe;

	// SLOW! Returns the number of elements in the queue. Client code should not 
	// be reliant on this value and the API is included only for debugging and 
	// testing purposes. It is not threadsafe.
	size_type size() const;

private:
	// alignment is required so that pointers to node_t's are at least 8-bytes.
	// This allows tagged_pointer to use the bottom 3-bits for its tag.
	#ifdef o32BIT
		#pragma warning(disable:4324) // structure was padded due to __declspec(align())
		oALIGN(8) 
	#endif
	struct node_t
	{
		node_t(const T& _Element)
			: next(nullptr, 0), value(_Element)
		{}

		node_t(T&& _Element)
			: next(nullptr, 0), value(std::move(_Element))
		{}

		tagged_pointer<node_t> next;
		value_type value;
		oStd::atomic_flag Flag;

		// A two-step trivial ref count. In try_pop, the race condition described in 
		// the original paper for non-trivial destructors is addressed by flagging 
		// which of the two conditions/code paths should be allowed to free the 
		// memory, so calling this query is destructive (thus the non-constness).
		bool ShouldDeallocate() threadsafe { return Flag.test_and_set(); }
	};
	#ifdef o32BIT
		#pragma warning(default:4324) // structure was padded due to __declspec(align())
	#endif

	typedef tagged_pointer<node_t> pointer_t;

	oCACHE_ALIGNED(pointer_t Head);
	oCACHE_ALIGNED(pointer_t Tail);
	oCACHE_ALIGNED(block_allocator_t<node_t> Pool);
	
	void internal_push(node_t* _pNode) threadsafe;
};

template<typename T>
concurrent_queue<T>::concurrent_queue()
{
	node_t* n = Pool.construct(T());
	
	// There's no potential for double-freeing here, so set it up for immediate 
	// deallocation in try_pop code.
	n->ShouldDeallocate();
	Head = Tail = pointer_t(n, 0);
}

template<typename T>
concurrent_queue<T>::~concurrent_queue()
{
	if (!empty())
		throw container_error(container_errc::not_empty);

	node_t* n = Head.ptr();
	Head = Tail = pointer_t(0, 0);
	
	// because the head value is destroyed in try_pop, don't double-destroy here.
	Pool.deallocate(n);
}

template<typename T>
void concurrent_queue<T>::internal_push(node_t* _pNode) threadsafe
{
	if (!_pNode) throw std::bad_alloc();
	pointer_t t, next;
	while (1)
	{
		t = thread_cast<pointer_t&>(Tail);
		next = t.ptr()->next;
		if (t == Tail)
		{
			if (!next.ptr())
			{
				if (pointer_t::CAS(&thread_cast<pointer_t&>(Tail).ptr()->next, pointer_t(_pNode, next.tag()+1), next))
					break;
			}

			else
				pointer_t::CAS(&Tail, pointer_t(next.ptr(), t.tag()+1), t);
		}
	}

	pointer_t::CAS(&Tail, pointer_t(_pNode, t.tag()+1), t);
}

template<typename T>
void concurrent_queue<T>::push(const_reference _Element) threadsafe
{
	internal_push(Pool.construct(_Element));
}

template<typename T>
void concurrent_queue<T>::push(value_type&& _Element) threadsafe
{
	internal_push(Pool.construct(std::move(_Element)));
}

template<typename T>
bool concurrent_queue<T>::try_pop(reference _Element) threadsafe
{
	pointer_t h, t, next;
	while (1)
	{
		h = thread_cast<pointer_t&>(Head);
		t = thread_cast<pointer_t&>(Tail);
		next = h.ptr()->next;
		if (h == Head)
		{
			if (h.ptr() == t.ptr())
			{
				if (!next.ptr()) return false;
				pointer_t::CAS(&Tail, pointer_t(next.ptr(), t.tag()+1), t);
			}

			else
			{
				if (pointer_t::CAS(&Head, pointer_t(next.ptr(), h.tag()+1), h))
				{
					// Yes, the paper says the assignment should be outside the CAS,
					// but we've worked around that so we can also call the destructor
					// here protected by the above CAS by flagging when the destructor
					// is done and the memory can truly be reclaimed, so the 
					// ShouldDeallocate() calls have been added to either clean up the
					// memory immediately now that the CAS has made next the dummy Head,
					// or clean it up lazily later at the bottom. Either way, do it only 
					// once.
					_Element = std::move(next.ptr()->value);
					next.ptr()->value.~T();
					if (next.ptr()->ShouldDeallocate())
						Pool.deallocate(next.ptr());

					break;
				}
			}
		}
	}

	if (h.ptr()->ShouldDeallocate())
		Pool.deallocate(h.ptr()); // dtor called explicitly above so just deallocate
	return true;
}

template<typename T>
bool concurrent_queue<T>::empty() const threadsafe
{
	return Head == Tail;
}

template<typename T>
typename concurrent_queue<T>::size_type concurrent_queue<T>::size() const
{
	// There's a dummy/extra node retained by this queue, so don't count that one.
	return Pool.count_allocated() - 1;
}

} // namespace oConcurrency

#endif