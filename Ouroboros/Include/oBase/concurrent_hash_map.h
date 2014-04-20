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
// http://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/
// A simple concurrent hash map
// Rules:
// A hash value of 0 is not allowed - it is used to flag invalid entries
// A value of invalid_value is not allowed - it is used to flag invalid entries
// The hash cannot be resized. To make a bigger hash, create a new one and re-hash 
// all entries.
#pragma once
#ifndef oBase_concurrent_hash_map
#define oBase_concurrent_hash_map

#include <oBase/allocate.h>
#include <oBase/byte.h>
#include <atomic>
#include <stdexcept>
#include <type_traits>

namespace ouro {

template<typename KeyT, typename ValueT>
class concurrent_hash_map
{
public:
	typedef KeyT key_type;
	typedef ValueT value_type;
	typedef unsigned int size_type;

	static const value_type invalid_value = value_type(-1);

	// returns the number of bytes required to performantly store the specified amount of entries
	// use this to allocate a memory buffer and pass it to the constructor.
	static size_type calc_size(size_type _MaxEntries)
	{
		static_assert(std::is_integral<key_type>::value, "key must be integral type");
		static_assert(std::is_integral<value_type>::value, "value must be integral type");

		size_type n = __max(8, next_pow2(_MaxEntries * 2));
		return sizeof(std::atomic<key_type>) * n + sizeof(std::atomic<value_type>) * n;
	}

	// Default empty ctor
	concurrent_hash_map() : Allocator(noop_allocator), ModuloMask(0), Keys(nullptr), Values(nullptr) {}

	// _pMemory should point to memory allocated to at least calc_size() 
	concurrent_hash_map(void* _pMemory, size_type _MaxEntries) : Allocator(noop_allocator) { initialize(_pMemory, _MaxEntries); }

	// given an allocator, this will make the proper allocations and free them upon exit.
	concurrent_hash_map(allocator _Allocator, size_type _MaxEntries) : Allocator(_Allocator) { initialize(_Allocator.allocate(calc_size(_MaxEntries), 0), _MaxEntries); }

	concurrent_hash_map(concurrent_hash_map&& _That) { operator=(std::move(_That)); }

	~concurrent_hash_map()
	{
		if (Keys)
			Allocator.deallocate(Keys);
	}

	concurrent_hash_map& operator=(concurrent_hash_map&& _That)
	{
		if (this != &_That)
		{
			ModuloMask = _That.ModuloMask; _That.ModuloMask = 0;
			Keys = _That.Keys; _That.Keys = nullptr;
			Values = _That.Values; _That.Values = nullptr;
			Allocator = std::move(_That.Allocator);
		}

		return *this;
	}

	void clear()
	{
		const size_type nEntries = ModuloMask+1;
		memset(Values, -1, sizeof(std::atomic<value_type>) * nEntries);
		memset(Keys, 0, sizeof(std::atomic<key_type>) * nEntries);
	}

	// concurrent: returns the max number of items that can be stored.
	size_type capacity() const { return ModuloMask; }

	// not concurrent: walks entire capacity counting valid key/values
	size_type size() const
	{
		size_type n = 0;
		for (size_type i = 0; i <= ModuloMask; i++)
			if (Keys[i].load(std::memory_order_relaxed) && Values[i].load(std::memory_order_relaxed) != invalid_value)
				n++;
		return n;
	}

	// not concurrent: returns true if there are no valid entries
	bool empty() const { return size() == 0; }

	// not concurrent: return a number [0,100] representing the percentage 
	// full this hash_map is at.
	size_type occupancy() const
	{
		return size() * 100 / capacity();
	}

	// not concurrent: determines if performance is degraded because there
	// are too many entries.
	bool needs_resize() const
	{
		return occupancy() > 75;
	}

	// concurrent: always sets the value adding an entry if necessary
	// returns the prior value (invalid_value implies an add)
	value_type set(const key_type& _Key, const value_type& _Value)
	{
		if (!_Key) throw std::invalid_argument("invalid key specified");
		for (key_type k = _Key;; k++)
		{
			k &= ModuloMask;
			std::atomic<key_type>& StoredKey = Keys[k];
			key_type probedKey = StoredKey.load(std::memory_order_relaxed);
			if (probedKey != _Key)
			{
				if (probedKey)
					continue;
				key_type prevKey = 0;
				StoredKey.compare_exchange_strong(prevKey, _Key, std::memory_order_relaxed);
				if (prevKey && prevKey != _Key)
					continue;
			}
			return Values[k].exchange(_Value, std::memory_order_relaxed);
		}
	}
	
	// concurrent: returns the value associated with the key or invalid_value 
	// to indicate the key was not found.
	value_type get(const key_type& _Key) const
	{
		for (key_type k = _Key;; k++)
		{
			k &= ModuloMask;
			std::atomic<key_type>& StoredKey = Keys[k];
			key_type probedKey = StoredKey.load(std::memory_order_relaxed);
			if (probedKey == _Key)
				return Values[k].load(std::memory_order_relaxed);
			if (!probedKey)
				return invalid_value;
		}
	}
	
	// concurrent: flags the specified key as no longer in use.
	value_type remove(const key_type& _Key) { return set(_Key, invalid_value); }

	// not concurrent: using remove() on a key doesn't maintain performance
	// so at times known not to be in contention, walk through and consolidate
	// keylists.
	size_type reclaim()
	{
		size_type n = 0;
		size_type i = 0;
		while (i < ModuloMask)
		{
			if (Values[i] == invalid_value && Keys[i])
			{
				Keys[i] = 0;
				n++;

				size_type ii = (i + 1) & ModuloMask;
				while (Keys[ii])
				{
					if (Values[ii] == invalid_value)
					{
						Keys[ii] = 0;
						n++;
					}

					else if ((Keys[ii] & ModuloMask) != ii) // move if key isn't where it supposed to be due to collision
					{
						key_type k = Keys[ii];
						value_type v = Values[ii];
						Keys[ii] = 0;
						Values[ii] = invalid_value;
						set(k, v);
					}

					i = __max(i, ii);
					ii = (ii + 1) & ModuloMask;
				}
			}
			else
				i++;
		}

		return n;
	}

private:
	size_type ModuloMask;
	std::atomic<key_type>* Keys;
	std::atomic<value_type>* Values;
	allocator Allocator;

	concurrent_hash_map(const concurrent_hash_map&);
	const concurrent_hash_map& operator=(const concurrent_hash_map&);

	void initialize(void* _pMemory, size_type _MaxEntries)
	{
		size_type Capacity = next_pow2(_MaxEntries * 2);
		ModuloMask = Capacity - 1;
		Keys = (std::atomic<key_type>*)_pMemory;
		Values = byte_add((std::atomic<value_type>*)Keys, sizeof(std::atomic<key_type>), Capacity);
		clear();
	}
};

} // namespace ouro

#endif
