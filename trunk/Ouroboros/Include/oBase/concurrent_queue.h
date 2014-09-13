// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Fine-grained concurrency FIFO queue based on:
// http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
// Use a concurrent fixed block allocator for concurrent_queue_nodes for 
// best performance. This also ensures concurrency of non-trivial 
// destructors.

#pragma once
#include <oConcurrency/concurrency.h>
#include <oBase/concurrent_growable_object_pool.h>
#include <oConcurrency/tagged_pointer.h>
#include <atomic>
#include <cstdint>
#include <stdexcept>

namespace ouro {

template<typename T>
struct oALIGNAS(oTAGGED_POINTER_ALIGNMENT) concurrent_queue_node
{
  typedef T value_type;
  typedef tagged_pointer<concurrent_queue_node<value_type>> pointer_type;
  
  concurrent_queue_node(const value_type& v) : next(nullptr, 0), value(v) { flag.clear(); }
  concurrent_queue_node(value_type&& v) : next(nullptr, 0), value(std::move(v)) { flag.clear(); }

  pointer_type next;
  value_type value;
  
	// A two-step trivial ref count. In try_pop, the race condition described in 
	// the original paper for non-trivial destructors is addressed by flagging 
	// which of the two conditions/code paths should be allowed to free the 
	// memory, so calling this query is destructive (thus the non-constness).
	bool should_deallocate() { return flag.test_and_set(); }

private:
	std::atomic_flag flag;
};

template<typename T>
class concurrent_queue
{
public:
	typedef uint32_t size_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
  typedef concurrent_queue_node<T> node_type;
  typedef typename node_type::pointer_type pointer_type;

	// This implementation auto-grows capacity in a threadsafe manner if the 
	// initial capacity is inadequate.
	concurrent_queue();
	~concurrent_queue();

	// Push an element into the queue.
	void push(const_reference val);
	void push(value_type&& val);

	// Returns false if the queue is empty
	bool try_pop(reference val);

	// Spins until an element can be popped from the queue
	void pop(reference val);

	// Spins until the queue is empty
	void clear();

	// Returns true if no elements are in the queue
	bool empty() const;

	// SLOW! Returns the number of elements in the queue. Client code should not 
	// be reliant on this value and the API is included only for debugging and 
	// testing purposes. It is not threadsafe.
	size_type size() const;

private:
	// alignment is required so that pointers to node_type's are at least 8-bytes.
	// This allows tagged_pointer to use the bottom 3-bits for its tag.
	
	oALIGNAS(oCACHE_LINE_SIZE) pointer_type head;
	oALIGNAS(oCACHE_LINE_SIZE) pointer_type tail;
	oALIGNAS(oCACHE_LINE_SIZE) concurrent_growable_object_pool<node_type> Pool;
	
	void internal_push(node_type* n);
};

template<typename T>
concurrent_queue<T>::concurrent_queue()
	: Pool(concurrent_growable_pool::max_blocks_per_chunk, oTAGGED_POINTER_ALIGNMENT)
{
	node_type* n = Pool.create(T());
	
	// There's no potential for double-freeing here, so set it up for immediate 
	// deallocation in try_pop code.
	n->should_deallocate();
	head = tail = pointer_type(n, 0);
}

template<typename T>
concurrent_queue<T>::~concurrent_queue()
{
	if (!empty())
		throw std::length_error("container not empty");

	node_type* n = head.ptr();
	head = tail = pointer_type(nullptr, 0);
	
	// because the head value is destroyed in try_pop, don't double-destroy here.
	Pool.deallocate(n);
}

template<typename T>
void concurrent_queue<T>::internal_push(node_type* n)
{
	if (!n) throw std::bad_alloc();
	pointer_type t, next;
	while (1)
	{
		t = tail;
		next = t.ptr()->next;
		if (t == tail)
		{
			if (!next.ptr())
			{
				if (tail.ptr()->next.cas(next, pointer_type(n, next.tag()+1)))
					break;
			}

			else
				tail.cas(t, pointer_type(next.ptr(), t.tag()+1));
		}
	}

	tail.cas(t, pointer_type(n, t.tag()+1));
}

template<typename T>
void concurrent_queue<T>::push(const_reference val)
{
	internal_push(Pool.create(val));
}

template<typename T>
void concurrent_queue<T>::push(value_type&& val)
{
	internal_push(Pool.create(std::move(val)));
}

template<typename T>
bool concurrent_queue<T>::try_pop(reference val)
{
	pointer_type h, t, next;
	while (1)
	{
		h = head;
		t = tail;
		next = h.ptr()->next;
		if (h == head)
		{
			if (h.ptr() == t.ptr())
			{
				if (!next.ptr()) return false;
				tail.cas(t, pointer_type(next.ptr(), t.tag()+1));
			}

			else
			{
				if (head.cas(h, pointer_type(next.ptr(), h.tag()+1)))
				{
					// Yes, the paper says the assignment should be outside the CAS,
					// but we've worked around that so we can also call the destructor
					// here protected by the above CAS by flagging when the destructor
					// is done and the memory can truly be reclaimed, so the 
					// should_deallocate() calls have been added to either clean up the
					// memory immediately now that the CAS has made next the dummy head,
					// or clean it up lazily later at the bottom. Either way, do it only 
					// once.
					val = std::move(next.ptr()->value);
					next.ptr()->value.~T();
					if (next.ptr()->should_deallocate())
						Pool.deallocate(next.ptr());

					break;
				}
			}
		}
	}

	if (h.ptr()->should_deallocate())
		Pool.deallocate(h.ptr()); // dtor called explicitly above so just deallocate
	return true;
}

template<typename T>
void concurrent_queue<T>::pop(reference val)
{
	while (!try_pop(val));
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
	return head == tail;
}

template<typename T>
typename concurrent_queue<T>::size_type concurrent_queue<T>::size() const
{
	// There's a dummy/extra node retained by this queue, so don't count that one.
	return Pool.size() - 1;
}

}
