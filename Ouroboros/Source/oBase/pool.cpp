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
#include <oBase/pool.h>
#include <oBase/allocate.h>
#include <oBase/bit.h>
#include <oBase/byte.h>
#include <oBase/macros.h>
#include <stdexcept>

namespace ouro {

pool::pool()
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, nfree(0)
	, head(nullidx)
	, owns_memory(false)
{}

pool::pool(pool&& _That)
	: blocks(_That.blocks)
	, stride(_That.stride)
	, nblocks(_That.nblocks)
	, nfree(_That.nfree)
	, head(_That.head)
	, owns_memory(_That.owns_memory)
{ 
	_That.owns_memory = false;
	_That.deinitialize();
}

pool::pool(void* memory, size_type block_size, size_type capacity, size_type alignment)
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, nfree(0)
	, head(nullidx)
	, owns_memory(false)
{
	if (!initialize(memory, block_size, capacity, alignment))
		throw std::invalid_argument("pool initialize failed");
}

pool::pool(size_type block_size, size_type capacity, size_type alignment)
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, head(nullidx)
	, owns_memory(false)
{
	if (!initialize(block_size, capacity, alignment))
		throw std::invalid_argument("pool initialize failed");
}

pool::~pool()
{
	deinitialize();
}

pool& pool::operator=(pool&& _That)
{
	if (this != &_That)
	{
		deinitialize();

		oMOVE0(blocks);
		oMOVE0(stride);
		oMOVE0(nblocks);
		oMOVE0(nfree);
		oMOVE0(owns_memory);
		head = _That.head; _That.head = nullidx;
	}

	return *this;
}

pool::index_type pool::initialize(void* memory, size_type block_size, size_type capacity, size_type alignment)
{
	if (capacity > max_capacity())
		return 0;

	if (block_size < sizeof(index_type))
		return 0;

	if (!ispow2(alignment))
		return 0;

	size_type req = __max(block_size, sizeof(index_type)) * capacity;
	if (memory)
	{
		if (!byte_aligned(memory, alignment))
			return 0;

		head = 0;
		blocks = memory;
		stride = block_size;
		nblocks = capacity;
		nfree = capacity;
		const index_type n = nblocks - 1;
		for (index_type i = 0; i < n; i++)
			*byte_add((index_type*)blocks, stride, i) = i + 1;
		*byte_add((index_type*)blocks, stride, n) = nullidx;
	}

	return req;
}

pool::size_type pool::initialize(size_type block_size, size_type capacity, size_type block_alignment)
{
	size_type req = initialize(nullptr, block_size, capacity, block_alignment);
	if (!req)
		return 0;

	allocate_options o;
	o.alignment = bithigh(block_alignment);

	void* p = default_allocate(req, o);
	return initialize(p, block_size, capacity, block_alignment);
}

void* pool::deinitialize()
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

pool::index_type pool::allocate()
{
	index_type i = index_type(head);
	if (i == nullidx)
		return nullidx;
	head = *(index_type*)pointer(i);
	nfree--;
	return i;
}

void pool::deallocate(index_type index)
{
	if (!owns(index))
		throw std::out_of_range("the specified index was not allocated from this allocator");
	*(index_type*)pointer(index) = index_type(head);
	head = index;
	nfree++;
}

// convert between allocated index and pointer values
void* pool::pointer(index_type index) const
{
	return index != nullidx ? byte_add(blocks, stride, index) : nullptr;
}

pool::index_type pool::index(void* pointer) const
{
	return pointer ? (index_type)index_of(pointer, blocks, stride) : nullidx;
}

}
