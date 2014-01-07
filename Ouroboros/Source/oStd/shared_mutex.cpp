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
#include <oStd/shared_mutex.h>
#include "crt_assert.h"

#if NTDDI_VERSION >= NTDDI_WIN7
	#define oHAS_SLIM_TRY_LOCK
#endif

using namespace oStd;

namespace ouro {

static_assert(sizeof(SRWLOCK) == (sizeof(void*)), "footprint does not match system implementation size");

shared_mutex::shared_mutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = thread::id();
	#endif
}

shared_mutex::~shared_mutex()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		if (!try_lock())
			oCRTASSERT(false, "shared_mutex is locked on destruction: this could result in a deadlock or race condition.");
	#endif
}

shared_mutex::native_handle_type shared_mutex::native_handle()
{
	return (PSRWLOCK)&Footprint;
}

void shared_mutex::lock()
{
	// @tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oCRTASSERT(!Footprint || ThreadID != this_thread::get_id(), "shared_mutex is non-recursive and already read/shared locked on this thread. This could result in a deadlock.");
	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = this_thread::get_id();
	#endif
}

bool shared_mutex::try_lock()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		return !!TryAcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void shared_mutex::unlock()
{
	#ifdef _DEBUG
		ThreadID = thread::id();
	#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

void shared_mutex::lock_shared()
{
	// @tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oCRTASSERT(!Footprint || ThreadID != this_thread::get_id(), "shared_mutex is non-recursive and already read/shared locked on this thread. This could result in a deadlock.");
	AcquireSRWLockShared((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = this_thread::get_id();
	#endif
}

bool shared_mutex::try_lock_shared()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		return !!TryAcquireSRWLockShared((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void shared_mutex::unlock_shared()
{
	#ifdef _DEBUG
		ThreadID = thread::id();
	#endif
	ReleaseSRWLockShared((PSRWLOCK)&Footprint);
}

} // namespace ouro