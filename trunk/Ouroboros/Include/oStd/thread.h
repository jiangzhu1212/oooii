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
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// Approximation of the upcoming C++11 std::thread interface. There has been no
// attention paid to proper exception behavior.
#pragma once
#ifndef oStd_thread_h
#define oStd_thread_h

#include <oStd/callable.h>
#include <oStd/chrono.h>
#include <functional>
#include <memory>

namespace oStd {

	namespace detail {

		struct thread_context
		{
			void* hThread;
			oCALLABLE Callable;
		};

	} // namespace detail

	class thread
	{
	public:
		class id;
		typedef void* native_handle_type;

		// Ctors either start the thread in an invalid (nonjoinable) state, or have
		// their thread proc specified.
		thread();
		oDEFINE_CALLABLE_CTOR_WRAPPERS(explicit, thread, initialize);

		// All threads must be join()'ed before allowing the thread to exit or, 
		// according to the C++11 standard, std::terminate will be called thus 
		// exiting the application.
		~thread();

		thread(thread&& _That) { *this = std::move(_That); }
		thread& operator=(thread&& _That);

		// Uses move operator to swap this and the specified thread
		void swap(thread& _That);

		// Block the calling thread until this thread's callable function exits
		// (basically a wait() call)
		void join();

		// Set this thread's ID to identity and no longer allow control of the 
		// thread from this interface
		void detach();

		// Returns true if join can still be called (i.e. if the thread isn't 
		// detached and hasn't yet exited)
		bool joinable() const;

		id get_id() const;
		native_handle_type native_handle();
		static unsigned int hardware_concurrency();

	protected:

		std::unique_ptr<detail::thread_context> Context;

		void initialize(const oCALLABLE& _ThreadProc);
		void move(thread& _That);

		thread(const thread&)/* = delete*/;
		const thread& operator=(const thread&)/* = delete*/;
	};

	class thread::id
	{	unsigned int ID;
	public:
		id();

		void swap(id& _That) { unsigned int tmp = ID; ID = _That.ID; _That.ID = tmp; }
		bool operator==(const id& _That) const { return ID == _That.ID; }
		bool operator!=(const id& _That) const { return ID != _That.ID; }
		bool operator<(const id& _That) const { return ID < _That.ID; }
		bool operator<=(const id& _That) const { return ID <= _That.ID; }
		bool operator>(const id& _That) const { return ID > _That.ID; }
		bool operator>=(const id& _That) const { return ID >= _That.ID; }
	};

	namespace this_thread
	{
		thread::id get_id();
		void yield();
		void __sleep_for(unsigned int _Milliseconds);

		template<typename Rep, typename Period> void sleep_for(const chrono::duration<Rep, Period>& _Duration)
		{
			chrono::milliseconds s = chrono::duration_cast<chrono::milliseconds>(_Duration);
			__sleep_for(static_cast<unsigned int>(s.count()));
		}

	} // namespace this_thread

} // namespace oStd

namespace std
{
	void swap(oStd::thread& _This, oStd::thread& _That);

} // namespace std

#endif
