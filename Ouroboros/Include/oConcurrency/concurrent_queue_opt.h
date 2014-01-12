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
// A thread-safe queue (FIFO) that uses atomics to ensure concurrency. This 
// implementation is advertised as more efficient than the standard MS queue
// (oConcurrency::concurrent_queue's algorithm) because it goes to great lengths to avoid 
// atomic activities, optimistically assuming contention does not happen and
// going back to fix any confusion if it does. Read more about it in the link
// in the citation below.
#pragma once
#ifndef oConcurrency_concurrent_queue_opt_h
#define oConcurrency_concurrent_queue_opt_h

#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/block_allocator.h>
#include <oConcurrency/tagged_pointer.h>

namespace oConcurrency {

template<typename T>
class concurrent_queue_opt
{
	/** <citation
		usage="Paper" 
		reason="Authors claim this is significantly more efficient than the MS-queue." 
		author="Edya Ladan-Mozes and Nir Shavit"
		description="http://people.csail.mit.edu/edya/publications/OptimisticFIFOQueue-journal.pdf"
		modifications="Modified to support types with dtors."
	/>*/

public:
	typedef size_t size_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;

	concurrent_queue_opt();
	~concurrent_queue_opt();

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
	// testing purposes. It is not.
	size_type size() const;

private:

	struct node_t;

	struct node_base_t
	{
		node_base_t(const T& _Element) : prev(nullptr, 0), next(nullptr, 0), value(_Element) {}
		tagged_pointer<node_t> prev;
		tagged_pointer<node_t> next;
		T value;
	};

	struct node_t : node_base_t
	{
		node_t(const T& _Element) : node_base_t(_Element) {}
		char padding[tagged_pointer<node_t>::required_alignment-(sizeof(node_base_t)%tagged_pointer<node_t>::required_alignment)];
	};
	static_assert((sizeof(node_t) % tagged_pointer<node_t>::required_alignment) == 0, "node_t not properly aligned");

	typedef tagged_pointer<node_t> pointer_t;

	oCACHE_ALIGNED(pointer_t Head);
	oCACHE_ALIGNED(pointer_t Tail);
	oCACHE_ALIGNED(block_allocator_t<node_t> Pool);

	void internal_push(node_t* _pNode);
	void fix_list(pointer_t _Tail, pointer_t _Head);
};

// @tony: TODO: Respect _Capacity
template<typename T>
concurrent_queue_opt<T>::concurrent_queue_opt()
{
	Head = Tail = pointer_t(Pool.construct(T()), 0);
}

template<typename T>
concurrent_queue_opt<T>::~concurrent_queue_opt()
{
	if (!empty())
		throw std::length_error("container not empty");

	node_t* n = Head.ptr();
	Head = Tail = pointer_t(nullptr, 0);

	// Use deallocate, not destroy because the dummy value should not be valid to
	// be deconstructed.
	Pool.deallocate(n);
}

template<typename T>
void concurrent_queue_opt<T>::internal_push(node_t* _pNode)
{
	if (!_pNode) throw std::bad_alloc();
	pointer_t t;
	while (true)
	{
		t = thread_cast<pointer_t&>(Tail);
		_pNode->next = pointer_t(t.ptr(), t.tag()+1);
		if (pointer_t::CAS(&Tail, pointer_t(_pNode, t.tag()+1), t))
		{
			// Seemingly during this time we have a new Tail, but Tail->next's prev 
			// pointer is still uninitialized (that happens below) but any time 
			// between the CAS and this next line a try_pop could try to deference
			// its firstNodePrev, which might be pointing to the same thing as t here.
			t.ptr()->prev = pointer_t(_pNode, t.tag());
			break;
		}
	}
}

template<typename T>
void concurrent_queue_opt<T>::push(const_reference _Element)
{
	internal_push(Pool.construct(_Element));
}

template<typename T>
void concurrent_queue_opt<T>::push(value_type&& _Element)
{
	internal_push(Pool.construct(std::move(_Element)));
}

template<typename T>
void concurrent_queue_opt<T>::fix_list(pointer_t _Tail, pointer_t _Head)
{
	pointer_t curNode, curNodeNext;
	curNode = thread_cast<pointer_t&>(_Tail);
	while ((_Head == Head) && (curNode != _Head))
	{
		curNodeNext = curNode.ptr()->next;
		curNodeNext.ptr()->prev = pointer_t(curNode.ptr(), curNode.tag()-1);
		curNode = pointer_t(curNodeNext.ptr(), curNode.tag()-1);
	}
}

template<typename T>
bool concurrent_queue_opt<T>::try_pop(reference _Element)
{
	pointer_t t, h, firstNodePrev;
	while (true)
	{
		h = thread_cast<pointer_t&>(Head);
		t = thread_cast<pointer_t&>(Tail);
		firstNodePrev = h.ptr()->prev;
		if (h == Head)
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

				_Element = std::move(firstNodePrev.ptr()->value);
				if (pointer_t::CAS(&Head, pointer_t(firstNodePrev.ptr(), h.tag()+1), h))
				{
					Pool.destroy(h.ptr());
					return true;
				}
			}

			else
				break;
		}
	}

	return false;
}

template<typename T>
void concurrent_queue_opt<T>::pop(reference _Element)
{
	while (!try_pop(_Element));
}

template<typename T>
void concurrent_queue_opt<T>::clear()
{
	value_type e;
	while (try_pop(e));
}

template<typename T>
bool concurrent_queue_opt<T>::empty() const
{
	return Head == Tail;
}

template<typename T>
typename concurrent_queue_opt<T>::size_type concurrent_queue_opt<T>::size() const
{
	// There's a dummy/extra node retained by this queue, so don't count that one.
	return empty() ? 0 : (Pool.count_allocated() - 1);
}

} // namespace oConcurrency

#endif
