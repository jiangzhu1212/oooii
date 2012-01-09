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
// Approximation of the upcoming C++11 oStd::mutex interface. Prefer using 
// oMutex.h wrappers over this directly in client code because oMutex.h wrappers
// have threadsafe-keyword protection and thus require less thread_cast'ing.
// This header is meant to just contain the code that will go away when a C++11
// compliant libc is provided by all our supported compilers.

#pragma once
#ifndef oStdMutex_h
#define oStdMutex_h

#include <oBasis/oStdThread.h>

// To keep the main classes neat, collect all the platform-specific forward
// declaration here. This is done in this vague manner to avoid including 
// platform headers in this file.
#if defined(_WIN32) || defined(_WIN64)
	#ifdef _WIN64
		#define oRECURSIVE_MUTEX_FOOTPRINT() mutable unsigned long long Footprint[5] // RTL_CRITICAL_SECTION
	#elif defined(_WIN32)
		#define oRECURSIVE_MUTEX_FOOTPRINT() mutable unsigned int Footprint[6]
	#endif

	#define oSHARED_MUTEX_FOOTPRINT() void* Footprint

	#define oMUTEX_FOOTPRINT() oSHARED_MUTEX_FOOTPRINT()

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

	class shared_mutex
	{
		oSHARED_MUTEX_FOOTPRINT();
		#ifdef _DEBUG
			oStd::thread::id ThreadID;
		#endif
		shared_mutex(const shared_mutex&); /* = delete */
		shared_mutex& operator=(const shared_mutex&); /* = delete */
	public:
		shared_mutex();
		~shared_mutex();

		// Exclusive
		void lock();
		bool try_lock();
		void unlock();

		// Shared
		void lock_shared();
		bool try_lock_shared();
		void unlock_shared();

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

		template<typename Rep,typename Period>
		bool try_lock_for(chrono::duration<Rep,Period> const& _RelativeTime)
		{
			chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(_RelativeTime);
			return try_lock_for(ms.count());
		}

		template<typename Clock,typename Duration>
		bool try_lock_until(chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
			return try_lock_until(duration);
		}
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

	template<class Mutex> class shared_lock
	{
		Mutex& m;
		shared_lock(const shared_lock&); /* = delete */
		shared_lock& operator=(const shared_lock&); /* = delete */
	public:
		typedef Mutex mutex_type;

		explicit shared_lock(mutex_type& _Mutex) : m(_Mutex) { m.lock_shared(); }
		shared_lock(mutex_type& _Mutex, adopt_lock_t) : m(_Mutex) {}
		~shared_lock() { m.unlock_shared(); }
	};

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
			oASSERT(pMutex, "Must have valid mutex");
			oASSERT(!pMutex || (pMutex && !OwnsLock), "Deadlock would occur");
			pMutex->lock();
			OwnsLock = true;
		}

		bool try_lock()
		{
			oASSERT(pMutex, "Must have valid mutex");
			oASSERT(!pMutex || (pMutex && !OwnsLock), "Deadlock would occur");
			OwnsLock = pMutex->try_lock();
			return OwnsLock;
		}

		template<typename Rep, typename Period> bool try_lock_for(oStd::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oASSERT(pMutex, "Must have valid mutex");
			oASSERT(!pMutex || (pMutex && !OwnsLock), "Deadlock would occur");
			OwnsLock = pMutex->try_lock_for(_RelativeTime);
			return OwnsLock;
		}

		template<typename Clock, typename Duration> bool try_lock_until(oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			oASSERT(pMutex, "Must have valid mutex");
			oASSERT(!pMutex || (pMutex && !OwnsLock), "Deadlock would occur");
			OwnsLock = pMutex->try_lock_until(_AbsoluteTime);
			return OwnsLock;
		}

		void unlock()
		{
			oASSERT(OwnsLock, "Cannot unlock a non-locked mutex (or a mutex whose lock is owned by another object)");
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
		oCALLABLE_TEMPLATE0 void call_once(once_flag& _Flag, oCALLABLE_PARAMS0) { call_once(_Flag, oCALLABLE_BIND0); }
		oCALLABLE_TEMPLATE1 void call_once(once_flag& _Flag, oCALLABLE_PARAMS1) { call_once(_Flag, oCALLABLE_BIND1); }
		oCALLABLE_TEMPLATE2 void call_once(once_flag& _Flag, oCALLABLE_PARAMS2) { call_once(_Flag, oCALLABLE_BIND2); }
		oCALLABLE_TEMPLATE3 void call_once(once_flag& _Flag, oCALLABLE_PARAMS3) { call_once(_Flag, oCALLABLE_BIND3); }
		oCALLABLE_TEMPLATE4 void call_once(once_flag& _Flag, oCALLABLE_PARAMS4) { call_once(_Flag, oCALLABLE_BIND4); }
		oCALLABLE_TEMPLATE5 void call_once(once_flag& _Flag, oCALLABLE_PARAMS5) { call_once(_Flag, oCALLABLE_BIND5); }
		oCALLABLE_TEMPLATE6 void call_once(once_flag& _Flag, oCALLABLE_PARAMS6) { call_once(_Flag, oCALLABLE_BIND6); }
		oCALLABLE_TEMPLATE7 void call_once(once_flag& _Flag, oCALLABLE_PARAMS7) { call_once(_Flag, oCALLABLE_BIND7); }
		oCALLABLE_TEMPLATE8 void call_once(once_flag& _Flag, oCALLABLE_PARAMS8) { call_once(_Flag, oCALLABLE_BIND8); }
		oCALLABLE_TEMPLATE9 void call_once(once_flag& _Flag, oCALLABLE_PARAMS9) { call_once(_Flag, oCALLABLE_BIND9); }
		oCALLABLE_TEMPLATE10 void call_once(once_flag& _Flag, oCALLABLE_PARAMS10) { call_once(_Flag, oCALLABLE_BIND10); }
	#endif
} // namespace oStd

#endif
