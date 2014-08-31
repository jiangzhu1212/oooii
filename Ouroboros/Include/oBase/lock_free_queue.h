// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Single-reader, single-writer queue that uses no explicit mechanism for
// synchronization.
#pragma once
#ifndef oBase_lock_free_queue_h
#define oBase_lock_free_queue_h

#include <oBase/allocate.h>
#include <oBase/concurrency.h>
#include <atomic>
#include <memory>

namespace ouro {

template<typename T>
class lock_free_queue
{
	// single reader, single writer queue using memory barriers to guarantee thread safety
	// (no locks or atomics)
public:
	typedef size_t size_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;

	lock_free_queue(size_type _Capacity = 10000, const allocator& _Allocator = default_allocator);
	~lock_free_queue();

	// Push an element into the queue.
	void push(const_reference _Element);
	
	// Returns false if the queue is empty, otherwise _Element is a valid element
	// from the head of the queue.
	bool try_pop(reference _Element);

	// Returns true if no elements are in the queue
	bool empty() const;
	
	// Returns the number of elements in the queue. Client code should not be 
	// reliant on this value and the API is included only for debugging and 
	// testing purposes. It is not.
	size_type size() const;

private:
	value_type* ElementPool;
	size_type NumElements;
	volatile size_type Read;
	volatile size_type Write;
	allocator Allocator;
};

template<typename T>
lock_free_queue<T>::lock_free_queue(size_type _Capacity, const allocator& _Allocator)
	: Allocator(_Allocator)
	, ElementPool(Allocator.construct_array<T>(_Capacity))
	, NumElements(_Capacity)
	, Read(0)
	, Write(0)
{}

template<typename T>
lock_free_queue<T>::~lock_free_queue()
{
	if (!empty())
		throw std::length_error("container not empty");

	Read = Write = 0;
	Allocator.destroy_array(ElementPool, NumElements);
	ElementPool = 0;
	NumElements = 0;
}

template<typename T>
void lock_free_queue<T>::push(const_reference _Element)
{
	size_type r = Read;
	size_type w = Write;

	if (((w+1) % NumElements) != r)
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		ElementPool[w++] = _Element;
		std::atomic_thread_fence(std::memory_order_seq_cst);
		Write = w % NumElements;
	}

	else
		throw std::overflow_error("lock_free_queue cannot hold any more elements");
}

template<typename T>
bool lock_free_queue<T>::try_pop(reference _Element)
{
	bool popped = false;
	size_type r = Read;
	size_type w = Write;

	if (r != w)
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		_Element = ElementPool[r];
		ElementPool[r++].~value_type();
		std::atomic_thread_fence(std::memory_order_seq_cst);
		Read = r % NumElements;
		popped = true;
	}

	return popped;
}

template<typename T>
bool lock_free_queue<T>::empty() const
{
	return Read == Write;
}

template<typename T>
typename lock_free_queue<T>::size_type lock_free_queue<T>::size() const
{
	return ((Write + NumElements) - Read) % NumElements;
}

}

#endif
