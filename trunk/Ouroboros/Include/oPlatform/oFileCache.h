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
#pragma once
#ifndef oFileCache_h
#define oFileCache_h

// on first request for a file it will be loaded from disc, and then subsequent requests will come from memory.
//	This behavior can be disabled though, which is useful during development. files can be manually evicted from the cache.

// {d6d85eac-5c76-4c89-a7f5-60c447f6480f}
oDEFINE_GUID_I(oFileCache, 0xd6d85eac, 0x5c76, 0x4c89, 0xa7, 0xf5, 0x60, 0xc4, 0x47, 0xf6, 0x48, 0x0f);
interface oFileCache : oInterface
{
	struct DESC
	{
		DESC() : Disable(false) {}

		bool Disable; // If true, every request will freshly load the file, i.e. no caching.
	};

	//Note that multiple threads can retrieve a file at the same time. they may get different oBuffer's
	virtual bool Retrieve(oStd::path_string& _Path, const oBuffer** _Buffer) threadsafe = 0;

	//Evict the given file from the cache. If requested again it will be reloaded from disk. no-op if the file is not currently cached.
	virtual void Evict(oStd::path_string& _Path) threadsafe = 0;
};	

bool oFileCacheCreate(const oFileCache::DESC& _Desc, threadsafe oFileCache** _ppObject);

#endif
