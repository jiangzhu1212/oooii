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
// Similar to Microsoft's InterlockedSList, this provides for a 
// singly linked list whose insertion has the behavior of a stack (LIFO).
// NOTE: The type T must include a member pointer pNext. All allocation of nodes 
// is the responsibility of the user because this data structure is often used 
// in very low-level implementations such as allocations themselves.
#ifndef oConcurrency_concurrent_stack_h
#define oConcurrency_concurrent_stack_h

#include <oBase/config.h>
#include <oConcurrency/oConcurrency.h> // for is_fifo

#ifdef oHAS_DOUBLE_WIDE_ATOMIC_BUG
	#include <oHLSL/oHLSLAtomics.h>
#else
	#include <atomic>
#endif

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
	void push(pointer _pElement);

	// Returns the top of the stack without removing the item from the stack.
	pointer peek() const;

	// If there are no elements, this return nullptr.
	pointer pop();

	// Returns true if no elements are in the stack
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

	// @tony: there's a warning about not handling registers correctly in the 32-bit std::atomic implementation
	// in VS2012 that results in registers being corrupt, so fall back on basic atomics until the lib gets fixed
	#ifdef oHAS_DOUBLE_WIDE_ATOMIC_BUG
		unsigned long long Head;
		bool head_compare_exchange_strong(unsigned long long& _Expected, unsigned long long _Desired)
		{
			unsigned long long Orig = 0;
			InterlockedCompareExchange(Head, _Expected, _Desired, Orig);
			bool Exchanged = Orig == _Expected;
			_Expected = Orig;
			return Exchanged;
		}
	#else
		std::atomic<unsigned long long> Head;
		bool head_compare_exchange_strong(unsigned long long& _Expected, unsigned long long _Desired)
		{
			return Head.compare_exchange_strong(_Expected, _Desired);
		}
	#endif
};

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
struct is_fifo<concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>> : std::false_type {};

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::concurrent_stack()
{
	Head = 0;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::~concurrent_stack()
{
	if (!empty())
		throw std::length_error("container not empty");
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
void concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::push(pointer _pElement)
{
	header_t New, Old;
	Old.All = Head;
	do 
	{
		if (Old.Size >= capacity)
			throw std::overflow_error("concurrent_stack cannot hold any more elements");
		_pElement->pNext = reinterpret_cast<pointer>(Old.pHead);
		New.Tag = Old.Tag + 1;
		New.Size = Old.Size + 1;
		New.pHead = reinterpret_cast<uintptr_t>(_pElement);

		if (reinterpret_cast<pointer>(New.pHead) != _pElement)
			throw std::overflow_error("the specified pointer is too large to be used in conjunction with tagging or other concurrency/ABA solutions");

	} while (!head_compare_exchange_strong(Old.All, New.All));
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pointer concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::peek() const
{
	header_t Old; Old.All = Head;
	return reinterpret_cast<pointer>(Old.pHead);
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pointer concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pop()
{
	header_t New, Old; Old.All = Head;
	pointer pElement = nullptr;
	do 
	{
		if (!Old.pHead)
			return nullptr;
		New.Tag = Old.Tag + 1;
		New.Size = Old.Size - 1;
		pElement = reinterpret_cast<pointer>(Old.pHead);
		New.pHead = reinterpret_cast<uintptr_t>(pElement->pNext);
	} while (!head_compare_exchange_strong(Old.All, New.All));

	return pElement;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
bool concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::empty() const
{
	header_t h; h.All = Head;
	return !h.pHead;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pointer concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::pop_all()
{
	pointer pElement = nullptr;
	header_t New, Old;
	New.Size = 0;
	New.pHead = 0;
	Old.All = Head;
	do 
	{
		New.Tag = Old.Tag + 1;
		pElement = reinterpret_cast<pointer>(Old.pHead);
	} while (!head_compare_exchange_strong(Old.All, New.All));
	return pElement;
}

template<typename T, size_t nTagBits, size_t nSizeBits, size_t nPointerBits>
typename concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::size_type concurrent_stack<T, nTagBits, nSizeBits, nPointerBits>::size() const
{
	header_t h; h.All = Head;
	return h.Size;
}

} // namespace oConcurrency

#endif
