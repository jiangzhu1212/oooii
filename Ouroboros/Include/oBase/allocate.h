/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// A generic interface to allocating memory that can be passed around to 
// other systems.
#ifndef oBase_allocate_h
#define oBase_allocate_h

#include <utility>

namespace ouro {

/* enum class */ namespace memory_alignment
{	enum value {

	align_to_default,
	align_to_2,
	align_to_4,
	align_to_8,
	align_to_16,
	align_to_32,
	align_to_64,
	align_to_128,
	align_to_256,
	align_to_512,
	align_to_1024,
	align_to_2048,
	align_to_4096,

	count,
	default_alignment = align_to_16,
	align_to_cache_line = align_to_64,

};}

/* enum class */ namespace memory_type
{	enum value {

	default_memory,
	io_read_write,
	cpu_writecombine,
	cpu_gpu_coherent,
	gpu_writecombine,
	gpu_readonly,
	gpu_on_chip,
	physical,

	count,

};}

/* enum class */ namespace subsystem
{	enum value {

	ai,
	app,
	bookkeeping,
	cpu,
	input,
	io,
	graphics,
	physics,
	sound,
	ui,

	count,

};}

union allocate_options
{
	allocate_options() : options(0) {}
	operator unsigned int() const { return options; }

	unsigned int options;
	struct
	{
		unsigned int alignment : 4; // memory_alignment::value, basically a shift to a power of two for alignment
		unsigned int type : 4; // the type of memory being required. More to come on this...
		unsigned int subsystem : 4; // subsystem::value
		unsigned int usage : 6; // subsystem-specific (thus often user-defined)
		unsigned int extra : 14; // passed through: can be used for user-data
	
		unsigned int get_alignment() const { return static_cast<memory_alignment::value>(alignment ? memory_alignment::default_alignment : 1 << alignment); }
	};
};

struct allocate_stats
{
	allocate_stats()
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

typedef void* (*allocate_fn)(size_t num_bytes, unsigned int options);
typedef void (*deallocate_fn)(const void* pointer);
typedef void (*deallocate_nonconst_fn)(void* pointer);

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
	allocator(allocate_fn _Allocate, deallocate_fn _Deallocate) : allocate(_Allocate), deallocate(_Deallocate) {}
	
	operator bool() const { return allocate && deallocate; }

	scoped_allocation scoped_allocate(size_t num_bytes, unsigned int options = memory_alignment::align_to_default) const { return scoped_allocation(allocate(num_bytes, options), num_bytes, deallocate); }

	template<typename T>
	T* construct(unsigned int options = memory_alignment::align_to_default) { void* p = allocate(sizeof(T), options); return (T*)new (p) T(); }
	
	template<typename T>
	T* construct_array(size_t _Capacity, unsigned int options = memory_alignment::align_to_default)
	{
		T* p = (T*)allocate(sizeof(T) * _Capacity, options);
		for (size_t i = 0; i < _Capacity; i++)
			new (p + i) T();
		return p;
	}

	template<typename T>
	void destroy(T* p) { if (p) p->~T(); }

	template<typename T>
	void destroy_array(T* p, size_t _Capacity) { if (p) { for (size_t i = 0; i < _Capacity; i++) p[i].~T(); deallocate(p); } }
};

void* default_allocate(size_t num_bytes, unsigned int options);
void default_deallocate(const void* pointer);

void* noop_allocate(size_t num_bytes, unsigned int options);
void noop_deallocate(const void* pointer);

extern allocator default_allocator;
extern allocator noop_allocator;

}

#endif
