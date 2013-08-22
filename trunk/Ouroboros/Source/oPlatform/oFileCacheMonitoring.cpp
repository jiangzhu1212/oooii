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

class oFileCacheMonitoringImpl : public oFileCacheMonitoring
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oFileCacheMonitoring);

	oFileCacheMonitoringImpl(const oFileCacheMonitoring::DESC& _Desc, bool* _pSuccess);

	bool Retrieve(oStd::path_string& _RelativePath, const oBuffer** _Buffer) threadsafe override;
	void Evict(oStd::path_string& _RelativePath) threadsafe override;

private:
	void FolderUpdate(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI);

	oRefCount RefCount;
	oFileCacheMonitoring::DESC Desc;

	oRef<threadsafe oFileCache> FileCache;
	oRef<threadsafe oStreamMonitor> FolderMonitor;
};

oFileCacheMonitoringImpl::oFileCacheMonitoringImpl(const oFileCacheMonitoring::DESC& _Desc, bool* _pSuccess) : Desc(_Desc)
{
	*_pSuccess = false;

	oFileCache::DESC cacheDesc;
	cacheDesc.Disable = Desc.Disable;
	if(!oFileCacheCreate(cacheDesc, &FileCache))
		return;

	oStd::uri_string monitorURI;
	oURIFromAbsolutePath(monitorURI, Desc.RootPath);

	oSTREAM_MONITOR_DESC md;
	md.Monitor = monitorURI;
	md.TraceEvents = true;
	md.WatchSubtree = true;

	if(!oStreamMonitorCreate(md, oBIND(&oFileCacheMonitoringImpl::FolderUpdate, this, oBIND1, oBIND2), &FolderMonitor))
		return;

	*_pSuccess = true;
}

bool oFileCacheMonitoringImpl::Retrieve(oStd::path_string& _RelativePath, const oBuffer** _Buffer) threadsafe
{
	oStd::path_string fullPath = Desc.RootPath;
	oEnsureSeparator(fullPath);
	oStrcat(fullPath, _RelativePath.c_str());

	return FileCache->Retrieve(fullPath, _Buffer);
}

void oFileCacheMonitoringImpl::Evict(oStd::path_string& _RelativePath) threadsafe
{
	oStd::path_string fullPath = Desc.RootPath;
	oEnsureSeparator(fullPath);
	oStrcat(fullPath, _RelativePath.c_str());

	FileCache->Evict(fullPath);
}

void oFileCacheMonitoringImpl::FolderUpdate(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI)
{
	if(_Event == oSTREAM_MODIFIED || _Event == oSTREAM_REMOVED)
	{
		oStd::path_string path;
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