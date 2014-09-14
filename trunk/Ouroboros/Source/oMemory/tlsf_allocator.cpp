// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/tlsf_allocator.h>
#include <oMemory/byte.h>
#include "tlsf.h"

#define USE_ALLOCATOR_STATS 1
#define USE_ALLOCATION_STATS 1
#define USE_LABEL 1

namespace ouro {

// How do I walk all pools?
static void o_tlsf_walk_heap(tlsf_t tlsf, tlsf_walker walker, void* user)
{
	tlsf_walk_pool(tlsf_get_pool(tlsf), walker, user);
}

struct walk_stats
{
	size_t largest_free_block_bytes;
	size_t num_free_blocks;
};

static void find_largest_free_block(void* ptr, size_t bytes, int used, void* user)
{
	if (!used)
	{
		walk_stats* stats = (walk_stats*)user;
		stats->largest_free_block_bytes = std::max(stats->largest_free_block_bytes, bytes);
		stats->num_free_blocks++;
	}
}

void tlsf_allocator::initialize(void* arena, size_t bytes)
{
	if (!byte_aligned(arena, 16))
		throw allocate_error(allocate_errc::alignment);
	heap = arena;
	heap_size = bytes;
	reset();
}

void* tlsf_allocator::deinitialize()
{
	#if USE_ALLOCATOR_STATS
		if (stats.num_allocations)
			throw allocate_error(allocate_errc::outstanding_allocations);
	#endif

	if (!valid())
		throw allocate_error(allocate_errc::corrupt);

	void* arena = heap;
	tlsf_destroy(heap);
	heap = nullptr;
	return arena;
}

tlsf_allocator::tlsf_allocator(tlsf_allocator&& that)
{
	operator=(std::move(that));
}

tlsf_allocator& tlsf_allocator::operator=(tlsf_allocator&& that)
{
	if (this != &that)
	{
		deinitialize();
		heap = that.heap; that.heap = nullptr;
		heap_size = that.heap_size; that.heap_size = 0;
		stats = that.stats; that.stats = allocator_stats();
	}

	return *this;
}

allocator_stats tlsf_allocator::get_stats() const
{
	allocator_stats s = stats;
	walk_stats ws;
	o_tlsf_walk_heap(heap, find_largest_free_block, &ws);
	s.largest_free_block_bytes = ws.largest_free_block_bytes;
	s.num_free_blocks = ws.num_free_blocks;
	return s;
}

void* tlsf_allocator::allocate(size_t bytes, const char* label, const allocate_options& options)
{
	bytes = std::max(bytes, size_t(1));

	#if USE_LABEL
		// tlsf uses a pointer at the end of a free block. When allocated, overwrite
		// that location with a label for debugging memory stomps.
		bytes += sizeof(const char*);
	#endif

	size_t align = options.get_alignment();
	void* p = align == 16 ? tlsf_malloc(heap, bytes) : tlsf_memalign(heap, align, bytes);
	if (p)
	{
		size_t block_size = tlsf_block_size(p);
		#if USE_ALLOCATOR_STATS
			stats.num_allocations++;
			stats.num_allocations_peak = std::max(stats.num_allocations_peak, stats.num_allocations);
			stats.allocated_bytes += block_size;
			stats.allocated_bytes_peak = std::max(stats.allocated_bytes_peak, stats.allocated_bytes);
		#endif

		#if USE_LABEL
			const char** label_dst = (const char**)byte_add(p, block_size - sizeof(const char*));
			*label_dst = label;
		#endif
	}

	#if USE_ALLOCATION_STATS
		allocation_stats s;
		s.pointer = p;
		s.label = label;
		s.size = bytes;
		s.options = options;
		s.ordinal = 0;
		s.frame = 0;
		s.operation = memory_operation::allocate;
		default_allocate_track(0, s);
	#endif

	return p;
}

void* tlsf_allocator::reallocate(void* ptr, size_t bytes)
{
	size_t block_size = ptr ? tlsf_block_size(ptr) : 0;
	void* p = tlsf_realloc(heap, ptr, bytes);
	if (p)
	{
		size_t diff = tlsf_block_size(p) - block_size;
		#if USE_ALLOCATOR_STATS
			stats.allocated_bytes += diff;
			stats.allocated_bytes_peak = std::max(stats.allocated_bytes_peak, stats.allocated_bytes);
		#endif
	}

	#if USE_ALLOCATION_STATS
		allocation_stats s;
		s.pointer = p;
		s.label = nullptr;
		s.size = bytes;
		s.options = allocate_options();
		s.ordinal = 0;
		s.frame = 0;
		s.operation = memory_operation::reallocate;
		default_allocate_track(0, s);
	#endif

	return p;
}

void tlsf_allocator::deallocate(void* ptr)
{
	if (ptr)
	{
		const size_t block_size = tlsf_block_size(ptr);
		tlsf_free(heap, ptr);

		#if USE_ALLOCATOR_STATS
			stats.num_allocations--;
			stats.allocated_bytes -= block_size;
		#endif
	}

	#if USE_ALLOCATION_STATS
		allocation_stats s;
		s.pointer = ptr;
		s.label = nullptr;
		s.size = 0;
		s.options = allocate_options();
		s.ordinal = 0;
		s.frame = 0;
		s.operation = memory_operation::deallocate;
		default_allocate_track(0, s);
	#endif
}

size_t tlsf_allocator::size(void* ptr) const 
{
	return tlsf_block_size(ptr);
}

bool tlsf_allocator::owns(void* ptr) const
{
	return ptr >= heap && ptr < byte_add(heap, heap_size);
}

bool tlsf_allocator::valid() const
{
	return heap && !tlsf_check(heap);
}

void tlsf_allocator::reset()
{
	if (!heap || !heap_size)
		throw allocate_error(allocate_errc::corrupt);
	heap = tlsf_create_with_pool(heap, heap_size);

	#if USE_ALLOCATOR_STATS
		stats = allocator_stats();
		stats.capacity_bytes = heap_size - tlsf_size();
	#endif
}

static void tlsf_walker(void* ptr, size_t bytes, int used, void* user)
{
	std::function<void(void* ptr, size_t bytes, bool used)>& 
		walker = *(std::function<void(void* ptr, size_t bytes, bool used)>*)user;
	walker(ptr, bytes, !!used);
}

void tlsf_allocator::walk_heap(const std::function<void(void* ptr, size_t bytes, bool used)>& enumerator)
{
	if (enumerator)
		o_tlsf_walk_heap(heap, tlsf_walker, (void*)&enumerator);
}

}
