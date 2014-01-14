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
// Approximation of the proposed C++17 shared_mutex interface.

#pragma once
#ifndef oStd_shared_mutex_h
#define oStd_shared_mutex_h

#include <mutex>
#include <thread>

namespace ouro {

	class shared_mutex
	{
		#if defined(_WIN32) || defined(_WIN64)
			void* Footprint;
		#else
			#error Unsupported platform (shared_mutex)
		#endif
		#ifdef _DEBUG
			std::thread::id ThreadID;
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

	template<class Mutex> class shared_lock
	{
		Mutex& m;
		shared_lock(const shared_lock&); /* = delete */
		shared_lock& operator=(const shared_lock&); /* = delete */
	public:
		typedef Mutex mutex_type;
		explicit shared_lock(mutex_type& _Mutex) : m(_Mutex) { m.lock_shared(); }
		shared_lock(mutex_type& _Mutex, std::adopt_lock_t) : m(_Mutex) {}
		~shared_lock() { m.unlock_shared(); }
	};

} // namespace ouro

#endif
