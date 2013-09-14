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
// Similar to Microsoft's InterlockedSList, this provides for a threadsafe 
// singly linked list whose insertion has the behavior of a stack (LIFO).
// NOTE: The type T must include a member pointer pNext. All allocation of nodes 
// is the responsibility of the user because this data structure is often used 
// in very low-level implementations such as allocations themselves.
#ifndef oConcurrency_concurrent_stack_h
#define oConcurrency_concurrent_stack_h

#include <oConcurrency/oConcurrency.h> // for is_fifo
#include <oConcurrency/concurrent_queue_base.h>
#include <oConcurrency/thread_safe.h>
#include <oStd/atomic.h>

namespace oConcurrency {

template<typename T
		, size_t nTagBits = 4
		, size_t nSizeBits = 12
		, size_t nPointerBits = 48
>
class concurrent_stack
{
public:
	typedef size_t size_type;
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;

	static const size_type capacity = (1 << nSizeBits) - 1;

	concurrent_stack();
	~concurrent_stack();

	// Push an element onto the stack.
	void push(pointer _pElement) threadsafe;

	// Returns the top of the stack without removing the item from the stack.
	pointer peek() const threadsafe;

	// If there are no elements, this return nullptr.
	pointer pop() threadsafe;

	// Returns true if no elements are in the stack
	bool empty() const threadsafe;

	// Returns the head that is thus a linked list of all items that were in the 
	// stack, leaving the stack empty.
	pointer pop_all() threadsafe;

	// Returns the number of elements in the stack. This is an instantaneous 
	// sampling and thus might not be valid by the very next line of code. Client
	// code should not be reliant on this value and the API is included only for 
	// debugging and testing purposes.
	size_type size() const threadsafe;

private:
	union header_t
	{
		unsigned long long All;
		struct
		{
			unsigned long long Tag : nTagBits;
			unsigned long long Size : nSizeBits;
			unsigned long long pHead : nPointerBits;
		};
	};

	header_t Head;
};

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
struct is_fifo<concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>> : std::false_type {};

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::concurrent_stack()
{
	Head.All = 0;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::~concurrent_stack()
{
	if (!empty())
		throw container_error(container_errc::not_empty);
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
void concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::push(pointer _pElement) threadsafe
{
	header_t New, Old;
	do 
	{
		Old.All = Head.All;
		if (Old.Size >= capacity)
			throw std::overflow_error("concurrent_stack cannot hold any more elements");
		_pElement->pNext = reinterpret_cast<pointer>(Old.pHead);
		New.Tag = Old.Tag + 1;
		New.Size = Old.Size + 1;
		New.pHead = reinterpret_cast<uintptr_t>(_pElement);

		if (reinterpret_cast<pointer>(New.pHead) != _pElement)
			throw std::overflow_error("the specified pointer is too large to be used in conjunction with tagging or other concurrency/ABA solutions");

	} while (!oStd::atomic_compare_exchange(&Head.All, New.All, Old.All));
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pointer concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::peek() const threadsafe
{
	header_t Old;
	Old.All = Head.All;
	return reinterpret_cast<pointer>(Old.pHead);
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pointer concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pop() threadsafe
{
	header_t New, Old;
	pointer pElement = nullptr;
	do 
	{
		Old.All = Head.All;
		if (!Old.pHead)
			return nullptr;
		New.Tag = Old.Tag + 1;
		New.Size = Old.Size - 1;
		pElement = reinterpret_cast<pointer>(Old.pHead);
		New.pHead = reinterpret_cast<uintptr_t>(pElement->pNext);
	} while (!oStd::atomic_compare_exchange(&Head.All, New.All, Old.All));
	return pElement;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
bool concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::empty() const threadsafe
{
	return !Head.pHead;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pointer concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pop_all() threadsafe
{
	pointer pElement = nullptr;
	header_t New, Old;
	New.Size = 0;
	New.pHead = 0;
	do 
	{
		Old.All = Head.All;
		New.Tag = Old.Tag + 1;
		pElement = reinterpret_cast<pointer>(Old.pHead);
	} while (!oStd::atomic_compare_exchange(&Head.All, New.All, Old.All));

	return pElement;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::size_type concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::size() const threadsafe
{
	return Head.Size;
}

} // namespace oConcurrency

#endif
