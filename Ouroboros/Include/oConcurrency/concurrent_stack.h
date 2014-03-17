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
// Similar to Microsoft's InterlockedSList, this provides for a concurrent 
// singly linked list whose insertion has the behavior of a stack (LIFO).
// This is an intrusive data structure and expects the stored type to have a 
// member pointer "next". all allocation of nodes is the responsibility of 
// the user because this data structure is often used in very low-level 
// implementations such as allocations themselves.
#ifndef oConcurrency_concurrent_stack_h
#define oConcurrency_concurrent_stack_h

#include <oConcurrency/concurrent_stack_traits.h>
#include <stdexcept>

namespace oConcurrency {

template<typename T, typename traits = concurrent_stack_traits64>
class concurrent_stack
{
public:
	typedef unsigned int size_type;
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	static const size_type capacity = (1 << traits::size_bits) - 1;

	concurrent_stack();
	~concurrent_stack();

	// Push an element onto the stack.
	void push(pointer _pElement);

	// Returns the top of the stack without removing the item from the stack.
	pointer peek() const;

	// If there are no elements, this return nullptr.
	pointer pop();

	// Returns true if no elements are in the stack.
	bool empty() const;

	// Returns the head that is thus a linked list of all items that were in the 
	// stack, leaving the stack empty.
	pointer pop_all();

	// Returns the number of elements in the stack. This is an instantaneous 
	// sampling and thus might not be valid by the very next line of code. Client
	// code should not be reliant on this value and the API is included only for 
	// debugging and testing purposes.
	size_type size() const;

private:
	typedef typename traits::head_type head_t;
	typename traits::atomic_type Head;
	inline bool head_cas(unsigned long long& _Expected, unsigned long long _Desired) { return traits::cas(Head, _Expected, _Desired); }
};

template<typename T, typename traits>
struct is_fifo<concurrent_stack<T, traits>> : std::false_type {};

template<typename T, typename traits>
concurrent_stack<T, traits>::concurrent_stack()
{
	Head = 0;
}

template<typename T, typename traits>
concurrent_stack<T, traits>::~concurrent_stack()
{
	if (!empty())
		throw std::length_error("container not empty");
}

template<typename T, typename traits>
void concurrent_stack<T, traits>::push(pointer _pElement)
{
	head_t New, Old;
	Old.all = Head;
	do 
	{
		if (Old.size >= capacity)
			throw std::overflow_error("concurrent_stack cannot hold any more elements");
		_pElement->next = reinterpret_cast<pointer>(Old.head);
		New.tag = Old.tag + 1;
		New.size = Old.size + 1;
		New.head = reinterpret_cast<uintptr_t>(_pElement);

		if (reinterpret_cast<pointer>(New.head) != _pElement)
			throw std::overflow_error("the specified pointer is too large to be used in conjunction with tagging or other concurrency/ABA solutions");

	} while (!head_cas(Old.all, New.all));
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::pointer concurrent_stack<T, traits>::peek() const
{
	head_t Old; Old.all = Head;
	return reinterpret_cast<pointer>(Old.head);
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::pointer concurrent_stack<T, traits>::pop()
{
	head_t New, Old; Old.all = Head;
	pointer pElement = nullptr;
	do 
	{
		if (!Old.head)
			return nullptr;
		New.tag = Old.tag + 1;
		New.size = Old.size - 1;
		pElement = reinterpret_cast<pointer>(Old.head);
		New.head = reinterpret_cast<uintptr_t>(pElement->next);
	} while (!head_cas(Old.all, New.all));

	return pElement;
}

template<typename T, typename traits>
bool concurrent_stack<T, traits>::empty() const
{
	head_t h; h.all = Head;
	return !h.head;
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::pointer concurrent_stack<T, traits>::pop_all()
{
	pointer pElement = nullptr;
	head_t New, Old;
	New.size = New.head = 0;
	Old.all = Head;
	do 
	{
		New.tag = Old.tag + 1;
		pElement = reinterpret_cast<pointer>(Old.head);
	} while (!head_cas(Old.all, New.all));
	return pElement;
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::size_type concurrent_stack<T, traits>::size() const
{
	head_t h; h.all = Head;
	return h.size;
}

} // namespace oConcurrency

#endif
