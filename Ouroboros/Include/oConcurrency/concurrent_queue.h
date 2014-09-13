// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Fine-grained concurrent FIFO queue based on:
// http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
// Use a concurrent fixed block allocator for concurrent_queue_nodes for 
// best performance. This also ensures concurrency of non-trivial 
// destructors.

#pragma once
#include <oCompiler.h>
#include <oConcurrency/tagged_pointer.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

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

template<typename T, typename Alloc = std::allocator<concurrent_queue_node<T>>>
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
	typedef Alloc allocator_type;

	// This implementation auto-grows capacity in a threadsafe manner if the 
	// initial capacity is inadequate.
	concurrent_queue(const allocator_type& a = allocator_type());
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

	// Walks the nodes in a non-concurrent manner and returns the count (not 
	// including the sentinel node). This should be used only for debugging.
	size_type size() const;

private:
	// alignment is required so that pointers to node_type's are at least 8-bytes.
	// This allows tagged_pointer to use the bottom 3-bits for its tag.
	
	oALIGNAS(oCACHE_LINE_SIZE) pointer_type head;
	oALIGNAS(oCACHE_LINE_SIZE) pointer_type tail;
	allocator_type alloc;

	void internal_init(std::false_type);
	void internal_init(std::true_type);

	void internal_push(node_type* n);

	bool internal_try_pop(reference val, std::false_type);
	bool internal_try_pop(reference val, std::true_type);
};

template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::internal_init(std::false_type)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, T());
	
	// There's no potential for double-freeing here, so set it up for immediate 
	// deallocation in try_pop code.
	n->should_deallocate();
	head = tail = pointer_type(n, 0);
}

template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::internal_init(std::true_type)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, T());
	head = tail = pointer_type(n, 0);
}

template<typename T, typename Alloc>
concurrent_queue<T, Alloc>::concurrent_queue(const allocator_type& a)
	: alloc(a)
{
	internal_init(std::is_trivially_destructible<T>());
}

template<typename T, typename Alloc>
concurrent_queue<T, Alloc>::~concurrent_queue()
{
	if (!empty())
		throw std::length_error("container not empty");

	node_type* n = head.ptr();
	head = tail = pointer_type(nullptr, 0);
	
	// sentinel value should already be destroyed, so deallocate directly here
	alloc.deallocate(n, sizeof(node_type));
}

template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::internal_push(node_type* n)
{
	if (!n) throw std::bad_alloc();
	pointer_type t, next;
	for (;;)
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

template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::push(const_reference val)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, val);
	internal_push(n);
}

template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::push(value_type&& val)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, std::move(val));
	internal_push(n);
}

template<typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::internal_try_pop(reference val, std::false_type)
{
	pointer_type h, t, next;
	for (;;)
	{
		h = head;
		t = tail;
		next = h.ptr()->next;
		auto nptr = next.ptr();
		if (h == head)
		{
			if (h.ptr() == t.ptr())
			{
				if (!nptr) return false;
				tail.cas(t, pointer_type(nptr, t.tag()+1));
			}

			else
			{
				if (head.cas(h, pointer_type(nptr, h.tag()+1)))
				{
					// Yes, the paper says the assignment should be outside the CAS,
					// but we've worked around that so we can also call the destructor
					// here protected by the above CAS by flagging when the destructor
					// is done and the memory can truly be reclaimed, so the 
					// should_deallocate() calls have been added to either clean up the
					// memory immediately now that the CAS has made next the dummy head,
					// or clean it up lazily later at the bottom. Either way, do it only 
					// once.
					val = std::move(nptr->value);
					nptr->value.~T();
					if (nptr->should_deallocate())
						alloc.deallocate(nptr, sizeof(node_type));
					break;
				}
			}
		}
	}

	auto p = h.ptr();
	if (p->should_deallocate())
		alloc.deallocate(p, sizeof(node_type)); // dtor called explicitly above so just deallocate
	
	return true;
}

template<typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::internal_try_pop(reference val, std::true_type)
{
	pointer_type h, t, next;
	for (;;)
	{
		h = head;
		t = tail;
		next = h.ptr()->next;
		auto nptr = next.ptr();
		if (h == head)
		{
			if (h.ptr() == t.ptr())
			{
				if (!nptr) return false;
				tail.cas(t, pointer_type(nptr, t.tag()+1));
			}

			else
			{
				val = std::move(nptr->value);
				if (head.cas(h, pointer_type(nptr, h.tag()+1)))
					break;
			}
		}
	}

	alloc.deallocate(h.ptr(), sizeof(node_type)); // dtor called explicitly above so just deallocate
	return true;
}

template<typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::try_pop(reference val)
{
	return internal_try_pop(val, std::is_trivially_destructible<T>());
}
	
template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::pop(reference val)
{
	while (!try_pop(val));
}

template<typename T, typename Alloc>
void concurrent_queue<T, Alloc>::clear()
{
	value_type e;
	while (try_pop(e));
}

template<typename T, typename Alloc>
bool concurrent_queue<T, Alloc>::empty() const
{
	return head == tail;
}

template<typename T, typename Alloc>
typename concurrent_queue<T, Alloc>::size_type concurrent_queue<T, Alloc>::size() const
{
	size_type n = 0;
	auto p = head.ptr()->next.ptr();
	while (p)
	{
		n++;
		p = p->next.ptr();
	}
	return n;
}

}
