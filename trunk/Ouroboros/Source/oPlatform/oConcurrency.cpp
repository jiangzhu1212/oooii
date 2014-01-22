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
#include <oConcurrency/oConcurrency.h>
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oCore/thread_traits.h>
#include <oPlatform/oSingleton.h>

// NOTE: Some of the concurrency requirements are actually requirements on the
// platform's memory management system, not on scheduling or other concurrency
// concepts. Those requirements APIs are implemented in the modules of memory
// management rather than here because they are cross-concurrency 
// implementations. They include:

// end_thread
// thread_at_exit

// Only one implementation can be enabled at a given time
//#define oHAS_oCONCURRENCY 1
#define oHAS_TBB 1
//#define oHAS_CONCRT 1

#ifndef oHAS_oCONCURRENCY
	#define oHAS_oCONCURRENCY 0
#endif

#ifndef oHAS_TBB
	#define oHAS_TBB 0
#endif

#ifndef oHAS_CONCRT
	#define oHAS_CONCRT 0
#endif

const char* oConcurrency::task_scheduler_name()
{
	return 
		#if oHAS_oCONCURRENCY
			"ouro::threadpool";
		#elif oHAS_TBB
			"TBB";
		#elif oHAS_CONCRT
			"conCRT/PPL";
		#else
			"serial";
		#endif
}

typedef ouro::process_heap::std_allocator<std::function<void()>> allocator_t;

#undef interface
#undef INTERFACE_DEFINED

#if oHAS_oCONCURRENCY
	#include <oBase/threadpool.h>
	using namespace ouro;
	typedef ouro::detail::task_group<allocator_t> task_group_t;
#elif oHAS_TBB
	#ifdef oHAS_MAKE_EXCEPTION_PTR
		#define copy_exception make_exception_ptr
	#endif
	#include <tbb/tbb.h>
	#include <tbb/task_scheduler_init.h>
	using namespace tbb;
	typedef tbb::task_group task_group_t;
#elif oHAS_CONCRT
	#include <concrt.h>
	#include <ppl.h>
	using namespace Concurrency;
	typedef Concurrency::task_group task_group_t;
#else
	
	class task_group_impl : public ouro::task_group
	{
	public:
		void run(const std::function<void()>& _Task) override { _Task(); }
		void wait() override {}
	};

	typedef task_group_impl task_group_t;

#endif

#define oHAS_SCHEDULER (oHAS_oCONCURRENCY || oHAS_TBB)
#define oIS_TBB_COMPATIBLE (oHAS_oCONCURRENCY || oHAS_TBB || oHAS_CONCRT)

#if oHAS_oCONCURRENCY

	bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
	{
		return oDispatchQueueCreateConcurrentGeneric(_DebugName, _InitialTaskCapacity, _ppQueue);
	}

#elif oIS_TBB_COMPATIBLE

	#include <oBasis/oDispatchQueueConcurrentT.h>
	#include <oConcurrency/task_group_threadpool.h>
	typedef oConcurrency::task_group_threadpool<task_group> threadpool_t;

	const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrentT<threadpool_t>* threadsafe const*)
	{
		// {d912957c-a621-4960-833f-55572a1a4abb}
		static const oGUID IID_oDispatchQueueConcurrentT = { 0xd912957c, 0xa621, 0x4960, { 0x83, 0x3f, 0x55, 0x57, 0x2a, 0x1a, 0x4a, 0xbb } };
		return IID_oDispatchQueueConcurrentT;
	}

	class task_group_impl : public ouro::task_group
	{
		task_group_t g;
	public:
		#if oHAS_oCONCURRENCY
			task_group() : oConcurrency::detail::task_group<oScheduler::allocator_type>(oScheduler::Singleton()->Threadpool) {}
		#endif
		void run(const std::function<void()>& _Task) override { g.run(_Task); }
		void wait() override { g.wait(); }
	};

	bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
	{
		bool success = false;
		oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentT<threadpool_t>(_DebugName, &success));
		return success;
	}

#else

	std::shared_ptr<oConcurrency::task_group> oConcurrency::make_task_group()
	{
		return std::make_shared<task_group_t>();
	}

	bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
	{
		return oDispatchQueueCreateConcurrentSerial(_DebugName, _InitialTaskCapacity, _ppQueue);
	}

#endif

#if oHAS_SCHEDULER
	class oScheduler : public oProcessSingleton<oScheduler>
	{
		oScheduler(const oScheduler&);
		const oScheduler& operator=(const oScheduler&);

	public:
		static const oGUID GUID;
	
		#if oHAS_oCONCURRENCY
			oScheduler() {}
			~oScheduler() { Threadpool.join(); }

			inline void dispatch(const std::function<void()>& _Task)
			{
				Threadpool.dispatch(_Task);
			}

			inline void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
			{
				oConcurrency::detail::parallel_for<16>(Threadpool, _Begin, _End, _Task);
			}

			threadpool<allocator_t> Threadpool;

		#elif defined(oHAS_TBB)
		
			oScheduler()
			{
				observer = new Observer;
				init = new task_scheduler_init;
			}

			~oScheduler()
			{
				delete init;
				delete observer;
			}

			inline void dispatch(const std::function<void()>& _Task)
			{
				// @oooii-kevin: For tasks with no dependency we use task::enqueue this 
				// ensures the main thread never has to participate in TBB threading and 
				// prioritizes tasks that are issued without dependency as these tend to be 
				// tasks that are longer running and behave more like raw threads.
				//
				// task::enqueue vs task::spawn
				// from http://software.intel.com/en-us/blogs/2010/05/04/tbb-30-new-today-version-of-intel-threading-building-blocks/
				// The TBB 3.0 schedule supports task::enqueue, which is effectively a “run me 
				// after the other things already pending” request. Although similar to 
				// spawning a task, an enqueued task is scheduled in a different manner. 
				// Enqueued tasks are valuable when approximately first-in first-out behavior 
				// is important, such as in situations where latency of response is more 
				// important than efficient throughput.

				task& taskToSpawn = *new(task::allocate_root()) TaskAdapter(_Task);
				task::enqueue(taskToSpawn);
			}

		private:
			class Observer : public task_scheduler_observer
			{
			public:
				Observer() { observe(); }
				void on_scheduler_entry(bool is_worker) override { if (is_worker) ouro::core_thread_traits::begin_thread("TBB Worker"); }
				void on_scheduler_exit(bool is_worker) override { if (is_worker) ouro::core_thread_traits::end_thread(); }
			};

			class TaskAdapter : public task
			{
			public:
				TaskAdapter(const std::function<void()>& _TaskAdapter) : Task(_TaskAdapter) {}
				TaskAdapter(std::function<void()>&& _TaskAdapter) { operator=(std::move(_TaskAdapter)); }
				TaskAdapter& operator=(TaskAdapter&& _That) { if (this != &_That) Task = std::move(_That.Task); return *this; }
				task* execute() { Task(); return nullptr; }
			private:
				std::function<void()> Task;
				TaskAdapter(const TaskAdapter&);
				const TaskAdapter& operator=(const TaskAdapter&);
			};

			task_scheduler_init* init;
			Observer* observer;
		#endif
	};
#endif

#if oHAS_SCHEDULER
	// {CFEBF25E-97EA-4BAB-AC50-D53474D3C758}
	const oGUID oScheduler::GUID = { 0xcfebf25e, 0x97ea, 0x4bab, { 0xac, 0x50, 0xd5, 0x34, 0x74, 0xd3, 0xc7, 0x58 } };
	oSINGLETON_REGISTER(oScheduler);
#endif

void oConcurrency::init_task_scheduler()
{
	#if oHAS_SCHEDULER
		oScheduler::Singleton();
	#endif
}

#if oHAS_CONCRT
	static void oConCRT_TaskAdapter(void* _pTask)
	{
		std::function<void()>* task = static_cast<std::function<void()>*>(_pTask);
		(*task)();
		delete task;
	}

	static void oConCRT_Dispatch(const std::function<void()>& _Task)
	{
		std::function<void()>* task = new std::function<void()>(std::move(_Task));
		CurrentScheduler::ScheduleTask(oConCRT_TaskAdapter, task);
	}
#endif

void oConcurrency::dispatch(const std::function<void()>& _Task)
{
	#if oHAS_SCHEDULER
		oScheduler::Singleton()->dispatch(_Task);
	#elif oHAS_CONCRT
		oConCRT_Dispatch(_Task);
	#else
		_Task();
	#endif
}

#if oHAS_oCONCURRENCY
	static void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
	{
		oScheduler::Singleton()->parallel_for(_Begin, _End, _Task);
	}
#endif

void oConcurrency::parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	#if oIS_TBB_COMPATIBLE
		::parallel_for(_Begin, _End, _Task);
	#else
		serial_for(_Begin, _End, _Task);
	#endif
}

void noop_fn() {}

namespace ouro {

		std::shared_ptr<task_group> task_group::make()
		{
			std::shared_ptr<task_group> p = std::move(std::make_shared<task_group_impl>());
			return p;
		}

	namespace future_requirements {

		void thread_at_exit(const std::function<void()>& _AtExit)
		{
			oConcurrency::thread_at_exit(_AtExit);
		}

	} // namespace future_requirements

} // namespace ouro
