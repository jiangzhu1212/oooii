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
#include <oBasis/oDispatchQueueGlobal.h>
#include <oBasis/oDispatchQueueGlobalT.h>
#include <oBase/concurrency.h>

struct oDispatchQueueGlobal_Impl
{
	bool Init(oInterface* _pSelf)
	{
		return true;
	}
	int ReferenceImpl() threadsafe
	{ 
		return (RefCount).Reference();
	}

	bool ReleaseImpl() threadsafe
	{ 
		if (RefCount.Release()) return true;
		else return false;
	}

	void Issue(const oTASK& _Task) threadsafe
	{
		ouro::dispatch(_Task);
	}
private:
	oRefCount RefCount;
};

bool oDispatchQueueCreateGlobal(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueGlobal** _ppDispatchQueue)
{
	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueueGlobalT<oDispatchQueueGlobal_Impl>(_DebugName, _InitialTaskCapacity, &success));
	return success;
}
