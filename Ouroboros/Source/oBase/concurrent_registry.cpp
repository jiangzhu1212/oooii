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
#include <oBase/concurrent_registry.h>
#if 0
namespace ouro {

namespace state
{	enum value {

	failed, 
	making, 
	made, 
	unmaking, 
	unmaking_to_failed

};}

concurrent_registry::size_type concurrent_registry::calc_size(size_type _Capacity)
{
	const size_type kQueueCapacity = _Capacity / 4;
	return byte_align(concurrent_hash_map<hash_type, index_type>::calc_size(_Capacity), oDEFAULT_MEMORY_ALIGNMENT)
		+ byte_align(_Capacity * (size_type)sizeof(void*), oDEFAULT_MEMORY_ALIGNMENT);
}

concurrent_registry::concurrent_registry()
	: Entry(nullptr)
	, Missing(nullptr)
	, Failed(nullptr)
	, Making(nullptr)
{
}

concurrent_registry::concurrent_registry(void* _pMemory
		, size_type _Capacity
		, const std::function<void*(scoped_allocation& _Compiled, const char* _Name)>& _Make
		, const std::function<void(void* _pEntry)>& _Unmake
		, scoped_allocation& _CompiledMissing
		, scoped_allocation& _CompiledFailed
		, scoped_allocation& _CompiledMaking)
	: Entry(nullptr)
	, Make(_Make)
	, Unmake(_Unmake)
{
	if (!byte_aligned(_pMemory, oDEFAULT_MEMORY_ALIGNMENT))
		throw std::invalid_argument("_pMemory must be default aligned");
#if 0
	const size_type kQueueCapacity = _Capacity / 4;

	Pool = std::move(concurrent_pool<file_info>(_Capacity));
	void* p = byte_align(byte_add(_pMemory, concurrent_pool<file_info>::calc_size(_Capacity)), oDEFAULT_MEMORY_ALIGNMENT);
	Lookup = std::move(concurrent_hash_map<hash_type, index_type>(p,  _Capacity));
	p = byte_align(byte_add(p, concurrent_hash_map<hash_type, index_type>::calc_size(_Capacity)), oDEFAULT_MEMORY_ALIGNMENT);
	Makes = std::move(concurrent_stack<index_type>(p, kQueueCapacity));
	p = byte_align(byte_add(p, concurrent_stack<index_type>::calc_size(kQueueCapacity)), oDEFAULT_MEMORY_ALIGNMENT);
	Unmakes = std::move(concurrent_stack<index_type>(p, kQueueCapacity));
	p = byte_align(byte_add(p, concurrent_stack<index_type>::calc_size(kQueueCapacity)), oDEFAULT_MEMORY_ALIGNMENT);
	Entry = (void**)p;

	Missing = Make(_CompiledMissing, "missing_placeholder");
	Failed = Make(_CompiledFailed, "failed_placeholder");
	Making = Make(_CompiledMaking, "making_placeholder");
	
	for (index_type i = 0; i < _Capacity; i++)
		Entry[i] = Missing;
#endif
}

concurrent_registry& concurrent_registry::operator=(concurrent_registry&& _That)
{
	if (this != &_That)
	{
		Pool = std::move(_That.Pool);
		Lookup = std::move(_That.Lookup);
		Makes = std::move(_That.Makes);
		Unmakes = std::move(_That.Unmakes);
		Make = std::move(_That.Make);
		Unmake = std::move(_That.Unmake);
		Entry = _That.Entry; _That.Entry = nullptr;
		Missing = _That.Missing; _That.Missing = nullptr;
		Failed = _That.Failed; _That.Failed = nullptr;
		Making = _That.Making; _That.Making = nullptr;
	}
	return *this;
}

concurrent_registry::~concurrent_registry()
{
	unmake_all();

	if (!Makes.empty() || !Unmakes.empty())
		throw std::exception("concurrent_registry should have been flushed before destruction");
	
	if (Missing) Unmake(Missing);
	if (Failed)	Unmake(Failed);
	if (Making)	Unmake(Making);
}

void* const* concurrent_registry::get(const char* _Name) const
{
	auto key = fnv1a<hash_type>(_Name);
	auto index = Lookup.get(key);
	return index == Lookup.invalid_value ? &Missing : &Entry[index];
}

const char* concurrent_registry::name(void** _ppEntry) const
{
	if (!in_range(_ppEntry, Entry, byte_add(Entry, Pool.capacity())))
		return "";
	index_type index = (index_type)index_of(_ppEntry, Entry);
	auto f = Pool.at(index);
	return f->state.load() == invalid ? "" : f->name;
}

void* const* concurrent_registry::make(const char* _Name, scoped_allocation& _Compiled, const path& _Path, bool _Force)
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
	auto key = fnv1a<hash_type>(_Name);
	index_type index = Lookup.get(key);

	if (index != Lookup.invalid_value)
	{
		e = Entry + index;
		auto f = Pool.at(index);
		int old = f->state;
		
		// handle noop cases
		if (old == state::making || (old == state::made && !_Force) || ((old == state::failed || old == state::unmaking_to_failed) && !_Compiled))
			return e;

		bool HasCompiled = !!_Compiled;

		// try to take this to making
		old = f->state.exchange(HasCompiled ? state::making : state::failed);
		if (old == state::making) // let the other one do the making
			return e;

		if (!HasCompiled && (_Force || old == state::unmaking))
		{
			f->state.store(state::unmaking_to_failed);
			Unmakes.push(index);
		}

		// anything else is some sort of making which is flagged
		oASSERT(_Force || old != state::made, "unexpected state");

		if (old == state::unmaking && !_Force)
			f->state.store(state::made);
		else
		{
			f->compiled = std::move(_Compiled);
			Makes.push(index);
		}
	}

	else
	{
		void* placeholder = !!_Compiled ? Making : Failed;

		// initialize a new entry
		file_info* f = new (Pool.at(Pool.allocate())) file_info();
		if (f)
		{
			f->state = !_Compiled ? state::failed : state::making;
			f->name = _Name;
			f->path = _Path;
			f->compiled = std::move(_Compiled);
			index = (index_type)Pool.index(f);
		}
		else
			return &Missing; // oom asset?

		e = Entry + index;
		
		// another thread is trying to do this same thing, so let it
		if (Lookup.invalid_value != Lookup.set(key, index))
		{
			f->~file_info();
			Pool.deallocate(Pool.index(f));
		}
		else if (f->state == state::failed)
			*e = Failed;
	}

	Makes.push(index);
	return e;
}

void* const* concurrent_registry::make(const char* _Name, scoped_allocation& _Compiled, bool _Force)
{
	return make(_Name, _Compiled, path(), _Force);
}

bool concurrent_registry::unmake(void* const* _ppEntry)
{
	//     Truth Table
	//	             entry
	// <no entry>    retf         
	// failed         q/unmaking
	// making         q/unmaking
	// made           q/unmaking
	// unmaking       noop
	// unm_to_err     unmaking

	if (!in_range(_ppEntry, Entry, Pool.capacity()))
		return false;

	index_type index = (index_type)byte_diff(Entry, _ppEntry);
	auto f = Pool.at(index);
	int old = f->state.exchange(state::unmaking);
	if (old == state::failed || old == state::making || old == state::made)
		Unmakes.push(index);
	return true;
}

void concurrent_registry::unmake_all()
{
	unsigned int left = flush(1000000000);
	if (left)
		throw std::exception("did not flush all outstanding items");

	const unsigned int n = capacity();
	for (unsigned int i = 0; i < n; i++)
	{
		void** e = Entry + i;
		if (*e != Missing && *e != Failed && *e != Making)
		{
			Unmake(*e);
			auto f = Pool.at(i);
			*e = f->state == state::unmaking ? &Missing : &Failed;
			f->state = state::failed;
			f->~file_info();
			Pool.deallocate(Pool.index(f));
		}
	}
}

unsigned int concurrent_registry::flush(unsigned int _MaxOperations)
{
	unsigned int n = _MaxOperations;
	index_type index;
	while (n && Unmakes.pop(index))
	{
		auto f = Pool.at(index);

		// a call to make saved this asset
		if (f->state != state::unmaking && f->state != state::unmaking_to_failed)
			continue;

		void** e = Entry + index;
		Unmake(*e);
		*e = f->state == state::unmaking ? &Missing : &Failed;
		f->state = state::failed;
		Pool.deallocate(Pool.index(f));
		n--;
	}

	unsigned int EstHashesToReclaim = _MaxOperations - n;

	while (n && Makes.pop(index))
	{
		auto f = Pool.at(index);
		
		// free any prior asset (should this count as an operation n--?)
		if (Entry[index] && Entry[index] != Missing && Entry[index] != Failed && Entry[index] != Making)
			Unmake(Entry[index]);

		if (!f->compiled)
		{
			Entry[index] = Failed;
			f->state.store(state::failed);
		}

		else
		{
			try
			{
				Entry[index] = Make(std::ref(f->compiled), f->name);
				f->state.store(state::made);
			}
		
			catch (std::exception& e)
			{
				Entry[index] = Failed;
				f->state.store(state::failed);
				oTRACEA("failed: %s", e.what());
			}
			n--;
		}
	}

	if (EstHashesToReclaim <= n)
		Lookup.reclaim();

	return Makes.size() + Unmakes.size();
}

} // namespace ouro
#endif