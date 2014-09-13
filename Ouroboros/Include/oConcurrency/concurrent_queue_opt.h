// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Fine-grained concurrent FIFO queue based on:
// http://people.csail.mit.edu/edya/publications/OptimisticFIFOQueue-journal.pdf
// This is advertised as more efficient than concurrent_queue's algorithm because 
// it goes to great lengths to avoid atomic operations, optimistically assuming 
// contention does not happen and going back to fix any confusion if it does. 
// My benchmarks show marginal if any benefits.

#pragma once
#include <oCompiler.h>
#include <oConcurrency/tagged_pointer.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace ouro {

template<typename T>
struct oALIGNAS(oTAGGED_POINTER_ALIGNMENT) concurrent_queue_opt_node
{
  typedef T value_type;
  typedef tagged_pointer<concurrent_queue_opt_node<value_type>> pointer_type;

	concurrent_queue_opt_node(const value_type& val) : prev(nullptr, 0), next(nullptr, 0), value(val) {}
	concurrent_queue_opt_node(value_type&& val) : prev(nullptr, 0), next(nullptr, 0), value(std::move(val)) {}
	pointer_type prev;
	pointer_type next;
	value_type value;
};

template<typename T, typename Alloc = std::allocator<concurrent_queue_opt_node<T>>>
class concurrent_queue_opt
{
public:
	typedef size_t size_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
  typedef concurrent_queue_opt_node<T> node_type;
  typedef typename node_type::pointer_type pointer_type;
	typedef Alloc allocator_type;

	concurrent_queue_opt(const allocator_type& a = allocator_type());
	~concurrent_queue_opt();

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

	oALIGNAS(oCACHE_LINE_SIZE) pointer_type head;
	oALIGNAS(oCACHE_LINE_SIZE) pointer_type tail;
	allocator_type alloc;

	void internal_push(node_type* n);
	void fix_list(pointer_type _tail, pointer_type _head);
};

template<typename T, typename Alloc>
concurrent_queue_opt<T, Alloc>::concurrent_queue_opt(const allocator_type& a)
	: alloc(a)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, T());
	head = tail = pointer_type(n, 0);
}

template<typename T, typename Alloc>
concurrent_queue_opt<T, Alloc>::~concurrent_queue_opt()
{
	if (!empty())
		throw std::length_error("container not empty");

	node_type* n = head.ptr();
	head = tail = pointer_type(nullptr, 0);

	// sentinel value should already be destroyed, so deallocate directly here
	alloc.deallocate(n, sizeof(node_type));
}

template<typename T, typename Alloc>
void concurrent_queue_opt<T, Alloc>::internal_push(node_type* n)
{
	if (!n) throw std::bad_alloc();
	pointer_type t;
	for (;;)
	{
		t = tail;
		n->next = pointer_type(t.ptr(), t.tag()+1);
		if (tail.cas(t, pointer_type(n, t.tag()+1)))
		{
			// Seemingly during this time we have a new tail, but tail->next's prev 
			// pointer is still uninitialized (that happens below) but any time 
			// between the CAS and this next line a try_pop could try to deference
			// its firstNodePrev, which might be pointing to the same thing as t here.
			t.ptr()->prev = pointer_type(n, t.tag());
			break;
		}
	}
}

template<typename T, typename Alloc>
void concurrent_queue_opt<T, Alloc>::push(const_reference val)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, val);
	internal_push(n);
}

template<typename T, typename Alloc>
void concurrent_queue_opt<T, Alloc>::push(value_type&& val)
{
	node_type* n = alloc.allocate(1);
	alloc.construct(n, std::move(val));
	internal_push(n);
}

template<typename T, typename Alloc>
void concurrent_queue_opt<T, Alloc>::fix_list(pointer_type _tail, pointer_type _head)
{
	pointer_type curNode, curNodeNext;
	curNode = _tail;
	while ((_head == head) && (curNode != _head))
	{
		curNodeNext = curNode.ptr()->next;
		curNodeNext.ptr()->prev = pointer_type(curNode.ptr(), curNode.tag()-1);
		curNode = pointer_type(curNodeNext.ptr(), curNode.tag()-1);
	}
}

template<typename T, typename Alloc>
bool concurrent_queue_opt<T, Alloc>::try_pop(reference val)
{
	pointer_type t, h, firstNodePrev;
	for (;;)
	{
		h = head;
		t = tail;
		firstNodePrev = h.ptr()->prev;
		if (h == head)
		{
			if (t != h)
			{
				// Not in the original paper, but there is a race condition where 
				// push adds a node, but leaves node->next->prev uninitialized for a
				// tick. This only manifests too when firstNodePrev.tag == h.tag, which 
				// is also very rare. If they are not equal, fix_list fixes the issue 
				// (or at least takes long enough that things settle). So here ensure 
				// time is not wasted getting to the end-game only to try to dereference 
				// a nullptr.
				if (firstNodePrev.ptr() == nullptr)
					continue;

				if (firstNodePrev.tag() != h.tag())
				{
					fix_list(t, h);
					continue;
				}

				val = std::move(firstNodePrev.ptr()->value);
				if (head.cas(h, pointer_type(firstNodePrev.ptr(), h.tag()+1)))
				{
					alloc.deallocate(h.ptr(), sizeof(node_type));
					return true;
				}
			}

			else
				break;
		}
	}

	return false;
}

template<typename T, typename Alloc>
void concurrent_queue_opt<T, Alloc>::pop(reference val)
{
	while (!try_pop(val));
}

template<typename T, typename Alloc>
void concurrent_queue_opt<T, Alloc>::clear()
{
	value_type e;
	while (try_pop(e));
}

template<typename T, typename Alloc>
bool concurrent_queue_opt<T, Alloc>::empty() const
{
	return head == tail;
}

template<typename T, typename Alloc>
typename concurrent_queue_opt<T, Alloc>::size_type concurrent_queue_opt<T, Alloc>::size() const
{
	size_type n = 0;
	auto p = head.ptr()->prev.ptr();
	while (p)
	{
		n++;
		p = p->prev.ptr();
	}
	return n;
}

}
