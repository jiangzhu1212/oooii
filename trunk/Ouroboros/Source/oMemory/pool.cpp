// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/pool.h>
#include <stdexcept>

namespace ouro {

pool::pool()
	: next(nullptr)
	, blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, nfree(0)
	, head(nullidx)
{}

pool::pool(pool&& that)
	: next(that.next)
	, blocks(that.blocks)
	, stride(that.stride)
	, nblocks(that.nblocks)
	, nfree(that.nfree)
	, head(that.head)
{ 
	that.deinitialize();
}

pool::pool(void* memory, size_type block_size, size_type capacity)
	: next(nullptr)
	, blocks(nullptr)
	, stride(0)
	, nblocks(0)
	, nfree(0)
	, head(nullidx)
{
	initialize(memory, block_size, capacity);
}

pool::~pool()
{
	deinitialize();
}

pool& pool::operator=(pool&& that)
{
	if (this != &that)
	{
		deinitialize();

		next = that.next; that.next = nullptr;
		blocks = that.blocks; that.blocks = nullptr;
		stride = that.stride; that.stride = 0;
		nblocks = that.nblocks; that.nblocks = 0;
		nfree = that.nfree; that.nfree = 0;
		head = that.head; that.head = nullidx;
	}

	return *this;
}

pool::size_type pool::initialize(void* memory, size_type block_size, size_type capacity)
{
	if (capacity > max_capacity())
		std::invalid_argument("capacity is too large");

	if (block_size < sizeof(index_type))
		std::invalid_argument("block_size must be a minimum of 4 bytes");

	size_type req = std::max(block_size, size_type(sizeof(index_type))) * capacity;

	if (memory)
	{
		head = 0;
		blocks = (uint8_t*)memory;
		stride = block_size;
		nblocks = capacity;
		nfree = capacity;
		const index_type n = nblocks - 1;
		for (index_type i = 0; i < n; i++)
			*(index_type*)(stride*i + blocks) = i + 1;
		*(index_type*)(stride*n + blocks) = nullidx;
	}

	return req;
}

void* pool::deinitialize()
{
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
		throw std::invalid_argument("pool does not own the specified index or pointer");
	*(index_type*)pointer(index) = index_type(head);
	head = index;
	nfree++;
}

// convert between allocated index and pointer values
void* pool::pointer(index_type index) const
{
	return index != nullidx ? (stride*index + blocks) : nullptr;
}

pool::index_type pool::index(void* ptr) const
{
	return ptr ? index_type(((uint8_t*)ptr - blocks) / stride) : nullidx;
}

}
