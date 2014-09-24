// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/small_block_allocator.h>
#include <oMemory/byte.h>
#include <stdexcept>

namespace ouro {

small_block_allocator::small_block_allocator()
{
	blksizes.fill(chunk_t::nullidx);
	partials.fill(chunk_t::nullidx);
}

small_block_allocator::small_block_allocator(small_block_allocator&& that)
	: chunks(std::move(that.chunks))
	, blksizes(std::move(that.blksizes))
	, partials(std::move(that.partials))
{
}

small_block_allocator::small_block_allocator(void* memory, size_type bytes, const uint16_t* block_sizes, size_type num_block_sizes)
{
	initialize(memory, bytes, block_sizes, num_block_sizes);
}

small_block_allocator::~small_block_allocator()
{
	deinitialize();
}

small_block_allocator& small_block_allocator::operator=(small_block_allocator&& that)
{
	if (this != &that)
	{
		deinitialize();

		chunks = std::move(that.chunks);
		blksizes = std::move(that.blksizes);
		partials = std::move(that.partials);
	}

	return *this;
}

void small_block_allocator::initialize(void* memory, size_type bytes, const uint16_t* block_sizes, size_type num_block_sizes)
{
	if (!byte_aligned(memory, chunk_size))
		throw std::invalid_argument("improperly aligned memory");
	if (!byte_aligned(bytes, chunk_size))
		throw std::invalid_argument("improperly sized memory");

	memcpy(blksizes.data(), block_sizes, sizeof(uint16_t) * std::min(max_num_block_sizes, num_block_sizes));
	
	// fill in any extra room with repeated value
	for (auto i = num_block_sizes, j = num_block_sizes-1; i < max_num_block_sizes; i++)
		blksizes[i] = blksizes[j];

	partials.fill(chunk_t::nullidx);

	const size_type cap = bytes / chunk_size;
	chunks.initialize(memory, chunk_size, cap);
}

void* small_block_allocator::deinitialize()
{
	blksizes.fill(chunk_t::nullidx);
	partials.fill(chunk_t::nullidx);
	return chunks.deinitialize();
}

void small_block_allocator::remove_chunk(chunk_t* c)
{
	if (c->prev != chunk_t::nullidx)
	{
		chunk_t* prev = (chunk_t*)chunks.pointer(c->prev);
		prev->next = c->next;
		c->prev = chunk_t::nullidx;
	}

	else
	{
		// is head, so update that pointer
		partials[c->bin] = c->next;
	}

	if (c->next != chunk_t::nullidx)
	{
		chunk_t* next = (chunk_t*)chunks.pointer(c->next);
		next->prev = c->prev;
		c->next = chunk_t::nullidx;
	}
}

uint16_t small_block_allocator::find_bin(size_t size)
{
	uint16_t lower = 0, upper = max_num_block_sizes-1;
	do
	{
		const uint16_t mid = (upper+lower) / 2;
		const size_t blksize = blksizes[mid];
		if (size == blksize)
			return mid;
		else if (size < blksize)
			upper = mid - 1;
		else
			lower = mid + 1;

	} while (upper >= lower);

	return chunk_t::nullidx;
}

void* small_block_allocator::allocate(size_t size)
{
	uint16_t bin = find_bin(size);
	if (bin == chunk_t::nullidx)
		return nullptr;

	uint16_t chunki = partials[bin];
	chunk_t* c = chunki == chunk_t::nullidx ? nullptr : (chunk_t*)chunks.pointer(chunki);
			
	// if there's no partial chunks allocate a new one to service this size
	if (!c)
	{
		chunki = (uint16_t)chunks.allocate_index();
		if (chunki == chunk_t::nullidx)
			return nullptr;
		c = (chunk_t*)chunks.pointer(chunki);
		void* mem = byte_align(&c[1], sizeof(void*));
		size_type cap = size_type((chunks.block_size() - byte_diff(mem, c)) / size);
		c->pool.initialize(mem, size_type(size), cap);
		c->prev = chunk_t::nullidx;
		c->next = chunk_t::nullidx;
		c->bin = bin;
		partials[bin] = chunki;
	}

	void* p = c->pool.allocate();
			
	// if allocating the last free block, remove the chunk from the list since it 
	// cannot service future requests.
	if (c->pool.empty())
		remove_chunk(c);
	return p;
}

void small_block_allocator::deallocate(void* ptr)
{
	if (!ptr)
		return;

	// aligning the ptr will get to its owning chunk
	chunk_t* c = byte_align_down((chunk_t*)ptr, chunk_size);

	const bool was_empty = c->pool.empty(); // record state prior to insertion
	c->pool.deallocate(ptr);

	// if this is the last block then we can free up this chunk for other sizes
	if (c->pool.full())
	{
		remove_chunk(c);
		c->pool.deinitialize(); // not really needed, maybe make this debug-only
		chunks.deallocate(c);
	}

	else if (was_empty) // empty chunks are in limbo, so reattach this to partials
	{
		const uint16_t bin = c->bin;
		const uint16_t head = partials[bin];
		const uint16_t idx = (uint16_t)chunks.index(c);
		c->prev = chunk_t::nullidx;
		c->next = head;
		partials[bin] = idx;

		if (head != chunk_t::nullidx)
		{
			chunk_t* old_head = (chunk_t*)chunks.pointer(head);
			old_head->prev = idx;
		}
	}
}

}
