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
// A thread-safe queue (FIFO) that uses atomics to ensure concurrency. This 
// implementation is based on The Maged Michael/Micael Scott paper cited below.
// I have benchmarked this against an implementation of the Optimistic FIFO by
// Edya Ladan-Mozes/Nir Shavit as well as tbb's concurrent_queue and concrt's
// concurrent_queue and found that in the types of situations I use queues, that
// the MS-queue remains the fastest. Typically my benchmark cases allow for
// an #ifdef exchange of queue types to test new implementations and benchmark,
// so it is as close to apples to apples as I can get. The main benchmark case
// I have is a single work queue threadpool, so contention on the list is high.
// It performs within 10% (sometimes slower, sometimes faster) of Window's
// threadpool, so I guess that one is implemented the same way and just 
// optimized a bit better. NOTE: This threadpool is for hammering a concurrent
// queue and for being a control case in benchmarks of queues and of other 
// systems. When a real threadpool is required our production code currently
// uses TBB which is a full 4x to 5x faster than Windows or the custom 
// threadpool.
#pragma once
#ifndef oConcurrentQueue_h
#define oConcurrentQueue_h

#include <oBasis/oAssert.h>
#include <oBasis/oBlockAllocatorGrowable.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oMacros.h>
#include <oBasis/oTaggedPointer.h>

template<typename T>
class oConcurrentQueue
{
	/** <citation
		usage="Paper" 
		reason="The MS queue is often used as the benchmark for other concurrent queue algorithms, so here is an implementation to use to compare such claims." 
		author="Maged M. Michael and Michael L. Scott"
		description="http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html"
		modifications="Modified to support types with dtors."
	/>*/

	struct node_t
	{
		node_t(const T& _Element)
			: next(nullptr, 0), value(_Element)
		{}

		oTaggedPointer<node_t> next;
		T value;
		oStd::atomic_flag Flag;

		// A two-step trivial ref count. In try_pop, we work around the race 
		// condtion described in the original paper for non-trivial destructors by
		// flagging which of the two conditions/code paths should be allowed to free
		// the memory, so calling this query is destructive (thus the non-constness).
		bool ShouldDeallocate() threadsafe { return Flag.test_and_set(); }
	};

	typedef oTaggedPointer<node_t> pointer_t;

	oStringS DebugName;
	pointer_t Head;
	pointer_t Tail;
	oBlockAllocatorGrowableT<node_t> Pool;

	void Initialize()
	{
		node_t* n = Pool.Create(T());
		n->ShouldDeallocate(); // There's no potential for double-freeing here, so set it up for immediate deallocation in try_pop code
		Head = Tail = pointer_t(n, 0);
	}

public:
	static const size_t DEFAULT_ELEMENT_COUNT = 100000;
	static const size_t DEFAULT_ELEMENT_POOL_SIZE = oMB(4);

	typedef long long difference_type;
	typedef const T& const_reference;
	typedef T& reference;
	typedef size_t size_type;
	typedef T value_type;

	//oBlockAllocatorGrowableT(size_t _NumBlocksPerChunk, oFUNCTION<void*(size_t _Size)> _PlatformAllocate = malloc, oFUNCTION<void(void* _Pointer)> _PlatformDeallocate = free)

	oConcurrentQueue(const char* _DebugName, size_t _ReserveElementCount = DEFAULT_ELEMENT_COUNT)
		: DebugName(_DebugName)
		, Pool(oBlockAllocatorGrowableT<node_t>::GetMaxNumBlocksPerChunk())
	{
		Pool.GrowByElements(_ReserveElementCount);
		Initialize();
	}

	~oConcurrentQueue()
	{
		oASSERT(empty(), "oConcurrentQueueMS %s not empty", debug_name());
		node_t* n = Head.ptr();
		Head = Tail = pointer_t(0, 0);
		Pool.Deallocate(n); // because the head value is destroyed in try_pop, don't double-destroy here.
	}

	//allocator_type get_allocator() const { return Pool.get_allocator(); }

	// attempt to push the specified element onto the end of the queue. This can
	// fail if the allocator runs out of memory.
	bool try_push(const value_type& _Element) threadsafe
	{
		node_t* n = Pool.Create(_Element);
		if (!n) return false;
		pointer_t t, next;
		while (1)
		{
			t = thread_cast<pointer_t&>(Tail);
			next = t.ptr()->next;
			if (t == Tail)
			{
				if (!next.ptr())
				{
					if (pointer_t::CAS(&thread_cast<pointer_t&>(Tail).ptr()->next, pointer_t(n, next.tag()+1), next))
						break;
				}

				else
					pointer_t::CAS(&Tail, pointer_t(next.ptr(), t.tag()+1), t);
			}
		}

		pointer_t::CAS(&Tail, pointer_t(n, t.tag()+1), t);
		return true;
	}

	// Blocks until the specified element can be pushed into the queue
	void push(const value_type& _Element) threadsafe { while(!try_push(_Element)); }

	// Returns false if the queue is empty
	bool try_pop(T& _Element) threadsafe
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
						_Element = next.ptr()->value;
						next.ptr()->value.~T();
						if (next.ptr()->ShouldDeallocate())
							Pool.Deallocate(next.ptr());

						break;
					}
				}
			}
		}

		if (h.ptr()->ShouldDeallocate())
			Pool.Deallocate(h.ptr()); // dtor called explicitly above, so just deallocate
		return true;
	}

	// Returns true if no elements are in the queue
	bool empty() const threadsafe { return thread_cast<pointer_t&>(Head) == thread_cast<pointer_t&>(Tail); }

	// This API is not considered threadsafe in concrt documentation, though I see
	// no reason that with an implementation like below it wouldn't be as 
	// threadsafe as an empty test. To conform and ensure there are no 
	// cast-it-away bugs when switching queue types, keep this API non-threadsafe
	void clear() { T e; while (try_pop(e)); }

	// Returns the string specified in the ctor
	inline const char* debug_name() const threadsafe { return DebugName; }

	// returns the maximum number of elements that can be pushed into the queue
	inline size_type capacity() const threadsafe { return oNumericLimits<size_type>::GetMax(); }

	// returns the number of elements currently in the queue. THIS IS SLOW! In 
	// order to ensure concurrency, no separate count for size is kept, so this 
	// traverses the entire queue counting the number of elements. THis is both
	// slow and not threadsafe, but is useful in debugging situations.

	// actually this is not implemented currently. There is no GetSize function on the block allocator.
	inline size_type unsafe_size() const { return Pool.GetSize(); } // SLOW!
};

#endif
