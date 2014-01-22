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
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oDispatchQueueConcurrentT.h>
#include <oBase/threadpool.h>
#include <oCore/thread_traits.h>

using namespace ouro;

typedef threadpool<core_thread_traits> threadpool_t;

const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrentT<threadpool_t>* threadsafe const*)
{
	// {C86BB586-33C0-493C-B940-75ED91F336A2}
	static const oGUID IID_oDispatchQueueConcurrentT = { 0xc86bb586, 0x33c0, 0x493c, { 0xb9, 0x40, 0x75, 0xed, 0x91, 0xf3, 0x36, 0xa2 } };
	return IID_oDispatchQueueConcurrentT;
}

bool oDispatchQueueCreateConcurrentGeneric(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue)
{
	if (!_DebugName || !_ppDispatchQueue)
		return oErrorSetLast(std::errc::invalid_argument);

	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueueConcurrentT<threadpool_t>(_DebugName, &success));
	return !!*_ppDispatchQueue;
}

struct oDispatchQueueConcurrentSerial : public oDispatchQueueConcurrent
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oDispatchQueueConcurrent, oDispatchQueueConcurrentSerial);

	oDispatchQueueConcurrentSerial(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
		: DebugName(_DebugName)
	{
		*_pSuccess = true;
	}

	bool Dispatch(const oTASK& _Task) threadsafe override
	{
		_Task();
		return true;
	}

	void Join() threadsafe override { IsJoinable = false; }
	void Flush() threadsafe override {}
	bool Joinable() const threadsafe override { return IsJoinable; }
	const char* GetDebugName() const threadsafe override { return DebugName; }

	sstring DebugName;
	bool IsJoinable;
	oRefCount RefCount;
};

const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrentSerial* threadsafe const*)
{
	// {2D488194-69EC-4081-8CDA-1EE3E6BE8F20}
	static const oGUID IID_DQC_Serial = { 0x2d488194, 0x69ec, 0x4081, { 0x8c, 0xda, 0x1e, 0xe3, 0xe6, 0xbe, 0x8f, 0x20 } };
	return IID_DQC_Serial;
}

bool oDispatchQueueCreateConcurrentSerial(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
{
	bool success = false;
	oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentSerial(_DebugName, _InitialTaskCapacity, &success));
	return success;
}
