// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/sbb_allocator.h>
#include <oBase/sbb.h>
#include <oString/string.h>
#include <oBase/assert.h>

#define USE_ALLOCATOR_STATS 1
#define USE_ALLOCATION_STATS 1

namespace ouro {

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
		stats->largest_free_block_bytes = max(stats->largest_free_block_bytes, bytes);
		stats->num_free_blocks++;
	}
}

size_t sbb_allocator::initialize(void* arena, size_t bytes, void* bookkeeping)
{
	size_t bookkeeping_size = sbb_bookkeeping_size(bytes, 16);

	if (bookkeeping)
	{
		deinitialize();

		heap = bookkeeping;
		heap_size = bytes;

		heap = sbb_create(arena, heap_size, 16, heap);

		#if USE_ALLOCATOR_STATS
			stats = allocator_stats();
			stats.capacity_bytes = heap_size - sbb_overhead((sbb_t)heap);
		#endif
	}

	return bookkeeping_size;
}

static void trace_leaks(void* ptr, size_t bytes, int used, void* user)
{
	if (used)
	{
		char mem[64];
		format_bytes(mem, bytes, 2);
		oTRACE("tlsf leak: 0x%p %s", ptr, mem);
	}
}

void* sbb_allocator::deinitialize()
{
	#if USE_ALLOCATOR_STATS
		if (stats.num_allocations)
		{
			sbb_walk_heap((sbb_t)heap, trace_leaks, nullptr);
			char str[96];
			snprintf(str, "allocator destroyed with %u outstanding allocations", stats.num_allocations);
			throw std::runtime_error(str);
		}
	#endif

	if (!valid())
		throw std::runtime_error("sbb heap is corrupt");

	void* arena = heap;
	sbb_destroy((sbb_t)heap);
	heap = nullptr;
	heap_size = 0;
	return arena;
}

sbb_allocator::sbb_allocator(sbb_allocator&& that)
{
	operator=(std::move(that));
}

sbb_allocator& sbb_allocator::operator=(sbb_allocator&& that)
{
	if (this != &that)
	{
		heap = that.heap; that.heap = nullptr;
		heap_size = that.heap_size; that.heap_size = 0;
		stats = that.stats; that.stats = allocator_stats();
	}

	return *this;
}

allocator_stats sbb_allocator::get_stats() const
{
	allocator_stats s = stats;
	walk_stats ws;
	sbb_walk_heap((sbb_t)heap, find_largest_free_block, &ws);
	s.largest_free_block_bytes = ws.largest_free_block_bytes;
	s.num_free_blocks = ws.num_free_blocks;
	return s;
}

const void* sbb_allocator::arena() const
{
	return sbb_arena((sbb_t)heap);
}

size_t sbb_allocator::arena_size() const
{
	return sbb_arena_bytes((sbb_t)heap);
}

void* sbb_allocator::allocate(size_t bytes, const char* label, const allocate_options& options)
{
	bytes = max(bytes, size_t(1));
	size_t align = options.get_alignment();
	void* p = align == 16 ? sbb_malloc((sbb_t)heap, bytes) : sbb_memalign((sbb_t)heap, align, bytes);
	if (p)
	{
		size_t block_size = nextpow2(bytes);//sbb_block_size(p);

		#if USE_ALLOCATOR_STATS
			stats.num_allocations++;
			stats.num_allocations_peak = max(stats.num_allocations_peak, stats.num_allocations);
			stats.allocated_bytes += block_size;
			stats.allocated_bytes_peak = max(stats.allocated_bytes_peak, stats.allocated_bytes);
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

void* sbb_allocator::reallocate(void* ptr, size_t bytes)
{
	size_t block_size = ptr ? sbb_block_size((sbb_t)heap, ptr) : 0;
	void* p = sbb_realloc((sbb_t)heap, ptr, bytes);
	if (p)
	{
		size_t diff = nextpow2(bytes)/*sbb_block_size(p)*/ - block_size;

		#if USE_ALLOCATOR_STATS
			stats.allocated_bytes += diff;
			stats.allocated_bytes_peak = max(stats.allocated_bytes_peak, stats.allocated_bytes);
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

void sbb_allocator::deallocate(void* ptr)
{
	if (ptr)
	{
		const size_t block_size = sbb_block_size((sbb_t)heap, ptr); // kinda slow... should sbb be modified to return block size freed?
		sbb_free((sbb_t)heap, ptr);

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

size_t sbb_allocator::size(void* ptr) const 
{
	return sbb_block_size((sbb_t)heap, ptr);
}

bool sbb_allocator::in_range(void* ptr) const
{
	return ptr >= heap && ptr < ((uint8_t*)heap + heap_size);
}

bool sbb_allocator::valid() const
{
	return heap && !sbb_check_heap((sbb_t)heap);
}

void sbb_allocator::reset()
{
	if (!heap || !heap_size)
		throw std::runtime_error("allocator not valid");
	heap = sbb_create(sbb_arena((sbb_t)heap), heap_size, 16, heap);

	#if USE_ALLOCATOR_STATS
		stats = allocator_stats();
		stats.capacity_bytes = heap_size - sbb_overhead((sbb_t)heap);
	#endif
}

static void sbb_walker(void* ptr, size_t bytes, int used, void* user)
{
	std::function<void(void* ptr, size_t bytes, bool used)>& 
		walker = *(std::function<void(void* ptr, size_t bytes, bool used)>*)user;
	walker(ptr, bytes, !!used);
}

void sbb_allocator::walk_heap(const std::function<void(void* ptr, size_t bytes, bool used)>& enumerator)
{
	if (enumerator)
		sbb_walk_heap((sbb_t)heap, sbb_walker, (void*)&enumerator);
}

}
