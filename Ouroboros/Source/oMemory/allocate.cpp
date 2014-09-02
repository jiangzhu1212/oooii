// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/allocate.h>
#include <oBase/macros.h>
#include <oBase/assert.h>

namespace ouro {

void* default_allocate(size_t size, const allocate_options& options, const char* label)
{
	return _aligned_malloc(size, options.get_alignment());
}

void default_deallocate(const void* pointer)
{
	_aligned_free((void*)pointer);
}

void* noop_allocate(size_t size, const allocate_options& options, const char* label)
{
	return nullptr;
}

void noop_deallocate(const void* pointer)
{
}

allocator default_allocator(default_allocate, default_deallocate);
allocator noop_allocator(noop_allocate, noop_deallocate);
	
const char* as_string(const memory_alignment& alignment)
{
	static const char* names[] = 
	{
		"align16 (default)",
		"align2",
		"align4",
		"align8",
		"align16 (default)",
		"align32",
		"align64 (cacheline)",
		"align128",
		"align256",
		"align512",
		"align1k",
		"align2k",
		"align4k",
		"align8k",
		"align16k",
		"align32k",
		"align64k",
	};
	static_assert(oCOUNTOF(names) == (uint32_t)memory_alignment::count, "array mismatch");
	return names[(uint32_t)alignment];
}

const char* as_string(const memory_type& type)
{
	static const char* names[] = 
	{
		"cpu",
		"cpu_writecombine",
		"cpu_gpu_coherent",
		"cpu_physical",
		"cpu_physical_uncached",
		"gpu_writecombine",
		"gpu_readonly",
		"gpu_on_chip",
		"io_read_write",
	};
	static_assert(oCOUNTOF(names) == (uint32_t)memory_type::count, "array mismatch");
	return names[(uint32_t)type];
}

void default_allocate_track(uint64_t allocation_id, const allocation_stats& stats)
{
}

}
