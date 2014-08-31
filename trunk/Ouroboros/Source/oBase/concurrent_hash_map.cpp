// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/concurrent_hash_map.h>
#include <oBase/allocate.h>
#include <oMemory/bit.h>
#include <oMemory/byte.h>
#include <oBase/macros.h>
#include <atomic>

#define DECLARE_KV std::atomic<key_type>* keys = (std::atomic<key_type>*)this->keys; std::atomic<value_type>* values = (std::atomic<value_type>*)this->values;

namespace ouro {

concurrent_hash_map::concurrent_hash_map()
	: modulo_mask(0)
	, keys(nullptr)
	, values(nullptr)
{}

concurrent_hash_map::concurrent_hash_map(concurrent_hash_map&& _That)
	: modulo_mask(_That.modulo_mask)
	, keys(_That.keys)
	, values(_That.values)
{
	_That.deinitialize();
}

concurrent_hash_map::concurrent_hash_map(void* memory, size_type capacity)
{
	if (!initialize(memory, capacity))
		throw std::invalid_argument("concurrent_hash_map initialize failed");
}

concurrent_hash_map::concurrent_hash_map(size_type capacity, const char* alloc_label, const allocator& alloc_)
{
	if (!initialize(capacity, alloc_label, alloc_))
		throw std::invalid_argument("concurrent_hash_map initialize failed");
}

concurrent_hash_map::~concurrent_hash_map()
{
	deinitialize();
}

concurrent_hash_map& concurrent_hash_map::operator=(concurrent_hash_map&& _That)
{
	if (this != &_That)
	{
		deinitialize();

		oMOVE0(modulo_mask);
		oMOVE0(keys);
		oMOVE0(values);
	}
	return *this;
}

concurrent_hash_map::size_type concurrent_hash_map::initialize(void* memory, size_type capacity, const allocator& alloc)
{
	const size_type n = __max(8, nextpow2(capacity * 2));
	const size_type key_bytes = n * sizeof(std::atomic<key_type>);
	const size_type value_bytes = n * sizeof(std::atomic<value_type>);
	const size_type req = key_bytes + value_bytes;
	if (memory)
	{
		modulo_mask = n - 1;
		keys = memory;
		values = byte_add(memory, key_bytes);
		memset(keys, 0xff/*nullkey*/, key_bytes);
		memset(values, nullidx, value_bytes);
		this->alloc = alloc;
	}
	return req;
}

concurrent_hash_map::size_type concurrent_hash_map::initialize(size_type capacity, const char* alloc_label, const allocator& alloc)
{
	size_type req = initialize(nullptr, capacity);
	return initialize(alloc.allocate(req, 0, alloc_label), capacity, alloc);
}

void* concurrent_hash_map::deinitialize()
{
	void* p = keys;
	if (alloc)
	{
		alloc.deallocate(p);
		p = nullptr;
	}
	modulo_mask = 0;
	alloc = default_allocator;
	keys = nullptr;
	values = nullptr;
	return p;
}

void concurrent_hash_map::clear()
{
	const size_type n = modulo_mask + 1;
	const size_type key_bytes = n * sizeof(std::atomic<key_type>);
	const size_type value_bytes = n * sizeof(std::atomic<value_type>);
	memset(keys, 0xff/*nullkey*/, key_bytes);
	memset(values, nullidx, value_bytes);
}

concurrent_hash_map::size_type concurrent_hash_map::size() const
{
	DECLARE_KV
	size_type n = 0;
	for (uint32_t i = 0; i <= modulo_mask; i++)
		if (keys[i].load(std::memory_order_relaxed) != nullkey && 
			values[i].load(std::memory_order_relaxed) != nullidx)
			n++;
	return n;
}

concurrent_hash_map::size_type concurrent_hash_map::reclaim()
{
	DECLARE_KV
	size_type n = 0;
	uint32_t i = 0;
	while (i <= modulo_mask)
	{
		if (values[i] == nullidx && keys[i] != nullkey)
		{
			keys[i] = nullkey;
			n++;

			uint32_t ii = (i + 1) & modulo_mask;
			while (keys[ii] != nullkey)
			{
				if (values[ii] == nullidx)
				{
					keys[ii] = nullkey;
					n++;
				}

				else if ((keys[ii] & modulo_mask) != ii) // move if key is misplaced due to collision
				{
					key_type k = keys[ii];
					value_type v = values[ii];
					keys[ii] = nullkey;
					values[ii] = nullidx;
					set(k, v);
				}

				i = __max(i, ii);
				ii = (ii + 1) & modulo_mask;
			}
		}
		else
			i++;
	}

	return n;
}

concurrent_hash_map::size_type concurrent_hash_map::migrate(concurrent_hash_map& that, size_type max_moves)
{
	DECLARE_KV
	size_type n = 0;
	uint32_t i = 0;
	while (i <= modulo_mask)
	{
		if (values[i] != nullidx && keys[i] != nullkey)
		{
			that.set(keys[i], values[i]);
			remove(keys[i]);
			if (++n >= max_moves)
				break;
		}
		
		i++;
	}

	return n;
}

concurrent_hash_map::value_type concurrent_hash_map::set(const key_type& key, const value_type& value)
{
	if (key == nullkey)
		throw std::invalid_argument("key must be non-zero");
	DECLARE_KV
	for (key_type k = key, j = 0;; k++, j++)
	{
		if (j > modulo_mask)
			throw std::length_error("concurrent_hash_map full");

		k &= modulo_mask;
		std::atomic<key_type>& stored = keys[k];
		key_type probed = stored.load(std::memory_order_relaxed);
		if (probed != key)
		{
			if (probed != nullkey)
				continue;
			key_type prev = nullkey;
			stored.compare_exchange_strong(prev, key, std::memory_order_relaxed);
			if (prev != nullkey && prev != key)
				continue;
		}
		return values[k].exchange(value, std::memory_order_relaxed);
	}
}
	
concurrent_hash_map::value_type concurrent_hash_map::get(const key_type& key) const
{
	if (key == nullkey)
		throw std::invalid_argument("key must be non-zero");
	DECLARE_KV
	for (key_type k = key;; k++)
	{
		k &= modulo_mask;
		std::atomic<key_type>& stored = keys[k];
		key_type probed = stored.load(std::memory_order_relaxed);
		if (probed == key)
			return values[k].load(std::memory_order_relaxed);
		if (probed == nullkey)
			return nullidx;
	}
}

}
