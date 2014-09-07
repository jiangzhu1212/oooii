// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_sbb_allocator_h
#define oBase_sbb_allocator_h

// Allocator implementation using the segregated binary buddy algorithm.

#include <oMemory/allocate.h>
#include <functional>

namespace ouro {

class sbb_allocator
{
public:
	sbb_allocator() : heap(nullptr), heap_size(0) {}
	sbb_allocator(void* arena, size_t bytes, void* bookkeeping) { initialize(arena, bytes, bookkeeping); }
	~sbb_allocator() { deinitialize(); }

	sbb_allocator(sbb_allocator&& that);
	sbb_allocator& operator=(sbb_allocator&& that);

	// creates an allocator for the specified arena and returns the required bookkeeping size
	// if bookkeeping is nullptr the initialization does not succeed 
	size_t initialize(void* arena, size_t bytes, void* bookkeeping);

	// invalidates the allocator and returns the memory passed in initialize 
	void* deinitialize();

	inline const void* bookkeeping() const { return heap; }
	const void* arena() const;
	inline size_t arena_size() const;
	allocator_stats get_stats() const;
	void* allocate(size_t bytes, const char* label = "?", const allocate_options& options = allocate_options());
	void* reallocate(void* ptr, size_t bytes);
	void deallocate(void* ptr);
	size_t size(void* ptr) const;
	bool owns(void* ptr) const;
	bool valid() const;
	void reset();
	void walk_heap(const std::function<void(void* ptr, size_t bytes, bool used)>& enumerator);

private:
	sbb_allocator(const sbb_allocator&);
	const sbb_allocator& operator=(const sbb_allocator&);

	void* heap;
	size_t heap_size;
	allocator_stats stats;
};

}

#endif
