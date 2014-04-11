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
// This header provides two stack implementations: 
// concurrent_stack: encapsulates its own object pool for members that get 
//                   pushed and popped and the other does
// concurrent_intrusive_stack: no memory management, instead using pointers 
//                             similarly to Microsoft's InterlockedSList.
#ifndef oBase_concurrent_stack_h
#define oBase_concurrent_stack_h

#include <oBase/concurrency.h> // for is_fifo
#include <oBase/concurrent_stack_traits.h>
#include <oBase/pool.h>
#include <stdexcept>

namespace ouro {

template<typename T, typename traits = concurrent_stack_traits32>
class concurrent_stack
{
public:
	typedef unsigned int size_type;
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type&& move_reference;
	static const size_type max_index = (1 << traits::size_bits) - 1;

	// Returns the number of bytes required to store the specified
	// maximum number of items.
	static size_type calc_size(size_type _Capacity);

	// default-constructed instances are invalid
	concurrent_stack();
	~concurrent_stack();
	concurrent_stack(concurrent_stack&& _That) { operator=(std::move(_That)); }
	concurrent_stack& operator=(concurrent_stack&& _That);

	// memory must point to at least the number of bytes returned by calc_size.
	concurrent_stack(void* _pMemory, size_type _Capacity);

	// returns the pointer specified in the ctor
	void* const get_memory_pointer() const;

	// returns the maximum number of items that can be pushed
	size_type capacity() const;

	// This returns the size field cached in the header. Under normal push/pop usage this
	// is an accurate count of the number of elements in the stack. However if any bulk
	// pushing is done it could be invalid.
	size_type size() const;

	// returns true if there are no elements in the stack.
	bool empty() const;

	// spins until the element is copied into the stack
	void push(const_reference _Value);

	// spins until the element is moved into the stack
	void push(move_reference _Value);

	// return true if an element is moved into value or false if empty
	bool pop(reference _Value);

	// if _MaxToVisit is a small value and there are leftover entries to 
	// process they will be reinserted. This action will make the size()
	// api untrustworthy. If size() is important to use, always flush the
	// full queue. Returns the number of items processed.
	size_type pop_all_and_enumerate(void (*_Visitor)(reference _Value, void* _pUserData), void* _pUserData, size_type _MaxToVisit = size_type(-1));

private:
	// choose a best-fit for bits used in stack storage
	typedef typename std::conditional<traits::pointer_bits < 16, unsigned short, 
		typename std::conditional<traits::pointer_bits < 32, unsigned int, 
		unsigned long long>::type>::type index_type;

	static const index_type invalid_index = index_type(-1) & ((index_type(1) << traits::pointer_bits) - 1);

	typename traits::atomic_type Head;
	concurrent_object_pool<value_type> Pool;
	typename index_type* Next;

	void initialize();
	index_type allocate_index();
	void push_index(index_type _Index);
};

template<typename T, typename traits = concurrent_stack_traits64>
class concurrent_intrusive_stack
{
public:
	typedef unsigned int size_type;
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	static const size_type capacity = (1 << traits::size_bits) - 1;

	// Initialized to a valid empty stack
	concurrent_intrusive_stack();
	~concurrent_intrusive_stack();

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
	typename traits::atomic_type Head;
};

template<typename T, typename traits>
struct is_fifo<concurrent_intrusive_stack<T, traits>> : std::false_type {};

template<typename T, typename traits>
struct is_fifo<concurrent_stack<T, traits>> : std::false_type {};

template<typename T, typename traits>
typename concurrent_stack<T, traits>::size_type concurrent_stack<T, traits>::calc_size(size_type _Capacity)
{
	return _Capacity * sizeof(index_type) + concurrent_object_pool<T>::calc_size(_Capacity);
}

template<typename T, typename traits>
concurrent_stack<T, traits>::concurrent_stack()
	: Next(nullptr)
{
	typename traits::head_type h;
	h.tag = 0;
	h.size = 0;
	h.next = invalid_index;
	Head = h.all;
}

template<typename T, typename traits>
concurrent_stack<T, traits>::~concurrent_stack()
{
	if (!empty())
		throw std::exception("concurrent_stack not empty");
}

template<typename T, typename traits>
concurrent_stack<T, traits>& concurrent_stack<T, traits>::operator=(concurrent_stack<T, traits>&& _That)
{
	if (this != &_That)
	{
		Head = std::move(_That.Head);
		Pool = std::move(_That.Pool);
		Entry = _That.Entry; _That.Entry = nullptr;
		Next = _That.Next; _That.Next = nullptr;
	}
	return *this;
}

template<typename T, typename traits>
concurrent_stack<T, traits>::concurrent_stack(void* _pMemory, size_type _Capacity)
{
	static_assert(((1ull << traits::pointer_bits)-1) <= (1ull << 24), "traits too large for pool");
	
	if (!byte_aligned(_pMemory, oDEFAULT_MEMORY_ALIGNMENT))
		throw std::invalid_argument("memory must be default-aligned");
	Pool = std::move(concurrent_object_pool<T>(_pMemory, _Capacity));
	Next = static_cast<index_type*>(byte_add(Pool.get_memory_pointer(), sizeof(T), _Capacity));
	initialize();
}

template<typename T, typename traits>
void* const concurrent_stack<T, traits>::get_memory_pointer() const
{
	return Pool.get_memory_pointer();
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::size_type concurrent_stack<T, traits>::capacity() const
{
	return Pool.capacity();
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::size_type concurrent_stack<T, traits>::size() const
{
	typename traits::head_type h; h.all = Head;
	return h.size;
}

template<typename T, typename traits>
bool concurrent_stack<T, traits>::empty() const
{
	typename traits::head_type h; h.all = Head;
	return h.next == invalid_index;
}

template<typename T, typename traits>
void concurrent_stack<T, traits>::push(const_reference _Value)
{
	value_type* v = Pool.create(_Value);
	push_index((index_type)Pool.index(v));
}

template<typename T, typename traits>
void concurrent_stack<T, traits>::push(move_reference _Value)
{
	value_type* v = Pool.create(std::move(_Value));
	push_index((index_type)Pool.index(v));
}

template<typename T, typename traits>
bool concurrent_stack<T, traits>::pop(reference _Value)
{
	index_type i;
	typename traits::head_type New, Old;
	do
	{	Old.all = Head;
		if (Old.next == invalid_index)
			return false;
		i = Old.next;
		New.tag = Old.tag + 1;
		New.size = Old.size - 1;
		New.next = Next[i];
	} while (!traits::cas(Head, Old.all, New.all));
	value_type* v = (value_type*)Pool.pointer(i);
	_Value = std::move(*v);
	Pool.destroy(v);
	return true;
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::size_type concurrent_stack<T, traits>::pop_all_and_enumerate(
	void (*_Visitor)(reference _Value, void* _pUserData), void* _pUserData, size_type _MaxToVisit)
{
	index_type i;
	typename traits::head_type New, Old;
	do
	{	Old.all = Head;
		if (Old.next == invalid_index)
			return 0;
		i = Old.next;
		New.tag = Old.tag + 1;
		New.size = 0;
		New.next = invalid_index;
	} while (!traits::cas(Head, Old.all, New.all));

	size_type n = 0;
	while (n < _MaxToVisit && i != invalid_index)
	{
		value_type* v = (value_type*)Pool.pointer(i);
		_Visitor(*v, _pUserData);
		i = Next[i];
		Pool.destroy(v);
		n++;
		// interesting: accum indices and do a single bulk free (needs pool api)
	}

	if (i != invalid_index) // push remainder
	{
		index_type count = 1;
		index_type last = i;
		while (Next[last] != invalid_index) // find the last node in the list
		{
			count++;
			last = Next[last];
		}

		// push the set
		typename traits::head_type New, Old;
		do
		{	Old.all = Head;
			Next[last] = (index_type)Old.next;
			New.tag = Old.tag + 1;
			New.size = Old.size + count; // this invalidates internal nodes' count - cut the feature.
			New.next = i;
		} while (!traits::cas(Head, Old.all, New.all));
	}
	return n;
}

template<typename T, typename traits>
void concurrent_stack<T, traits>::initialize()
{
	typename traits::head_type h;
	h.tag = 0;
	h.size = 0;
	h.next = invalid_index;
	Head.store(h.all);
	Pool.clear();
	const size_t Capacity = Pool.capacity();
	for (size_t i = 0; i < Capacity; i++)
		Next[i] = invalid_index;
}

template<typename T, typename traits>
typename concurrent_stack<T, traits>::index_type concurrent_stack<T, traits>::allocate_index()
{
	index_type i = Pool.allocate();
	while (i == Pool.invalid_index)
	{
		std::this_thread::yield();
		i = Pool.allocate();
	}
	return i;
}

template<typename T, typename traits>
void concurrent_stack<T, traits>::push_index(index_type _Index)
{
	typename traits::head_type New, Old;
	do
	{	Old.all = Head;
		Next[_Index] = static_cast<index_type>(Old.next);
		New.tag = Old.tag + 1;
		New.size = Old.size + 1;
		New.next = _Index;
	} while (!traits::cas(Head, Old.all, New.all));
}


template<typename T, typename traits>
concurrent_intrusive_stack<T, traits>::concurrent_intrusive_stack()
{
	Head = 0;
}

template<typename T, typename traits>
concurrent_intrusive_stack<T, traits>::~concurrent_intrusive_stack()
{
	if (!empty())
		throw std::length_error("container not empty");
}

template<typename T, typename traits>
void concurrent_intrusive_stack<T, traits>::push(pointer _pElement)
{
	typename traits::head_type New, Old; 
	do 
	{
		Old.all = Head;
		if (Old.size >= capacity)
			throw std::overflow_error("concurrent_intrusive_stack cannot hold any more elements");
		_pElement->next = reinterpret_cast<pointer>(Old.next);
		New.tag = Old.tag + 1;
		New.size = Old.size + 1;
		New.next = reinterpret_cast<uintptr_t>(_pElement);

		if (reinterpret_cast<pointer>(New.next) != _pElement)
			throw std::overflow_error("the specified pointer is too large to be used in conjunction with tagging or other concurrency/ABA solutions");

	} while (!traits::cas(Head, Old.all, New.all));
}

template<typename T, typename traits>
typename concurrent_intrusive_stack<T, traits>::pointer concurrent_intrusive_stack<T, traits>::peek() const
{
	typename traits::head_type Old; Old.all = Head;
	return reinterpret_cast<pointer>(Old.next);
}

template<typename T, typename traits>
typename concurrent_intrusive_stack<T, traits>::pointer concurrent_intrusive_stack<T, traits>::pop()
{
	typename traits::head_type New, Old;
	pointer pElement = nullptr;
	do 
	{
		Old.all = Head;
		if (!Old.next)
			return nullptr;
		New.tag = Old.tag + 1;
		New.size = Old.size - 1;
		pElement = reinterpret_cast<pointer>(Old.next);
		New.next = reinterpret_cast<uintptr_t>(pElement->next);
	} while (!traits::cas(Head, Old.all, New.all));

	return pElement;
}

template<typename T, typename traits>
bool concurrent_intrusive_stack<T, traits>::empty() const
{
	typename traits::head_type h; h.all = Head;
	return !h.next;
}

template<typename T, typename traits>
typename concurrent_intrusive_stack<T, traits>::pointer concurrent_intrusive_stack<T, traits>::pop_all()
{
	pointer pElement = nullptr;
	typename traits::head_type New, Old;
	New.size = New.next = 0;
	Old.all = Head;
	do 
	{
		New.tag = Old.tag + 1;
		pElement = reinterpret_cast<pointer>(Old.next);
	} while (!traits::cas(Head, Old.all, New.all));
	return pElement;
}

template<typename T, typename traits>
typename concurrent_intrusive_stack<T, traits>::size_type concurrent_intrusive_stack<T, traits>::size() const
{
	typename traits::head_type h; h.all = Head;
	return h.size;
}

} // namespace ouro

#endif
