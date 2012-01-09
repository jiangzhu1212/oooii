// $(header)
// Single-reader, single-writer queue that uses no explicit mechanism for
// synchronization.
#pragma once
#ifndef oLockFreeQueue_h
#define oLockFreeQueue_h

#include <oBasis/oStdAtomic.h>
#include <oBasis/oThreadsafe.h>
#include <memory>

template<typename T, typename Alloc = std::allocator<T> >
class oLockFreeQueue
{
	// single reader, single writer queue using memory barriers to guarantee thread safety
	// (no locks or atomics)
	
	T* ElementPool;
	size_t NumElements;
	volatile size_t Read;
	volatile size_t Write;
	Alloc Allocator;

public:
	typedef Alloc AllocatorType;
	typedef T ElementType;

	inline oLockFreeQueue(size_t _Capacity = 100000, Alloc _Allocator = Alloc())
		: Allocator(_Allocator)
		, ElementPool(_Allocator.allocate(_Capacity))
		, NumElements(_Capacity)
		, Read(0)
		, Write(0)
	{
		for (size_t i = 0; i < NumElements; i++)
			new(ElementPool + i) T();
	}

	~oLockFreeQueue()
	{
		oASSERT(empty(), "queue not empty");
		Read = Write = 0;
		Allocator.deallocate(ElementPool, NumElements);
		ElementPool = 0;
		NumElements = 0;
	}

	inline bool try_push(const T& e) threadsafe
	{
		bool pushed = false;
		size_t r = Read;
		size_t w = Write;

		if (((w+1) % NumElements) != r)
		{
			oStd::atomic_thread_fence_read_write();
			ElementPool[w++] = e;
			oStd::atomic_thread_fence_read_write();
			Write = w % NumElements;
			pushed = true;
		}
	
		return pushed;
	}

	inline void push(const T& e) threadsafe { while (!try_push(e)); }

	inline bool try_pop(T& e) threadsafe
	{
		bool popped = false;
		size_t r = Read;
		size_t w = Write;

		if (r != w)
		{
			oStd::atomic_thread_fence_read_write();
			e = ElementPool[r];
			ElementPool[r++].~T();
			oStd::atomic_thread_fence_read_write();
			Read = r % NumElements;
			popped = true;
		}

		return popped;
	}

	inline bool valid() const threadsafe { return !!ElementPool; }
	inline bool empty() const threadsafe { return Read == Write; }
	inline size_t capacity() const threadsafe { return NumElements; }
	inline size_t unsafe_size() const { return ((Write + NumElements) - Read) % NumElements; }
};

#endif
