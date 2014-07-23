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
#include <oBase/sbb_allocator.h>
#include <oBase/sbb.h>
#include <oBase/assert.h>
#include <oBase/byte.h>
#include <oBase/fixed_string.h>
#include <oBase/throw.h>

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
	size_t bookkeeping_size = sbb_bookkeeping_size(bytes, default_alignment);

	if (bookkeeping)
	{
		deinitialize();

		heap = bookkeeping;
		heap_size = bytes;

		heap = sbb_create(arena, heap_size, default_alignment, heap);
		stats = allocate_stats();
		stats.capacity_bytes = heap_size - sbb_overhead(heap);
	}

	return bookkeeping_size;
}

static void trace_leaks(void* ptr, size_t bytes, int used, void* user)
{
	if (used)
	{
		sstring mem;
		format_bytes(mem, bytes, 2);
		oTRACE("sbb leak: 0x%p %s", ptr, mem.c_str());
	}
}

void* sbb_allocator::deinitialize()
{
	if (stats.num_allocations)
	{
		sbb_walk_heap(heap, trace_leaks, nullptr);
		throw std::runtime_error(formatf("allocator destroyed with %u outstanding allocations", stats.num_allocations));
	}

	if (!valid())
		throw std::runtime_error("sbb heap is corrupt");

	void* arena = heap;
	sbb_destroy(heap);
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
		stats = that.stats; that.stats = allocate_stats();
	}

	return *this;
}

allocate_stats sbb_allocator::get_stats() const
{
	allocate_stats s = stats;
	walk_stats ws;
	sbb_walk_heap(heap, find_largest_free_block, &ws);
	s.largest_free_block_bytes = ws.largest_free_block_bytes;
	s.num_free_blocks = ws.num_free_blocks;
	return s;
}

const void* sbb_allocator::arena() const
{
	return sbb_arena(heap);
}

size_t sbb_allocator::arena_size() const
{
	return sbb_arena_bytes(heap);
}

void* sbb_allocator::allocate(size_t bytes, size_t align)
{
	void* p = sbb_memalign(heap, align, max(bytes, size_t(1)));
	if (p)
	{
		size_t block_size = nextpow2(bytes);//sbb_block_size(p);
		stats.num_allocations++;
		stats.num_allocations_peak = max(stats.num_allocations_peak, stats.num_allocations);
		stats.allocated_bytes += block_size;
		stats.allocated_bytes_peak = max(stats.allocated_bytes_peak, stats.allocated_bytes);
	}

	return p;
}

void* sbb_allocator::reallocate(void* ptr, size_t bytes)
{
	size_t block_size = ptr ? sbb_block_size(heap, ptr) : 0;
	void* p = sbb_realloc(heap, ptr, bytes);
	if (p)
	{
		size_t diff = nextpow2(bytes)/*sbb_block_size(p)*/ - block_size;
		stats.allocated_bytes += diff;
		stats.allocated_bytes_peak = max(stats.allocated_bytes_peak, stats.allocated_bytes);
	}
	return p;
}

void sbb_allocator::deallocate(void* ptr)
{
	if (ptr)
	{
		const size_t block_size = sbb_block_size(heap, ptr); // kinda slow... should sbb be modified to return block size freed?
		sbb_free(heap, ptr);
		stats.num_allocations--;
		stats.allocated_bytes -= block_size;
	}
}

size_t sbb_allocator::size(void* ptr) const 
{
	return sbb_block_size(heap, ptr);
}

bool sbb_allocator::in_range(void* ptr) const
{
	return ptr >= heap && ptr < byte_add(heap, heap_size);
}

bool sbb_allocator::valid() const
{
	return heap && !sbb_check_heap(heap);
}

void sbb_allocator::reset()
{
	if (!heap || !heap_size)
		throw std::runtime_error("allocator not valid");
	heap = sbb_create(sbb_arena(heap), heap_size, default_alignment, heap);
	stats = allocate_stats();
	stats.capacity_bytes = heap_size - sbb_overhead(heap);
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
		sbb_walk_heap(heap, sbb_walker, (void*)&enumerator);
}

} // namespace ouro
