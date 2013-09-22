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
#include <oPlatform/oFileCache.h>
#include <oPlatform/oStream.h>
#include <oConcurrency/mutex.h>
#include <oBasis/oLockThis.h>
#include <oStd/atomic.h>
#include <oPlatform/oRegistry.h>

using namespace ouro;

class oFileCacheImpl : public oFileCache
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oFileCache);

	oFileCacheImpl(const oFileCache::DESC& _Desc, bool* _pSuccess);
	~oFileCacheImpl();

	bool Retrieve(path_string& _Uri, const oBuffer** _Buffer) threadsafe override;
	void Evict(path_string& _Path) threadsafe override;

private:
	oRefCount RefCount;
	oFileCache::DESC Desc;
	
	intrusive_ptr<threadsafe oRegistry> Registry;
};

oFileCacheImpl::oFileCacheImpl(const oFileCache::DESC& _Desc, bool* _pSuccess) : Desc(_Desc)
{
	*_pSuccess = false;

	if(!oRegistryCreate(&Registry))
		return;

	*_pSuccess = true;
}

oFileCacheImpl::~oFileCacheImpl()
{
}

bool oFileCacheImpl::Retrieve(path_string& _Path, const oBuffer** _Buffer) threadsafe
{
	uri_string suri;
	oURIFromAbsolutePath(suri, _Path);
	oURI uri = suri;

	if(!Desc.Disable) //never load from the cache if not disabled
	{
		if(Registry->Exists(uri))
		{
			if(Registry->Get(uri, (oInterface**)_Buffer))
				return true;
		}
	}
	
	//It is possible for more than one thread to load the file here, slow in that case, but should be rare
	intrusive_ptr<threadsafe oStreamReader> fileReader;
	if (!oStreamReaderCreate(_Path, &fileReader))
		return oErrorSetLast(std::errc::invalid_argument, "Could not open supplied file %s", _Path);

	oSTREAM_DESC fileDesc;
	fileReader->GetDesc(&fileDesc);

	intrusive_ptr<oBuffer> buffer;
	if(!oBufferCreate("oFileCache entry buffer", oBuffer::New(oSizeT(fileDesc.Size)), oSizeT(fileDesc.Size), oBuffer::Delete, &buffer))
		return oErrorSetLast(std::errc::invalid_argument, "Could not create an oBuffer to hold the file %s", _Path);
		
	oSTREAM_READ r;
	r.pData = buffer->GetData();
	r.Range = fileDesc;
	if(!fileReader->Read(r))
	{
		return oErrorSetLast(std::errc::invalid_argument, "Could not read supplied file");
	}

	if(!Desc.Disable)
	{
		Registry->Add(uri, buffer); //might fail if more than one thread is here at once. doesn't matter though, both will just pull the entry below.

		if(!Registry->Get(uri, (oInterface**)_Buffer))
			return false;
	}
	else
	{
		buffer->Reference();
		*_Buffer = buffer;
	}

	return true;
}

void oFileCacheImpl::Evict(path_string& _Path) threadsafe
{
	uri_string suri;
	oURIFromAbsolutePath(suri, _Path);
	oURI uri = suri;

	if(Registry->Exists(uri))
	{
		Registry->Remove(uri);
	}
}

bool oFileCacheCreate(const oFileCache::DESC& _Desc, threadsafe oFileCache** _ppObject)
{
	bool success = false;
	oCONSTRUCT(_ppObject, oFileCacheImpl(_Desc, &success));
	return success;
}