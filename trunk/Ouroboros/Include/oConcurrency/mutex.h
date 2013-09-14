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
// Thin wrapper for Std::mutex that provides threadsafe specification to 
// reduce thread_cast throughout the code. Prefer using these wrappers over 
// using std::mutex directly.
#pragma once
#ifndef oConcurrency_mutex_h
#define oConcurrency_mutex_h

#include <oConcurrency/thread_safe.h>
#include <oStd/mutex.h>

namespace oConcurrency {

class mutex
{
	oStd::mutex M;
	oStd::mutex& m() const threadsafe { return thread_cast<oStd::mutex&>(M); }
	mutex(const mutex&);
	mutex& operator=(const mutex&);
public:
	mutex() {}
	~mutex() {}
	void lock() threadsafe { m().lock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	void unlock() threadsafe { m().unlock(); }
	typedef oStd::recursive_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

class recursive_mutex
{
	oStd::recursive_mutex M;
	oStd::recursive_mutex& m() const threadsafe { return thread_cast<oStd::recursive_mutex&>(M); }
	recursive_mutex(const recursive_mutex&);
	recursive_mutex& operator=(const recursive_mutex&);
public:
	recursive_mutex() {}
	~recursive_mutex() {}
	void lock() threadsafe { m().lock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	void unlock() threadsafe { m().unlock(); }
	typedef oStd::recursive_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

class shared_mutex
{
	oStd::shared_mutex M;
	oStd::shared_mutex& m() const threadsafe { return thread_cast<oStd::shared_mutex&>(M); }
	shared_mutex(const shared_mutex&);
	shared_mutex& operator=(const shared_mutex&);
public:
	shared_mutex() {}
	~shared_mutex() {}
	void lock() threadsafe { m().lock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	void unlock() threadsafe { m().unlock(); }
	void lock_shared() const threadsafe { m().lock_shared(); }
	bool try_lock_shared() const threadsafe { return m().try_lock_shared(); }
	void unlock_shared() const threadsafe { m().unlock_shared(); }
	typedef oStd::recursive_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

class timed_mutex
{
	oStd::timed_mutex M;
	oStd::timed_mutex& m() const threadsafe { return thread_cast<oStd::timed_mutex&>(M); }
	timed_mutex(const timed_mutex&);
	timed_mutex& operator=(const timed_mutex&);
public:
	timed_mutex() {}
	~timed_mutex() {}
	void lock() threadsafe { m().lock(); }
	void unlock() threadsafe { m().unlock(); }
	bool try_lock() threadsafe { return m().try_lock(); }
	template<typename Rep, typename Period> bool try_lock_for(oStd::chrono::duration<Rep, Period> const& _RelativeTime) threadsafe { return m().try_lock_for(_RelativeTime); }
	template<typename Clock, typename Duration> bool try_lock_until(oStd::chrono::time_point<Clock, Duration> const& _AbsoluteTime) threadsafe { return m().try_lock_until(_AbsoluteTime); }
	typedef oStd::timed_mutex::native_handle_type native_handle_type;
	native_handle_type native_handle() const threadsafe { return m().native_handle(); }
};

template<class MutexT> class lock_guard
{
	threadsafe MutexT& m;
public:
	inline lock_guard(threadsafe MutexT& _Mutex) : m(_Mutex) { m.lock(); }
	inline ~lock_guard() { m.unlock(); }
};

template<class MutexT> class shared_lock
{
	const threadsafe MutexT& m;
public:
	inline shared_lock(const threadsafe MutexT& _Mutex) : m(_Mutex) { m.lock_shared(); }
	inline ~shared_lock() { m.unlock_shared(); }
};

template <class Mutex> class unique_lock
{
	oStd::unique_lock<Mutex> Lock;
	oStd::unique_lock<Mutex>& L() threadsafe { return thread_cast<oStd::unique_lock<Mutex>&>(Lock); }
	const oStd::unique_lock<Mutex>& L() const threadsafe { return thread_cast<const oStd::unique_lock<Mutex>&>(Lock); }
public:
	typedef Mutex mutex_type;

	unique_lock() {}
	explicit unique_lock(threadsafe mutex_type& _Mutex) : Lock(thread_cast<mutex_type&>(_Mutex)) {}
	unique_lock(threadsafe mutex_type& _Mutex, oStd::adopt_lock_t _AdoptLock) : Lock(thread_cast<mutex_type&>(_Mutex), _AdoptLock) {}
	unique_lock(threadsafe mutex_type& _Mutex, oStd::defer_lock_t _DeferLock) : Lock(thread_cast<mutex_type&>(_Mutex), _DeferLock) {}
	unique_lock(mutex_type& _Mutex, oStd::try_to_lock_t _TryToLock) : Lock(thread_cast<mutex_type&>(_Mutex), _TryToLock) {}
	template<typename Clock, typename Duration> unique_lock(threadsafe mutex_type& _Mutex, oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime) : Lock(thread_cast<mutex_type&>(_Mutex), _AbsoluteTime) {}
	template<typename Rep,typename Period> unique_lock(threadsafe mutex_type& _Mutex, oStd::chrono::duration<Rep,Period> const& _RelativeTime) : Lock(thread_cast<mutex_type&>(_Mutex), _RelativeTime) {}
	~unique_lock() {}
	unique_lock(unique_lock&& _That) : Lock(_That.Lock) {}
	unique_lock& operator=(unique_lock&& _That) { Lock = _That.Lock; }
	void swap(unique_lock&& _That) { oStd::swap(Lock, _That.Lock); }
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

} // namespace oConcurrency

#endif
