// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/concurrent_registry.h>
#include <oBase/assert.h>
#include <oMemory/byte.h>

namespace ouro {

namespace state
{	enum value {

	invalid = 0,
	failed, 
	making, 
	made, 
	unmaking, 
	unmaking_to_failed

};}

concurrent_registry::concurrent_registry()
	: entries(nullptr)
	, missing(nullptr)
	, failed(nullptr)
	, making(nullptr)
	, owns_memory(false)
{}

concurrent_registry::concurrent_registry(concurrent_registry&& _That)
	: entries(_That.entries)
	, missing(_That.missing)
	, failed(_That.failed)
	, making(_That.making)
	, owns_memory(_That.owns_memory)
{
	_That.entries = nullptr;
	_That.missing = nullptr;
	_That.failed = nullptr;
	_That.making = nullptr;
	_That.owns_memory = false;
	pool = std::move(_That.pool);
	lookup = std::move(_That.lookup);
	makes = std::move(_That.makes);
	unmakes = std::move(_That.unmakes);
	lifetime = std::move(_That.lifetime);
}

concurrent_registry::concurrent_registry(void* memory, size_type capacity
	, const lifetime_t& lifetime
	, placeholder_source_t& placeholder_source)
{
	if (!initialize(memory, capacity, lifetime, placeholder_source))
		throw std::invalid_argument("concurrent_registry initialize failed");
}

concurrent_registry::concurrent_registry(size_type capacity, const lifetime_t& lifetime, placeholder_source_t& placeholder_source)
{
	if (!initialize(capacity, lifetime, placeholder_source))
		throw std::invalid_argument("concurrent_registry initialize failed");
}

concurrent_registry::~concurrent_registry()
{
	deinitialize();
}

concurrent_registry& concurrent_registry::operator=(concurrent_registry&& _That)
{
	if (this != &_That)
	{
		pool = std::move(_That.pool);
		lookup = std::move(_That.lookup);
		makes = std::move(_That.makes);
		unmakes = std::move(_That.unmakes);
		lifetime = std::move(_That.lifetime);
		oMOVE0(entries);
		oMOVE0(missing);
		oMOVE0(failed);
		oMOVE0(making);
		oMOVE0(owns_memory);
	}
	return *this;
}

concurrent_registry::size_type concurrent_registry::initialize(void* memory, size_type capacity, const lifetime_t& lifetime, placeholder_source_t& placeholder_source)
{
	const size_type pool_req = byte_align(pool.calc_size(capacity), oDEFAULT_MEMORY_ALIGNMENT);
	const size_type lookup_req = byte_align(lookup.initialize(nullptr, capacity), oDEFAULT_MEMORY_ALIGNMENT);
	const size_type entries_req = capacity * sizeof(void*);

	if (!pool_req || !lookup_req)
		return 0;

	const size_type total_req = pool_req + lookup_req + entries_req;

	if (memory)
	{
		// if so, pool is already initialized
		void* p = byte_add(memory, pool_req);
		lookup.initialize(p, capacity);
		p = byte_add(p, lookup_req);
		entries = (void**)p;

		this->lifetime = lifetime;

		missing = lifetime.create(placeholder_source.compiled_missing, "missing_placeholder");
		failed = lifetime.create(placeholder_source.compiled_failed, "failed_placeholder");
		making = lifetime.create(placeholder_source.compiled_making, "making_placeholder");
		
		for (index_type i = 0; i < capacity; i++)
			entries[i] = missing;
	}

	return total_req;
}
	
concurrent_registry::size_type concurrent_registry::initialize(size_type capacity, const lifetime_t& lifetime, placeholder_source_t& placeholder_source)
{
	size_type req = initialize(nullptr, capacity, lifetime, placeholder_source);
	void* memory = default_allocate(req, 0);
	owns_memory = true;
	return initialize(memory, capacity, lifetime, placeholder_source);
}

void* concurrent_registry::deinitialize()
{
	void* p = nullptr;
	if (entries)
	{
		if (!makes.empty() || !unmakes.empty())
			throw std::exception("concurrent_registry should have been flushed before destruction");

		unmake_all();

		if (missing) { lifetime.destroy(missing); missing = nullptr; }
		if (failed)	{ lifetime.destroy(failed); failed = nullptr; }
		if (making)	{ lifetime.destroy(making); making = nullptr; }

		memset(entries, 0, sizeof(void*) * pool.capacity());
		entries = nullptr;
		lifetime = lifetime_t();
		lookup.deinitialize();

		p = pool.deinitialize();
		if (owns_memory)
		{
			default_deallocate(p);
			owns_memory = false;
			p = nullptr;
		}
	}

	return p;
}

void concurrent_registry::unmake_all()
{
	size_type remaining = flush(~0u);
	if (remaining)
		throw std::exception("did not flush all outstanding items");

	const size_type n = capacity();
	for (size_type i = 0; i < n; i++)
	{
		void** e = entries + i;
		if (*e && *e != missing && *e != failed && *e != making)
		{
			lifetime.destroy(*e);
			auto f = pool.typed_pointer(i);
			*e = f->state == state::unmaking ? missing : failed;
			pool.destroy(f);
		}
	}
}

concurrent_registry::size_type concurrent_registry::flush(size_type max_operations)
{
	size_type n = max_operations;
	file_info* f = nullptr;
	while (n && unmakes.pop(&f))
	{
		// a call to make saved this asset
		if (f->state != state::unmaking && f->state != state::unmaking_to_failed)
			continue;
		index_type index = pool.index(f);
		void** e = entries + index;
		lifetime.destroy(*e);
		*e = f->state == state::unmaking ? missing : failed;
		pool.destroy(f);
		n--;
	}

	size_type EstHashesToReclaim = max_operations - n;

	while (n && makes.pop(&f))
	{
		index_type index = pool.index(f);

		void** e = entries + index;
		
		// free any prior asset (should this count as an operation n--?)
		if (*e && *e != missing && *e != failed && *e != making)
			lifetime.destroy(*e);

		if (!f->compiled)
		{
			*e = failed;
			f->state.store(state::failed);
		}

		else
		{
			try
			{
				*e = lifetime.create(std::ref(f->compiled), f->name);
				f->state.store(state::made);
			}
		
			catch (std::exception& ex)
			{
				*e = failed;
				f->state.store(state::failed);
				oTRACEA("failed: %s", ex.what());
			}
			n--;
		}
	}

	if (EstHashesToReclaim <= n)
		lookup.reclaim();

	return makes.size() + unmakes.size();
}

concurrent_registry::hash_type concurrent_registry::hash(const char* name)
{
	return fnv1a<hash_type>(name);
}

concurrent_registry::entry_type concurrent_registry::get(hash_type key) const
{
	auto index = lookup.get(key);
	return index == lookup.nullidx ? &missing : entries + index;
}

const char* concurrent_registry::name(entry_type entry) const
{
	if (!in_range(entry, entries, byte_add(entry, pool.capacity())))
		return "";
	index_type index = (index_type)index_of(entry, entries);
	auto f = pool.typed_pointer(index);
	return f->state.load() == state::invalid ? "" : f->name;
}

concurrent_registry::entry_type concurrent_registry::make(hash_type key, const char* name, scoped_allocation& compiled, const path& path, bool force)
{
	//                    Truth Table
	//	             n/c       n/c/f             n            n/f
	// <no entry> ins/q/making  ins/q/making  ins/err       ins/err
	// failed       q/making      q/making      noop          noop
	// making         noop          noop        noop          noop
	// made           noop        q/making      noop      q/unm_to_err
	// unmaking       made        q/making  q/unm_to_err  q/unm_to_err
	// unm_to_err   q/making      q/making      noop          noop

	void** e = nullptr;
	index_type index = lookup.get(key);
	file_info* f = nullptr;

	if (index != lookup.nullidx)
	{
		e = entries + index;
		f = pool.typed_pointer(index);
		int old = f->state;
		
		// handle noop cases
		if (old == state::making || (old == state::made && !force) || ((old == state::failed || old == state::unmaking_to_failed) && !compiled))
			return e;

		bool HasCompiled = !!compiled;

		// try to take this to making
		old = f->state.exchange(HasCompiled ? state::making : state::failed);
		if (old == state::making) // let the other one do the making
			return e;

		if (!HasCompiled && (force || old == state::unmaking))
		{
			f->state.store(state::unmaking_to_failed);
			unmakes.push(f);
		}

		// anything else is some sort of making which is flagged
		oASSERT(force || old != state::made, "unexpected state");

		if (old == state::unmaking && !force)
			f->state.store(state::made);
		else
		{
			f->compiled = std::move(compiled);
			makes.push(f);
		}
	}

	else
	{
		void* placeholder = !!compiled ? making : failed;

		// initialize a new entry
		f = pool.create();
		if (f)
		{
			f->state = !compiled ? state::failed : state::making;
			f->name = name;
			f->path = path;
			f->compiled = std::move(compiled);
			index = (index_type)pool.index(f);
		}
		else
			return &missing; // oom asset?

		e = entries + index;
		
		// another thread is trying to do this same thing, so let it
		if (lookup.nullidx != lookup.set(key, index))
			pool.destroy(f);
		else if (f->state == state::failed)
			*e = failed;
	}

	makes.push(f);
	return e;
}

bool concurrent_registry::unmake(entry_type entry)
{
	//     Truth Table
	//	             entry
	// <no entry>    retf         
	// failed         q/unmaking
	// making         q/unmaking
	// made           q/unmaking
	// unmaking       noop
	// unm_to_err     unmaking

	if (!in_range(entry, entries, pool.capacity()))
		return false;

	index_type index = (index_type)index_of(entry, entries);
	auto f = pool.typed_pointer(index);
	int old = f->state.exchange(state::unmaking);
	if (old == state::failed || old == state::making || old == state::made)
		unmakes.push(f);
	return true;
}

}
