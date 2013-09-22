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
// Consolidate the implementation for translating an oThreadpool interfaces to
// an oDispatchQueueConcurrent implementation.
#pragma once
#ifndef oDispatchQueueConcurrentT_h
#define oDispatchQueueConcurrentT_h

#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBase/fixed_string.h>
#include <oBasis/oRefCount.h>
#include <oConcurrency/thread_safe.h>

template<typename ThreadpoolT> struct oDispatchQueueConcurrentT : public oDispatchQueueConcurrent
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	// Client code must define:
	// const oGUID& oGetGUID(
	// threadsafe const oDispatchQueueConcurrentT<ThreadpoolT>* threadsafe const*)
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oDispatchQueue, oDispatchQueueConcurrentT<ThreadpoolT>);

	oDispatchQueueConcurrentT(const char* _DebugName, bool* _pSuccess)
		: DebugName(_DebugName)
	{
		*_pSuccess = true;
	}

	bool Dispatch(const oTASK& _Task) threadsafe override
	{
		try { Threadpool.dispatch(_Task); }
		catch (...) { return false; }
		return true;
	}

	void Flush() threadsafe override { Threadpool.flush(); }
	bool Joinable() const threadsafe override { return  Threadpool.joinable(); }
	void Join() threadsafe override { return Threadpool.join(); }
	const char* GetDebugName() const threadsafe override { return DebugName; }

	ThreadpoolT Threadpool;
	oRefCount RefCount;
	ouro::sstring DebugName;
};

#endif
