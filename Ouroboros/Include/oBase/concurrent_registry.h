// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// a name<->index mapping for objects whose creation and destruction must
// be done at a controlled time.

#ifndef oBase_concurrent_registry_h
#define oBase_concurrent_registry_h

#include <oMemory/allocate.h>
#include <oBase/concurrent_hash_map.h>
#include <oBase/concurrent_object_pool.h>
#include <oBase/concurrent_stack.h>
#include <oBase/path.h>
#include <oBase/pool.h>
#include <functional>

namespace ouro {

class concurrent_registry
{
public:

	struct placeholder_source_t
	{
		scoped_allocation compiled_missing;
		scoped_allocation compiled_failed;
		scoped_allocation compiled_making;
	};

	struct lifetime_t
	{
		std::function<void*(scoped_allocation& compiled, const char* name)> create;
		std::function<void(void* entry)> destroy;
	};

	typedef unsigned int size_type;
	typedef unsigned int index_type;
	typedef unsigned long long hash_type;
	typedef void* const* entry_type;
	static const index_type nullidx = index_type(-1);

	static hash_type hash(const char* name);
	
	// non-concurrent api

	// ctor creates as empty
	concurrent_registry();

	// ctor that moves an existing pool into this one
	concurrent_registry(concurrent_registry&& _That);

	// The status assets are queued immediately. Call flush() on the device
	// thread to have them created and ready.
	concurrent_registry(void* memory, size_type capacity
		, const lifetime_t& lifetime, placeholder_source_t& placeholder_source);

	// The status assets are queued immediately. Call flush() on the device
	// thread to have them created and ready.
	concurrent_registry(size_type capacity, const lifetime_t& lifetime, placeholder_source_t& placeholder_source);

	// dtor
	~concurrent_registry();

	// calls deinit on this, moves that's memory under the same config
	concurrent_registry& operator=(concurrent_registry&& _That);

	// Initializes the registry with external memory. If nullptr then the object is not initialized
	// nor is lifetime or placeholder_source accessed. In all cases this returns the number of 
	// bytes required.
	size_type initialize(void* memory, size_type capacity, const lifetime_t& lifetime, placeholder_source_t& placeholder_source);
	
	// Initializes the registry with internal memory and returns the number of bytes allocated.
	size_type initialize(size_type capacity, const lifetime_t& lifetime, placeholder_source_t& placeholder_source);

	// returns either the external memory specified in initialize() or nullptr if internal memory
	// is used.
	void* deinitialize();

	inline size_type capacity() const { return pool.capacity(); }
	inline size_type size() const { return pool.size(); }
	inline bool empty() const { return pool.empty(); }

	// Removes all entries from the registry (except placeholders). This should be 
	// called from the thread where all device operations have to occur because the
	// flush is immediate.
	void unmake_all();

	// Flushs the Makes and Unmakes queues. This should be called from the thread where 
	// all device operations have to occur. Returns the number of operations completed.
	size_type flush(size_type max_operations = ~0u);


	// concurrent api

	// returns a pointer to an asset. The asset itself may be reloaded or unmade
	// but this pointer will be stable and useable until remove is called on it
	// and the next flush() is called.
	entry_type get(hash_type key) const;
	inline entry_type get(const char* name) const { return get(hash(name)); }

	// returns the name of the entry or the empty string if _pAsset is not an entry
	// in this registry.
	const char* name(entry_type entry) const;
	inline const char* name(hash_type key) const { return name(get(key)); }
	
	// Creates a runtime object from the compiled source and returns a handle to it that.
	// will be valid until it is removed. If compiled is a nullptr then the entry will
	// be set to the error asset. If another thread calls make the most-correct will win,
	// i.e. either a valid cached version or a thread attempting to make a new entry if
	// the entry is missing or in error. There is an explicit-key version that is useful
	// if some values come from a hard-coded enumerated type, otherwise the hash will be
	// based on the name passed in.
	entry_type make(hash_type key, const char* name, scoped_allocation& compiled, const path& path, bool force = false);
	inline entry_type make(const char* name, scoped_allocation& compiled, const path& path, bool force = false) { return make(hash(name), name, compiled, path, force); }
	inline entry_type make(hash_type key, const char* name, scoped_allocation& compiled, bool force = false) { return make(key, name, compiled, path(), force); }
	inline entry_type make(const char* name, scoped_allocation& compiled, bool force = false) { return make(hash(name), name, compiled, path(), force); }
	
	// Removes the entry allocated from this registry. The entry is still valid and 
	// present until the next call to flush so any device processing can finish using
	// it.
	bool unmake(entry_type entry);
	inline bool unmake(const hash_type key) { return unmake(get(key)); }
	inline bool unmake(const char* name) { return unmake(get(name)); }

private:

	struct file_info
	{
		file_info() : next(nullptr) { state = 0; }
		~file_info() { state = 0; }
		file_info* next;
		std::atomic_int state;
		mstring name;
		path path;
		scoped_allocation compiled;
	};
	
	concurrent_object_pool<file_info> pool;
	concurrent_hash_map lookup;
	concurrent_stack<file_info> makes;
	concurrent_stack<file_info> unmakes;
	lifetime_t lifetime;
	void** entries;
	void* missing;
	void* failed;
	void* making;
	bool owns_memory;
};

}

#endif
