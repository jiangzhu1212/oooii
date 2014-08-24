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
// Allocator implementation using the two-level segregated fit allocation 
// algorithm. This is a wrapper for the excellent allocator found at 
// http://tlsf.baisoku.org/.
#pragma once
#ifndef oBase_tlsf_allocator_h
#define oBase_tlsf_allocator_h

#include <oBase/allocate.h>
#include <functional>

namespace ouro {

class tlsf_allocator
{
public:
	static const size_t default_alignment = 16;

	typedef 

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
	allocate_stats get_stats() const;
	void* allocate(size_t bytes, size_t align = default_alignment);
	void* reallocate(void* ptr, size_t bytes);
	void deallocate(void* ptr);
	size_t size(void* ptr) const;
	bool in_range(void* ptr) const;
	bool valid() const;
	void reset();
	void walk_heap(const std::function<void(void* ptr, size_t bytes, bool used)>& enumerator);

private:
	tlsf_allocator(const tlsf_allocator&);
	const tlsf_allocator& operator=(const tlsf_allocator&);

	void* heap;
	size_t heap_size;
	allocate_stats stats;
};

}

#endif
