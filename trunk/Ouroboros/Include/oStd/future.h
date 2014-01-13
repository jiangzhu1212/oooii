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
// This is a start toward implementing std::future.

// NOTE: GCC Source, VS2010, just::thread, all these standards guys are 
// implementing std::async as a right-now-alloc-a-thread-and-exec-task style
// system. That is awful. RESOLVED: The foreseeable future is TBB. This 
// codebase still tries to abstract interfacing with the scheduler (i.e. Grand 
// Central Dispatch remains a contender on mac-y systems) but basically the idea
// is that there is one system scheduler to avoid oversubscription and tasks can 
// wait on child tasks. Allocation of a task is separate from dispatching it, 
// and all tasks delete themselves and their dependency footprint after 
// execution. This seems consistent with what is done in TBB and what can be 
// done in Windows Thread Pools or Grand Central Dispatch.

// NOTE: There are now 3 ways to call a function: no return (void) return by 
// value and return by reference, so each concept in this header: 
// future_detail::oCommitment, promise, future, and packaged_task each have 3
// template implementations: <T>, <void>, and <T&>. async can call a function
// that returns a value, or has no return so there's permutations for that too.

#pragma once
#ifndef oStd_future_h
#define oStd_future_h

#ifndef oHAS_WORKSTEALING_FUTURE

#include <oStd/callable.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <system_error>
#include <utility>

namespace ouro {
	namespace future_requirements {
		// Platform requirements. The following interfaces are required to properly 
		// support future (I notice that gcc punts on the on-thread-exit APIs
		// because there's no well-defined API exposed in C++ for implementing such API,
		// which is why it's here as a platform dependency).
		void thread_at_exit(const std::function<void()>& _AtExit);

		// To better accommodate middlewares and platform-specific implementations
		// like PPL and TBB (TBB's headers are polluted with platform headers), 
		// declare an API for use in future, but not its implementation. There 
		// must be a platform implementation of the factory for waitable_task for
		// ouro::future to be usable. For most systems, a task_group is the 
		// appropriate implementation to ensure proper work-stealing with the 
		// overall scheduler is enabled. This is not public API; just an 
		// implementation detail of future.
		struct waitable_task
		{
			// Waits for the task to finish
			virtual void wait() = 0;
		};

		// This is not public API, use promise, packaged_task, or async.
		std::shared_ptr<waitable_task> make_waitable_task(const std::function<void()>& _Task);

	} // namespace future_requirements

/*enum class*/ namespace future_status { enum value { ready, timeout, deferred }; };
/*enum class*/ namespace future_errc { enum value { broken_promise, future_already_retrieved, promise_already_satisfied, no_state }; };

const std::error_category& future_category();

/*constexpr*/ inline std::error_code make_error_code(future_errc::value _Errc) { return std::error_code(static_cast<int>(_Errc), future_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(future_errc::value _Errc) { return std::error_condition(static_cast<int>(_Errc), future_category()); }

template<typename T> class future;
template<typename T> class promise;

namespace future_detail {

	void* oCommitmentAllocate(size_t _CommitmentSize);
	void oCommitmentDeallocate(void* _pPointer);

	class future_error : public std::logic_error
	{
	public:
		future_error(future_errc::value _Errc) : logic_error(future_category().message(_Errc)) {}
	};

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

	#define oFUTURE_ASSERT(_Expr, _Text) assert(_Expr && _Text)
	#define oFUTURE_THROW(_Expr, _Errc) do { if (!(_Expr)) throw future_detail::future_error(future_errc::_Errc); } while(false)
	#define oFUTURE_TRY try {
	#define oFUTURE_ENDTRY } catch (...) { _Promise.set_exception(std::current_exception()); }
	#define oFUTURE_WAIT_AND_CHECK_ERROR() do \
	{ wait(); \
		if (has_exception()) std::rethrow_exception(pException); \
	} while(false)

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
		{}
		~oCommitmentState() {}

		void reference() { ++RefCount; }
		// release() must exist only in templated derivations because it must also
		// be responsible for calling the dtor of the typed value stored in the 
		// commitment.

		bool is_ready() const { return State.IsReady; }
		bool has_value() const { return State.HasValue; }
		bool has_exception() const { return State.HasException; }
		bool has_future() const { return State.HasFuture; }
		bool is_deferred() const { return State.IsDeferred; }
		bool work_steals() const { return !!Task; }

		void wait_adopt(std::unique_lock<std::mutex>& _Lock)
		{
			if (!is_ready())
			{
				oFUTURE_ASSERT(!work_steals(), "wait_adopt is incompatible with work stealing");
				oFUTURE_ASSERT(!is_deferred(), "deferred launch policy not currently supported");
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
					std::unique_lock<std::mutex> lock(Mutex);
					wait_adopt(lock);
				}
			}
		}

		template<typename Rep, typename Period>
		future_status::value wait_for(std::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oFUTURE_ASSERT(!work_steals(), "wait_for cannot be called on a work-stealing future");
			std::unique_lock<std::mutex> lock(Mutex);
			if (is_deferred())
				return future_status::deferred;
			while (!is_ready()) // Guarded Suspension
				CV.wait_for(lock, _RelativeTime);
			if (is_ready())
				return future_status::ready;
			return future_status::timeout;
		}

		void set_exception(std::exception_ptr _pException)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_THROW(!is_ready(), promise_already_satisfied);
			pException = _pException;
			State.HasException = true;
			State.HasExceptionAtThreadExit = true;
			State.IsReady = true;
		}

		void set_exception_at_thread_exit(std::exception_ptr _pException)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_THROW(!is_ready() && !State.HasExceptionAtThreadExit, promise_already_satisfied);
			pException = _pException;
			State.HasExceptionAtThreadExit = true;
		}

		// @tony: packaged_task needs to schedule the task, so expose this, 
		// though it is an implementation detail. (but then all of oCommitment is an 
		// implementation detail of promise/future/packaged_task)
		void commit_to_task(const std::function<void()>& _Task)
		{
			Task = future_requirements::make_waitable_task(_Task);
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
			bool HasFuture:1;
			bool IsDeferred:1;
			bool IsReady:1;
		};

		// NOTE: When a task is associated with this commitment, it's wait/blocking
		// system is used to delegate context to the task's scheduler so work 
		// stealing can be done. This means the CV/Mutex aren't use to synchronize,
		// so if work_steals(), it's all on Task->wait(). if !work_steals(), then
		// unique locks and conditions are responsible.
		std::shared_ptr<future_requirements::waitable_task> Task;
		mutable std::condition_variable CV;
		mutable std::mutex Mutex;
		STATE State;
		std::atomic<int> RefCount;
		std::exception_ptr pException;

		void set_future_attached()
		{
			std::unique_lock<std::mutex> lock(Mutex);
			State.HasFuture = true;
		}

		void notify(std::unique_lock<std::mutex>& _Lock)
		{
			State.IsReady = true;
			_Lock.unlock();
			if (!work_steals())
				CV.notify_all();
		}

		void notify_value_set()
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_THROW(!is_ready(), promise_already_satisfied);
			State.HasValue = true;
			State.HasValueAtThreadExit = true;
			State.IsReady = true;
			notify(lock);
		}

		void notify_value_set_at_thread_exit(std::unique_lock<std::mutex>& _Lock, std::function<void()>& _NotifyValueSetAndRelease)
		{
			if (!has_value()) // don't override a fulfilled promise
			{
				oFUTURE_THROW(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
				State.HasValueAtThreadExit = true;
				_Lock.unlock();
				reference();
				future_requirements::thread_at_exit(_NotifyValueSetAndRelease);
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
				//std::unique_lock<std::mutex> lock(Mutex);
				State.IsReady = true;
				//lock.unlock();
				CV.notify_all();
			}
		}

		template<typename T, typename U> void internal_set_value_ref(void* _pValueMemory, const U& _Value)
		{
			oFUTURE_THROW(!is_ready(), promise_already_satisfied);
			oFUTURE_ASSERT(!State.HasValueAtThreadExit, "Should the value be destroyed before setting a new one?");
			*(T**)_pValueMemory = &const_cast<U&>(_Value);
			State.HasValue = true;
			State.IsReady = true;
		}

		template<typename T, typename U> void internal_set_value(void* _pValueMemory, const U& _Value)
		{
			oFUTURE_THROW(!is_ready(), promise_already_satisfied);
			oFUTURE_ASSERT(!State.HasValueAtThreadExit, "Should the value be destroyed before setting a new one?");
			::new(_pValueMemory) T(_Value);
			State.HasValue = true;
			State.IsReady = true;
		}

		template<typename T, typename U> void internal_set_value(void* _pValueMemory, U&& _Value)
		{
			oFUTURE_THROW(!is_ready(), promise_already_satisfied);
			oFUTURE_ASSERT(!State.HasValueAtThreadExit, "Should the value be destroyed before setting a new one?");
			::new(_pValueMemory) T(std::forward<U>(_Value));
			State.HasValue = true;
			State.IsReady = true;
		}
		
		template <typename T, typename U> void typed_set_value_ref(void* _pValueMemory, const U& _Value)
		{
			if (work_steals())
				internal_set_value_intrusive_ptr<T>(_pValueMemory, _Value);
			else
			{
				std::unique_lock<std::mutex> lock(Mutex);
				internal_set_value_intrusive_ptr<T>(_pValueMemory, _Value);
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
				std::unique_lock<std::mutex> lock(Mutex);
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
				std::unique_lock<std::mutex> lock(Mutex);
				internal_set_value<T>(_pValueMemory, std::move(_Value));
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void set_value_at_thread_exit(void* _pValueMemory, U&& _Value, std::function<void()>&& _NotifyValueSetAndRelease)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_ASSERT(!work_steals(), "packaged_task won't call this");
			oFUTURE_THROW(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
			::new(_pValueMemory) T(std::forward<U>(_Value));
			reference();
			notify_value_set_at_thread_exit(lock, std::move(_NotifyValueSetAndRelease));
		}

		template <typename T, typename U> void set_value_at_thread_exit_ref(void* _pValueMemory, const U& _Value, std::function<void()>&& _NotifyValueSetAndRelease)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_ASSERT(!work_steals(), "packaged_task won't call this");
			oFUTURE_THROW(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
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
			if (--RefCount == 0)
			{
				oFUTURE_THROW(is_ready(), broken_promise);
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
			set_value_at_thread_exit<T>(&Value, Value, std::bind(&oCommitment<T>::notify_value_set_and_release, this));
		}

		template <typename U> void set_value_at_thread_exit(U&& _Value)
		{
			set_value_at_thread_exit<T>(&Value, std::move(_Value), std::bind(&oCommitment<T>::notify_value_set_and_release, this));
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
			if (--RefCount == 0)
			{
				oFUTURE_THROW(is_ready(), broken_promise);
				delete this;
			}
		}

		template <typename U> void set_value(const U& _Value)
		{
			typed_set_value_intrusive_ptr<T>(&Value, _Value);
		}

		template <typename U> void set_value_at_thread_exit(const U& _Value)
		{
			set_value_at_thread_exit_intrusive_ptr<T>(&Value, _Value, std::bind(&oCommitment<T&>::notify_value_set_and_release, this));
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
			if (--RefCount == 0)
			{
				oFUTURE_THROW(is_ready(), broken_promise);
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

	template<typename BaseT> class future_state_interface
	{
	public:
		inline bool valid() const { return !!This()->Commitment; }
		inline bool is_ready() const { oFUTURE_THROW(valid(), no_state); return This()->Commitment->is_ready(); }
		inline bool has_value() const { oFUTURE_THROW(valid(), no_state); return This()->Commitment->has_value(); }
		inline bool has_exception() const { oFUTURE_THROW(valid(), no_state); return This()->Commitment->has_exception(); }
		inline bool is_deferred() const { oFUTURE_THROW(valid(), no_state); return This()->Commitment->is_deferred(); }
		inline bool work_steals() const { oFUTURE_THROW(valid(), no_state); return This()->Commitment->work_steals(); }
		inline void wait() { oFUTURE_THROW(valid(), no_state); This()->Commitment->wait(); }

		template<typename Rep, typename Period>
		future_status::value wait_for(std::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oFUTURE_THROW(valid(), no_state); 
			return This()->Commitment->wait_for(_RelativeTime);
		}

		template<typename Clock, typename Duration>
		future_status::value wait_until(std::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			oFUTURE_THROW(valid(), no_state); 
			chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
			return wait_for(duration);
		}
		
	private:
		BaseT* This() { return static_cast<BaseT*>(this); }
		const BaseT* This() const { return static_cast<const BaseT*>(this); }
	};

	// @tony: The spec says there are no copy-constructors... but I really
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
	std::shared_ptr<future_detail::oCommitment<_CommitmentType>> Commitment; \
	template <typename> friend class future_detail::future_state_interface; \
	template <typename> friend class promise; \
	template <typename> friend class shared_future; \
	template <typename> friend class packaged_task; \
	future(const std::shared_ptr<future_detail::oCommitment<_CommitmentType>>& _pCommitment) \
	: Commitment(_pCommitment) \
	{	oFUTURE_THROW(!Commitment->has_future(), future_already_retrieved); \
	Commitment->set_future_attached(); \
	} \
	future_detail::oCommitmentState* get_commitment_state() { return Commitment.get(); }

#define oDEFINE_PROMISE_SET_ERROR() \
	void set_exception(std::exception_ptr _pException) { oFUTURE_THROW(Commitment, no_state); Commitment->set_exception(_pException); } \
	void set_exception_at_thread_exit(std::exception_ptr _pException) { oFUTURE_THROW(Commitment, no_state); Commitment->set_exception_at_thread_exit(_pException); } \

#define oDEFINE_FUTURE_CTORS() future() {} ~future() {} oDEFINE_FUTURE_MOVE_CTOR(future)
#define oDEFINE_PROMISE_CTORS() ~promise() { oFUTURE_THROW(!Commitment || Commitment->is_ready() || !Commitment->has_future(), broken_promise); } oDEFINE_FUTURE_MOVE_CTOR(promise)

	// @oooii-jeff: The spec says there are no copy-constructors for 
	// packaged_task, but MSVC2010 casting packaged_task to a std::function 
	// is not move aware (and uses the copy constructor):
	// http://connect.microsoft.com/VisualStudio/feedback/details/649274/std-bind-and-std-function-are-not-move-aware 
	// So for now enable the copy constructor that's needed, but let it behave 
	// like a move constructor.
#define oDEFINE_PACKAGED_TASK_MOVE_CTORS() \
	public: packaged_task(packaged_task&& _That) : Commitment(std::move(_That.Commitment)), Function(std::move(_That.Function)) {} \
	public: packaged_task(const packaged_task& _That) : Commitment(std::move(const_cast<packaged_task&>(_That).Commitment)), Function(std::move(const_cast<packaged_task&>(_That).Function)) {} \
	public: packaged_task& operator=(packaged_task&& _That) { if (this != &_That) { Commitment = std::move(_That.Commitment); Function = std::move(_That.Function); } return *this; }

#define oDEFINE_PACKAGED_TASK_CTORS() \
	oDEFINE_PACKAGED_TASK_MOVE_CTORS() \
	private: packaged_task& operator=(packaged_task const&)/* = delete*/; \
	public: packaged_task() {} ~packaged_task() {}

} // namespace detail

template<typename T> class future : public future_detail::future_state_interface<future<T> >
{
	oFUTURE_FRIEND_INTERFACE(T);
public:
	oDEFINE_FUTURE_CTORS();
	// Sets last error if this fails, but then continues to move the commitment
	// value as the return value. @tony: This is not elegant as throwing an 
	// exception, but the codebase isn't quite there.
	T get() { oFUTURE_THROW(valid(), no_state); return Commitment->move_value(); }
};

template <class T> class future<T&> : public future_detail::future_state_interface<future<T&> >
{
	oFUTURE_FRIEND_INTERFACE(T&);
public:
	oDEFINE_FUTURE_CTORS();
	T& get() { oFUTURE_THROW(valid(), no_state); return Commitment->copy_value(); }
};

template<> class future<void> : public future_detail::future_state_interface<future<void> >
{
	oFUTURE_FRIEND_INTERFACE(void);
public:
	oDEFINE_FUTURE_CTORS();
	void get() { Commitment->no_value(); }
};

template<typename T> class promise
{
public:
	promise(const std::shared_ptr<future_detail::oCommitment<T>>& _pCommitment)
		: Commitment(_pCommitment)
	{}

	promise() { Commitment = std::make_shared<future_detail::oCommitment<T>>(); }
	oDEFINE_PROMISE_CTORS();
	oDEFINE_PROMISE_SET_ERROR();

	future<T> get_future()
	{
		oFUTURE_THROW(!Commitment->has_future(), future_already_retrieved);
		return std::move(future<T>(Commitment));
	}

	void set_value(const T& _Value) { oFUTURE_THROW(Commitment, no_state); Commitment->set_value(_Value); }
	void set_value(T&& _Value) { oFUTURE_THROW(Commitment, no_state); Commitment->set_value(std::move(_Value)); }
	void set_value_at_thread_exit(const T& _Value) { oFUTURE_THROW(Commitment, no_state); Commitment->set_value_at_thread_exit(_Value); }
	void set_value_at_thread_exit(T&& _Value) { oFUTURE_THROW(Commitment, no_state); Commitment->set_value_at_thread_exit(std::move(_Value)); }

private:
	std::shared_ptr<future_detail::oCommitment<T>> Commitment;
};

template <class T> class promise<T&>
{
public:
	promise(const std::shared_ptr<future_detail::oCommitment<T&>>& _pCommitment)
		: Commitment(_pCommitment)
	{}

	promise() { future_detail::oCommitmentCreate(&Commitment); }
	oDEFINE_PROMISE_CTORS();
	oDEFINE_PROMISE_SET_ERROR();

	future<T&> get_future()
	{
		oFUTURE_THROW(!Commitment->has_future(), future_already_retrieved);
		return std::move(future<T&>(Commitment));
	}

	void set_value(const T& _Value) { oFUTURE_THROW(Commitment, no_state); Commitment->set_value(_Value); }
	void set_value_at_thread_exit(const T& _Value) { oFUTURE_THROW(Commitment, no_state); Commitment->set_value_at_thread_exit(_Value); }

private:
	std::shared_ptr<future_detail::oCommitment<T&>> Commitment;
};

template<> class promise<void>
{
public:
	promise(const std::shared_ptr<future_detail::oCommitment<void>>& _pCommitment)
		: Commitment(_pCommitment)
	{}

	promise() { Commitment = std::make_shared<future_detail::oCommitment<void>>(); }
	oDEFINE_PROMISE_CTORS();
	oDEFINE_PROMISE_SET_ERROR();

	future<void> get_future()
	{
		oFUTURE_THROW(!Commitment->has_future(), future_already_retrieved);
		return std::move(future<void>(Commitment));
	}

	void set_value() { Commitment->set_no_value(); }

private:
	std::shared_ptr<future_detail::oCommitment<void>> Commitment;
};

namespace future_detail {
	template<typename result_type> 
	inline void fulfill_promise_helper(promise<result_type>& _Promise, std::function<result_type(void)>& Function__) { _Promise.set_value(Function__()); }
	template<> 
	inline void fulfill_promise_helper<void>(promise<void>& _Promise, std::function<void(void)>& Function__) { Function__(); _Promise.set_value(); }
};

#ifdef oHAS_VARIADIC_TEMPLATES
	#error reimplement packaged_task using variadic templates
#else
	oCALLABLE_TEMPLATE0
	class packaged_task {};

	#define oPACKAGEDTASK(_nArgs) \
	template<typename result_type oCALLABLE_CONCAT(oARG_COMMA_TYPENAMES,_nArgs)> \
	class packaged_task<result_type(oCALLABLE_CONCAT(oARG_PARTIAL_TYPENAMES,_nArgs))> \
	{ \
	public: \
		oDEFINE_PACKAGED_TASK_CTORS() \
		typedef std::function<result_type(oCALLABLE_CONCAT(oARG_PARTIAL_TYPENAMES,_nArgs))> Callable; \
		\
		explicit packaged_task(oCALLABLE_PARAMS0) \
		{ \
			Commitment = std::make_shared<future_detail::oCommitment<result_type>>(); \
			Function = std::move(oCALLABLE_PASS0); \
		} \
		\
		bool valid() const { return Function; } \
		void swap(packaged_task& _Other) { if (this != &_Other) { std::swap(Commitment, _Other.Commitment); std::swap(Function, _Other.Function); } } \
		\
		future<result_type> get_future() { oFUTURE_THROW(!Commitment->has_future(), future_already_retrieved); return std::move(future<result_type>(Commitment)); } \
		\
		void operator ()(oCALLABLE_CONCAT(oARG_DECL,_nArgs)) \
		{ \
			std::function<result_type()> function1 = std::tr1::bind(Function oCALLABLE_CONCAT(oARG_COMMA_PASS,_nArgs)); \
			fulfill_promise(std::move(promise<result_type>(Commitment)), function1); \
		} \
		\
		void commit_to_task(oCALLABLE_CONCAT(oARG_DECL,_nArgs)) \
		{ \
			std::function<result_type()> function1 = std::tr1::bind(Function oCALLABLE_CONCAT(oARG_COMMA_PASS,_nArgs)); \
			std::function<void()> function2 = std::tr1::bind(packaged_task::fulfill_promise, std::move(promise<result_type>(Commitment)), function1); \
			Commitment.get()->commit_to_task(std::move(function2)); \
		} \
		\
		void reset() { packaged_task(std::move(Function)).swap(*this); } \
		\
	private: \
		std::shared_ptr<future_detail::oCommitment<result_type>> Commitment; \
		Callable Function; \
		\
		static void fulfill_promise(promise<result_type> _Promise, std::function<result_type(void)> Function__) \
		{ oFUTURE_TRY future_detail::fulfill_promise_helper(_Promise, Function__); oFUTURE_ENDTRY } \
	};
	oCALLABLE_PROPAGATE(oPACKAGEDTASK)
#endif

// @tony: I don't quite understand the launch policies yet because 
// current implementations are very poor, so punt on that interface for now and
// always do the right thing, which is to hand the task over to the scheduler.

// future<result_of(callable)> async(callable, args...);
#ifdef oHAS_VARIADIC_TEMPLATES
	#error reimplement async using variadic templates
#else
	#define oASYNC(_nArgs) oCALLABLE_CONCAT(oCALLABLE_TEMPLATE,_nArgs) future<oCALLABLE_CONCAT(oCALLABLE_RETURN_TYPE,_nArgs)> async(oCALLABLE_CONCAT(oCALLABLE_PARAMS,_nArgs)) \
	{	packaged_task<oCALLABLE_CONCAT(oCALLABLE_RETURN_TYPE,_nArgs)(oCALLABLE_CONCAT(oARG_PARTIAL_TYPENAMES,_nArgs))> p(oCALLABLE_PASS0); \
		p.commit_to_task(oCALLABLE_CONCAT(oARG_PASS,_nArgs)); \
		return std::move(p.get_future()); \
	}
	oCALLABLE_PROPAGATE(oASYNC)
#endif

} // namespace ouro

#endif
#endif