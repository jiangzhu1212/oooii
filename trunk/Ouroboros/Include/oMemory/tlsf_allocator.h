// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Allocator implementation using the two-level segregated fit allocation 
// algorithm. This is a wrapper for the excellent allocator found at 
// http://tlsf.baisoku.org/.

#pragma once
#include <oMemory/allocate.h>
#include <functional>

namespace ouro {

class tlsf_allocator
{
public:
	tlsf_allocator() : heap(nullptr), heap_size(0) {}
	tlsf_allocator(void* arena, size_t bytes) { initialize(arena, bytes); }
	~tlsf_allocator() { deinitialize(); }

	tlsf_allocator(tlsf_allocator&& that);
	tlsf_allocator& operator=(tlsf_allocator&& that);

	// creates an allocator for the specified arena
	void initialize(void* arena, size_t bytes);

	// invalidates the allocator and returns the memory passed in initialize 
	void* deinitialize();

	inline const void* arena() const { return heap; }
	inline size_t arena_size() const { return heap_size; }
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
	tlsf_allocator(const tlsf_allocator&);
	const tlsf_allocator& operator=(const tlsf_allocator&);

	void* heap;
	size_t heap_size;
	allocator_stats stats;
};

}
