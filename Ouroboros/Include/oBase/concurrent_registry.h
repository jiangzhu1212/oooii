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
// a name<->index mapping for objects whose creation and destruction must
// be done at a controlled time.

#ifndef oBase_concurrent_registry_h
#define oBase_concurrent_registry_h

#include <oBase/allocate.h>
#include <oBase/concurrent_hash_map.h>
#include <oBase/concurrent_queue.h>
#include <oBase/path.h>
#include <oBase/pool.h>
#include <oBase/invalid.h>
#include <functional>

namespace ouro {
#if 0
class concurrent_registry
{
public:
	typedef unsigned int size_type;
	typedef unsigned short index_type;
	static const unsigned short invalid_index = index_type(-1);
	
	// returns the size in bytes of the memory buffer required by the ctor
	static size_type calc_size(size_type _Capacity);

	concurrent_registry();
	concurrent_registry(concurrent_registry&& _That) { operator=(std::move(_That)); }
	
	// The status assets are queued immediately. Call flush() on the device
	// thread to have them created and ready.
	concurrent_registry(void* _pMemory
		, size_type _Capacity
		, const std::function<void*(scoped_allocation& _Compiled, const char* _Name)>& _Create
		, const std::function<void(void* _pEntry)>& _Destroy
		, scoped_allocation& _CompiledMissing
		, scoped_allocation& _CompiledFailed
		, scoped_allocation& _CompiledMaking);
	
	concurrent_registry& operator=(concurrent_registry&& _That);
	~concurrent_registry();

	size_type capacity() const { return Pool.capacity(); }
	size_type size() const { return Pool.size(); }
	bool empty() const { return Pool.empty(); }

	// returns a pointer to an asset. The asset itself may be reloaded or unmade
	// but this pointer will be stable and useable until remove is called on it
	// and the next flush() is called.
	void* const* get(const char* _Name) const;

	// returns the name of the entry or the empty string if _pAsset is not an entry
	// in this registry.
	const char* name(void** _ppEntry) const;
	
	// Creates a runtime object from the compiled source and returns a handle to it that.
	// will be valid until it is removed. If _Compiled is a nullptr then the entry will
	// be set to the error asset. If another thread calls make the most-correct will win,
	// i.e. either a valid cached version or a thread attempting to make a new entry if
	// thee entry is missing or in error.
	void* const* make(const char* _Name, scoped_allocation& _Compiled, bool _Force = false);
	void* const* make(const char* _Name, scoped_allocation& _Compiled, const path& _Path, bool _Force = false);
	
	// Removes the entry allocated from this registry. The entry is still valid and 
	// present until the next call to flush so any device processing can finish using
	// it.
	bool unmake(void* const* _ppEntry);
	inline bool unmake(const char* _Name) { return unmake(get(_Name)); }

	// Removes all entries from the registry (except placeholders). This should be 
	// called from the thread where all device operations have to occur because the
	// flush is immediate.
	void unmake_all();

	// Flushs the Makes and Unmakes queues. This should be called from the thread where 
	// all device operations have to occur. Returns the number of operations completed.
	unsigned int flush(size_type _MaxOperations);

private:
	typedef unsigned long long hash_type;

	struct file_info
	{
		std::atomic_int state;
		mstring name;
		path path;
		scoped_allocation compiled;
	};
	
	concurrent_pool<file_info> Pool;
	concurrent_hash_map<hash_type, index_type> Lookup;
	concurrent_stack<index_type> Makes;
	concurrent_stack<index_type> Unmakes;
	std::function<void*(scoped_allocation& _Compiled, const char* _Name)> Make;
	std::function<void(void* _pEntry)> Unmake;
	void** Entry;
	void* Missing;
	void* Failed;
	void* Making;
};
#endif
} // namespace ouro

#endif
