// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// An approximation of C++11's std::future but with dangling calls for a client 
// lib to implement. GCC Source, VS2010, just::thread: all implement std::async 
// as a right-now-alloc-a-thread-and-exec-task style system. Such implementa-
// tions have a lot more overhead than a threadpool so reimplement future here 
// with some dangling interface usage so that a work-stealing threadpool can be 
// implemented to improve performance.

#pragma once
#include <oConcurrency/concurrency.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <system_error>
#include <utility>

// Vardiatic templates replaces this, but until they're available use callable
// macros even though it pokes back into oBase.
#include <oBase/callable.h>

namespace ouro {

enum class launch { async, deferred };
enum class future_status { ready, timeout, deferred };
enum class future_errc { broken_promise, future_already_retrieved, promise_already_satisfied, no_state, not_work_stealing, no_implementation };

const std::error_category& future_category();

/*constexpr*/ inline std::error_code make_error_code(future_errc errc) { return std::error_code(static_cast<int>(errc), future_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(future_errc errc) { return std::error_condition(static_cast<int>(errc), future_category()); }

class future_error : public std::logic_error
{
public:
	future_error(future_errc errc) : logic_error(future_category().message(static_cast<int>(errc))) {}
};

template<typename T> class future;
template<typename T> class promise;

namespace future_detail {

	void* commitment_allocate(size_t _CommitmentSize);
	void commitment_deallocate(void* _pPointer);

	// This should be implemented by client code to register a function on the calling thread
	// that will be executed in FIFO order upon thread termination.
	void at_thread_exit(const std::function<void()>& _AtExit);

	#define oCOMMITMENT_COPY_MOVE_CTORS() \
		commitment(const commitment&)/* = delete*/; \
		const commitment& operator=(const commitment&)/* = delete*/; \
		commitment(commitment&&)/* = delete*/; \
		commitment& operator=(commitment&&)/* = delete*/;

	#define oCOMMITMENT_NEW_DEL()
		inline void* operator new(size_t size, void* memory) { return memory; } \
		inline void operator delete(void* p, void* memory) {} \
		inline void* operator new(size_t size) { return commitment_allocate(size); } \
		inline void* operator new[](size_t size) { return commitment_allocate(size); } \
		inline void operator delete(void* p) { commitment_deallocate(p); } \
		inline void operator delete[](void* p) { commitment_deallocate(p); }

	#define oFUTURE_CHECK(_Expr, errc) do { if (!(_Expr)) throw future_error(future_errc::errc); } while(false)
	#define oFUTURE_WAIT_AND_CHECK_ERROR() do { wait(); if (has_exception()) std::rethrow_exception(pException); } while(false)

	class commitment_state
	{
		// This is not public API, use promise, packaged_task, or async.

		// Common state between a promise and a future OR a packaged_task and a 
		// future. CV/Mutex synchronize promises with futures, but packaged_task are
		// the only ones meeting an commitment, and use the underlying system task
		// scheduler to enable work stealing, so there's a lot of if work_steals()
		// code to either ignore the mutex and use the schedule OR use the mutex.
		
		// This base class encapsulates all the state that is not the actual value,
		// since that requires a templated type.

		commitment_state(const commitment_state&)/* = delete*/;
		const commitment_state& operator=(const commitment_state&)/* = delete*/;
		commitment_state(commitment_state&&)/* = delete*/;
		commitment_state& operator=(commitment_state&&)/* = delete*/;

	public:
		commitment_state() 
			: RefCount(1)
		{}
		~commitment_state() {}

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
				oFUTURE_CHECK(work_steals(), not_work_stealing);
				oFUTURE_CHECK(is_deferred(), no_implementation);
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
		future_status wait_for(std::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oFUTURE_CHECK(work_steals(), not_work_stealing);
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
			oFUTURE_CHECK(!is_ready(), promise_already_satisfied);
			pException = _pException;
			State.HasException = true;
			State.HasExceptionAtThreadExit = true;
			State.IsReady = true;
		}

		void set_exception_at_thread_exit(std::exception_ptr _pException)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_CHECK(!is_ready() && !State.HasExceptionAtThreadExit, promise_already_satisfied);
			pException = _pException;
			State.HasExceptionAtThreadExit = true;
		}

		void commit_to_task(const std::function<void()>& _Task)
		{
			Task = make_task_group();
			Task->run(_Task);
		}

	private:
		template<typename> friend class commitment;
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

		// NOTE: When a task is associated with this commitment it wait/blocking
		// system is used to delegate context to the task's scheduler so work 
		// stealing can be done. This means the CV/Mutex aren't use to synchronize,
		// so if work_steals() it's all on Task->wait(). If !work_steals() then
		// unique locks and conditions are responsible.
		std::shared_ptr<task_group> Task;
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
			oFUTURE_CHECK(!is_ready(), promise_already_satisfied);
			State.HasValue = true;
			State.HasValueAtThreadExit = true;
			State.IsReady = true;
			notify(lock);
		}

		void notify_value_set_at_thread_exit(std::unique_lock<std::mutex>& _Lock, std::function<void()>& _NotifyValueSetAndRelease)
		{
			if (!has_value()) // don't override a fulfilled promise
			{
				oFUTURE_CHECK(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
				State.HasValueAtThreadExit = true;
				_Lock.unlock();
				reference();
				at_thread_exit(_NotifyValueSetAndRelease);
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

		template<typename T, typename U> void internal_set_value_ref(void* _pValueMemory, const U& value)
		{
			oFUTURE_CHECK(!is_ready(), promise_already_satisfied);
			oFUTURE_CHECK(!State.HasValueAtThreadExit, no_implementation);
			*(T**)_pValueMemory = &const_cast<U&>(value);
			State.HasValue = true;
			State.IsReady = true;
		}

		template<typename T, typename U> void internal_set_value(void* _pValueMemory, const U& value)
		{
			oFUTURE_CHECK(!is_ready(), promise_already_satisfied);
			oFUTURE_CHECK(!State.HasValueAtThreadExit, no_implementation);
			::new(_pValueMemory) T(value);
			State.HasValue = true;
			State.IsReady = true;
		}

		template<typename T, typename U> void internal_set_value(void* _pValueMemory, U&& value)
		{
			oFUTURE_CHECK(!is_ready(), promise_already_satisfied);
			oFUTURE_CHECK(!State.HasValueAtThreadExit, no_implementation);
			::new(_pValueMemory) T(std::forward<U>(value));
			State.HasValue = true;
			State.IsReady = true;
		}
		
		template <typename T, typename U> void typed_set_value_ref(void* _pValueMemory, const U& value)
		{
			if (work_steals())
				internal_set_value_intrusive_ptr<T>(_pValueMemory, value);
			else
			{
				std::unique_lock<std::mutex> lock(Mutex);
				internal_set_value_intrusive_ptr<T>(_pValueMemory, value);
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void typed_set_value(void* _pValueMemory, const U& value)
		{
			if (work_steals())
				internal_set_value<T>(_pValueMemory, value);
			else
			{
				std::unique_lock<std::mutex> lock(Mutex);
				internal_set_value<T>(_pValueMemory, value);
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void typed_set_value(void* _pValueMemory, U&& value)
		{
			if (work_steals())
				internal_set_value<T>(_pValueMemory, std::move(value));
			else
			{
				std::unique_lock<std::mutex> lock(Mutex);
				internal_set_value<T>(_pValueMemory, std::move(value));
				lock.unlock();
				CV.notify_all();
			}
		}

		template <typename T, typename U> void set_value_at_thread_exit(void* _pValueMemory, U&& value, std::function<void()>&& _NotifyValueSetAndRelease)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_CHECK(work_steals(), not_work_stealing);
			oFUTURE_CHECK(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
			::new(_pValueMemory) T(std::forward<U>(value));
			reference();
			notify_value_set_at_thread_exit(lock, std::move(_NotifyValueSetAndRelease));
		}

		template <typename T, typename U> void set_value_at_thread_exit_ref(void* _pValueMemory, const U& value, std::function<void()>&& _NotifyValueSetAndRelease)
		{
			std::unique_lock<std::mutex> lock(Mutex);
			oFUTURE_CHECK(work_steals(), not_work_stealing);
			oFUTURE_CHECK(!is_ready() && !State.HasValueAtThreadExit, promise_already_satisfied);
			*(T**)_pValueMemory = &const_cast<U&>(value);
			reference();
			notify_value_set_at_thread_exit(lock, std::move(_NotifyValueSetAndRelease));
		}
	};

	template<typename T> class commitment : public commitment_state
	{
		// This is not public API, use promise, packaged_task, or async.

		// Stores both state and a typed value. This is not public API, use 
		// promise, packaged_task, or async.

		oCOMMITMENT_COPY_MOVE_CTORS();
	public:
		oCOMMITMENT_NEW_DEL();

		commitment() {}
		~commitment() {}

		void release()
		{
			if (--RefCount == 0)
			{
				oFUTURE_CHECK(is_ready(), broken_promise);
				if (has_value() || State.HasValueAtThreadExit)
					reinterpret_cast<T*>(&Value)->~T();
				delete this;
			}
		}

		template <typename U> void set_value(const U& value)
		{
			typed_set_value<T>(&Value, value);
		}

		template <typename U> void set_value(U&& value)
		{
			typed_set_value<T>(&Value, std::move(value));
		}

		template <typename U> void set_value_at_thread_exit(const U& value)
		{
			set_value_at_thread_exit<T>(&Value, Value, std::bind(&commitment<T>::notify_value_set_and_release, this));
		}

		template <typename U> void set_value_at_thread_exit(U&& value)
		{
			set_value_at_thread_exit<T>(&Value, std::move(value), std::bind(&commitment<T>::notify_value_set_and_release, this));
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

	template<typename T> class commitment<T&> : public commitment_state
	{
		// This is not public API, use promise, packaged_task, or async.

		oCOMMITMENT_COPY_MOVE_CTORS();
	public:
		oCOMMITMENT_NEW_DEL();

		commitment() {}
		~commitment() {}

		void release()
		{
			if (--RefCount == 0)
			{
				oFUTURE_CHECK(is_ready(), broken_promise);
				delete this;
			}
		}

		template <typename U> void set_value(const U& value)
		{
			typed_set_value_intrusive_ptr<T>(&Value, value);
		}

		template <typename U> void set_value_at_thread_exit(const U& value)
		{
			set_value_at_thread_exit_intrusive_ptr<T>(&Value, value, std::bind(&commitment<T&>::notify_value_set_and_release, this));
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

	template<> class commitment<void> : public commitment_state
	{
		// This is not public API, use promise, packaged_task, or async.

		oCOMMITMENT_COPY_MOVE_CTORS();
	public:
		oCOMMITMENT_NEW_DEL();

		commitment() {}
		~commitment() {}

		void release()
		{
			if (--RefCount == 0)
			{
				oFUTURE_CHECK(is_ready(), broken_promise);
				delete this;
			}
		}
	
	private:
		template<typename> friend class future;
		template<typename> friend class promise;

		void no_value() { oFUTURE_WAIT_AND_CHECK_ERROR(); }
	};

	template<typename BaseT> class future_state_interface
	{
	public:
		inline bool valid() const { return !!This()->Commitment; }
		inline bool is_ready() const { oFUTURE_CHECK(valid(), no_state); return This()->Commitment->is_ready(); }
		inline bool has_value() const { oFUTURE_CHECK(valid(), no_state); return This()->Commitment->has_value(); }
		inline bool has_exception() const { oFUTURE_CHECK(valid(), no_state); return This()->Commitment->has_exception(); }
		inline bool is_deferred() const { oFUTURE_CHECK(valid(), no_state); return This()->Commitment->is_deferred(); }
		inline bool work_steals() const { oFUTURE_CHECK(valid(), no_state); return This()->Commitment->work_steals(); }
		inline void wait() { oFUTURE_CHECK(valid(), no_state); This()->Commitment->wait(); }

		template<typename Rep, typename Period>
		future_status wait_for(std::chrono::duration<Rep,Period> const& _RelativeTime)
		{
			oFUTURE_CHECK(valid(), no_state); 
			return This()->Commitment->wait_for(_RelativeTime);
		}

		template<typename Clock, typename Duration>
		future_status wait_until(std::chrono::time_point<Clock,Duration> const& _AbsoluteTime)
		{
			oFUTURE_CHECK(valid(), no_state); 
			chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
			return wait_for(duration);
		}
		
	private:
		BaseT* This() { return static_cast<BaseT*>(this); }
		const BaseT* This() const { return static_cast<const BaseT*>(this); }
	};

	// The spec says there are no copy-constructors... but I really need to pass this to a std::bind. 
	// There is a bug in MSVC2010 that disallows std::move through std::bind:
	// http://connect.microsoft.com/VisualStudio/feedback/details/649274/std-bind-and-std-function-are-not-move-aware 
	// So until then, use a loophole in the spec: There are no copy-ctors for future or promise objects. 
	// Thus USE a copy ctor, const_cast it away and eviscerate the source.
	#define oFUTURE_MOVE_CTOR(_Classname) \
		_Classname(const _Classname& _That) : Commitment(std::move(const_cast<_Classname&>(_That).Commitment)) {} \
		_Classname& operator=(const _Classname& _That) { if (this != &_That) { Commitment = std::move(const_cast<_Classname&>(_That).Commitment); } return *this; } \
		_Classname(_Classname&& _That) : Commitment(std::move(_That.Commitment)) {} \
		_Classname& operator=(_Classname&& _That) { if (this != &_That) { Commitment = std::move(_That.Commitment); } return *this; }

		// Certain base classes, permutations of the same concept, and special-case
		// interfaces need direct access to the commitment, or act as a factor for a 
		// future, so centralize this access here between the template types for future.
	#define oFUTURE_FRIEND_INTERFACE(_CommitmentType) \
		std::shared_ptr<future_detail::commitment<_CommitmentType>> Commitment; \
		template <typename> friend class future_detail::future_state_interface; \
		template <typename> friend class promise; \
		template <typename> friend class shared_future; \
		template <typename> friend class packaged_task; \
		future(const std::shared_ptr<future_detail::commitment<_CommitmentType>>& _pCommitment) \
		: Commitment(_pCommitment) \
		{	oFUTURE_CHECK(!Commitment->has_future(), future_already_retrieved); \
		Commitment->set_future_attached(); \
		} \
		future_detail::commitment_state* get_commitment_state() { return Commitment.get(); }

	#define oDEFINE_PROMISE_SET_ERROR() \
		void set_exception(std::exception_ptr _pException) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_exception(_pException); } \
		void set_exception_at_thread_exit(std::exception_ptr _pException) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_exception_at_thread_exit(_pException); } \

	#define oFUTURE_CTORS() future() {} ~future() {} oFUTURE_MOVE_CTOR(future)
	#define oPROMISE_CTORS() ~promise() { oFUTURE_CHECK(!Commitment || Commitment->is_ready() || !Commitment->has_future(), broken_promise); } oFUTURE_MOVE_CTOR(promise)

	// The spec says there are no copy-constructors for packaged_task but MSVC2010 casting packaged_task to a std::function 
	// is not move aware (and uses the copy constructor):
	// http://connect.microsoft.com/VisualStudio/feedback/details/649274/std-bind-and-std-function-are-not-move-aware 
	// So for now enable the copy constructor that's needed, but let it behave like a move constructor.
	#define oPACKAGED_TASK_MOVE_CTORS() \
		public: packaged_task(packaged_task&& _That) : Commitment(std::move(_That.Commitment)), Function(std::move(_That.Function)) {} \
		public: packaged_task(const packaged_task& _That) : Commitment(std::move(const_cast<packaged_task&>(_That).Commitment)), Function(std::move(const_cast<packaged_task&>(_That).Function)) {} \
		public: packaged_task& operator=(packaged_task&& _That) { if (this != &_That) { Commitment = std::move(_That.Commitment); Function = std::move(_That.Function); } return *this; }

	#define oPACKAGED_TASK_CTORS() \
		oPACKAGED_TASK_MOVE_CTORS() \
		private: packaged_task& operator=(packaged_task const&)/* = delete*/; \
		public: packaged_task() {} ~packaged_task() {}

	#define oPROMISE_COMMON(_Type) \
		private: std::shared_ptr<future_detail::commitment<_Type>> Commitment; \
		public: promise(const std::shared_ptr<future_detail::commitment<_Type>>& _pCommitment) : Commitment(_pCommitment) {} \
		public: promise() { Commitment = std::make_shared<future_detail::commitment<_Type>>(); } \
		public: oPROMISE_CTORS(); \
		public: oDEFINE_PROMISE_SET_ERROR(); \
		public: future<_Type> get_future() { oFUTURE_CHECK(!Commitment->has_future(), future_already_retrieved); return future<_Type>(Commitment); }

} // namespace detail

template<typename T> class future : public future_detail::future_state_interface<future<T> >
{
	oFUTURE_FRIEND_INTERFACE(T);
public:
	oFUTURE_CTORS();
	T get() { oFUTURE_CHECK(valid(), no_state); return Commitment->move_value(); }
};

template <class T> class future<T&> : public future_detail::future_state_interface<future<T&> >
{
	oFUTURE_FRIEND_INTERFACE(T&);
public:
	oFUTURE_CTORS();
	T& get() { oFUTURE_CHECK(valid(), no_state); return Commitment->copy_value(); }
};

template<> class future<void> : public future_detail::future_state_interface<future<void> >
{
	oFUTURE_FRIEND_INTERFACE(void);
public:
	oFUTURE_CTORS();
	void get() { Commitment->no_value(); }
};

template<typename T> class promise
{
	oPROMISE_COMMON(T);
public:
	void set_value(const T& value) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_value(value); }
	void set_value(T&& value) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_value(std::move(value)); }
	void set_value_at_thread_exit(const T& value) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_value_at_thread_exit(value); }
	void set_value_at_thread_exit(T&& value) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_value_at_thread_exit(std::move(value)); }

};

template <class T> class promise<T&>
{
	oPROMISE_COMMON(T&);
public:
	void set_value(const T& value) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_value(value); }
	void set_value_at_thread_exit(const T& value) { oFUTURE_CHECK(Commitment, no_state); Commitment->set_value_at_thread_exit(value); }
};

template<> class promise<void>
{
	oPROMISE_COMMON(void);
public:
	void set_value() { Commitment->set_no_value(); }
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
		oPACKAGED_TASK_CTORS() \
		typedef std::function<result_type(oCALLABLE_CONCAT(oARG_PARTIAL_TYPENAMES,_nArgs))> Callable; \
		\
		explicit packaged_task(oCALLABLE_PARAMS0) \
		{ \
			Commitment = std::make_shared<future_detail::commitment<result_type>>(); \
			Function = std::move(oCALLABLE_PASS0); \
		} \
		\
		bool valid() const { return Function; } \
		void swap(packaged_task& _Other) { if (this != &_Other) { std::swap(Commitment, _Other.Commitment); std::swap(Function, _Other.Function); } } \
		\
		future<result_type> get_future() { oFUTURE_CHECK(!Commitment->has_future(), future_already_retrieved); return std::move(future<result_type>(Commitment)); } \
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
			Commitment.get()->commit_to_task(function2); \
		} \
		\
		void reset() { packaged_task(std::move(Function)).swap(*this); } \
		\
	private: \
		std::shared_ptr<future_detail::commitment<result_type>> Commitment; \
		Callable Function; \
		\
		static void fulfill_promise(promise<result_type> _Promise, std::function<result_type(void)> Function__) \
		{ try { future_detail::fulfill_promise_helper(_Promise, Function__); } catch (std::exception&) { _Promise.set_exception(std::current_exception()); } } \
	};
	oCALLABLE_PROPAGATE(oPACKAGEDTASK)
#endif

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

}
