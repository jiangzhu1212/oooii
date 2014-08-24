/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// Approximation of the upcoming C++1x std::mutex objects.

#pragma once
#ifndef oCore_mutex_h
#define oCore_mutex_h

#include <cassert>
#include <chrono>
#include <thread>
#include <oBase/callable.h>

// To keep the main classes neat, collect all the platform-specific forward
// declaration here. This is done in this vague manner to avoid including 
// platform headers in this file.
namespace ouro {

	class mutex
	{
	public:
		mutex();
		~mutex();

		void lock();
		bool try_lock();
		void unlock();

		typedef void* native_handle_type;
		native_handle_type native_handle();

	protected:
		#if defined(_WIN32) || defined(_WIN64)
			void* Footprint;
		#else
			#error unsupported platform (mutex)
		#endif
		#ifdef _DEBUG
			std::thread::id ThreadID;
		#endif
	
	private:
		mutex(const mutex&); /* = delete */
		mutex& operator=(const mutex&); /* = delete */
	};

	class shared_mutex : public mutex
	{
	public:
		shared_mutex() {}

		void lock_shared();
		bool try_lock_shared();
		void unlock_shared();

	private:
		shared_mutex(const shared_mutex&); /* = delete */
		shared_mutex& operator=(const shared_mutex&); /* = delete */
	};

	class recursive_mutex
	{
	public:
		recursive_mutex();
		~recursive_mutex();

		void lock();
		bool try_lock();
		void unlock();

		typedef void* native_handle_type;
		native_handle_type native_handle();

	private:
		#ifdef _WIN64
			mutable unsigned long long Footprint[5]; // RTL_CRITICAL_SECTION
		#elif defined(_WIN32)
			mutable unsigned int Footprint[6];
		#else
			#error unsupported platform (recursive_mutex)
		#endif

		recursive_mutex(const recursive_mutex&); /* = delete */
		recursive_mutex& operator=(const recursive_mutex&); /* = delete */
	};

	class timed_mutex : public mutex
	{
	public:
		timed_mutex() {}
		~timed_mutex() {}

		template<typename Rep, typename Period>
		bool try_lock_for(std::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(_RelativeTime);
			Rep count = ms.count();
			if (Rep(count) != count)
				throw std::range_error("_RelativeTime is too large");
			return try_lock_for(static_cast<unsigned int>(ms.count()));
		}

		template<typename Clock, typename Duration>
		bool try_lock_until(std::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			auto duration = time_point_cast<std::chrono::high_resolution_clock::time_point>(_AbsoluteTime) - 
				std::chrono::high_resolution_clock::now();
			return try_lock_until(duration);
		}

	private:
		timed_mutex(timed_mutex const&); /* = delete */
		timed_mutex& operator=(timed_mutex const&); /* = delete */
		bool try_lock_for(unsigned int _TimeoutMS);
	};

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

	template <class Mutex> class lock_guard
	{
	public:
		typedef Mutex mutex_type;

		explicit lock_guard(mutex_type& _Mutex) : m(_Mutex) { m.lock(); }
		lock_guard(mutex_type& _Mutex, adopt_lock_t) : m(_Mutex) {}
		~lock_guard() { m.unlock(); }
	private:
		Mutex& m;
		lock_guard(const lock_guard&); /* = delete */
		lock_guard& operator=(const lock_guard&); /* = delete */
	};

	template<class Mutex> class shared_lock
	{
	public:
		typedef Mutex mutex_type;
		explicit shared_lock(mutex_type& _Mutex) : m(_Mutex) { m.lock_shared(); }
		shared_lock(mutex_type& _Mutex, adopt_lock_t) : m(_Mutex) {}
		~shared_lock() { m.unlock_shared(); }
	private:
		Mutex& m;
		shared_lock(const shared_lock&); /* = delete */
		shared_lock& operator=(const shared_lock&); /* = delete */
	};

#define assert_not_owner() \
	assert(pMutex && "Must have valid mutex"); \
	assert((!pMutex || (pMutex && !OwnsLock)) && "Deadlock would occur")

	template <class Mutex> class unique_lock
	{
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
		unique_lock(mutex_type& _Mutex, std::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
			: pMutex(&_Mutex)
			, OwnsLock(pMutex->try_lock_until(_AbsoluteTime))
		{}

		template<typename Rep,typename Period>
		unique_lock(mutex_type& _Mutex, std::chrono::duration<Rep,Period> const& _RelativeTime)
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
			std::swap(pMutex, _That.pMutex);
			std::swap(OwnsLock, _That.OwnsLock);
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

		template<typename Rep, typename Period> bool try_lock_for(std::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			assert_not_owner();
			OwnsLock = pMutex->try_lock_for(_RelativeTime);
			return OwnsLock;
		}

		template<typename Clock, typename Duration> bool try_lock_until(std::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
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

		unique_lock(unique_lock const&); /* = delete */
		unique_lock& operator=(unique_lock const&); /* = delete */
	};

	class once_flag
	{
	public:
		/*constexpr*/ once_flag();

	private:
		#if defined(_WIN32) || defined(_WIN64)
			void* Footprint;
		#else
			#error unsupported platform (once_flag)
		#endif
		once_flag(const once_flag&);
		once_flag& operator=(const once_flag&);
	};

	void call_once(once_flag& _Flag, const std::function<void>& _Function);
	#ifndef oHAS_VARIADIC_TEMPLATES
		#define oDEFINE_CALLABLE_call_once(_nArgs) oCALLABLE_CONCAT(oCALLABLE_TEMPLATE,_nArgs) void call_once(once_flag& _Flag, oCALLABLE_CONCAT(oCALLABLE_PARAMS,_nArgs)) { call_once(_Flag, oCALLABLE_CONCAT(oCALLABLE_BIND,_nArgs)); }
		oCALLABLE_PROPAGATE(oDEFINE_CALLABLE_call_once);
	#endif
} // namespace ouro

#endif
