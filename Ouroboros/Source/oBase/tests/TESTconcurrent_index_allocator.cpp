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
#include <oBase/concurrent_index_allocator.h>
#include <oBase/index_allocator.h>
#include <oBase/macros.h>
#include <oBase/throw.h>
#include <vector>

namespace ouro {
	namespace tests {

template<typename IndexAllocatorT>
static void test_index_allocator()
{
	const size_t CAPACITY = 4;
	std::vector<unsigned int> buffer(256, 0xcccccccc);

	IndexAllocatorT a(buffer.data(), CAPACITY);

	oCHECK(a.empty(), "index_allocator did not initialize correctly.");
	oCHECK(a.capacity() == CAPACITY, "Capacity mismatch.");

	unsigned int index[4];
	oFORI(i, index)
		index[i] = a.allocate();

	oCHECK(a.invalid_index == a.allocate(), "allocate succeed past allocator capacity");

	oFORI(i, index)
		oCHECK(index[i] == static_cast<unsigned int>(i), "Allocation mismatch %u.", i);

	a.deallocate(index[1]);
	a.deallocate(index[0]);
	a.deallocate(index[2]);
	a.deallocate(index[3]);

	oCHECK(a.empty(), "A deallocate failed.");
}

void TESTindex_allocator()
{
	test_index_allocator<index_allocator>();
}

void TESTconcurrent_index_allocator()
{
	test_index_allocator<concurrent_index_allocator>();
}

	} // namespace tests
} // namespace ouro
