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
#include <oBase/tlsf_allocator.h>
#include <oBase/assert.h>
#include <oBase/byte.h>
#include <oBase/compiler_config.h>
#include <oBase/fixed_string.h>
#include <oBase/throw.h>
#include <tlsf.h>

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

void tlsf_allocator::initialize(void* arena, size_t bytes)
{
	oCHECK(byte_aligned(arena, default_alignment), "tlsf arena must be 16-byte aligned");
	reset();
}

static void trace_leaks(void* ptr, size_t bytes, int used, void* user)
{
	if (used)
	{
		sstring mem;
		format_bytes(mem, bytes, 2);
		oTRACE("tlsf leak: 0x%p %s", ptr, mem.c_str());
	}
}

void* tlsf_allocator::deinitialize()
{
	if (stats.num_allocations)
	{
		tlsf_walk_heap(heap, trace_leaks, nullptr);
		throw std::runtime_error(formatf("allocator destroyed with %u outstanding allocations", stats.num_allocations));
	}

	if (!valid())
		throw std::runtime_error("tlsf heap is corrupt");

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
		heap = that.heap; that.heap = nullptr;
		heap_size = that.heap_size; that.heap_size = 0;
		stats = that.stats; that.stats = allocate_stats();
	}

	return *this;
}

allocate_stats tlsf_allocator::get_stats() const
{
	allocate_stats s = stats;
	walk_stats ws;
	tlsf_walk_heap(heap, find_largest_free_block, &ws);
	s.largest_free_block_bytes = ws.largest_free_block_bytes;
	s.num_free_blocks = ws.num_free_blocks;
	return s;
}

void* tlsf_allocator::allocate(size_t bytes, size_t align)
{
	void* p = tlsf_memalign(heap, align, max(bytes, size_t(1)));
	if (p)
	{
		size_t block_size = tlsf_block_size(p);
		stats.num_allocations++;
		stats.num_allocations_peak = max(stats.num_allocations_peak, stats.num_allocations);
		stats.allocated_bytes += block_size;
		stats.allocated_bytes_peak = max(stats.allocated_bytes_peak, stats.allocated_bytes);
	}

	return p;
}

void* tlsf_allocator::reallocate(void* ptr, size_t bytes)
{
	size_t block_size = ptr ? tlsf_block_size(ptr) : 0;
	void* p = tlsf_realloc(heap, ptr, bytes);
	if (p)
	{
		size_t diff = tlsf_block_size(p) - block_size;
		stats.allocated_bytes += diff;
		stats.allocated_bytes_peak = max(stats.allocated_bytes_peak, stats.allocated_bytes);
	}
	return p;
}

void tlsf_allocator::deallocate(void* ptr)
{
	if (ptr)
	{
		const size_t block_size = tlsf_block_size(ptr);
		tlsf_free(heap, ptr);
		stats.num_allocations--;
		stats.allocated_bytes -= block_size;
	}
}

size_t tlsf_allocator::size(void* ptr) const 
{
	return tlsf_block_size(ptr);
}

bool tlsf_allocator::in_range(void* ptr) const
{
	return ptr >= heap && ptr < byte_add(heap, heap_size);
}

bool tlsf_allocator::valid() const
{
	return heap && !tlsf_check_heap(heap);
}

void tlsf_allocator::reset()
{
	if (!heap || !heap_size)
		throw std::runtime_error("allocator not valid");
	heap = tlsf_create(heap, heap_size);
	stats = allocate_stats();
	stats.capacity_bytes = heap_size - tlsf_overhead();
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
		tlsf_walk_heap(heap, tlsf_walker, (void*)&enumerator);
}

} // namespace ouro
