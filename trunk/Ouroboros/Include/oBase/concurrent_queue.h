/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// A thread-safe queue (FIFO) that uses atomics to ensure concurrency. This 
// implementation is based on The Maged Michael/Michael Scott paper cited below.
#pragma once
#ifndef oBase_concurrent_queue_h
#define oBase_concurrent_queue_h

#include <oBase/concurrency.h>
#include <oBase/concurrent_growable_object_pool.h>
#include <oBase/tagged_pointer.h>
#include <atomic>

namespace ouro {

template<typename T>
class concurrent_queue
{
	/** <citation
		usage="Paper" 
		reason="The MS queue is often used as the benchmark for other concurrent queue algorithms, so here is an implementation to use to compare such claims." 
		author="Maged M. Michael and Michael L. Scott"
		description="http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html"
		modifications="Modified to support types with dtors."
	/>*/

public:
	typedef size_t size_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;

	// This implementation auto-grows capacity in a threadsafe manner if the 
	// initial capacity is inadequate.
	concurrent_queue();
	~concurrent_queue();

	// Push an element into the queue.
	void push(const_reference _Element);
	void push(value_type&& _Element);

	// Returns false if the queue is empty
	bool try_pop(reference _Element);

	// Spins until an element can be popped from the queue
	void pop(reference _Element);

	// Spins until the queue is empty
	void clear();

	// Returns true if no elements are in the queue
	bool empty() const;

	// SLOW! Returns the number of elements in the queue. Client code should not 
	// be reliant on this value and the API is included only for debugging and 
	// testing purposes. It is not threadsafe.
	size_type size() const;

private:
	// alignment is required so that pointers to node_t's are at least 8-bytes.
	// This allows tagged_pointer to use the bottom 3-bits for its tag.
	
	oALIGNAS(oTAGGED_POINTER_ALIGNMENT) struct node_t
	{
		node_t(const T& _Element)
			: next(nullptr, 0)
			, value(_Element)
		{ flag.clear(); }

		node_t(T&& _Element)
			: next(nullptr, 0)
			, value(std::move(_Element))
		{ flag.clear(); }

		tagged_pointer<node_t> next;
		value_type value;

		// A two-step trivial ref count. In try_pop, the race condition described in 
		// the original paper for non-trivial destructors is addressed by flagging 
		// which of the two conditions/code paths should be allowed to free the 
		// memory, so calling this query is destructive (thus the non-constness).
		bool should_deallocate() { return flag.test_and_set(); }

	private:
		std::atomic_flag flag;
	};

	typedef tagged_pointer<node_t> pointer_t;

	oALIGNAS(oCACHE_LINE_SIZE) pointer_t Head;
	oALIGNAS(oCACHE_LINE_SIZE) pointer_t Tail;
	oALIGNAS(oCACHE_LINE_SIZE) concurrent_growable_object_pool<node_t> Pool;
	
	void internal_push(node_t* _pNode);
};

template<typename T>
concurrent_queue<T>::concurrent_queue()
	: Pool(concurrent_growable_pool::max_blocks_per_chunk, oTAGGED_POINTER_ALIGNMENT)
{
	node_t* n = Pool.create(T());
	
	// There's no potential for double-freeing here, so set it up for immediate 
	// deallocation in try_pop code.
	n->should_deallocate();
	Head = Tail = pointer_t(n, 0);
}

template<typename T>
concurrent_queue<T>::~concurrent_queue()
{
	if (!empty())
		throw std::length_error("container not empty");

	node_t* n = Head.pointer();
	Head = Tail = pointer_t(0, 0);
	
	// because the head value is destroyed in try_pop, don't double-destroy here.
	Pool.deallocate(n);
}

template<typename T>
void concurrent_queue<T>::internal_push(node_t* _pNode)
{
	if (!_pNode) throw std::bad_alloc();
	pointer_t t, next;
	while (1)
	{
		t = Tail;
		next = t.pointer()->next;
		if (t == Tail)
		{
			if (!next.pointer())
			{
				if (Tail.pointer()->next.cas(next, pointer_t(_pNode, next.tag()+1)))
					break;
			}

			else
				Tail.cas(t, pointer_t(next.pointer(), t.tag()+1));
		}
	}

	Tail.cas(t, pointer_t(_pNode, t.tag()+1));
}

template<typename T>
void concurrent_queue<T>::push(const_reference _Element)
{
	internal_push(Pool.create(_Element));
}

template<typename T>
void concurrent_queue<T>::push(value_type&& _Element)
{
	internal_push(Pool.create(std::move(_Element)));
}

template<typename T>
bool concurrent_queue<T>::try_pop(reference _Element)
{
	pointer_t h, t, next;
	while (1)
	{
		h = Head;
		t = Tail;
		next = h.pointer()->next;
		if (h == Head)
		{
			if (h.pointer() == t.pointer())
			{
				if (!next.pointer()) return false;
				Tail.cas(t, pointer_t(next.pointer(), t.tag()+1));
			}

			else
			{
				if (Head.cas(h, pointer_t(next.pointer(), h.tag()+1)))
				{
					// Yes, the paper says the assignment should be outside the CAS,
					// but we've worked around that so we can also call the destructor
					// here protected by the above CAS by flagging when the destructor
					// is done and the memory can truly be reclaimed, so the 
					// should_deallocate() calls have been added to either clean up the
					// memory immediately now that the CAS has made next the dummy Head,
					// or clean it up lazily later at the bottom. Either way, do it only 
					// once.
					_Element = std::move(next.pointer()->value);
					next.pointer()->value.~T();
					if (next.pointer()->should_deallocate())
						Pool.deallocate(next.pointer());

					break;
				}
			}
		}
	}

	if (h.pointer()->should_deallocate())
		Pool.deallocate(h.pointer()); // dtor called explicitly above so just deallocate
	return true;
}

template<typename T>
void concurrent_queue<T>::pop(reference _Element)
{
	while (!try_pop(_Element));
}

template<typename T>
void concurrent_queue<T>::clear()
{
	value_type e;
	while (try_pop(e));
}

template<typename T>
bool concurrent_queue<T>::empty() const
{
	return Head == Tail;
}

template<typename T>
typename concurrent_queue<T>::size_type concurrent_queue<T>::size() const
{
	// There's a dummy/extra node retained by this queue, so don't count that one.
	return Pool.size() - 1;
}

}

#endif
