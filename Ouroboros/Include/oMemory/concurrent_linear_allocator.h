// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// O(1) fine-grained concurrent linear allocator: cannot free but is extremely 
// quick to allocate

#pragma once
#include <oMemory/byte.h>
#include <oCompiler.h> // oDEFAULT_MEMORY_ALIGNMENT
#include <atomic>
#include <cstdint>

namespace ouro {

class concurrent_linear_allocator
{
public:
	static const size_t default_alignment = oDEFAULT_MEMORY_ALIGNMENT;

	// ctor creates as empty
	concurrent_linear_allocator() : base(nullptr), end(nullptr) { head.store(nullptr); }

	// ctor moves an existing pool into this one
	concurrent_linear_allocator(concurrent_linear_allocator&& that)
		: base(that.base), end(that.end)
	{ head.store(that.head.load()); that = concurrent_linear_allocator(); }

	// ctor creates as a valid pool using external memory
	concurrent_linear_allocator(void* memory, size_t capacity) { initialize(memory, capacity); }

	// dtor
	~concurrent_linear_allocator() { deinitialize(); }

	// calls deinitialize on this, moves that's memory under the same config
	concurrent_linear_allocator& operator=(concurrent_linear_allocator&& that)
	{
		if (this != &that)
		{
			deinitialize();
			base = that.base; that.base = nullptr;
			end = that.end; that.end = nullptr;
			head.store(that.head.load()); that.head.store(nullptr);
		}
		return *this;
	}
	
	// returns bytes required for memory; pass nullptr to obtain size, allocate
	// and then pass that to memory in a second call to initialize the class.
	size_t initialize(void* memory, size_t capacity)
	{
		if (memory)
		{
			base = (uint8_t*)memory;
			end = base + capacity;
			head = base;
		}

		return capacity;
	}

	// deinitializes the pool and returns the memory passed to initialize()
	void* deinitialize()
	{
		return base;
	}

	// returns the number of bytes available
	inline size_t size_free() const { return size_t(end - head.load()); }

	// returns the number of allocated bytes
	inline size_t size() const { return size_t(head.load() - base); }

	// returns true of there are no outstanding allocations
	inline bool full() const { return capacity() == size_free(); }

	// returns the max number of bytes that can be allocated
	inline size_t capacity() const { return size_t(end - base); }

	// returns true if all bytes have been allocated
	inline bool empty() const { return head.load() >= end; }

	void* allocate(size_t bytes, size_t alignment = default_alignment)
	{
		uint8_t *n, *o, *p;
		o = head;
		do
		{
			p = byte_align(o, alignment);
			n = p + bytes;
			if (n >= end)
				return nullptr;
		} while (!head.compare_exchange_strong(o, n));
		return p;
	}

	template<typename T> T* allocate(size_t size = sizeof(T), size_t alignment = default_alignment) { return (T*)allocate(size, alignment); }

	// reset the linear allocator to full availability
	void reset() { head.store(base); }

	// simple range check that returns true if this index/pointer could have been allocated from this pool
	bool owns(void* ptr) const { return ptr >= base && ptr < end; }

private:
	uint8_t* base;
	uint8_t* end;
	std::atomic<uint8_t*> head;
};

}
