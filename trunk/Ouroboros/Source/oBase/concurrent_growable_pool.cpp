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
#include <oBase/concurrent_growable_pool.h>
#include <oBase/allocate.h>
#include <oMemory/byte.h>
#include <oBase/compiler_config.h>
#include <oBase/macros.h>
#include <stdexcept>

namespace ouro {

concurrent_growable_pool::concurrent_growable_pool()
	: last_allocate(nullptr)
	, last_deallocate(nullptr)
	, block_size(0)
	, block_alignment(0)
	, capacity_per_chunk(0)
{}

concurrent_growable_pool::concurrent_growable_pool(concurrent_growable_pool&& _That)
{
	oMOVE_ATOMIC0(last_allocate);
	oMOVE_ATOMIC0(last_deallocate);
	oMOVE0(block_size);
	oMOVE0(block_alignment);
	oMOVE0(capacity_per_chunk);
	chunks = std::move(_That.chunks);
}

concurrent_growable_pool::concurrent_growable_pool(size_type block_size, size_type capacity_per_chunk, size_type block_alignment)
	: last_allocate(nullptr)
	, last_deallocate(nullptr)
	, block_size(0)
	, block_alignment(0)
	, capacity_per_chunk(0)
{
	if (!initialize(block_size, capacity_per_chunk, block_alignment))
		throw std::exception("concurrent_growable_pool initialize failed");
}

concurrent_growable_pool::~concurrent_growable_pool()
{
	deinitialize();
}

concurrent_growable_pool& concurrent_growable_pool::operator=(concurrent_growable_pool&& _That)
{
	if (this != &_That)
	{
		oMOVE_ATOMIC0(last_allocate);
		oMOVE_ATOMIC0(last_deallocate);
		oMOVE0(block_size);
		oMOVE0(block_alignment);
		oMOVE0(capacity_per_chunk);
		chunks = std::move(chunks);
	}
	return *this;
}

bool concurrent_growable_pool::initialize(size_type _block_size, size_type _capacity_per_chunk, size_type _block_alignment)
{
	if (!chunks.empty())
		return false;
	last_allocate = nullptr;
	last_deallocate = nullptr;
	block_size = _block_size;
	block_alignment = __max(_block_alignment, oDEFAULT_MEMORY_ALIGNMENT);
	capacity_per_chunk = _capacity_per_chunk;
	grow(capacity_per_chunk);
	return true;
}

void concurrent_growable_pool::deinitialize()
{
	shrink(0);
	block_size = 0;
	capacity_per_chunk = 0;
}
	
concurrent_growable_pool::chunk_t* concurrent_growable_pool::allocate_chunk()
{
	concurrent_pool pool;
	size_type req = pool.initialize(nullptr, block_size, capacity_per_chunk);
	req = byte_align((size_type)sizeof(chunk_t), oCACHE_LINE_SIZE) + byte_align(req, block_alignment);
	chunk_t* c = new (default_allocate(req, memory_alignment::cacheline)) chunk_t();
	void* p = byte_align(c + 1, block_alignment);
	c->pool.initialize(p, block_size, capacity_per_chunk, block_alignment);
	c->next = nullptr;
	return c;
}

concurrent_pool* concurrent_growable_pool::find_pool(void* _Pointer) const
{
	concurrent_pool* pool = last_deallocate; // racy but only an optimization hint

	// check the cached version to avoid a linear lookup
	if (pool && pool->owns(_Pointer))
		return pool;

	// iterating over the list inside the stack is safe because we're only
	// ever going to allow growth of the list which since it occurs on top
	// of any prior head means the rest of the list remains valid for traversal
	chunk_t* c = chunks.peek();
	while (c)
	{
		if (c->pool.owns(_Pointer))
			return &c->pool;
		c = c->next;
	}

	return nullptr;
}

void concurrent_growable_pool::grow(size_type capacity)
{
	const size_type target_nchunks = (capacity + capacity_per_chunk - 1) / capacity_per_chunk;
	chunk_t* c = chunks.peek();
	size_type nchunks = num_chunks();
	while (nchunks++ < target_nchunks)
	{
		chunk_t* c = allocate_chunk();
		last_allocate = &c->pool; // racy but only an optimization hint
		chunks.push(c);
	}
}

void concurrent_growable_pool::shrink(size_type capacity)
{
	chunk_t* c = chunks.pop_all();
	chunk_t* to_free = nullptr;
	const size_type target_nchunks = (capacity + capacity_per_chunk - 1) / capacity_per_chunk;
	size_type nchunks = 0;
	while (c)
	{
		chunk_t* tmp = c;
		c = c->next;

		if (tmp->pool.full())
		{
			tmp->next = to_free;
			to_free = tmp;
		}

		else
			chunks.push(tmp);

		nchunks++;
	}

	c = to_free;
	while (c && nchunks > target_nchunks)
	{
		chunk_t* tmp = c;
		c->pool.deinitialize();
		c = c->next;
		default_deallocate(tmp);
		nchunks--;
	}

	while (c)
	{
		chunk_t* tmp = c;
		c = c->next;
		chunks.push(tmp);
	}

	// reset cached values
	last_allocate = last_deallocate = nullptr;
}

concurrent_growable_pool::size_type concurrent_growable_pool::num_chunks() const
{
	size_type n = 0;
	chunk_t* c = chunks.peek();
	while (c)
	{
		n++;
		c = c->next;
	}
	return n;
}

concurrent_growable_pool::size_type concurrent_growable_pool::count_free() const
{
	size_type n = 0;
	chunk_t* c = chunks.peek();
	while (c)
	{
		n += c->pool.count_free();
		c = c->next;
	}
	return n;
}

bool concurrent_growable_pool::owns(void* pointer) const
{
	if (pointer)
	{
		chunk_t* c = chunks.peek();
		while (c)
		{
			if (c->pool.owns(pointer))
				return true;
			c = c->next;
		}
	}
	return false;
}

void* concurrent_growable_pool::allocate_pointer()
{
	void* p = nullptr;

	// check cached last-used chunk (racy but it's only an optimization hint)
	concurrent_pool* pool = last_allocate;
	if (pool)
		p = pool->allocate_pointer();

	// search for another chunk
	if (!p)
	{
		// iterating over the list inside the stack is safe because we're only
		// ever going to allow growth of the list which since it occurs on top
		// of any prior head means the rest of the list remains valid for traversal
		chunk_t* c = chunks.peek();
		while (c)
		{
			p = c->pool.allocate_pointer();
			if (p)
			{
				last_allocate = &c->pool; // racy but only an optimization hint
				break;
			}
			c = c->next;
		}

		// still no memory? add another chunk
		if (!p)
		{
			chunk_t* new_chunk = allocate_chunk();
			p = new_chunk->pool.allocate_pointer();
			chunks.push(new_chunk);
			last_allocate = &new_chunk->pool; // racy but only an optimization hint
		}
	}

	return p;
}

void concurrent_growable_pool::deallocate(void* pointer)
{
	concurrent_pool* pool = find_pool(pointer);
	if (!pool)
		throw std::exception("deallocate called on a dangling pointer from a chunk that has probably been shrink()'ed");
	last_deallocate = pool; // racy but only an optimization hint
	pool->deallocate(pointer);
}

}
