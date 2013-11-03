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
#include <oConcurrency/mutex.h>
#include <oPlatform/Windows/oWinSkeleton.h>
#include <oPlatform/oSingleton.h>
#include <oCore/process_heap.h>

using namespace oConcurrency;

struct oWinSkeletonContext : oProcessSingleton<oWinSkeletonContext>
{
	static const oGUID GUID;

	oWinSkeletonContext() {}
	~oWinSkeletonContext()
	{
		oASSERT(Sources.empty(), "");
	}

	void RegisterSkeletonSource(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _Get)
	{
		lock_guard<shared_mutex> lock(Mutex);
		if (Sources.find(_hSkeleton) != Sources.end())
			throw std::exception("redundant registration");
		Sources[_hSkeleton] = _Get;
	}

	void UnregisterSkeletonSource(HSKELETON _hSkeleton)
	{
		lock_guard<shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it != Sources.end())
			Sources.erase(it);
	}

	bool GetSkeletonDesc(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton)
	{
		// prevent Unregister() from being called during a read
		shared_lock<shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it == Sources.end())
			return oErrorSetLast(std::errc::invalid_argument);
		it->second(_pSkeleton);
		return true;
	}

	bool Close(HSKELETON _hSkeleton)
	{
		return true;
	}

	shared_mutex Mutex;
	std::map<HSKELETON
		, oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>
		, std::less<HSKELETON>
		, ouro::process_heap::std_allocator<std::pair<HSKELETON, oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>>>> 
	Sources;
};

// {7E5F5608-2C7A-43E7-B7A0-46293FEC653C}
const oGUID oWinSkeletonContext::GUID = { 0x7e5f5608, 0x2c7a, 0x43e7, { 0xb7, 0xa0, 0x46, 0x29, 0x3f, 0xec, 0x65, 0x3c } };

oSINGLETON_REGISTER(oWinSkeletonContext);

// Function exposure
void oWinRegisterSkeletonSource(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _Get) { oWinSkeletonContext::Singleton()->RegisterSkeletonSource(_hSkeleton, _Get); }
void oWinUnregisterSkeletonSource(HSKELETON _hSkeleton) { oWinSkeletonContext::Singleton()->UnregisterSkeletonSource(_hSkeleton); }
bool oWinGetSkeletonDesc(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton) { return oWinSkeletonContext::Singleton()->GetSkeletonDesc(_hSkeleton, _pSkeleton); }
