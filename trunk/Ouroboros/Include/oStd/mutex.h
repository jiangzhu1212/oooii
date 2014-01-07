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
// Approximation of the upcoming C++11 oStd::mutex interface.

#pragma once
#ifndef oStd_mutex_h
#define oStd_mutex_h

#include <cassert>
#include <oStd/thread.h>

// To keep the main classes neat, collect all the platform-specific forward
// declaration here. This is done in this vague manner to avoid including 
// platform headers in this file.
#if defined(_WIN32) || defined(_WIN64)
	#ifdef _WIN64
		#define oRECURSIVE_MUTEX_FOOTPRINT() mutable unsigned long long Footprint[5] // RTL_CRITICAL_SECTION
	#elif defined(_WIN32)
		#define oRECURSIVE_MUTEX_FOOTPRINT() mutable unsigned int Footprint[6]
	#endif

	#define oMUTEX_FOOTPRINT() void* Footprint

	#define oONCEFLAG_FOOTPRINT() void* Footprint

#else
	#error Unsupported platform (oRECURSIVE_MUTEX_FOOTPRINT, oMUTEX_FOOTPRINT, oSHARED_MUTEX_FOOTPRINT)
#endif

namespace oStd {

	// Do not acquire ownership of the mutex.
	struct defer_lock_t {};
	// Try to acquire ownership of the mutex without blocking.
	struct try_to_lock_t {};
	// Assume the calling thread has already obtained mutex ownership and 
	// manage it.
	struct adopt_lock_t {};
	extern const adopt_lock_t adopt_lock;
	extern const defer_lock_t defer_lock;
	extern const try_to_lock_t try_to_lock;

	class mutex
	{
		oMUTEX_FOOTPRINT();
		#ifdef _DEBUG
			oStd::thread::id ThreadID;
		#endif
		mutex(const mutex&); /* = delete */
		mutex& operator=(const mutex&); /* = delete */
	public:
		mutex();
		~mutex();

		void lock();
		bool try_lock();
		void unlock();

		typedef void* native_handle_type;
		native_handle_type native_handle();
	};

	class recursive_mutex
	{
		oRECURSIVE_MUTEX_FOOTPRINT();
		recursive_mutex(const recursive_mutex&); /* = delete */
		recursive_mutex& operator=(const recursive_mutex&); /* = delete */
	public:
		recursive_mutex();
		~recursive_mutex();

		void lock();
		bool try_lock();
		void unlock();

		typedef void* native_handle_type;
		native_handle_type native_handle();
	};

	class timed_mutex
	{
		mutex Mutex;
		#ifdef _DEBUG
			oStd::thread::id ThreadID;
		#endif
		timed_mutex(timed_mutex const&); /* = delete */
		timed_mutex& operator=(timed_mutex const&); /* = delete */

		bool try_lock_for(unsigned int _TimeoutMS);

	public:
		timed_mutex() {}
		~timed_mutex() {}

		void lock() { Mutex.lock(); }
		void unlock() { Mutex.unlock(); }
		bool try_lock() { return Mutex.try_lock(); }

		template<typename Rep, typename Period>
		bool try_lock_for(chrono::duration<Rep,Period> const& _RelativeTime)
		{
			chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(_RelativeTime);
			Rep count = ms.count();
			assert(Rep(count) == count && "RelativeTime is too large");
			return try_lock_for(static_cast<unsigned int>(ms.count()));
		}

		template<typename Clock, typename Duration>
		bool try_lock_until(chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
			return try_lock_until(duration);
		}

		typedef mutex::native_handle_type native_handle_type;
		inline native_handle_type native_handle() { return Mutex.native_handle(); }
	};

	template <class Mutex> class lock_guard
	{
		Mutex& m;
		lock_guard(const lock_guard&); /* = delete */
		lock_guard& operator=(const lock_guard&); /* = delete */
	public:
		typedef Mutex mutex_type;

		explicit lock_guard(mutex_type& _Mutex) : m(_Mutex) { m.lock(); }
		lock_guard(mutex_type& _Mutex, adopt_lock_t) : m(_Mutex) {}
		~lock_guard() { m.unlock(); }
	};

#define assert_not_owner() \
	assert(pMutex && "Must have valid mutex"); \
	assert((!pMutex || (pMutex && !OwnsLock)) && "Deadlock would occur")

	template <class Mutex> class unique_lock
	{
		unique_lock(unique_lock const&); /* = delete */
		unique_lock& operator=(unique_lock const&); /* = delete */
	public:
		typedef Mutex mutex_type;

		unique_lock()
			: pMutex(nullptr)
			, OwnsLock(false)
		{}
		
		explicit unique_lock(mutex_type& _Mutex)
			: pMutex(&_Mutex)
			, OwnsLock(false)
		{
			lock();
			OwnsLock = true;
		}

		unique_lock(mutex_type& _Mutex, adopt_lock_t)
			: pMutex(&_Mutex)
			, OwnsLock(true)
		{}

		unique_lock(mutex_type& _Mutex, defer_lock_t)
			: pMutex(&_Mutex)
			, OwnsLock(false)
		{}

		unique_lock(mutex_type& _Mutex, try_to_lock_t)
			: pMutex(&_Mutex)
			, OwnsLock(pMutex->try_lock())
		{}

		template<typename Clock,typename Duration>
		unique_lock(mutex_type& _Mutex, oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
			: pMutex(&_Mutex)
			, OwnsLock(pMutex->try_lock_until(_AbsoluteTime))
		{}

		template<typename Rep,typename Period>
		unique_lock(mutex_type& _Mutex, oStd::chrono::duration<Rep,Period> const& _RelativeTime)
			: pMutex(&_Mutex)
			, OwnsLock(pMutex->try_lock_for(_RelativeTime))
		{}

		~unique_lock()
		{
			if (OwnsLock)
				unlock();
		}

		unique_lock(unique_lock&& _That)
			: OwnsLock(_That.OwnsLock)
			, pMutex(_That.release())
		{}

		unique_lock& operator=(unique_lock&& _That)
		{
			if (OwnsLock)
				unlock();

			OwnsLock = _That.OwnsLock;
			pMutex = _That.release();
			return *this;
		}

		void swap(unique_lock&& _That)
		{
			oStd::swap(pMutex, _That.pMutex);
			oStd::swap(OwnsLock, _That.OwnsLock);
		}

		void lock()
		{
			assert_not_owner();
			pMutex->lock();
			OwnsLock = true;
		}

		bool try_lock()
		{
			assert_not_owner();
			OwnsLock = pMutex->try_lock();
			return OwnsLock;
		}

		template<typename Rep, typename Period> bool try_lock_for(oStd::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			assert_not_owner();
			OwnsLock = pMutex->try_lock_for(_RelativeTime);
			return OwnsLock;
		}

		template<typename Clock, typename Duration> bool try_lock_until(oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			assert_not_owner();
			OwnsLock = pMutex->try_lock_until(_AbsoluteTime);
			return OwnsLock;
		}

		void unlock()
		{
			assert(OwnsLock && "Cannot unlock a non-locked mutex (or a mutex whose lock is owned by another object)");
			if (pMutex)
			{
				pMutex->unlock();
				OwnsLock = false;
			}
		}

		/*explicit*/ operator bool() const { return owns_lock(); }
		bool owns_lock() const { return OwnsLock; }
		mutex_type* mutex() const { return pMutex; }
		mutex_type* release() { mutex_type* pCopy = pMutex; OwnsLock = false; pMutex = nullptr; return pCopy; }

	private:
		mutex_type* pMutex;
		bool OwnsLock;
	};

	class once_flag
	{
		oONCEFLAG_FOOTPRINT();
		once_flag(const once_flag&);
		once_flag& operator=(const once_flag&);
	public:
		/*constexpr*/ once_flag();
	};

	void call_once(once_flag& _Flag, oCALLABLE _Function);
	#ifndef oHAS_VARIADIC_TEMPLATES
		#define oDEFINE_CALLABLE_call_once(_nArgs) oCALLABLE_CONCAT(oCALLABLE_TEMPLATE,_nArgs) void call_once(once_flag& _Flag, oCALLABLE_CONCAT(oCALLABLE_PARAMS,_nArgs)) { call_once(_Flag, oCALLABLE_CONCAT(oCALLABLE_BIND,_nArgs)); }
		oCALLABLE_PROPAGATE(oDEFINE_CALLABLE_call_once);
	#endif
} // namespace oStd

#endif
