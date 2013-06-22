/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#pragma once
#ifndef oFileCacheMonitoring_h
#define oFileCacheMonitoring_h

// A version of oFileCache that will auto evict files if they are changed. Files are below a single root folder, and pathes given
//	to retrieve should be relative to that path. You can still manually evict files as well.

// {7142b4f7-5ced-4540-93d4-44c9e0eb1303}
oDEFINE_GUID_I(oFileCacheMonitoring, 0x7142b4f7, 0x5ced, 0x4540, 0x93, 0xd4, 0x44, 0xc9, 0xe0, 0xeb, 0x13, 0x03);
interface oFileCacheMonitoring : oInterface
{
	struct DESC
	{
		DESC() : Disable(false) {}

		bool Disable; // Not sure if this is useful for this version of oFileCache. changed files get evicted automatically anyway.
		oStd::path_string RootPath;
	};

	//Path is relative to the root path in the Desc.
	virtual bool Retrieve(oStd::path_string& _RelativePath, const oBuffer** _Buffer) threadsafe = 0;

	//Evict the given file from the cache. If requested again it will be reloaded from disk. no-op if the file is not currently cached.
	virtual void Evict(oStd::path_string& _RelativePath) threadsafe = 0;
};	

bool oFileCacheMonitoringCreate(const oFileCacheMonitoring::DESC& _Desc, threadsafe oFileCacheMonitoring** _ppObject);

#endif
