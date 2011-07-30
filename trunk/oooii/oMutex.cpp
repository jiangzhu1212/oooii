/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oooii/oMutex.h>
#include <oooii/oAssert.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>

oSTATICASSERT(sizeof(oMutex) == sizeof(CRITICAL_SECTION));
#ifdef _DEBUG
	oSTATICASSERT(sizeof(oRWMutex) == sizeof(SRWLOCK) + sizeof(size_t));
#else
	oSTATICASSERT(sizeof(oRWMutex) == sizeof(SRWLOCK));
#endif

oMutex::oMutex()
{
	InitializeCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oMutex::~oMutex()
{
	if (!TryLock())
	{
		oASSERT(false, "oMutex is locked on destruction. This could result in a deadlock or race condition.");
		Unlock();
	}

	DeleteCriticalSection((LPCRITICAL_SECTION)Footprint);
}

void* oMutex::GetNativeHandle() const threadsafe
{
	return (LPCRITICAL_SECTION)Footprint;
}

void oMutex::Lock() const threadsafe
{
	EnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

bool oMutex::TryLock() const threadsafe
{
	return !!TryEnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

void oMutex::Unlock() const threadsafe
{
	LeaveCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oRWMutex::oRWMutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = 0;
	#endif
}

oRWMutex::~oRWMutex()
{
	#ifdef _DEBUG
		#ifdef oHAS_RWMUTEX_TRYLOCK

			if (!TryLockRead())
				oASSERT(false, "oRWMutex is locked on destruction. This could result in a deadlock or race condition.");
			UnlockRead();

			if (!TryLock())
				oASSERT(false, "oRWMutex is locked on destruction. This could result in a deadlock or race condition.");
			Unlock();

		#endif
	#endif
}

void* oRWMutex::GetNativeHandle() threadsafe
{
	return (PSRWLOCK)&Footprint;
}

const void* oRWMutex::GetNativeHandle() const threadsafe
{
	return (PSRWLOCK)&Footprint;
}

void oRWMutex::Lock() threadsafe
{
	// @oooii-tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oASSERT(!Footprint || ThreadID != ::GetCurrentThreadId(), "oRWMutex is non-recursive and already read/shared locked on this thread. This could result in a deadlock.");

	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);

	#ifdef _DEBUG
		ThreadID = ::GetCurrentThreadId();
	#endif
}

void oRWMutex::LockRead() const threadsafe
{
	// @oooii-tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oASSERT(!Footprint || ThreadID != ::GetCurrentThreadId(), "oRWMutex is non-recursive and already read/shared locked on this thread. This could result in a deadlock.");
	
	AcquireSRWLockShared((PSRWLOCK)&Footprint);

	#ifdef _DEBUG
		ThreadID = ::GetCurrentThreadId();
	#endif
}

void oRWMutex::Unlock() threadsafe
{
	#ifdef _DEBUG
		ThreadID = 0;
	#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

void oRWMutex::UnlockRead() const threadsafe
{
	#ifdef _DEBUG
		ThreadID = 0;
	#endif
	ReleaseSRWLockShared((PSRWLOCK)&Footprint);
}

#ifdef oHAS_RWMUTEX_TRYLOCK
	bool oRWMutex::TryLock() threadsafe
	{
		return !!TryAcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	}

	bool oRWMutex::TryLockRead() const threadsafe
	{
		return !!TryAcquireSRWLockShared((PSRWLOCK)&Footprint);
	}
#endif
