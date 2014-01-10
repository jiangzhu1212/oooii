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
// Code common to thread local and concurrent index allocators
#pragma once
#ifndef oBase_index_allocator_base_h
#define oBase_index_allocator_base_h

#include <utility>

namespace ouro {

template<typename traits>
class index_allocator_base
{
public:
	static const unsigned int invalid_index = traits::index_mask;
	static const size_t index_size = sizeof(unsigned int);
	static const unsigned int max_index = traits::index_mask - 1;

	// deallocate all indices
	void reset()
	{
		// Seed list with next free index (like a next pointer in an slist)
		unsigned int* indices = static_cast<unsigned int*>(pArena);
		const size_t cap = capacity();
		for (unsigned int i = 0; i < cap; i++)
			indices[i] = i+1;
		indices[cap-1] = invalid_index; // last node has no next
		Freelist = 0;
	}

	// number of indices allocated (SLOW! this loops through entire freelist 
	// each call)
	size_t size() const
	{
		size_t nFree = 0;
		if (capacity())
			nFree = count_free(Freelist & invalid_index);
		return capacity() - nFree;
	}

	bool valid() const { return !!pArena; }
	bool empty() const { return size() == 0; } // (SLOW! see size())
	size_t capacity() const { return ArenaBytes / index_size; }

	// This can be used to get the pointer passed to the constructor so it can
	// be freed if client code did not keep any other reference.
	void* const get_arena() const { return pArena; }

protected:
	void* pArena;
	size_t ArenaBytes;
	typename traits::index_type Freelist;

	size_t count_free(unsigned int _CurrentIndex) const
	{
		size_t nFree = 0;
		while (_CurrentIndex != invalid_index)
		{
			nFree++;
			if (nFree > capacity())
				throw std::runtime_error("num free is more than the capacity");

			if (_CurrentIndex >= capacity())
				throw std::runtime_error(
				"while following the freelist, an index is "
				"present that is greater than the capacity");

			_CurrentIndex = static_cast<unsigned int*>(pArena)[_CurrentIndex];
		}

		return nFree;
	}

	index_allocator_base()
		: pArena(nullptr)
		, ArenaBytes(0)
		, Freelist(invalid_index)
	{}

	index_allocator_base& operator=(index_allocator_base&& _That)
	{
		if (this != &_That)
		{
			if (valid())
			{
				if (!empty())
					throw std::runtime_error("an index allocator has outstanding allocations");
			}

			pArena = _That.pArena; _That.pArena = nullptr;
			ArenaBytes = _That.ArenaBytes; _That.ArenaBytes = 0;
			
			Freelist = (unsigned int)_That.Freelist;
			_That.Freelist = invalid_index;
		}
		return *this;
	}

	// size in bytes, not # of indices
	index_allocator_base(void* _pArena, size_t _SizeofArena)
		: pArena(_pArena)
		, ArenaBytes(_SizeofArena)
		, Freelist(invalid_index)
	{
		if (capacity() > max_index)
			throw std::invalid_argument(
			"cannot index entire specified arena because part of the index is reserved "
			"(might be for tagging to prevent ABA in concurrent cases).");
		reset();
	}

	virtual ~index_allocator_base()
	{
		if (valid())
		{
			if (!empty())
				throw std::runtime_error("an index allocator has outstanding allocations");
			pArena = nullptr;
			ArenaBytes = 0;
		}
	}

protected:
	typedef traits traits_type;

private:
	index_allocator_base(const index_allocator_base&); /* = delete; */
	const index_allocator_base& operator=(index_allocator_base&); /* = delete; */
};

} // namespace ouro

#endif
