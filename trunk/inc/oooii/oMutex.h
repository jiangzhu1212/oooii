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

// Mutex for multithreaded synchronization. Though mutex is really a 
// platform concept, it is so fundamental to ensure threadsafe API and 
// is really a very simple object to wrap that a bootstrap exception
// is made and some forward-declaritive hoops are jumped through to
// ensure this interface is available even for generic code.
#pragma once
#ifndef oMutex_h
#define oMutex_h

#include <oooii/oMutexInternal.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

class oMutex : oNoncopyable
{
	// oMutex is recursive. oMutex is assumed mutable, so all API is const
	oMUTEX_FOOTPRINT();
public:
	oMutex();
	~oMutex();
	void* GetNativeHandle() const threadsafe;
	void Lock() const threadsafe;
	bool TryLock() const threadsafe;
	void Unlock() const threadsafe;

	class ScopedLock : oNoncopyable
	{
		const threadsafe oMutex& m;
	public:
		inline ScopedLock(const threadsafe oMutex& _Mutex) : m(_Mutex) { m.Lock(); }
		inline ~ScopedLock() { m.Unlock(); }
	};
};

class oRWMutex : oNoncopyable
{
	// oRWMutex is non-recursive.
	oRWMUTEX_FOOTPRINT();
public:
	oRWMutex();
	~oRWMutex();

	void* GetNativeHandle() threadsafe;
	const void* GetNativeHandle() const threadsafe;
	void Lock() threadsafe; // Locks exclusively, no other lock attempt can go through
	void LockRead() const threadsafe; // Locks shared, other shared locks can go through, but not exclusives
	void Unlock() threadsafe;
	void UnlockRead() const threadsafe;

	#ifdef oHAS_RWMUTEX_TRYLOCK
		bool TryLock() threadsafe;
		bool TryLockRead() const threadsafe;
	#endif

	class ScopedLock : oNoncopyable
	{
		threadsafe oRWMutex& m;
	public:
		inline ScopedLock(threadsafe oRWMutex& _Mutex) : m(_Mutex) { m.Lock(); }
		inline ~ScopedLock() { m.Unlock(); }
	};

	class ScopedLockRead : oNoncopyable
	{
		const threadsafe oRWMutex& m;
	public:
		inline ScopedLockRead(const threadsafe oRWMutex& _Mutex) : m(_Mutex) { m.LockRead(); }
		inline ~ScopedLockRead() { m.UnlockRead(); }
	};
};

#endif
