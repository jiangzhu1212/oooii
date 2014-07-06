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

namespace ouro {

class concurrent_hash_map
{
public:
	typedef unsigned long long key_type;
	typedef unsigned int value_type;
	typedef unsigned int size_type;

	static const value_type nullidx = value_type(-1);


	// non-concurrent api

	// constructs an empty hash map
	concurrent_hash_map();

	// moves another hash map into a new one
	concurrent_hash_map(concurrent_hash_map&& _That);

	// ctor creates as a valid hash map using external memory
	concurrent_hash_map(void* memory, size_type capacity);

	// ctor creates as a valid hash map using internally allocated memory
	concurrent_hash_map(size_type capacity);

	// dtor
	~concurrent_hash_map();

	// calls deinit on this, moves that's memory under the same config
	concurrent_hash_map& operator=(concurrent_hash_map&& _That);

	// initialize the hash map with external memory. Returns required 
	// size for the specified capacity. Pass nullptr for memory to 
	// calculate the size only. Note: This calculates size for optimal 
	// performance which is twice the number of entries asked for rounded 
	// up to the next power of two.
	size_type initialize(void* memory, size_type capacity);

	// initialize the hash map with internally allocated memory that will 
	// be freed on deinitialization.
	size_type initialize(size_type capacity);

	// invalidates the hash map. returns the memory passed in initialize 
	// or nullptr if internally allocated memory was used.
	void* deinitialize();

	// returns the hash map to the empty state it was after initialize
	void clear();

	// returns the number of entries in the hash map
	size_type size() const;

	// returns true if there are no entries
	inline bool empty() const { return size() == 0; }

	// returns the percentage used of capacity
	inline size_type occupancy() const { return (size() * 100) / capacity(); }

	// returns true if performance is degraded due to high occupancy
	// which begins to occur at 75% occupancy
	inline bool needs_resize() const { return occupancy() > 75; }

	// walk through nullidx entries and eviscerate the entries to reduce 
	// occupancy. Returns the number reclaimed
	size_type reclaim();

	// walks through valid entries and inserts them into the specified hash 
	// map up until the specified number of inserts was done.
	size_type migrate(concurrent_hash_map& _That, size_type max_moves = size_type(-1));

	
	// concurrent api

	// returns the absolute capacity within the hash map
	size_type capacity() const { return modulo_mask; }

	// sets the specified key to the specified value and 
	// returns the prior value. nullidx implies this was a first add.
	// Set to nullidx to "clear" an entry. It will affect all api 
	// correctly and only degrade performances until reclaim() is called.
	value_type set(const key_type& key, const value_type& value);

	// flags the key as no longer in use
	// returns the prior value
	inline value_type remove(const key_type& key) { return set(key, nullidx); }

	// returns the value associated with the key or nullidx
	// if the key was not found
	value_type get(const key_type& key) const;

private:
	unsigned int modulo_mask;
	bool owns_memory;
	void* keys;
	void* values;

	concurrent_hash_map(const concurrent_hash_map&);
	const concurrent_hash_map& operator=(const concurrent_hash_map&);
};

} // namespace ouro

#endif