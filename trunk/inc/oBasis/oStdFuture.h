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
// This is a start toward implementing std::future. At this time
// we favor oERROR over the std::error_code stuff in C++11.

// NOTE: This is kinda lame... VS2010's std::bind doesn't support
// std::move ... so move ctor's can't be bound to function calls
// that take parameters by value. This means that the promise/
// future differentiation isn't very meaningful because promises
// have to be passed by reference/pointer. See oWinBaseWindow for 
// some examples of usage.

#pragma once
#ifndef oStdFuture_h
#define oStdFuture_h

#include <oBasis/oAssert.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oStdMutex.h>
#include <system_error>
#include <utility>

//#define oSTD_FUTURE_USE_EXCEPTIONS

#ifdef oSTD_FUTURE_USE_EXCEPTIONS
	#include <exception>
#endif

namespace oStd {

/*enum class*/ namespace future_status { enum value { ready, timeout, deferred }; };
/*enum class*/ namespace future_errc { enum value { broken_promise, future_alreadyTetrieved,  promise_already_satisfied, no_state }; };

#ifdef oSTD_FUTURE_USE_EXCEPTIONS
	#define check_promise() do { if (has_value()) throw future_error(make_error_code(future_errc::promise_already_satisfied)); } while (false)
	#define check_state() do { if (!Promised) throw future_error(make_error_code(future_errc::no_state)); } while (false)
#else
	#define check_promise() do \
	{ if (has_value()) \
		{	oASSERT(false, "promise_already_satisfied"); \
			set_error(oERROR_GENERIC, "promise_already_satisfied"); \
			return; \
		} \
	} while (false)

	#define check_state() do \
	{	if (!Promised) \
		{	oASSERT(false, "no_state"); \
			set_error(oERROR_GENERIC, "no_state"); \
		} \
	} while (false)

#endif

#ifdef oHAS_MOVE_CTOR
	#define oDEFINE_STD_FUTURE_MOVE_CTORS() \
		future(future&& _That) : Promised(_That.Promised) { _That.Promised = nullptr; } \
		future& operator=(future&& _That) { future(std::move(_That)).swap(*this); return *this; }
#else
	#define oDEFINE_STD_FUTURE_MOVE_CTORS()
#endif

#define oDEFINE_STD_FUTURE_COMMON_INTERFACE() \
	oDEFINE_STD_FUTURE_MOVE_CTORS(); \
	void swap(future& _That) { std::swap(Promised, _That.Promised); } \
	bool valid() const { return !!Promised; } \
	bool is_ready() const { return Promised->is_ready(); } \
	bool has_value() const { return Promised->has_value(); } \
	void wait() { Promised->wait(); } \
	bool has_exception() const; \
	bool has_error() const { return Promised->has_error(); } \
	oERROR get_error() const { return Promised->Error; } \
	const char* get_error_string() const { return Promised->ErrorString; } \
	inline bool set_last_error() const { return oErrorSetLast(get_error(), get_error_string()); } \
	template<typename Rep, typename Period> \
	future_status::value wait_for(oStd::chrono::duration<Rep,Period> const& TelativeTime) \
	{	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(TelativeTime); \
		return Promise->wait_for(static_cast<unsigned int>(ms.count())); \
	} \
	template<typename Clock, typename Duration> \
	future_status::value wait_until(oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime) \
	{	chrono::highTesolution_clock::duration duration = time_point_cast<chrono::highTesolution_clock::time_point>(_AbsoluteTime) - chrono::highTesolution_clock::now(); \
		return Promise->wait_for(duration); \
	}

namespace detail {

	class promised_base
	{
	public:
	//protected:
	//	template <typename> friend class promise;
	//	template <typename> friend class future;
	//	template <typename> friend class shared_future;

		enum state_flags
		{
			future_attached = 1,
			initialized = 2,
			value_set = 4,
			error_set = 8,
			deferred = 16,
		};

		promised_base()
			: State(0)
			, Error(oERROR_NONE)
		{}

		mutable mutex Mutex;
		mutable condition_variable CV;
		int State;
		oRefCount RefCount;
		
		// std::exception_ptr Exception
		oERROR Error;
		oStringL ErrorString;

		bool is_ready() const { return has_value() || has_error(); }
		bool has_value() const { return !!(State & value_set); }
		bool has_error() const { return !!(State & error_set); }
		bool has_future() const { return !!(State & future_attached); }
		bool is_deferred() const { return !!(State & deferred); }

		void wait_adopt(unique_lock<mutex>& _Lock)
		{
			if (!is_ready())
			{
				if (is_deferred())
				{
					State &=~ deferred;
					_Lock.unlock();
					oASSERT(false, "What now?");//execute_deferred_function();
				}

				else
				{
					while (!is_ready()) // Guarded Suspension
						CV.wait(_Lock);
				}
			}
		}

		void wait()
		{
			unique_lock<mutex> lock(Mutex);
			wait_adopt(lock);
		}
	
		future_status::value wait_for(unsigned int _TimeoutMS)
		{
			unique_lock<mutex> lock(Mutex);
			if (is_deferred())
				return future_status::deferred;
			while (!is_ready()) // Guarded Suspension
				CV.wait_for(lock, _TimeoutMS);
			if (is_ready())
				return future_status::ready;
			return future_status::timeout;
		}

		void set_value()
		{
			unique_lock<mutex> lock(Mutex);
			check_promise();
			State |= initialized | value_set;
			lock.unlock();
			CV.notify_all();
		}

		void set_value_at_thread_exit()
		{
			unique_lock<mutex> lock(Mutex);
			check_promise();
			State |= initialized;
			oThreadAtExit(oBIND(&promised_base::set_value, this));
		}

		void set_errorV(oERROR _Error, const char* _Format, va_list _Args)
		{
			vsprintf_s(ErrorString, _Format, _Args);
			State |= error_set;
			CV.notify_all();
		}

		inline void set_error(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); set_errorV(_Error, _Format, args); va_end(args); }
		inline void take_last_error() { set_error(oErrorGetLast(), oErrorGetLastString()); }
	};

	template<typename T> class promised : public promised_base
	{
		typedef typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type storage_type;
		storage_type Value;
	
	public:

		#ifdef oHAS_MOVE_CTOR
			template<typename U> void set_value(U&& _Value);
			template<typename U> void set_value_at_thread_exit(U&& _Value);
		#else
			template<typename U> void set_value(U& _Value);
			template<typename U> void set_value_at_thread_exit(U& _Value);
		#endif

		T move()
		{
			unique_lock<mutex> lock(Mutex);
			wait_adopt(lock);
			if (has_error())
				oErrorSetLast(Error, ErrorString);
			return std::move(*reinterpret_cast<T*>(&Value));
		}

		typename std::add_lvalue_reference<T>::type copy()
		{
			unique_lock<mutex> lock(Mutex);
			wait_adopt(lock);
			if (has_error())
				oErrorSetLast(Error, ErrorString);
			return Value;
		}

		void release()
		{
			if (RefCount.Release())
			{
				if (has_value())
					reinterpret_cast<T*>(&Value)->~T();
				delete this;
			}
		}
	};

	template <class T, class _Alloc>
	class promised_alloc : public promised<T>
	{
		_Alloc Allocator;
	public:
			explicit promised_alloc(const _Alloc& _Allocator) : __alloc_(_Allocator) {}
	};

	template <typename T> class promised<T&> : public promised_base
	{
		typedef T* storage_type;
		storage_type Value;
	
	public:
		void set_value(T& _Value);
		void set_value_at_thread_exit(T& _Value);

		void release()
		{
			if (RefCount.Release())
				delete this;
		}
	};

	template <typename T> template <typename U>
	#ifdef oHAS_MOVE_CTOR
		void promised<T>::set_value(U&& _Value)
	#else
		void promised<T>::set_value(U& _Value)
	#endif
	{
		unique_lock<mutex> lock(Mutex);
		check_promise();
		::new(&Value) T(std::forward<U>(_Value));
		State |= value_set;
		lock.unlock();
		CV.notify_all();
	}

	template <class T> template <class U>
	#ifdef oHAS_MOVE_CTOR
		void promised<T>::set_value_at_thread_exit(U&& _Value)
	#else
		void promised<T>::set_value_at_thread_exit(U& _Value)
	#endif
	{
		unique_lock<mutex> lock(Mutex);
		check_promise();
		::new(&Value) T(std::forward<U>(_Value));
		State |= detail::promised_base::initialized;
		oThreadAtExit(oBIND(&promised_base::set_value, this));
		lock.unlock();
	}

	template<typename T>
	class release_promise_on_exit
	{
		promised<T>* Promised;
	public:
			release_promise_on_exit(promised<T>* _Promised)
				: Promised(_Promised)
			{}

			~release_promise_on_exit() { Promised->release(); }
	};

	template<>
	class release_promise_on_exit<void>
	{
		promised_base* Promised;
	public:
		release_promise_on_exit(promised_base* _Promised)
			: Promised(_Promised)
		{}

		~release_promise_on_exit() { if (Promised->RefCount.Release()) delete Promised; }
	};

} // namespace detail

template<typename T> class future
{
	detail::promised<T>* Promised;

	future(const future&)/* = delete*/;
	future& operator=(const future&)/* = delete*/;

	template <class> friend class promise;
	template <class> friend class shared_future;
	
	future(detail::promised<T>* _Promised)
		: Promised(_Promised)
	{
		if (Promised->has_future())
			oASSERT(false, "throw future_error(make_error_code(future_errc::future_alreadyTetrieved));");
		Promised->RefCount.Reference();
		Promised->State |= detail::promised_base::future_attached;
	}

public:
	future() : Promised(nullptr) {}
	~future() { if (Promised) Promised->release(); }

	//shared_future<T> share();

	// Sets last error if this fails, but then continues to move the Promised value
	T get()
	{
		detail::release_promise_on_exit<T> hold(Promised);
		detail::promised<T>* p = Promised;
		Promised = nullptr;
		return p->move();
	}

	oDEFINE_STD_FUTURE_COMMON_INTERFACE();
};

template <class T> class future<T&>
{
	detail::promised<T&>* Promised;

	future(const future&)/* = delete*/;
	future& operator=(const future&)/* = delete*/;

	template <class> friend class promise;
	template <class> friend class shared_future;
	
	future(detail::promised<T&>* _Promised)
		: Promised(_Promised)
	{
		if (Promised->has_future())
			oASSERT(false, "throw future_error(make_error_code(future_errc::future_alreadyTetrieved));");
		Promised->RefCount.Reference();
		Promised->State |= detail::promised_base::future_attached;
	}

public:
	future() : Promised(nullptr) {}
	~future() { if (Promised) Promised->release(); }
	
	//shared_future<T&> share();

	// Sets last error if this fails, but then continues to move the Promised value
	T& get()
	{
		detail::release_promise_on_exit hold(Promised);
		detail::promised<T&>* p = Promised;
		Promised = nullptr;
		return p->copy();
	}

	oDEFINE_STD_FUTURE_COMMON_INTERFACE();
};

template <> class future<void>
{
	detail::promised_base* Promised;

	future(const future&)/* = delete*/;
	future& operator=(const future&)/* = delete*/;

	template <class> friend class promise;
	template <class> friend class shared_future;
	
	future(detail::promised_base* _Promised)
		: Promised(_Promised)
	{
		if (Promised->has_future())
			oASSERT(false, "throw future_error(make_error_code(future_errc::future_alreadyTetrieved));");
		Promised->RefCount.Reference();
		Promised->State |= detail::promised_base::future_attached;
	}

public:
	future() : Promised(nullptr) {}
	~future() { if (Promised->RefCount.Release()) delete this; }

	//shared_future<void> share();

	void get()
	{
		detail::release_promise_on_exit<void> hold(Promised);
		//detail::promised_base* p = Promised;
		//Promised = nullptr;
		//p->copy();
	}
	
	oDEFINE_STD_FUTURE_COMMON_INTERFACE();
};

template<typename T> class promise
{
	detail::promised<T>* Promised;

	promise(promise const&)/* = delete*/;
	promise& operator=(promise const&)/* = delete*/;

public:
	promise() : Promised(new detail::promised<T>()) {}
	//template <class _Alloc> promise(allocator_arg_t, const _Alloc& _Allocator);

	#ifdef oHAS_MOVE_CTOR
		promise(promise&& _That) : Promised(_That.Promised) { _That.Promised = nullptr; }
		promise& operator=(promise&& _That) { promise(std::move(_That)).swap(*this); *this; }
	#endif

	~promise()
	{
		if (Promised)
		{
			if (!Promised->has_value() && Promised->RefCount > 1)
			{
				//Promised->set_exception(make_exception_ptr(future_error(make_error_code(future_errc::broken_promise))));
				set_error(oERROR_GENERIC, "future_errc::broken_promise");
			}

			Promised->release();
		}
	}

	future<T> get_future()
	{
		check_state();
		return future<T>(Promised);
	}

	void swap(promise& _That) { std::swap(Promised, _That.Promised); }

	void set_value(const T& _Value)
	{
		check_state();
		Promised->set_value(_Value);
	}

	void set_value_at_thread_exit(const T& _Value)
	{
		check_state();
		Promised->set_value_at_thread_exit(_Value);
	}

	#ifdef oHAS_MOVE_CTOR
		void set_value(T&& _Value)
		{
			check_state();
			Promised->set_value(std::move(_Value));
		}

		void set_value_at_thread_exit(T&& _Value)
		{
			check_state();
			Promised->set_value_at_thread_exit(std::move(_Value));
		}
	#endif
	//void set_exception(exception_ptr _ExceptionPtr);
	//void set_exception_at_thread_exit(exception_ptr _ExceptionPtr);

	void set_errorV(oERROR _Error, const char* _Format, va_list _Args)
	{
		check_state();
		Promised->set_errorV(_Error, _Format, _Args);
	}

	inline void set_error(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); set_errorV(_Error, _Format, args); va_end(args); }
	inline void take_last_error() { set_error(oErrorGetLast(), oErrorGetLastString()); }
};

template <class T> class promise<T&>
{
	detail::promised<T&>* Promised;

	promise(promise const&)/* = delete*/;
	promise& operator=(promise const&)/* = delete*/;

public:
	promise() : Promised(new detail::promised<T&>()) {}
	//template <class _Allocator> promise(allocator_arg_t, const _Allocator& _Allocator);

#ifdef oHAS_MOVE_CTOR
	promise(promise&& _That) : Promised(_That.Promised) { _That.Promised = nullptr; }
	promise& operator=(promise&& _That) { promise(std::move(_That)).swap(*this); *this; }
#endif

	~promise()
	{
		if (Promised)
		{
			if (!Promised->has_value() && Promised->RefCount > 1)
			{
				//Promised->set_exception(make_exception_ptr(future_error(make_error_code(future_errc::broken_promise))));
				set_error(oERROR_GENERIC, "future_errc::broken_promise");
			}

			Promised->release();
		}
	}

	void swap(promise& _That) { std::swap(Promised, _That.Promised); }

	future<T&> get_future()
	{
		check_state();
		return future<T&>(Promised);
	}

	void set_value(T& _Value)
	{
		check_state();
		Promised->set_value(_Value);
	}

	void set_value_at_thread_exit(T& _Value)
	{
		check_state();
		Promised->set_value_at_thread_exit(_Value);
	}
	
	//void set_exception(exception_ptr _ExceptionPtr);
	//void set_exception_at_thread_exit(exception_ptr _ExceptionPtr);
	void set_errorV(oERROR _Error, const char* _Format, va_list _Args)
	{
		check_state();
		Promised->set_errorV(_Error, _Format, _Args);
	}

	inline void set_error(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); set_errorV(_Error, _Format, args); va_end(args); }
	inline void take_last_error() { set_error(oErrorGetLast(), oErrorGetLastString()); }
};

template <> class promise<void>
{
	detail::promised_base* Promised;

public:
	promise() : Promised(new detail::promised_base()) {}
	//template <class _Allocator> promise(allocator_arg_t, const _Allocator& _Allocator);

	#ifdef oHAS_MOVE_CTOR
		promise(promise&& _That) : Promised(_That.Promised) { _That.Promised = nullptr; }
		promise& operator=(promise&& _That) { promise(std::move(_That)).swap(*this); *this; }
	#endif

	~promise()
	{
		if (Promised)
		{
			if (!Promised->has_value() && Promised->RefCount > 1)
			{
				//Promised->set_exception(make_exception_ptr(future_error(make_error_code(future_errc::broken_promise))));
				set_error(oERROR_GENERIC, "future_errc::broken_promise");
			}

			if (Promised->RefCount.Release())
				delete Promised;
		}
	}

	void swap(promise& _That) { std::swap(Promised, _That.Promised); }

	// retrieving the result
	future<void> get_future()
	{
		check_state();
		return future<void>(Promised);
	}

	void set_value()
	{
		check_state();
		Promised->set_value();
	}

	void set_value_at_thread_exit()
	{
		check_state();
		Promised->set_value_at_thread_exit();
	}

	//void set_exception(exception_ptr _ExceptionPtr);
	//void set_exception_at_thread_exit(exception_ptr _ExceptionPtr);
	void set_errorV(oERROR _Error, const char* _Format, va_list _Args)
	{
		check_state();
		Promised->set_errorV(_Error, _Format, _Args);
	}

	inline void set_error(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); set_errorV(_Error, _Format, args); va_end(args); }
	inline void take_last_error() { set_error(oErrorGetLast(), oErrorGetLastString()); }
};

} // namespace oStd

namespace std {
	template<typename T> void swap(oStd::future<T>& _This, oStd::future<T>& _That) { _This.swap(_That); }
	template<typename T> void swap(oStd::promise<T>& _This, oStd::promise<T>& _That) { _This.swap(_That); }

	//std::vector<oStd::future<> >

} // namespace std

#endif
