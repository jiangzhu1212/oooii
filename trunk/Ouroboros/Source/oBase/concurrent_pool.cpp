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
#include <oBase/concurrent_pool.h>
#include <oBase/allocate.h>
#include <oBase/bit.h>
#include <oBase/byte.h>
#include <oBase/macros.h>
#include <stdexcept>

namespace ouro {

static_assert(sizeof(concurrent_pool) == oCACHE_LINE_SIZE, "unexpected class size");
static_assert(sizeof(std::atomic_uint) == sizeof(unsigned int), "mismatch atomic size");

struct tagged
{
	tagged() {}
	tagged(unsigned int _all) : all(_all) {}

	union
	{
		unsigned int all;
		struct
		{
			unsigned int index : 24;
			unsigned int tag : 8;
		};
	};
};

concurrent_pool::concurrent_pool()
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, owns_memory(false)
{
	tagged h(nullidx);
	head = h.all;
	for (char& c : cache_padding)
		c = 0;
}

concurrent_pool::concurrent_pool(concurrent_pool&& _That)
	: blocks(_That.blocks)
	, stride(_That.stride)
	, nblocks(_That.nblocks)
	, head(_That.head)
	, owns_memory(_That.owns_memory)
{ 
	_That.owns_memory = false;
	_That.deinitialize();
}

concurrent_pool::concurrent_pool(void* memory, size_type block_size, size_type capacity, size_type alignment)
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, owns_memory(false)
{
	if (!initialize(memory, block_size, capacity, alignment))
		throw std::invalid_argument("concurrent_pool initialize failed");
}

concurrent_pool::concurrent_pool(size_type block_size, size_type capacity, size_type alignment)
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, owns_memory(false)
{
	if (!initialize(block_size, capacity, alignment))
		throw std::invalid_argument("concurrent_pool initialize failed");
}

concurrent_pool::~concurrent_pool()
{
	deinitialize();
}

concurrent_pool& concurrent_pool::operator=(concurrent_pool&& _That)
{
	if (this != &_That)
	{
		deinitialize();

		oMOVE0(blocks);
		oMOVE0(stride);
		oMOVE0(nblocks);
		oMOVE0(owns_memory);
		head = _That.head; _That.head = nullidx;
	}

	return *this;
}

concurrent_pool::index_type concurrent_pool::initialize(void* memory, size_type block_size, size_type capacity, size_type alignment)
{
	if (capacity > max_capacity())
		return 0;

	if (block_size < sizeof(index_type))
		return 0;

	if (!ispow2(alignment))
		return 0;

	for (char& c : cache_padding)
		c = 0;

	size_type req = __max(block_size, sizeof(index_type)) * capacity;
	if (memory)
	{
		if (!byte_aligned(memory, alignment))
			return 0;

		head = 0;
		blocks = memory;
		stride = block_size;
		nblocks = capacity;
		const index_type n = nblocks - 1;
		for (index_type i = 0; i < n; i++)
			*byte_add((index_type*)blocks, stride, i) = i + 1;
		*byte_add((index_type*)blocks, stride, n) = nullidx;
	}

	return req;
}

concurrent_pool::size_type concurrent_pool::initialize(size_type block_size, size_type capacity, size_type block_alignment)
{
	size_type req = initialize(nullptr, block_size, capacity, block_alignment);
	if (!req)
		return 0;

	allocate_options o;
	o.alignment = bithigh(block_alignment);

	void* p = default_allocate(req, o);
	return initialize(p, block_size, capacity, block_alignment);
}

void* concurrent_pool::deinitialize()
{
	if (owns_memory)
	{
		default_deallocate(blocks);
		blocks = nullptr;
	}

	void* p = blocks;
	blocks = nullptr;
	stride = 0;
	nblocks = 0;
	head = nullidx;
	return p;
}

concurrent_pool::size_type concurrent_pool::count_free() const
{
	tagged o(head);
	size_type n = 0;
	index_type i = o.index;
	while (i != nullidx)
	{
		n++;
		i = *byte_add((index_type*)blocks, stride, i);
	}
	return n;
}

bool concurrent_pool::empty() const
{
	tagged o(head);
	return o.index == nullidx;
}

concurrent_pool::index_type concurrent_pool::allocate()
{
	index_type i;
	tagged n, o(head);
	do
	{	i = o.index;
		if (i == nullidx)
			break;
		n.tag = o.tag + 1;
		n.index = *byte_add((index_type*)blocks, stride, i);
	} while (!head.compare_exchange_strong(o.all, n.all));
	return i;
}

void concurrent_pool::deallocate(index_type index)
{
	tagged n, o(head);
	do
	{	*byte_add((index_type*)blocks, stride, index) = o.index;
		n.tag = o.tag + 1;
		n.index = index;
	} while (!head.compare_exchange_strong(o.all, n.all));
}

// convert between allocated index and pointer values
void* concurrent_pool::pointer(index_type index) const
{
	return index != nullidx ? byte_add(blocks, stride, index) : nullptr;
}

concurrent_pool::index_type concurrent_pool::index(void* pointer) const
{
	return pointer ? (index_type)index_of(pointer, blocks, stride) : nullidx;
}

} // namespace ouro
