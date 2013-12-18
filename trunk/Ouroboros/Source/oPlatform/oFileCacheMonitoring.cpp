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
#include <oPlatform/oFileCacheMonitoring.h>
#include <oPlatform/oFileCache.h>
#include <oPlatform/oStream.h>
#include <oConcurrency/mutex.h>
#include <oBasis/oLockThis.h>

using namespace ouro;

class oFileCacheMonitoringImpl : public oFileCacheMonitoring
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oFileCacheMonitoring);

	oFileCacheMonitoringImpl(const oFileCacheMonitoring::DESC& _Desc, bool* _pSuccess);

	bool Retrieve(path_string& _RelativePath, const oBuffer** _Buffer) threadsafe override;
	void Evict(path_string& _RelativePath) threadsafe override;

private:
	void FolderUpdate(oSTREAM_EVENT _Event, const uri_string& _ChangedURI);

	oRefCount RefCount;
	oFileCacheMonitoring::DESC Desc;

	intrusive_ptr<threadsafe oFileCache> FileCache;
	intrusive_ptr<threadsafe oStreamMonitor> FolderMonitor;
};

oFileCacheMonitoringImpl::oFileCacheMonitoringImpl(const oFileCacheMonitoring::DESC& _Desc, bool* _pSuccess) : Desc(_Desc)
{
	*_pSuccess = false;

	oFileCache::DESC cacheDesc;
	cacheDesc.Disable = Desc.Disable;
	if(!oFileCacheCreate(cacheDesc, &FileCache))
		return;

	uri_string monitorURI;
	oURIFromAbsolutePath(monitorURI, Desc.RootPath);

	oSTREAM_MONITOR_DESC md;
	md.Monitor = monitorURI;
	md.TraceEvents = true;
	md.WatchSubtree = true;

	if(!oStreamMonitorCreate(md, std::bind(&oFileCacheMonitoringImpl::FolderUpdate, this, oBIND1, oBIND2), &FolderMonitor))
		return;

	*_pSuccess = true;
}

bool oFileCacheMonitoringImpl::Retrieve(path_string& _RelativePath, const oBuffer** _Buffer) threadsafe
{
	path_string fullPath = Desc.RootPath;
	if (fullPath[fullPath.length()-1] != '/')
		strlcat(fullPath, "/");

	strlcat(fullPath, _RelativePath);

	return FileCache->Retrieve(fullPath, _Buffer);
}

void oFileCacheMonitoringImpl::Evict(path_string& _RelativePath) threadsafe
{
	path_string fullPath = Desc.RootPath;
	if (fullPath[fullPath.length()-1] != '/')
		strlcat(fullPath, "/");

	strlcat(fullPath, _RelativePath);

	FileCache->Evict(fullPath);
}

void oFileCacheMonitoringImpl::FolderUpdate(oSTREAM_EVENT _Event, const uri_string& _ChangedURI)
{
	if(_Event == oSTREAM_MODIFIED || _Event == oSTREAM_REMOVED)
	{
		path_string path;
		oURIToPath(path, _ChangedURI);
		FileCache->Evict(path);
	}
}

bool oFileCacheMonitoringCreate(const oFileCacheMonitoring::DESC& _Desc, threadsafe oFileCacheMonitoring** _ppObject)
{
	bool success = false;
	oCONSTRUCT(_ppObject, oFileCacheMonitoringImpl(_Desc, &success));
	return success;
}