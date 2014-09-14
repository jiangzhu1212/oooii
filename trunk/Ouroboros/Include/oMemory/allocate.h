// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// A generic interface for allocating memory.

#pragma once

#include <cstdint>
#include <system_error>
#include <utility>

namespace ouro {

// _____________________________________________________________________________
// Error handling
	
enum class allocate_errc { invalid, invalid_ptr, out_of_memory, fragmented, corrupt, alignment, outstanding_allocations };

const std::error_category& allocate_category();

/*constexpr*/ inline std::error_code make_error_code(allocate_errc err_code) { return std::error_code(static_cast<int>(err_code), allocate_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(allocate_errc err_code) { return std::error_condition(static_cast<int>(err_code), allocate_category()); }

class allocate_error : public std::logic_error
{
public:
	allocate_error(allocate_errc err_code) 
		: logic_error(allocate_category().message(static_cast<int>(err_code))) {}
};

// _____________________________________________________________________________
// Definitions of standard memory configuration options

enum class memory_operation
{
	none,
	allocate,
	reallocate,
	deallocate,
	relocate,
	
	count, 
};

enum class memory_alignment
{
	align_default,
	align2,
	align4,
	align8,
	align16,
	align32,
	align64,
	align128,
	align256,
	align512,
	align1k,
	align2k,
	align4k,
	align8k,
	align16k,
	align32k,
	align64k,

	count,
	cacheline = align64,
	default_alignment = align16,
};

enum class memory_type
{
	cpu,
	cpu_writecombine,
	cpu_gpu_coherent,
	cpu_physical,
	cpu_physical_uncached,
	gpu_writecombine,
	gpu_readonly,
	gpu_on_chip,
	io_read_write,

	count,

};

union allocate_options
{
	allocate_options() : options(0) {}
	allocate_options(uint32_t o) : options(o) {}
	allocate_options(const memory_alignment& a) : options((uint32_t)a) {}
	operator uint32_t() const { return options; }

	uint32_t options;
	struct
	{
		uint32_t alignment : 6; // memory_alignment
		uint32_t type : 4;
		uint32_t category : 22; // passed through: can be used for user-markup
	};

	size_t get_alignment() const { return static_cast<size_t>(size_t(1) << (alignment ? alignment : (size_t)memory_alignment::align16)); }
};

// _____________________________________________________________________________
// Standard ways of tracking and reporting memory

struct allocation_stats
{
	allocation_stats()
		: pointer(nullptr)
		, label("unlabeled")
		, size(0)
		, options(0)
		, ordinal(0)
		, frame(0)
		, operation(memory_operation::none)
	{}

	void* pointer;
	const char* label;
	size_t size;
	allocate_options options;
	uint32_t ordinal;
	uint32_t frame;
	memory_operation operation;
};

struct allocator_stats
{
	allocator_stats()
		: allocated_bytes(0)
		, allocated_bytes_peak(0)
		, capacity_bytes(0)
		, num_allocations(0)
		, num_allocations_peak(0)
		, allocation_capacity(0)
		, largest_free_block_bytes(0)
		, num_free_blocks(0)
	{}

	size_t allocated_bytes;
	size_t allocated_bytes_peak;
	size_t capacity_bytes;
	size_t num_allocations;
	size_t num_allocations_peak;
	size_t allocation_capacity;
	size_t largest_free_block_bytes;
	size_t num_free_blocks;
};

// _____________________________________________________________________________
// Allocator definitions: callback functions, RAII pointer wrapper and a simple 
// allocator interface that can easily be passed around and retained by objects.

typedef void* (*allocate_fn)(size_t num_bytes, const allocate_options& options, const char* label);
typedef void (*deallocate_fn)(const void* pointer);
typedef void (*deallocate_nonconst_fn)(void* pointer);
typedef void (*allocate_track_fn)(uint64_t allocator_id, const allocation_stats& stats);
typedef void (*allocate_heap_walk_fn)(void* ptr, size_t bytes, int used, void* user);

class scoped_allocation
{
public:
	scoped_allocation()
		: pointer(nullptr)
		, num_bytes(0)
		, deallocate(nullptr)
	{}

	scoped_allocation(void* _pointer, size_t _num_bytes, deallocate_fn _deallocate)
		: pointer(_pointer)
		, num_bytes(_num_bytes)
		, deallocate(_deallocate)
	{}

	scoped_allocation(void* _pointer, size_t _num_bytes, deallocate_nonconst_fn _deallocate)
		: pointer(_pointer)
		, num_bytes(_num_bytes)
		, deallocate((deallocate_fn)_deallocate)
	{}

	scoped_allocation(scoped_allocation&& that) 
		: pointer(that.pointer)
		, num_bytes(that.num_bytes)
		, deallocate(that.deallocate)
	{
		that.pointer = nullptr;
		that.num_bytes = 0;
		that.deallocate = nullptr;
	}

	~scoped_allocation() { if (pointer && deallocate) deallocate(pointer); }

	scoped_allocation& operator=(scoped_allocation&& that)
	{
		if (this != &that)
		{
			if (pointer && deallocate) deallocate(pointer);
			pointer = that.pointer; that.pointer = nullptr;
			num_bytes = that.num_bytes; that.num_bytes = 0;
			deallocate = that.deallocate; that.deallocate = nullptr;
		}
		return *this;
	}

	void swap(scoped_allocation& that)
	{
		std::swap(pointer, that.pointer);
		std::swap(num_bytes, that.num_bytes);
		std::swap(deallocate, that.deallocate);
	}

	// Don't eviscerate size and deallocate functions so that release() can be passed as a parameter
	// to a function that might also want to take results from size() and get_deallocate()
	void* release() { void* p = pointer; pointer = nullptr; /*num_bytes = 0; deallocate = nullptr;*/ return p; }

	operator bool() const { return !!pointer; }
	template<typename T> operator T*() const { return static_cast<T*>(pointer); }
	size_t size() const { return num_bytes; }
	deallocate_fn get_deallocate() { return deallocate; }

private:
	void* pointer;
	size_t num_bytes;
	deallocate_fn deallocate;

	scoped_allocation(const scoped_allocation&);/* = delete; */
	const scoped_allocation& operator=(const scoped_allocation&);/* = delete; */
};

struct allocator
{
	allocate_fn allocate;
	deallocate_fn deallocate;

	allocator() : allocate(nullptr), deallocate(nullptr) {}
	allocator(allocate_fn alloc, deallocate_fn dealloc) : allocate(alloc), deallocate(dealloc) {}
	
	operator bool() const { return allocate && deallocate; }

	bool operator==(const allocator& that) const { return allocate == that.allocate && deallocate == that.deallocate; }

	scoped_allocation scoped_allocate(size_t num_bytes, const allocate_options& options = allocate_options(), const char* label = "") const { return scoped_allocation(allocate(num_bytes, options, label), num_bytes, deallocate); }

	template<typename T>
	T* construct(uint32_t options = memory_alignment::align_default, const char* label = "unlabeled") { void* p = allocate(sizeof(T), options, label); return (T*)new (p) T(); }
	
	template<typename T>
	T* construct_array(size_t capacity, const allocate_options& options = allocate_options(), const char* label = "unlabeled")
	{
		T* p = (T*)allocate(sizeof(T) * capacity, options, label);
		for (size_t i = 0; i < capacity; i++)
			new (p + i) T();
		return p;
	}

	template<typename T>
	void destroy(T* p) { if (p) p->~T(); }

	template<typename T>
	void destroy_array(T* p, size_t _Capacity) { if (p) { for (size_t i = 0; i < _Capacity; i++) p[i].~T(); deallocate(p); } }
};

// _____________________________________________________________________________
// Default implementations

void* default_allocate(size_t num_bytes, const allocate_options& options, const char* label = "");
void default_deallocate(const void* pointer);
void default_allocate_track(uint64_t allocation_id, const allocation_stats& stats);

void* noop_allocate(size_t num_bytes, const allocate_options& options, const char* label = "");
void noop_deallocate(const void* pointer);

extern allocator default_allocator;
extern allocator noop_allocator;

}
