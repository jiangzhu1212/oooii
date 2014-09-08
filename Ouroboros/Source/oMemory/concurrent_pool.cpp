// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/concurrent_pool.h>
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
	: next(nullptr)
	, blocks(nullptr)
	, stride(0)
	, nblocks(0)
{
	tagged h(nullidx);
	head = h.all;
}

concurrent_pool::concurrent_pool(concurrent_pool&& that)
	: next(that.next)
	, blocks(that.blocks)
	, stride(that.stride)
	, nblocks(that.nblocks)
	, head(that.head)
{ 
	that.deinitialize();
}

concurrent_pool::concurrent_pool(void* memory, size_type block_size, size_type capacity)
	: blocks(nullptr)
	, stride(0)
	, nblocks(0)
{
	initialize(memory, block_size, capacity);
}

concurrent_pool::~concurrent_pool()
{
	deinitialize();
}

concurrent_pool& concurrent_pool::operator=(concurrent_pool&& that)
{
	if (this != &that)
	{
		deinitialize();

		next = that.next; that.next = nullptr;
		blocks = that.blocks; that.blocks = nullptr;
		stride = that.stride; that.stride = 0;
		nblocks = that.nblocks; that.nblocks = 0;
		head = that.head; that.head = nullidx;
	}

	return *this;
}

concurrent_pool::size_type concurrent_pool::initialize(void* memory, size_type block_size, size_type capacity)
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
		const index_type n = nblocks - 1;
		for (index_type i = 0; i < n; i++)
			*(index_type*)(stride*i + blocks) = i + 1;
		*(index_type*)(stride*n + blocks) = nullidx;
	}
	
	return req;
}

void* concurrent_pool::deinitialize()
{
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
		i = *(index_type*)(stride*i + blocks);
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
		n.index = *(index_type*)(stride*i + blocks);
	} while (!head.compare_exchange_strong(o.all, n.all));
	return i;
}

void concurrent_pool::deallocate(index_type index)
{
	tagged n, o(head);
	do
	{	*(index_type*)(stride*index + blocks) = o.index;
		n.tag = o.tag + 1;
		n.index = index;
	} while (!head.compare_exchange_strong(o.all, n.all));
}

// convert between allocated index and pointer values
void* concurrent_pool::pointer(index_type index) const
{
	return index != nullidx ? (stride*index + blocks) : nullptr;
}

concurrent_pool::index_type concurrent_pool::index(void* pointer) const
{
	return pointer ? index_type(((uint8_t*)pointer - blocks) / stride) : nullidx;
}

}
