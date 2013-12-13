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
// Allocator implementation using the two-level segregated fit allocation 
// algorithm. This is a wrapper for the excellent allocator found at 
// http://tlsf.baisoku.org/.
#pragma once
#ifndef oBase_tlsf_allocator_h
#define oBase_tlsf_allocator_h

#include <functional>

namespace ouro {

class tlsf_allocator
{
public:
	static const size_t kDefaultAlignment = 16;

	struct stats
	{
		stats()
			: num_allocations(0)
			, bytes_allocated(0)
			, bytes_allocated_peak(0)
			, bytes_free(0)
		{}

		size_t num_allocations;
		size_t bytes_allocated;
		size_t bytes_allocated_peak;
		size_t bytes_free;
	};

	tlsf_allocator();
	tlsf_allocator(void* _pArena, size_t _Size);
	~tlsf_allocator();

	tlsf_allocator(tlsf_allocator&& _That);
	tlsf_allocator& operator=(tlsf_allocator&& _That);

	inline const void* arena() const { return pArena; }
	inline size_t arena_size() const { return Size; }
	inline stats get_stats() const { return Stats; }
	void* allocate(size_t _Size, size_t _Alignment = kDefaultAlignment);
	void* reallocate(void* _Pointer, size_t _Size);
	void deallocate(void* _Pointer);
	size_t size(void* _Pointer) const;
	bool in_range(void* _Pointer) const;
	bool valid() const;
	void reset();
	void walk_heap(const std::function<void(void* _Pointer, size_t _Size, bool _Used)>& _Enumerator);

private:
	tlsf_allocator(const tlsf_allocator&);
	const tlsf_allocator& operator=(const tlsf_allocator&);

	void* hHeap;
	void* pArena;
	size_t Size;
	stats Stats;
};

} // namespace ouro

#endif
