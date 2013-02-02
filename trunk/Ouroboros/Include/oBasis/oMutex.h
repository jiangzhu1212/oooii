/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Thin wrapper for Std::mutex that provides threadsafe specification to 
// reduce thread_cast throughout the code. Prefer using these wrappers over 
// using std::mutex directly.

// NOTE: @oooii-tony: POLICY QUESTION. Is a mutex ever really const? If I have
// MyVal GetMyVal() const threadsafe that needs to lock the mutex, should that
// be const? or should we largely punt const-ness on threadsafe accessors 
// because technically the lock is modified, but conceptually other ideas such
// as multiple concurrent readers are still valid.

// Notes/discussion about timed_mutex, which hasn't yet been implemented here.
// http://software.intel.com/en-us/blogs/2008/09/17/pondering-timed-mutex/

#pragma once
#ifndef oMutex_h
#define oMutex_h

#include <oBasis/oStdMutex.h>
#include <oBasis/oThreadsafe.h>

class oMutex
{
	oStd::mutex M;
	oStd::mutex& m() const threadsafe { return thread_cast<oStd::mutex&>(M); }
	oMutex(const oMutex&);
	oMutex& operator=(const oMutex&);
public:
	oMutex() {}
	~oMutex() {}
	void lock() threadsafe { m().lock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	void unlock() threadsafe { m().unlock(); }
	typedef oStd::recursive_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

class oRecursiveMutex
{
	oStd::recursive_mutex M;
	oStd::recursive_mutex& m() const threadsafe { return thread_cast<oStd::recursive_mutex&>(M); }
	oRecursiveMutex(const oRecursiveMutex&);
	oRecursiveMutex& operator=(const oRecursiveMutex&);
public:
	oRecursiveMutex() {}
	~oRecursiveMutex() {}
	void lock() threadsafe { m().lock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	void unlock() threadsafe { m().unlock(); }
	typedef oStd::recursive_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

class oSharedMutex
{
	oStd::shared_mutex M;
	oStd::shared_mutex& m() const threadsafe { return thread_cast<oStd::shared_mutex&>(M); }
	oSharedMutex(const oSharedMutex&);
	oSharedMutex& operator=(const oSharedMutex&);
public:
	oSharedMutex() {}
	~oSharedMutex() {}
	void lock() threadsafe { m().lock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	void unlock() threadsafe { m().unlock(); }
	void lock_shared() const threadsafe { m().lock_shared(); }
	bool try_lock_shared() const threadsafe { return m().try_lock_shared(); }
	void unlock_shared() const threadsafe { m().unlock_shared(); }
	typedef oStd::recursive_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

template<class MutexT> class oLockGuard
{
	threadsafe MutexT& m;
public:
	inline oLockGuard(threadsafe MutexT& _Mutex) : m(_Mutex) { m.lock(); }
	inline ~oLockGuard() { m.unlock(); }
};

template<class MutexT> class oSharedLock
{
	const threadsafe MutexT& m;
public:
	inline oSharedLock(const threadsafe MutexT& _Mutex) : m(_Mutex) { m.lock_shared(); }
	inline ~oSharedLock() { m.unlock_shared(); }
};

template <class Mutex> class oUniqueLock
{
	oStd::unique_lock<Mutex> Lock;
	oStd::unique_lock<Mutex>& L() threadsafe { return thread_cast<oStd::unique_lock<Mutex>&>(Lock); }
	const oStd::unique_lock<Mutex>& L() const threadsafe { return thread_cast<const oStd::unique_lock<Mutex>&>(Lock); }
public:
	typedef Mutex mutex_type;

	oUniqueLock() {}
	explicit oUniqueLock(threadsafe mutex_type& _Mutex) : Lock(thread_cast<mutex_type&>(_Mutex)) {}
	oUniqueLock(threadsafe mutex_type& _Mutex, oStd::adopt_lock_t _AdoptLock) : Lock(thread_cast<mutex_type&>(_Mutex), _AdoptLock) {}
	oUniqueLock(threadsafe mutex_type& _Mutex, oStd::defer_lock_t _DeferLock) : Lock(thread_cast<mutex_type&>(_Mutex), _DeferLock) {}
	oUniqueLock(mutex_type& _Mutex, oStd::try_to_lock_t _TryToLock) : Lock(thread_cast<mutex_type&>(_Mutex), _TryToLock) {}
	template<typename Clock, typename Duration> oUniqueLock(threadsafe mutex_type& _Mutex, oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime) : Lock(thread_cast<mutex_type&>(_Mutex), _AbsoluteTime) {}
	template<typename Rep,typename Period> oUniqueLock(threadsafe mutex_type& _Mutex, oStd::chrono::duration<Rep,Period> const& _RelativeTime) : Lock(thread_cast<mutex_type&>(_Mutex), _RelativeTime) {}
	~oUniqueLock() {}
	oUniqueLock(oUniqueLock&& _That) : Lock(_That.Lock) {}
	oUniqueLock& operator=(oUniqueLock&& _That) { Lock = _That.Lock; }
	void swap(oUniqueLock&& _That) { oStd::swap(Lock, _That.Lock); }
	void lock() threadsafe { L().lock(); }
	bool try_lock() threadsafe { return L().try_lock(); }
	template<typename Rep, typename Period> bool try_lock_for(oStd::chrono::duration<Rep,Period> const& _RelativeTime) threadsafe { return L().try_lock_for(_RelativeTime); }
	template<typename Clock, typename Duration> bool try_lock_until(oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime) threadsafe { return L().try_lock_until(_AbsoluteTime); }
	void unlock() threadsafe { L().unlock(); }
	/*explicit*/ operator bool() const threadsafe { return owns_lock(); }
	bool owns_lock() const threadsafe { return L().owns_lock(); }
	mutex_type* mutex() const threadsafe { return L().mutex(); }
	mutex_type* release() threadsafe { L().release(); }
};

#endif
