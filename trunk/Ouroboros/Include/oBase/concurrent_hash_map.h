// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// A simple concurrent hash map
// http://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/

// Rules:
// A hash value of nullkey is not allowed - it is used to flag invalid entries
// A value of nullidx is not allowed - it is used to flag invalid entries
// The hash cannot be resized. To make a bigger hash, create a new one and re-hash 
// all entries.

#pragma once
#include <oMemory/allocate.h>
#include <cstdint>
#include <atomic>
#include <memory>

namespace ouro {

class concurrent_hash_map
{
public:
	typedef uint64_t key_type;
	typedef uint32_t value_type;
	typedef uint32_t size_type;

	static const key_type nullkey = key_type(-1);
	static const value_type nullidx = value_type(-1);


	// non-concurrent api

	// constructs an empty hash map
	concurrent_hash_map();

	// moves another hash map into a new one
	concurrent_hash_map(concurrent_hash_map&& that);

	// ctor creates as a valid hash map using external memory
	concurrent_hash_map(void* memory, size_type capacity);

	// ctor creates as a valid hash map using internally allocated memory
	concurrent_hash_map(size_type capacity, const char* alloc_label = "concurrent_hash_map", const allocator& alloc = default_allocator);

	// dtor
	~concurrent_hash_map();

	// calls deinit on this, moves that's memory under the same config
	concurrent_hash_map& operator=(concurrent_hash_map&& that);

	// initialize the hash map with external memory. Returns required 
	// size for the specified capacity. Pass nullptr for memory to 
	// calculate the size only. Note: This calculates size for optimal 
	// performance which is twice the number of entries asked for rounded 
	// up to the next power of two.
	size_type initialize(void* memory, size_type capacity, const allocator& alloc = noop_allocator);

	// initialize the hash map with internally allocated memory that will 
	// be freed on deinitialization.
	size_type initialize(size_type capacity, const char* alloc_label = "concurrent_hash_map", const allocator& alloc = default_allocator);

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
	// occupancy. Returns the number reclaimed.
	size_type reclaim();

	// moves entries from this map to that for the specified number of moves
	// so this can be done over multiple steps.
	size_type migrate(concurrent_hash_map& that, size_type max_moves = size_type(-1));

	
	// concurrent api

	// returns the absolute capacity within the hash map
	size_type capacity() const { return modulo_mask; }

	// sets the specified key to the specified value and 
	// returns the prior value. nullidx implies this was a first add.
	// Set to nullidx to "clear" an entry. It will affect all api 
	// correctly and only degrade performances until reclaim() is called.
	// if the hash is full, this will throw.
	value_type set(const key_type& key, const value_type& value);

	// flags the key as no longer in use
	// returns the prior value
	inline value_type remove(const key_type& key) { return set(key, nullidx); }

	// returns the value associated with the key or nullidx
	// if the key was not found
	value_type get(const key_type& key) const;

private:
	std::atomic<key_type>* keys;
	std::atomic<value_type>* values;
	uint32_t modulo_mask;
	allocator alloc;

	concurrent_hash_map(const concurrent_hash_map&);
	const concurrent_hash_map& operator=(const concurrent_hash_map&);
};

}
