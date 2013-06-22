/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#pragma once
#ifndef oConcurrency_index_allocator_base_h
#define oConcurrency_index_allocator_base_h

#include <oConcurrency/thread_safe.h>

namespace oConcurrency {

	class index_allocator_base
	{
	protected:
		static const unsigned int tag_bits = 8;
		static const unsigned int tag_mask = 0xff000000;
		static const unsigned int tagged_invalid_index = ~tag_mask;
		static const unsigned int tagged_max_index = ~tag_mask & (tagged_invalid_index - 1);

	public:
		static const unsigned int invalid_index = tagged_invalid_index;
		static const size_t index_size = sizeof(unsigned int);

		// deallocate all indices
		void reset();

		inline bool valid() const { return Arena != 0; }
		inline bool empty() const { return size() == 0; } // (SLOW! see size())

		// number of indices allocated (SLOW! this loops through entire freelist 
		// each call)
		size_t size() const;

		size_t capacity() const threadsafe;

		// This can be used to get the pointer passed to the constructor so it can
		// be freed if client code did not keep any other reference.
		void* const get_arena() const { return Arena; }

	protected:
		void* Arena;
		size_t ArenaBytes;
		unsigned int Freelist;

		size_t count_free(unsigned int _CurrentIndex, unsigned int _InvalidIndex) const;

		// size in bytes, not # of indices
		index_allocator_base(void* _pArena, size_t _SizeofArena);
		virtual ~index_allocator_base();

	private:
		index_allocator_base(const index_allocator_base&); /* = delete; */
		const index_allocator_base& operator=(index_allocator_base&); /* = delete; */

		index_allocator_base(index_allocator_base&&); /* = delete; */
		index_allocator_base& operator=(index_allocator_base&&); /* = delete; */
	};

} // namespace oConcurrency

#endif
