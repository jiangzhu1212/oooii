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
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBase/fixed_string.h>
#include <oBasis/oRefCount.h>
#include <oConcurrency/threadpool.h>

using namespace ouro;

struct oDispatchQueuePrivate_Impl : oDispatchQueuePrivate
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oDispatchQueue, oDispatchQueuePrivate);

	oDispatchQueuePrivate_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess);

	bool Dispatch(const oTASK& _Task) threadsafe override
	{
		try { Threadpool.dispatch(_Task); }
		catch (...) { return false; }
		return true;
	}
	void Flush() threadsafe override { Threadpool.flush(); }
	bool Joinable() const threadsafe override { return Threadpool.joinable(); }
	void Join() threadsafe override { Threadpool.join(); }
	const char* GetDebugName() const threadsafe override { return DebugName; }

protected:
	oConcurrency::threadpool<std::allocator<oTASK>> Threadpool;
	oRefCount RefCount;
	sstring DebugName;
};

// _DebugName must be a constant string
oDispatchQueuePrivate_Impl::oDispatchQueuePrivate_Impl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
	: Threadpool(1)
	, DebugName(_DebugName)
{
	*_pSuccess = true;
}

bool oDispatchQueueCreatePrivate(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueuePrivate** _ppDispatchQueue)
{
	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueuePrivate_Impl(_DebugName, _InitialTaskCapacity, &success));
	return !!*_ppDispatchQueue;
}
