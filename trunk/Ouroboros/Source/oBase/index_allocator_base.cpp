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
#include <oBase/index_allocator_base.h>

namespace ouro {

index_allocator_base::index_allocator_base(void* _pArena, size_t _SizeofArena)
	: Arena(_pArena)
	, ArenaBytes(_SizeofArena)
	, Freelist(invalid_index)
{
	if (capacity() > tagged_max_index)
		throw std::invalid_argument(
		"cannot index entire specified arena because several bits are reserved for "
		"tagging to address concurrency ABA issues.");

	reset();
}

index_allocator_base::~index_allocator_base()
{
	if (valid())
	{
		if (!empty())
			throw std::runtime_error("an index allocator has outstanding allocations");
		Arena = nullptr;
		ArenaBytes = 0;
	}
}

size_t index_allocator_base::capacity() const
{
	return ArenaBytes / index_size;
}

void index_allocator_base::reset()
{
	// Seed list with next free index (like a next pointer in an slist)
	unsigned int* indices = static_cast<unsigned int*>(Arena);
	const size_t cap = capacity();
	for (unsigned int i = 0; i < cap; i++)
		indices[i] = i+1;
	indices[cap-1] = invalid_index; // last node has no next
	Freelist = 0;
}

size_t index_allocator_base::count_free(unsigned int _CurrentIndex, unsigned int _InvalidIndex) const
{
	size_t nFree = 0;
	while (_CurrentIndex != _InvalidIndex)
	{
		nFree++;
		if (nFree > capacity())
			throw std::runtime_error("num free is more than the capacity");

		if (_CurrentIndex >= capacity())
			throw std::runtime_error(
			"while following the freelist, an index is "
			"present that is greater than the capacity");

		_CurrentIndex = static_cast<unsigned int*>(Arena)[_CurrentIndex];
	}

	return nFree;
}

size_t index_allocator_base::size() const
{
	size_t nFree = 0;
	if (capacity())
		nFree = count_free(Freelist & ~tag_mask, tagged_invalid_index);
	return capacity() - nFree;
}

} // namespace ouro
