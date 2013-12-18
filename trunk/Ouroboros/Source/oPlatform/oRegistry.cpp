
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
#include <oPlatform/oRegistry.h>
#include <oConcurrency/mutex.h>
#include <tbb/concurrent_hash_map.h>

using namespace ouro;
using namespace oConcurrency;

struct oRegistryTBB : oRegistry
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oRegistryTBB(bool* _pSuccess)
		: Epoch(0)
	{
		*_pSuccess = true;
	}

	void SetEpoch(int _Epoch) threadsafe override { Epoch = _Epoch; }
	int GetEpoch() const threadsafe override { return Epoch; }
	int GetNumEntries() threadsafe const override { return oInt(hash().size()); }
	void Clear() threadsafe override { lock_guard<shared_mutex> lock(TopologyMutex); hash().clear(); }
	bool Exists(const oURI& _URIReference) const threadsafe override { return hash().count(_URIReference) == 1; }
	bool Add(const oURI& _URIReference, oInterface* _pInterface, int _Status = oInvalid) threadsafe override;
	bool Remove(const oURI& _URIReference) threadsafe override;
	bool Get(const oURI& _URIReference, oInterface** _ppInterface, oREGISTRY_DESC* _pDesc = nullptr) const threadsafe override;
	bool Set(const oURI& _URIReference, oInterface* _pInterface, int _Status = oInvalid) threadsafe override;
	void Enum(std::function<bool(const oURI& _URIReference, oInterface* _pInterface, const oREGISTRY_DESC& _Desc)> _Visitor) threadsafe override;
private:
	struct RECORD
	{
		oREGISTRY_DESC Desc;
		intrusive_ptr<oInterface> Entry;
	};

	struct TBBURIHash
	{
		static size_t hash(const oURI& _URI) { return _URI.Hash(); }
		static bool equal(const oURI& a, const oURI& b) { return a == b; }
	};

	shared_mutex TopologyMutex;
	oRefCount RefCount;
	int Epoch;

	typedef tbb::concurrent_hash_map<oURI, RECORD, TBBURIHash> hashmap_t;
	hashmap_t Hash;

	hashmap_t& hash() threadsafe { return thread_cast<hashmap_t&>(Hash); }
	const hashmap_t& hash() const threadsafe { return thread_cast<const hashmap_t&>(Hash); }
};

bool oRegistryCreate(threadsafe oRegistry** _ppRegistry)
{
	bool success = false;
	oCONSTRUCT(_ppRegistry, oRegistryTBB(&success));
	return success;
}

bool oRegistryTBB::Add(const oURI& _URIReference, oInterface* _pInterface, int _Status) threadsafe
{
	shared_lock<shared_mutex> lock(TopologyMutex); // shared because hash is threadsafe in this case
	hashmap_t::accessor a;
	bool inserted = hash().insert(a, _URIReference);
	if (inserted)
	{
		a->second.Desc.Status = _Status;
		a->second.Desc.AccessEpoch = Epoch;
		a->second.Desc.AccessCount = 0;
		ouro::system::now(&a->second.Desc.Accessed);
		a->second.Entry = _pInterface;
	}
	
	return inserted;
}

bool oRegistryTBB::Remove(const oURI& _URIReference) threadsafe
{
	shared_lock<shared_mutex> lock(TopologyMutex); // shared because hash is threadsafe in this case
	return hash().erase(_URIReference);
}

bool oRegistryTBB::Get(const oURI& _URIReference, oInterface** _ppInterface, oREGISTRY_DESC* _pDesc) const threadsafe
{
	shared_lock<shared_mutex> lock(TopologyMutex); // shared because hash is threadsafe in this case
	hashmap_t::accessor a;
	bool found = hash().find(a, _URIReference);
	if (found)
	{
		if (a->second.Entry)
		{
			*_ppInterface = a->second.Entry;
			(*_ppInterface)->Reference();
		}

		a->second.Desc.AccessEpoch = Epoch;
		a->second.Desc.AccessCount++;
		ouro::system::now(&a->second.Desc.Accessed);

		if (_pDesc)
			*_pDesc = a->second.Desc;
	}

	return found;
}

bool oRegistryTBB::Set(const oURI& _URIReference, oInterface* _pInterface, int _Status) threadsafe
{
	shared_lock<shared_mutex> lock(TopologyMutex); // shared because hash is threadsafe in this case
	hashmap_t::accessor a;
	bool found = hash().find(a, _URIReference);
	if (found)
	{
		a->second.Desc.Status = _Status;
		a->second.Desc.AccessEpoch = Epoch;
		a->second.Desc.AccessCount = 0;
		ouro::system::now(&a->second.Desc.Accessed);
		a->second.Entry = _pInterface;
	}

	return found;
}

void oRegistryTBB::Enum(std::function<bool(const oURI& _URIReference, oInterface* _pInterface, const oREGISTRY_DESC& _Desc)> _Visitor) threadsafe
{
	lock_guard<shared_mutex> lock(TopologyMutex); // exclusive because iterators are not threadsafe
	oFOR(auto& pair, hash())
		if (!_Visitor(pair.first, pair.second.Entry, pair.second.Desc))
			break;
}
