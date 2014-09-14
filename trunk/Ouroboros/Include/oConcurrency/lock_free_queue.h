// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Single-reader, single-writer queue that uses only memory barriers to protect
// concurrency.

#pragma once
#include <oConcurrency/concurrency.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace ouro {

template<typename T, typename Alloc = std::allocator<T>>
class lock_free_queue
{
public:
	typedef uint32_t size_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef Alloc allocator_type;


	// returns the number of bytes required to pass as memory to initialize().
	static size_type calc_size(size_type capacity);
	

	// non-concurrent api

	// default ctor is uninitialized
	lock_free_queue();

	// capacity must be a power of two.
	lock_free_queue(size_type capacity, const allocator_type& a = allocator_type());
	~lock_free_queue();

	// capacity must be a power of two.
	void initialize(size_type capacity, const allocator_type& a = allocator_type());
	
	// capacity must be a power of two.
	void initialize(void* memory, size_type capacity);

	// returns memory passed to initialize or nullptr if allocator was specified
	void* deinitialize();

	// Returns the number of elements in the queue. Client code should not be 
	// reliant on this value and the API is included only for debugging and 
	// testing purposes. It is not concurrent.
	size_type size() const;


	// concurrent api

	// Push an element into the queue.
	void push(const_reference val);
	
	// Returns false if the queue is empty, otherwise val is a valid element
	// from the head of the queue.
	bool try_pop(reference val);

	// Returns true if no elements are in the queue
	bool empty() const;

	// Returns the max number of elements that can be stored
	size_type capacity() const;
	
private:
	value_type* elements;
	size_type wrap_mask;
	volatile size_type read;
	volatile size_type write;
	bool owns_memory;
	allocator_type alloc;
};

template<typename T, typename Alloc>
lock_free_queue<T, Alloc>::lock_free_queue()
	: elements(nullptr)
	, wrap_mask(0)
	, read(0)
	, write(0)
	, owns_memory(false)
{
}

template<typename T, typename Alloc>
typename lock_free_queue<T, Alloc>::size_type lock_free_queue<T, Alloc>::calc_size(size_type capacity)
{
	return sizeof(T) * capacity;
}

template<typename T, typename Alloc>
lock_free_queue<T, Alloc>::lock_free_queue(size_type capacity, const allocator_type& a)
{
	initialize(capacity, a);
}

template<typename T, typename Alloc>
lock_free_queue<T, Alloc>::~lock_free_queue()
{
	if (!empty())
		throw std::length_error("container not empty");

	if (deinitialize())
		throw std::invalid_argument("container not empty");
}

template<typename T, typename Alloc>
void lock_free_queue<T, Alloc>::initialize(size_type capacity, const allocator_type& a)
{
	if (capacity & (capacity-1))
		throw std::invalid_argument("capacity must be a power of two");
	alloc = a;
	elements = memory;
	read = write = 0;	
	wrap_mask = capacity - 1;
	owns_memory = true;
}

template<typename T, typename Alloc>
void lock_free_queue<T, Alloc>::initialize(void* memory, size_type capacity)
{
	if (capacity & (capacity-1))
		throw std::invalid_argument("capacity must be a power of two");
	elements = memory;
	read = write = 0;	
	wrap_mask = capacity - 1;
	owns_memory = false;
}

template<typename T, typename Alloc>
void* lock_free_queue<T, Alloc>::deinitialize()
{
	void* mem = owns_memory ? elements : nullptr;
	if (owns_memory)
		alloc.deallocate(elements, capacity());
	elements = nullptr;
	read = write = 0;
	wrap_mask = 0;
	owns_memory = false;
	alloc = allocator_type();
	return mem;
}

template<typename T, typename Alloc>
typename lock_free_queue<T, Alloc>::size_type lock_free_queue<T, Alloc>::size() const
{
	return ((write + wrap_mask) - read) & wrap_mask;
}

template<typename T, typename Alloc>
void lock_free_queue<T, Alloc>::push(const_reference val)
{
	size_type r = read;
	size_type w = write;

	if (((w+1) & wrap_mask) != r)
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		elements[w++] = val;
		std::atomic_thread_fence(std::memory_order_seq_cst);
		write = w & wrap_mask;
	}

	else
		throw std::length_error("lock_free_queue cannot hold any more elements");
}

template<typename T, typename Alloc>
bool lock_free_queue<T, Alloc>::try_pop(reference val)
{
	bool popped = false;
	size_type r = read;
	size_type w = write;

	if (r != w)
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		val = elements[r];
		elements[r++].~value_type();
		std::atomic_thread_fence(std::memory_order_seq_cst);
		read = r & wrap_mask;
		popped = true;
	}

	return popped;
}

template<typename T, typename Alloc>
bool lock_free_queue<T, Alloc>::empty() const
{
	return read == write;
}

template<typename T, typename Alloc>
typename lock_free_queue<T, Alloc>::size_type lock_free_queue<T, Alloc>::capacity() const
{
	return wrap_mask ? (wrap_mask + 1) : 0;
}

}
