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
// This is a start toward implementing std::future. At this time
// we favor oERROR over the std::error_code stuff in C++11.

// NOTE: GCC Source, VS2010, just::thread, all these standards guys are 
// implementing std::async as a right-now-alloc-a-thread-and-exec-task style
// system. That is aweful. RESOLVED: The foreseeable future is TBB. This 
// codebase still tries to abstract interfacing with the scheduler (i.e. Grand 
// Central Dispatch remains a contender on mac-y systems) but basically the idea
// is that there is one system scheduler to avoid oversubscription and tasks can 
// wait on child tasks. Allocation of a task is separate from dispatching it, 
// and all tasks delete themselves and their dependency footprint after 
// execution. This seems consistent with what is done in TBB and what can be 
// done in Windows Thread Pools or Grand Central Dispatch.

// NOTE: There are now 3 ways to call a function: no return (void) return by 
// value and return by reference, so each concept in this header: 
// detail::oCommitment, promise, future, and packaged_task each have 3
// template implementations: <T>, <void>, and <T&>. async can call a function
// that returns a value, or has no return so there's permutations for that too.

#pragma once
#ifndef oStdFuture_h
#define oStdFuture_h

#include <oBasis/oAssert.h>
#include <oBasis/oCallable.h>
#include <oBasis/oError.h>
#include <oBasis/oRef.h>
#include <oBasis/oStdAtomic.h>
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oStdMutex.h>
#include <system_error>
#include <utility>

//#define oSTD_FUTURE_USE_EXCEPTIONS
#ifdef oSTD_FUTURE_USE_EXCEPTIONS
	#include <exception>
#endif

// Also declared in oBasis/oThread.h, but needs to be defined using platform-
// specifics in oPlatform. See oThread.h for more details, here redeclare to 
// avoid the include.
void oThreadAtExit(std::function<void()> _AtExit);

namespace oStd {

/*enum class*/ namespace future_status { enum value { ready, timeout, deferred }; };
/*enum class*/ namespace future_errc { enum value { broken_promise, future_already_retrieved, promise_already_satisfied, no_state }; };

const std::error_category& future_category();

/*constexpr*/ inline std::error_code make_error_code(future_errc::value _Errc) { return std::error_code(static_cast<int>(_Errc), future_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(future_errc::value _Errc) { return std::error_condition(static_cast<int>(_Errc), future_category()); }

template<typename T> class future;
template<typename T> class promise;

namespace detail {

	interface oWaitableTask
	{
		// This is not public API, use promise, packaged_task, or async.

		// @oooii-tony: This class is not implemented in oBasis. At this time 
		// threadpool schedulers are still wild-westy. Intel's TBB dominates 
		// benchmarks and the answers from the C++11 standards world are just 
		// terrible (see rant at top of header). It may be that the launch policy as 
		// a standard is also flawed (when would deferred really make sense instead 
		// of just calling the function?) In order to use concurrency through 
		// std::future in a way that enables work-stealing, your platform must 
		// implement these API. Based on what I see in MSVC, GCC Just::thread, and 
		// others, maybe it's time to declare TBB oBasis-grade code?

		// blocks the calling thread until this task is finished. This gives the 
		// opportunity to the scheduler to take over this thread for work-stealing.
		virtual void wait() = 0;

		// intrusive refcounting api
		virtual void Reference() = 0;
		virtual void Release() = 0;
	};

	// This is not public API, use promise, packaged_task, or async.
	// @oooii-tony: This class is not implemented in oBasis. It is the factory
	// for an oWaitableTask that allows some platform-specific system to implement
	// a more robust task scheduler than std::future otherwise would allow.
	oAPI bool oWaitableTaskCreate(oTASK&& _Task, oWaitableTask** _ppWaitableTask);

	oAPI void* oCommitmentAllocate(size_t _CommitmentSize);
	oAPI void oCommitmentDeallocate(void* _pPointer);

	#define oCOMMITMENT_COPY_MOVE_CTORS() \
		oCommitment(const oCommitment&)/* = delete*/; \
		const oCommitment& operator=(const oCommitment&)/* = delete*/; \
		oCommitment(oCommitment&&)/* = delete*/; \
		oCommitment& operator=(oCommitment&&)/* = delete*/;

	#define oCOMMITMENT_NEW_DEL()
		inline void* operator new(size_t size, void* memory) { return memory; } \
		inline void operator delete(void* p, void* memory) {} \
		inline void* operator new(size_t size) { return oCommitmentAllocate(size); } \
		inline void* operator new[](size_t size) { return oCommitmentAllocate(size); } \
		inline void operator delete(void* p) { oCommitmentDeallocate(p); } \
		inline void operator delete[](void* p) { oCommitmentDeallocate(p); }

	#ifdef oSTD_FUTURE_USE_EXCEPTIONS
		oAPI void oThrowFutureError(future_errc::value _Errc);

		#define oFUTURE_ASSERT(_Expr, _Errc) do { if (!(_Expr)) detail::oThrowFutureError(future_errc::_Errc); } while(false)
		#define oFUTURE_TRY try {
		#define oFUTURE_ENDTRY } catch (...) { _Promise.set_exception(std::current_exception()); }
		#define oFUTURE_WAIT_AND_CHECK_ERROR() do \
		{ wait(); \
			if (has_error()) oErrorSetLast(Error, ErrorString); \
			if (has_exception()) std::rethrow_exception(pException); \
		} while(false)
	#else
		#define oFUTURE_ASSERT(_Expr, _Errc) oASSERT(_Expr, "%s", future_category().message(future_errc::_Errc).c_str())
		#define oFUTURE_TRY
		#define oFUTURE_ENDTRY
		#define oFUTURE_WAIT_AND_CHECK_ERROR() do \
			{ wait(); \
			if (has_error()) oErrorSetLast(Error, ErrorString); \
			} while(false)
	#endif

	class oCommitmentState
	{
		// This is not public API, use promise, packaged_task, or async.

		// Common state between a promise and a future OR a packaged_task and a 
		// future. CV/Mutex synchronize promises with futures, but packaged_task are
		// the only ones meeting an oCommitment, and use the underlying system task
		// scheduler to enable work stealing, so there's a lot of if work_steals()
		// code to either ignore the mutex and use the schedule OR use the mutex.
		
		// This base class encapsulates all the state that is not the actual value,
		// since that requires a templated type.

		oCommitmentState(const oCommitmentState&)/* = delete*/;
		const oCommitmentState& operator=(const oCommitmentState&)/* = delete*/;
		oCommitmentState(oCommitmentState&&)/* = delete*/;
		oCommitmentState& operator=(oCommitmentState&&)/* = delete*/;

	public:
		oCommitmentState() 
			: RefCount(1)
			, Error(oERROR_NONE)
		{
			*ErrorString = 0;
		}

		~oCommitmentState() {}

		void reference() { oStd::atomic_increment(&RefCount); }
		// release() must exist only in templated derivations because it must also
		// be responsible for calling the dtor of the typed value stored in the 
		// commitment.

		bool is_ready() const { return State.IsReady; }
		bool has_value() const { return State.HasValue; }
		bool has_error() const { return State.HasError; }
		bool has_exception() const { return State.HasException; }
		bool has_future() const { return State.HasFuture; }
		bool is_deferred() const { return State.IsDeferred; }
		bool work_steals() const { return !!Task; }
		oERROR get_error() const { return Error; }
		const char* get_error_string() const { return ErrorString; }

		void wait_adopt(unique_lock<mutex>& _Lock)
		{
			if (!is_ready())
			{
				oASSERT(!work_steals(), "wait_adopt is incompatible with work stealing");
				oASSERT(!is_deferred(), "deferred launch policy not currently supported");
				while (!is_ready()) // Guarded Suspension
					CV.wait(_Lock);
			}
		}

		void wait()
		{
			if (!is_ready())
			{
				if (work_steals())
					Task->wait();
				else
				{
					unique_lock<mutex> lock(Mutex);
					wait_adopt(lock);
				}
			}
		}

		template<typename Rep, typename Period>
		future_status::value wait_for(oStd::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oASSERT(!work_steals(), "wait_for cannot be called on a work-stealing future");
			unique_lock<mutex> lock(Mutex);
			if (is_deferred())
				return future_status::deferred;
			while (!is_ready()) // Guarded Suspension
				CV.wait_for(lock, _RelativeTime);
			if (is_ready())
				return future_status::ready;
			return future_status::timeout;
		}

		bool set_errorV(oERROR _Error, const char* _Format, va_list _Args)
		{
			unique_lock<mutex> lock(Mutex);
			oFUTURE_ASSERT(!is_ready(), promise_already_satisfied);			
			oVPrintf(ErrorString, _Format, _Args);
			State.HasError = true;
			State.HasErrorAtThreadExit = true;
			State.IsReady = true;
			notify(lock);
			return false;
		}

		void set_error_at_thread_exitV(oTASK&& _NotifyAndRelease, oERROR _Error, const char* _Format, va_list _Args)
		{
			unique_lock<mutex> lock(Mutex);
			oFUTURE_ASSERT(!is_ready() && !State.HasErrorAtThreadExit, promise_already_satisfied);			
			oVPrintf(ErrorString, _Format, _Args);
			State.HasErrorAtThreadExit = true;
			lock.unlock();
			reference();
			oThreadAtExit(std::move(_NotifyAndRelease));
		}

		void set_exception(std::exception_ptr _pException)
		{
			unique_lock<mutex> lock(Mutex);
			oFUTURE_ASSERT(!is_ready(), promise_already_satisfied);
			pException = _pException;
			State.HasException = true;
			State.HasExceptionAtThreadExit = true;
			State.IsReady = true;
		}

		void set_exception_at_thread_exit(std::exception_ptr _pException)
		{
			unique_lock<mutex> lock(Mutex);
			oFUTURE_ASSERT(!is_ready() && !State.HasExceptionAtThreadExit, promise_already_satisfied);
			pException = _pException;
			State.HasExceptionAtThreadExit = true;
		}

		// @oooii-tony: packaged_task needs to schedule the task, so expose this, 
		// though it is an implementation detail. (but then all of oCommitment is an 
		// implementation detail of promise/future/packaged_task)
		void commit_to_task(oTASK&& _Task)
		{
			oVERIFY(oWaitableTaskCreate(std::move(_Task), &Task));
		}

	private:
		template<typename> friend class oCommitment;
		template<typename> friend class future;
		template<typename> friend class promise;

		struct STATE
		{
			STATE() { memset(this, 0, sizeof(STATE)); }
			bool HasValue:1;
			bool HasValueAtThreadExit:1;
			bool HasException:1;
			bool HasExceptionAtThreadExit:1;
			bool HasError:1;
			bool HasErrorAtThreadExit:1;
			bool HasFuture:1;
			bool IsDeferred:1;
			bool IsReady:1;
		};

		// NOTE: When a task is associated with this commitment, it's wait/blocking
		// system is used to delegate context to the task's scheduler so work 
		// stealing can be done. This means the CV/Mutex aren't use to synchronize,
		// so if work_steals(), it's all on Task->wait(). if !work_steals(), then
		// unique locks and conditions are responsible.
		oRef<oWaitableTask> Task;
		mutable condition_variable CV;
		mutable mutex Mutex;
		STATE State;
		int RefCount;
		oERROR Error;
		char ErrorString[512];
		std::exception_ptr pException;

		void set_future_attached()
		{
			unique_lock<mutex> lock(Mutex);
			State.HasFuture = true;
		}

		void notify(unique_lock<mutex>& _Lock)
		{
			State.IsReady = true;
			_Lock.unlock();
			if (!work_steals())
				CV.notify_all();
		}

		void notify_value_set()
		{
			unique_lock<mutex> lock(Mutex);
			oFUTURE_ASSERT(!is_ready(), promise_already_satisfied);
			State.HasValue = true;
			State.HasValueAtThreadExit = true;
			State.IsReady = true;
			notify(lock);
		}

		void notify_value_set_at_thread_exit(unique_lock<mutex>& _Lock, oTASK&& _NotifyValueSetAndRelease)
		{
			if (!has_value()) // don't override a fulfilled promise
			{
				oFUTURE_ASSERT(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
				State.HasValueAtThreadExit = true;
				_Lock.unlock();
				reference();
				oThreadAtExit(std::move(_NotifyValueSetAndRelease));
			}
		}

		// for functions that return void
		void set_no_value()
		{
			if (work_steals())
				State.IsReady = true;
			else
			{
				// This doesn't require a lock since the instant a future sees this flag 
				// (i.e. in a pre-block check or even in a spurious wakeup) it can be
				// unblocked.
				//unique_lock<mutex> lock(Mutex);
				State.IsReady = true;
				//lock.unlock();
				CV.notify_all();
			}
		}

		template<typename T, typename U> void internal_set_value_ref(void* _pValueMemory, const U& _Value)
		{
			oFUTURE_ASSERT(!is_ready(), promise_already_satisfied);
			oASSERT(!State.HasValueAtThreadExit, "Should the value be destroyed before setting a new one?");
			*(T**)_pValueMemory = &const_cast<U&>(_Value);
			State.HasValue = true;
			State.IsReady = true;
		}

		template<typename T, typename U> void internal_set_value(void* _pValueMemory, const U& _Value)
		{
			oFUTURE_ASSERT(!is_ready(), promise_already_satisfied);
			oASSERT(!State.HasValueAtThreadExit, "Should the value be destroyed before setting a new one?");
			::new(_pValueMemory) T(_Value);
			State.HasValue = true;
			State.IsReady = true;
		}

		template<typename T, typename U> void internal_set_value(void* _pValueMemory, U&& _Value)
		{
			oFUTURE_ASSERT(!is_ready(), promise_already_satisfied);
			oASSERT(!State.HasValueAtThreadExit, "Should the value be destroyed before setting a new one?");
			::new(_pValueMemory) T(std::forward<U>(_Value));
			State.HasValue = true;
			State.IsReady = true;
		}
		
		template <typename T, typename U> void typed_set_value_ref(void* _pValueMemory, const U& _Value)
		{
			if (work_steals())
				internal_set_value_ref<T>(_pValueMemory, _Value);
			else
			{
				unique_lock<mutex> lock(Mutex);
				internal_set_value_ref<T>(_pValueMemory, _Value);
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void typed_set_value(void* _pValueMemory, const U& _Value)
		{
			if (work_steals())
				internal_set_value<T>(_pValueMemory, _Value);
			else
			{
				unique_lock<mutex> lock(Mutex);
				internal_set_value<T>(_pValueMemory, _Value);
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void typed_set_value(void* _pValueMemory, U&& _Value)
		{
			if (work_steals())
				internal_set_value<T>(_pValueMemory, std::move(_Value));
			else
			{
				unique_lock<mutex> lock(Mutex);
				internal_set_value<T>(_pValueMemory, std::move(_Value));
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void set_value_at_thread_exit(void* _pValueMemory, U&& _Value, oTASK&& _NotifyValueSetAndRelease)
		{
			unique_lock<mutex> lock(Mutex);
			oASSERT(!work_steals(), "packaged_task won't call this");
			oFUTURE_ASSERT(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
			::new(_pValueMemory) T(std::forward<U>(_Value));
			reference();
			notify_value_set_at_thread_exit(lock, std::move(_NotifyValueSetAndRelease));
		}

		template <typename T, typename U> void set_value_at_thread_exit_ref(void* _pValueMemory, const U& _Value, oTASK&& _NotifyValueSetAndRelease)
		{
			unique_lock<mutex> lock(Mutex);
			oASSERT(!work_steals(), "packaged_task won't call this");
			oFUTURE_ASSERT(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
			*(T**)_pValueMemory = &const_cast<U&>(_Value);
			reference();
			notify_value_set_at_thread_exit(lock, std::move(_NotifyValueSetAndRelease));
		}
	};

	template<typename T> class oCommitment : public oCommitmentState
	{
		// This is not public API, use promise, packaged_task, or async.

		// Stores both state and a typed value. This is not public API, use 
		// promise, packaged_task, or async.

		oCOMMITMENT_COPY_MOVE_CTORS();
	public:
		oCOMMITMENT_NEW_DEL();

		oCommitment() {}
		~oCommitment() {}

		void release()
		{
			int NewRef = oStd::atomic_decrement(&RefCount);
			if (NewRef == 0)
			{
				oFUTURE_ASSERT(is_ready(), broken_promise);
				if (has_value() || State.HasValueAtThreadExit)
					reinterpret_cast<T*>(&Value)->~T();
				delete this;
			}
		}

		template <typename U> void set_value(const U& _Value)
		{
			typed_set_value<T>(&Value, _Value);
		}

		template <typename U> void set_value(U&& _Value)
		{
			typed_set_value<T>(&Value, std::move(_Value));
		}

		template <typename U> void set_value_at_thread_exit(const U& _Value)
		{
			set_value_at_thread_exit<T>(&Value, Value, oBIND(&oCommitment<T>::notify_value_set_and_release, this));
		}

		template <typename U> void set_value_at_thread_exit(U&& _Value)
		{
			set_value_at_thread_exit<T>(&Value, std::move(_Value), oBIND(&oCommitment<T>::notify_value_set_and_release, this));
		}

	private:
		typedef typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type storage_type;
		storage_type Value;

		template<typename> friend class future;

		void notify_value_set_and_release()
		{
			if (!is_ready()) // if we haven't fulfilled a promise, do it now
				notify_value_set();
			release();
		}

		T move_value()
		{
			oFUTURE_WAIT_AND_CHECK_ERROR();
			return std::move(*reinterpret_cast<T*>(&Value));
		}

		bool move_value(T* _pOutValue)
		{
			oFUTURE_WAIT_AND_CHECK_ERROR();
			if (!has_error())
				*_pOutValue = std::move(*reinterpret_cast<T*>(&Value));
			return !has_error();
		}
	};

	// This is not public API, use promise, packaged_task, or async.
	template<typename T> void oCommitmentCreate(oCommitment<T>** _ppCommitment)
	{
		*_ppCommitment = new oCommitment<T>(); // @oooii-tony: Allocation from a header, how will this affect DLL usage? I guess templates are always a bad idea across DLLs...
	}

	template<typename T> class oCommitment<T&> : public oCommitmentState
	{
		// This is not public API, use promise, packaged_task, or async.

		oCOMMITMENT_COPY_MOVE_CTORS();
	public:
		oCOMMITMENT_NEW_DEL();

		oCommitment() {}
		~oCommitment() {}

		void release()
		{
			int NewRef = oStd::atomic_decrement(&RefCount);
			if (NewRef == 0)
			{
				oFUTURE_ASSERT(is_ready(), broken_promise);
				delete this;
			}
		}

		template <typename U> void set_value(const U& _Value)
		{
			typed_set_value_ref<T>(&Value, _Value);
		}

		template <typename U> void set_value_at_thread_exit(const U& _Value)
		{
			set_value_at_thread_exit_ref<T>(&Value, _Value, oBIND(&oCommitment<T&>::notify_value_set_and_release, this));
		}

	private:
		typedef T* storage_type;
		storage_type Value;

		template<typename> friend class future;

		void notify_value_set_and_release()
		{
			if (!is_ready()) // if we haven't fulfilled a promise, do it now
				notify_value_set();
			release();
		}

		typename std::add_lvalue_reference<T>::type copy_value()
		{
			oFUTURE_WAIT_AND_CHECK_ERROR();
			return *Value;
		}

		bool copy_value(typename std::add_lvalue_reference<T>::type* _pOutValue)
		{
			oFUTURE_WAIT_AND_CHECK_ERROR();
			if (!has_error())
				*_pOutValue = *Value;
			return !has_error();
		}
	};

	template<> class oCommitment<void> : public oCommitmentState
	{
		// This is not public API, use promise, packaged_task, or async.

		oCOMMITMENT_COPY_MOVE_CTORS();
	public:
		oCOMMITMENT_NEW_DEL();

		oCommitment() {}
		~oCommitment() {}

		void release()
		{
			int NewRef = oStd::atomic_decrement(&RefCount);
			if (NewRef == 0)
			{
				oFUTURE_ASSERT(is_ready(), broken_promise);
				delete this;
			}
		}
	
	private:
		template<typename> friend class future;
		template<typename> friend class promise;

		void no_value()
		{
			oFUTURE_WAIT_AND_CHECK_ERROR();
		}
	};

	// This is not public API, use promise, packaged_task, or async.
	inline void oCommitmentCreate(oCommitment<void>** _ppCommitment)
	{
		*_ppCommitment = new oCommitment<void>(); // @oooii-tony: Allocation from a header, how will this affect DLL usage? I guess templates are always a bad idea across DLLs...
	}

	template<typename BaseT> class future_state_interface
	{
	public:
		inline bool valid() const { return !!This()->Commitment; }
		inline bool is_ready() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->is_ready(); }
		inline bool has_value() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->has_value(); }
		inline bool has_error() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->has_error(); }
		inline bool has_exception() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->has_exception(); }
		inline bool is_deferred() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->is_deferred(); }
		inline bool work_steals() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->work_steals(); }
		inline oERROR get_error() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->get_error(); }
		inline const char* get_error_string() const { oFUTURE_ASSERT(valid(), no_state); return This()->Commitment->get_error_string(); }
		inline bool set_last_error() const { oFUTURE_ASSERT(valid(), no_state); return oErrorSetLast(get_error(), get_error_string()); }
		inline void wait() { oFUTURE_ASSERT(valid(), no_state); This()->Commitment->wait(); }

		template<typename Rep, typename Period>
		future_status::value wait_for(oStd::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oFUTURE_ASSERT(valid(), no_state); 
			return This()->Commitment->wait_for(_RelativeTime);
		}

		template<typename Clock, typename Duration>
		future_status::value wait_until(oStd::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			oFUTURE_ASSERT(valid(), no_state); 
			chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
			return wait_for(duration);
		}
		
	private:
		BaseT* This() { return static_cast<BaseT*>(this); }
		const BaseT* This() const { return static_cast<const BaseT*>(this); }
	};

	// @oooii-tony: The spec says there are no copy-constructors... but I really
	// need to pass this to a std::bind. There is a bug in MSVC2010 that disallows 
	// std::move through std::bind:
	// http://connect.microsoft.com/VisualStudio/feedback/details/649274/std-bind-and-std-function-are-not-move-aware 
	// So until then, use a loophole in the spec: There are no copy-ctors for future
	// or promise objects. Thus USE a copy ctor, const_cast it away and eviscerate 
	// the source.
#define oDEFINE_FUTURE_MOVE_CTOR(_Classname) \
	_Classname(const _Classname& _That) : Commitment(std::move(const_cast<_Classname&>(_That).Commitment)) {} \
	_Classname& operator=(const _Classname& _That) { if (this != &_That) { Commitment = std::move(const_cast<_Classname&>(_That).Commitment); } return *this; } \
	_Classname(_Classname&& _That) : Commitment(std::move(_That.Commitment)) {} \
	_Classname& operator=(_Classname&& _That) { if (this != &_That) { Commitment = std::move(_That.Commitment); } return *this; }

	// Certain base classes, permutations of the same concept, and special-case
	// interfaces need direct access to the commitment, or act as a factor for a 
	// future, so centralize this access here between the template types for future.
#define oFUTURE_FRIEND_INTERFACE(_CommitmentType) \
	oRef<detail::oCommitment<_CommitmentType>> Commitment; \
	template <typename> friend class detail::future_state_interface; \
	template <typename> friend class promise; \
	template <typename> friend class shared_future; \
	template <typename> friend class packaged_task; \
	future(detail::oCommitment<_CommitmentType>* _pCommitment) \
	: Commitment(_pCommitment) \
	{	oFUTURE_ASSERT(!Commitment->has_future(), future_already_retrieved); \
	Commitment->set_future_attached(); \
	} \
	detail::oCommitmentState* get_commitment_state() { return Commitment; }

#define oDEFINE_PROMISE_SET_ERROR() \
	void set_exception(std::exception_ptr _pException) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_exception(_pException); } \
	void set_exception_at_thread_exit(std::exception_ptr _pException) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_exception_at_thread_exit(_pException); } \
	bool set_errorV(oERROR _Error, const char* _Format, va_list _Args){ oFUTURE_ASSERT(Commitment, no_state); return Commitment->set_errorV(_Error, _Format, _Args); } \
	bool set_error(oERROR _Error, const char* _Format, ...) { va_list args; va_start(args, _Format); bool success = set_errorV(_Error, _Format, args); va_end(args); return success; } \
	bool take_last_error() { return set_error(oErrorGetLast(), oErrorGetLastString()); }

#define oDEFINE_FUTURE_CTORS() future() {} ~future() {} oDEFINE_FUTURE_MOVE_CTOR(future)
#define oDEFINE_PROMISE_CTORS() promise() {} ~promise() { oFUTURE_ASSERT(!Commitment || Commitment->is_ready() || !Commitment->has_future(), broken_promise); } oDEFINE_FUTURE_MOVE_CTOR(promise)

#define oDEFINE_PACKAGED_TASK_CTORS() \
	private: packaged_task(packaged_task const&)/* = delete*/; \
	private: packaged_task& operator=(packaged_task const&)/* = delete*/; \
	public: packaged_task() {} ~packaged_task() {}

} // namespace detail

template<typename T> class future : public detail::future_state_interface<future<T> >
{
	oFUTURE_FRIEND_INTERFACE(T);
public:
	oDEFINE_FUTURE_CTORS();
	// Sets last error if this fails, but then continues to move the commitment
	// value as the return value. @oooii-tony: This is not elegant as throwing an 
	// exception, but the codebase isn't quite there.
	T get() { oFUTURE_ASSERT(valid(), no_state); return Commitment->move_value(); }
	bool get(T* _pOutValue) { oFUTURE_ASSERT(valid(), no_state); return Commitment->move_value(_pOutValue); }
};

template <class T> class future<T&> : public detail::future_state_interface<future<T&> >
{
	oFUTURE_FRIEND_INTERFACE(T&);
public:
	oDEFINE_FUTURE_CTORS();
	T& get() { oFUTURE_ASSERT(valid(), no_state); return Commitment->copy_value(); }

	// Pass the address of the reference and this all works out
	bool get(T* _pOutValue) { oFUTURE_ASSERT(valid(), no_state); return Commitment->copy_value(_pOutValue); }
};

template<> class future<void> : public detail::future_state_interface<future<void> >
{
	oFUTURE_FRIEND_INTERFACE(void);
public:
	oDEFINE_FUTURE_CTORS();
	void get() { Commitment->no_value(); }
};

template<typename T> class promise
{
public:
	oDEFINE_PROMISE_CTORS();
	oDEFINE_PROMISE_SET_ERROR();

	future<T> get_future()
	{
		oFUTURE_ASSERT(!Commitment, future_already_retrieved);
		detail::oCommitmentCreate(&Commitment);
		return std::move(future<T>(Commitment));
	}

	void set_value(const T& _Value) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_value(_Value); }
	void set_value(T&& _Value) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_value(std::move(_Value)); }
	void set_value_at_thread_exit(const T& _Value) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_value_at_thread_exit(_Value); }
	void set_value_at_thread_exit(T&& _Value) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_value_at_thread_exit(std::move(_Value)); }

private:
	oRef<detail::oCommitment<T>> Commitment;
};

template <class T> class promise<T&>
{
public:
	oDEFINE_PROMISE_CTORS();
	oDEFINE_PROMISE_SET_ERROR();

	future<T&> get_future()
	{
		oFUTURE_ASSERT(!Commitment, future_already_retrieved);
		detail::oCommitmentCreate(&Commitment);
		return std::move(future<T&>(Commitment));
	}

	void set_value(const T& _Value) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_value(_Value); }
	void set_value_at_thread_exit(const T& _Value) { oFUTURE_ASSERT(Commitment, no_state); Commitment->set_value_at_thread_exit(_Value); }

private:
	oRef<detail::oCommitment<T&>> Commitment;
};

template<> class promise<void>
{
public:
	oDEFINE_PROMISE_CTORS();
	oDEFINE_PROMISE_SET_ERROR();

	future<void> get_future()
	{
		oFUTURE_ASSERT(!Commitment, future_already_retrieved);
		oCommitmentCreate(&Commitment);
		return std::move(future<void>(Commitment));
	}

	void set_value() { Commitment->set_no_value(); }

private:
	oRef<detail::oCommitment<void>> Commitment;
};

enum packaged_task_t { out_value };

template<typename T> class packaged_task
{
public:
	typedef T result_type;
	oDEFINE_PACKAGED_TASK_CTORS();

	// template<callable, args...> packaged_task(callable, args...)
	#define oDEFINE_PACKAGED_TASK_CTOR(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) \
		explicit packaged_task(oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
		{	promise<T> p; \
			Future = p.get_future(); \
			oTASK f = oBIND(packaged_task::fulfill_promise<oCONCAT(oCALLABLE_ARG_TYPENAMES_PASS,_nArgs)>, p, oCONCAT(oCALLABLE_PASS,_nArgs)); \
			Future.get_commitment_state()->commit_to_task(std::move(f)); \
		}
	oCALLABLE_PROPAGATE(oDEFINE_PACKAGED_TASK_CTOR)

	packaged_task(packaged_task&& _That)
		: Promise(std::move(_That.Promise))
	{}
	packaged_task& operator=(packaged_task&& _That) { if (this != &_That) { Promise = std::move(_That.Promise); } return *this; }

	operator bool() const { return Future.valid(); }
	future<T> get_future() { return std::move(Future); }

private:
	future<T> Future;

	#define oDEFINE_FULFILL_PROMISE(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) \
		static void fulfill_promise(promise<T> _Promise, oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
		{ oFUTURE_TRY _Promise.set_value(oCONCAT(oCALLABLE_CALL,_nArgs)); oFUTURE_ENDTRY }
	oCALLABLE_PROPAGATE(oDEFINE_FULFILL_PROMISE)
};

template<typename T> class packaged_task<T&>
{
public:
	typedef T& result_type;
	oDEFINE_PACKAGED_TASK_CTORS();

	// template<callable, args...> packaged_task(callable, args...)
	#define oDEFINE_PACKAGED_TASK_CTOR_REF(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) \
		explicit packaged_task(oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
		{	promise<T&> p; \
			Future = p.get_future(); \
			oTASK f = oBIND(packaged_task::fulfill_promise<oCONCAT(oCALLABLE_ARG_TYPENAMES_PASS,_nArgs)>, p, oCONCAT(oCALLABLE_PASS,_nArgs)); \
			Future.get_commitment_state()->commit_to_task(std::move(f)); \
		}
		oCALLABLE_PROPAGATE(oDEFINE_PACKAGED_TASK_CTOR_REF)

	packaged_task(packaged_task&& _That)
		: Promise(std::move(_That.Promise))
	{}
	packaged_task& operator=(packaged_task&& _That) { if (this != &_That) { Promise = std::move(_That.Promise); } return *this; }

	operator bool() const { return Future.valid(); }
	future<T&> get_future() { return std::move(Future); }

private:
	future<T&> Future;

	#define oDEFINE_FULFILL_PROMISE_REF(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) \
		static void fulfill_promise(promise<T&> _Promise, oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
		{ oFUTURE_TRY _Promise.set_value(oCONCAT(oCALLABLE_CALL,_nArgs)); oFUTURE_ENDTRY }
	oCALLABLE_PROPAGATE(oDEFINE_FULFILL_PROMISE_REF)
};

template<> class packaged_task<void>
{
public:
	typedef void result_type;
	oDEFINE_PACKAGED_TASK_CTORS();

	#define oDEFINE_PROMISE_VOID_CTOR(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) \
	explicit packaged_task(oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
	{	promise<void> p; \
		Future = p.get_future(); \
		oTASK f = oBIND(packaged_task::fulfill_promise<oCONCAT(oCALLABLE_ARG_TYPENAMES_PASS,_nArgs)>, p, oCONCAT(oCALLABLE_PASS,_nArgs)); \
		Future.get_commitment_state()->commit_to_task(std::move(f)); \
	}
	oCALLABLE_PROPAGATE(oDEFINE_PROMISE_VOID_CTOR)

	operator bool() const { return Future.valid(); }
	future<void> get_future() { return std::move(Future); }

private:
	future<void> Future;

	#define oDEFINE_FULFILL_PROMISE_VOID(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) \
		static void fulfill_promise(promise<void> _Promise, oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
		{ oFUTURE_TRY oCONCAT(oCALLABLE_CALL,_nArgs); _Promise.set_value(); oFUTURE_ENDTRY }
	oCALLABLE_PROPAGATE(oDEFINE_FULFILL_PROMISE_VOID)
};

// @oooii-tony: I don't quite understand the launch policies yet because 
// current implementations are very poor, so punt on that interface for now and
// always do the right thing, which is to hand the task over to the scheduler.

// future<result_of(callable)> async(callable, args...);
#ifdef oHAS_VARIADIC_TEMPLATES
	#error reimplement async using variadic templates
#else
	#define oASYNC(_nArgs) oCONCAT(oCALLABLE_TEMPLATE,_nArgs) future<oCONCAT(oCALLABLE_RETURN_TYPE,_nArgs)> async(oCONCAT(oCALLABLE_PARAMS,_nArgs)) \
	{	packaged_task<oCONCAT(oCALLABLE_RETURN_TYPE,_nArgs)> p(oCONCAT(oCALLABLE_PASS,_nArgs)); \
		return std::move(p.get_future()); \
	}
	oCALLABLE_PROPAGATE(oASYNC)
#endif

} // namespace oStd

inline void intrusive_ptr_add_ref(oStd::detail::oWaitableTask* _pWaitableTask) { _pWaitableTask->Reference(); }
inline void intrusive_ptr_release(oStd::detail::oWaitableTask* _pWaitableTask) { _pWaitableTask->Release(); }

template<typename T> void intrusive_ptr_add_ref(oStd::detail::oCommitment<T>* _pCommitment) { _pCommitment->reference(); }
template<typename T> void intrusive_ptr_release(oStd::detail::oCommitment<T>* _pCommitment) { _pCommitment->release(); }

#endif
