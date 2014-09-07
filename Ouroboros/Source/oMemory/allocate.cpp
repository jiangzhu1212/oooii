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

namespace detail
{
	class allocate_category_impl : public std::error_category
	{
	public:
		const char* name() const override { return "allocate"; }
		std::string message(int errcode) const override
		{
			switch (errcode)
			{
				case allocate_errc::invalid: return "invalid: allocator is not in a valid state";
				case allocate_errc::invalid_ptr: return "invalid_ptr: deallocate attempt on a pointer not managed by the allocator";
				case allocate_errc::out_of_memory: return "out_of_memory: there is no memory available from this allocator";
				case allocate_errc::fragmented: return "fragmented: not enough contiguous memory to fulfill request";
				case allocate_errc::corrupt: return "corrupt: allocator bookkeeping is corrupt";
				case allocate_errc::alignment: return "alignment: allocator arena is aligned incorrectly";
				case allocate_errc::outstanding_allocations: return "outstanding_allocations: an allocator is being destroyed while there are still allocations";
				default: return "unrecognized allocate error";
			}
		}
	};
}

const std::error_category& allocate_category()
{
	static detail::allocate_category_impl sSingleton;
	return sSingleton;
}

}
