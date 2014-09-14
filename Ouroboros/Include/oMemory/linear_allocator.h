// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// O(1) linear allocator: cannot free but is extremely quick to allocate

#pragma once
#include <oMemory/byte.h>
#include <oCompiler.h> // oDEFAULT_MEMORY_ALIGNMENT
#include <cstdint>

namespace ouro {

class linear_allocator
{
public:
	static const size_t default_alignment = oDEFAULT_MEMORY_ALIGNMENT;

	// ctor creates as empty
	linear_allocator() : base(nullptr), end(nullptr), head(nullptr) {}

	// ctor moves an existing pool into this one
	linear_allocator(linear_allocator&& that)
		: base(that.base), end(that.end), head(that.head)
	{ that = linear_allocator(); }

	// ctor creates as a valid pool using external memory
	linear_allocator(void* memory, size_t capacity) { initialize(memory, capacity); }

	// dtor
	~linear_allocator() { deinitialize(); }

	// calls deinitialize on this, moves that's memory under the same config
	linear_allocator& operator=(linear_allocator&& that)
	{
		if (this != &that)
		{
			deinitialize();
			base = that.base; that.base = nullptr;
			end = that.end; that.end = nullptr;
			head = that.head; that.head = nullptr;
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
	inline size_t size_free() const { return size_t(end - head); }

	// returns the number of allocated bytes
	inline size_t size() const { return size_t(head - base); }

	// returns true of there are no outstanding allocations
	inline bool full() const { return capacity() == size_free(); }

	// returns the max number of bytes that can be allocated
	inline size_t capacity() const { return size_t(end - base); }

	// returns true if all bytes have been allocated
	inline bool empty() const { return head >= end; }

	void* allocate(size_t bytes, size_t alignment = default_alignment)
	{ 
		uint8_t* p = byte_align(head, alignment);
		uint8_t* h = p + bytes;
		if (h <= end)
		{
			head = h;
			return p;
		}
		return nullptr;
	}

	template<typename T> T* allocate(size_t size = sizeof(T), size_t alignment = default_alignment) { return (T*)allocate(size, alignment); }

	// reset the linear allocator to full availability
	void reset() { head = base; }

	// simple range check that returns true if this index/pointer could have been allocated from this pool
	bool owns(void* ptr) const { return ptr >= base && ptr < end; }

private:
	uint8_t* base;
	uint8_t* end;
	uint8_t* head;
};

}
