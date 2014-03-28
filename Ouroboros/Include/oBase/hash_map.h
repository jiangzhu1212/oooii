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
// Simple open addressing linear probe hash map. This auto-grows with 
// occupancy > 75%. Since this has to rehash on removal, there's extra
// API to mark entries for removal and sweep them up later with a 
// slightly more efficient rehash-all pass.

#pragma once
#ifndef oBase_hash_map_h
#define oBase_hash_map_h

#include <oBase/allocate.h>
#include <functional>
#include <memory.h>
#include <stdlib.h>

namespace ouro {

template<typename HashT, typename ValueT>
class hash_map
{
	hash_map(const hash_map&);/* = delete; */
	const hash_map& operator=(const hash_map&);/* = delete; */

public:
	typedef unsigned int size_type;
	typedef HashT hash_type;
	typedef ValueT value_type;

	// Lifetime management
	hash_map() : Keys(nullptr), Values(nullptr), Size(0), Capacity(0) {}
	hash_map(hash_map&& _That) { operator=(std::move(_That)); }
	hash_map(size_type _IntendedMaxEntries, const allocator& _Allocator = default_allocator) 
		: Keys(nullptr)
		, Values(nullptr)
		, Size(0)
		, Capacity(0)
		, Allocator(_Allocator)
	{
		static_assert(std::is_integral<hash_type>::value, "hash type must be an integral");
		allocate(optimal_capacity(_IntendedMaxEntries));
	}

	hash_map& operator=(hash_map&& _That)
	{
		if (this != &_That)
		{
			Keys = _That.Keys; _That.Keys = nullptr;
			Values = _That.Values; _That.Values = nullptr;
			Allocator = std::move(_That.Allocator);
			Size = _That.Size; _That.Size = 0;
			Capacity = _That.Capacity; _That.Capacity = 0;
		}
		return *this;
	}

	~hash_map()
	{
		Allocator.destroy_array(Keys, Capacity);
		Keys = nullptr;
		Allocator.destroy_array(Values, Capacity);
		Values = nullptr;
		Size = 0;
		Capacity = 0;
	}

	size_type size() const { return Size; }
	size_type capacity() const { return Capacity; }
	bool empty() const { return size() == 0; }

	// number between 0 and 100 describing the percentage full
	// the hash is. Performance degrades after 75% occupancy
	size_type occupancy() const { return (Size * 100) / Capacity; }

	void clear()
	{
		memset(Keys, 0, sizeof(hash_type) * Capacity);
		for (size_type i = 0; i < Capacity; i++)
			Values[i] = value_type();
		Size = 0;
	}

	void swap(hash_map& _That)
	{
		std::swap(Keys, _That.Keys);
		std::swap(Values, _That.Values);
		std::swap(Allocator, _That.Allocator);
		std::swap(Size, _That.Size);
		std::swap(Capacity, _That.Capacity);
	}

	void resize(const size_type& _NewSize)
	{
		hash_map h(_NewSize);
		enumerate([&](const hash_type& _HashKey, value_type& _Value)->bool
		{
			h.add(_HashKey, std::move(_Value));
			return true;
		});
		swap(h);
	}

	bool exists(const hash_type& _Key) const { return find_existing(_Key) != invalid_index; }

	bool add(const hash_type& _Key, const value_type& _Value, bool _AllowResize = true)
	{
		if (_AllowResize && occupancy() >= 75)
			resize(size() * 2);
		size_type i = find_existing_or_new(_Key);
		if (i == invalid_index || Keys[i] || Size >= (Capacity-1)) // full or exists
			return false;
		Keys[i] = _Key;
		Values[i] = _Value;
		Size++;
		return true;
	}

	bool add(const hash_type& _Key, value_type&& _Value, bool _AllowResize = true)
	{
		if (_AllowResize && occupancy() >= 75)
			resize(size() * 2);
		size_type i = find_existing_or_new(_Key);
		if (i == invalid_index || Keys[i] || Size >= (Capacity-1)) // full or exists
			return false;
		Keys[i] = _Key;
		Values[i] = std::move(_Value);
		Size++;
		return true;
	}

	bool remove(const hash_type& _Key)
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			return false;
		Keys[i] = 0;
		Values[i] = value_type();
		fix_collisions(_Key, i);
		Size--;
		return true;
	}

	// Replaces a hash key with the mark hash key. This is used for sweeping.
	bool mark(const hash_type& _Key, const hash_type& _Mark)
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			return false;
		Keys[i] = _Mark;
		return true;
	}

	// Sweeps through all marked entries and destroys them.
	size_type sweep(const hash_type& _Mark)
	{
		size_type Removed = 0;
		// do the fix collisions, but also check for any remove marks along the way
		size_type i = 0;
		while (i < Capacity)
		{
			if (Keys[i] == _Mark)
			{
				Keys[i] = 0;
				Removed++;
				Size--;

				// fix collisions
				size_type ii = (i + 1) % Capacity;
				while (Keys[ii])
				{
					if (Keys[ii] == _Mark)
					{
						Keys[ii] = 0;
						Values[ii] = value_type();
						Removed++;
						Size--;
					}

					else
						fix(ii);

					i = __max(i, ii);
					ii = (ii + 1) % Capacity;
				}
			}
			else
				i++;
		}

		return Removed;
	}

	bool set(const hash_type& _Key, const value_type& _Value)
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			return false;
		Values[i] = _Value;
		return true;
	}

	bool set(const hash_type& _Key, value_type&& _Value)
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			return false;
		Values[i] = std::move(_Value);
		return true;
	}

	value_type get(const hash_type& _Key) const
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			throw std::invalid_argument("key not found");
		return Values[i];
	}

	value_type* get_ptr(const hash_type& _Key) const
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			throw std::invalid_argument("key not found");
		return &Values[i];
	}

	bool get(const hash_type& _Key, value_type* _pValue) const
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			return false;
		*_pValue = Values[i];
		return true;
	}

	bool get_ptr(const hash_type& _Key, value_type** _ppValue) const
	{
		size_type i = find_existing(_Key);
		if (i == invalid_index)
			return false;
		*_ppValue = &Values[i];
		return true;
	}

	void enumerate_const(const std::function<bool(const hash_type& _HashKey, const value_type& _Value)>& _Enumerator) const
	{
		for (size_type i = 0; i < Capacity; i++)
			if (Keys[i] && !_Enumerator(Keys[i], Values[i]))
				return;
	}

	void enumerate(const std::function<bool(const hash_type& _HashKey, value_type& _Value)>& _Enumerator) const
	{
		for (size_type i = 0; i < Capacity; i++)
			if (Keys[i] && !_Enumerator(Keys[i], Values[i]))
				return;
	}

private:
	static const size_type invalid_index = size_type(-1);

	size_type find_existing_or_new(const hash_type& _Key)
	{
		if (_Key)
		{
			hash_type* end = Keys + Capacity;
			hash_type* start = Keys + (_Key % Capacity);
			for (hash_type* h = start; h < end; h++)
				if (!*h || *h == _Key)
					return size_type(h - Keys);
			for (hash_type* h = Keys; h < start; h++)
				if (!*h || *h == _Key)
					return size_type(h - Keys);
		}

		return invalid_index;
	}

	size_type find_existing(const hash_type& _Key) const
	{
		if (_Key)
		{
			hash_type* end = Keys + Capacity;
			hash_type* h = Keys + (_Key % Capacity);
			while (*h)
			{
				if (*h == _Key)
					return size_type(h - Keys);
				h++;
				if (h >= end)
					h = Keys;
			}
		}

		return invalid_index;
	}

	void fix(const size_type& _Index)
	{
		if ((Keys[_Index] % Capacity) != _Index) // move if key isn't where it supposed to be due to collision
		{
			hash_type k = Keys[_Index];
			Keys[_Index] = 0;
			size_type NewI = find_existing_or_new(k); // will only find new since we just moved the existing
			Keys[NewI] = k;
			Values[NewI] = std::move(Values[_Index]);
		}
	} 
	void fix_collisions(const hash_type& _RemovedKey, const size_type& _RemovedIndex)
	{
		// fix up collisions that might've been after this entry
		size_type i = (_RemovedIndex + 1) % Capacity;
		while (Keys[i])
		{
			fix(i);
			i = (i + 1) % Capacity;
		}
	}

private:

	// This hash map is basically a linear search with a smart starting 
	// index guess. All its performance benefits come from minimizing
	// collisions or said in another way over-allocating the storage, so
	// here are two accessors that semi-document good numbers for the 
	// given user number of entries before performance degrades.
	static size_type optimal_capacity(const size_type& _NumEntries) { return __max(8, _NumEntries * 2); }
	static size_type balanced_capacity(const size_type& _NumEntries) { return __max(8, 3 * _NumEntries / 2); }

	void allocate(const size_type& _Capacity)
	{
		Keys = Allocator.construct_array<hash_type>(_Capacity, memory_alignment::align_to_default);
		Values = Allocator.construct_array<value_type>(_Capacity, memory_alignment::align_to_default);
		Capacity = _Capacity;
		Size = 0;
		memset(Keys, 0, sizeof(hash_type) * Capacity);
	}

	hash_type* Keys;
	value_type* Values;
	allocator Allocator;
	size_type Size;
	size_type Capacity;
};

} // namespace ouro

#endif
