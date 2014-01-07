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
#include <oStd/mutex.h>
#include "crt_assert.h"

#if NTDDI_VERSION >= NTDDI_WIN7
	#define oHAS_SLIM_TRY_LOCK
#endif

static_assert(sizeof(oStd::recursive_mutex) == sizeof(CRITICAL_SECTION), "");
#ifdef _DEBUG
	static_assert(sizeof(oStd::mutex) == sizeof(SRWLOCK) + sizeof(size_t), "");
#else
	static_assert(sizeof(oStd::mutex) == sizeof(SRWLOCK), "");
#endif

class std_backoff
{
public:
	std_backoff();

	// A spin that degrades (spins longer) with each call until it makes more 
	// sense to yield.
	void pause();

	// A spin that degrades (spins longer) with each call (returning true) until 
	// it would yield, then it returns false without yielding.
	bool try_pause();

	// Resets the degradation of the spin loop.
	void reset();

private:
	static const size_t SpinThreshold = 16;
	size_t SpinCount;
	void spin(size_t _Count);
};

inline std_backoff::std_backoff()
	: SpinCount(1)
{}

#pragma optimize("", off)
inline void std_backoff::spin(size_t _Count)
{
	for (size_t i = 0; i < _Count; i++) {}
}
#pragma optimize("", on)

inline void std_backoff::pause()
{
	if (SpinCount <= SpinThreshold)
	{
		spin(SpinCount);
		SpinCount *= 2;
	}

	else
		oStd::this_thread::yield();
}

inline bool std_backoff::try_pause()
{
	if (SpinCount <= SpinThreshold)
	{
		spin(SpinCount);
		SpinCount *= 2;
		return true;
	}

	return false;
}

inline void std_backoff::reset()
{
	SpinCount = 1;
}

namespace oStd {
	const adopt_lock_t adopt_lock;
	const defer_lock_t defer_lock;
	const try_to_lock_t try_to_lock;
} // namespace oStd

oStd::mutex::mutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
}

oStd::mutex::~mutex()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		if (!try_lock())
			oCRTASSERT(false, "mutex is locked on destruction: this could result in a deadlock or race condition.");
	#endif
}

oStd::mutex::native_handle_type oStd::mutex::native_handle()
{
	return (PSRWLOCK)&Footprint;
}

void oStd::mutex::lock()
{
	// @tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oCRTASSERT(!Footprint || ThreadID != oStd::this_thread::get_id(), "mutex is non-recursive and already locked on this thread. This could result in a deadlock.");
	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = oStd::this_thread::get_id();
	#endif
}

bool oStd::mutex::try_lock()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		return !!TryAcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void oStd::mutex::unlock()
{
	#ifdef _DEBUG
		ThreadID = oStd::thread::id();
	#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

oStd::recursive_mutex::recursive_mutex()
{
	InitializeCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oStd::recursive_mutex::~recursive_mutex()
{
	if (!try_lock())
	{
		oCRTASSERT(false, "recursive_mutex is locked on destruction: this could result in a deadlock or race condition.");
		unlock();
	}

	DeleteCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oStd::recursive_mutex::native_handle_type oStd::recursive_mutex::native_handle()
{
	return (LPCRITICAL_SECTION)Footprint;
}

void oStd::recursive_mutex::lock()
{
	EnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

bool oStd::recursive_mutex::try_lock()
{
	return !!TryEnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

void oStd::recursive_mutex::unlock()
{
	LeaveCriticalSection((LPCRITICAL_SECTION)Footprint);
}

bool oStd::timed_mutex::try_lock_for(unsigned int _TimeoutMS)
{
	// Based on:
	// http://software.intel.com/en-us/blogs/2008/09/17/pondering-timed-mutex/

	std_backoff bo;

	do 
	{
		if (Mutex.try_lock())
			return true;

		if (!bo.try_pause())
		{
			oStd::this_thread::sleep_for(oStd::chrono::milliseconds(10));
			_TimeoutMS -= 10;
			bo.reset();
		}

	} while ((int)_TimeoutMS > 0);

	return false;
}

oStd::once_flag::once_flag()
	: Footprint(0)
{
	InitOnceInitialize((PINIT_ONCE)Footprint);
}

BOOL CALLBACK InitOnceCallback(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context)
{
	std::function<void()>* pFN = (std::function<void()>*)Parameter;
	(*pFN)();
	return TRUE;
}

void oStd::call_once(oStd::once_flag& _Flag, std::function<void()> _Function)
{
	InitOnceExecuteOnce(*(PINIT_ONCE*)&_Flag, InitOnceCallback, &_Function, nullptr);
}
