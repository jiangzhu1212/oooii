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
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>

// NOTE: Some of the concurrency requirements are actually requirements on the
// platform's memory management system, not on scheduling or other concurrency
// concepts. Those requirements APIs are implemented in the modules of memory
// management rather than here because they are cross-concurrency 
// implementations. They include:

// enable_leak_tracking_threadlocal
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
			"oConcurrency::threadpool";
		#elif oHAS_TBB
			"TBB";
		#elif oHAS_CONCRT
			"conCRT/PPL";
		#else
			"serial";
		#endif
}

typedef oProcessHeapAllocator<oTASK> allocator_t;

#if oHAS_oCONCURRENCY
	#include <oConcurrency/threadpool.h>
	using namespace oConcurrency;
	typedef oConcurrency::detail::task_group<allocator_t> task_group_t;
#elif oHAS_TBB
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
	
	class task_group_impl: public oConcurrency::task_group
	{
	public:
		void run(const oTASK& _Task) threadsafe override { _Task(); }
		void wait() threadsafe override {}
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

	class task_group_impl : public oConcurrency::task_group
	{
		task_group_t g;
	public:
		#if oHAS_oCONCURRENCY
			task_group() : oConcurrency::detail::task_group<oScheduler::allocator_type>(oScheduler::Singleton()->Threadpool) {}
		#endif
		void run(const oTASK& _Task) threadsafe override { oThreadsafe(g).run(_Task); }
		void wait() threadsafe override { oThreadsafe(g).wait(); }
	};

	std::shared_ptr<oConcurrency::task_group> oConcurrency::make_task_group()
	{
		oConcurrency::enable_leak_tracking_threadlocal(false);
		std::shared_ptr<oConcurrency::task_group> g = std::move(std::make_shared<task_group_impl>());
		oConcurrency::enable_leak_tracking_threadlocal(true);
		return std::move(g);
	}

	bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
	{
		bool success = false;
		oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentT<threadpool_t>(_DebugName, &success));
		return success;
	}

#else

	std::shared_ptr<oConcurrency::task_group> oConcurrency::make_task_group()
	{
		return std::move(std::make_shared<task_group_t>());
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

			inline void dispatch(const oTASK& _Task)
			{
				enable_leak_tracking_threadlocal(false);
				Threadpool.dispatch(_Task);
				enable_leak_tracking_threadlocal(true);
			}

			inline void parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task)
			{
				oConcurrency::detail::parallel_for<16>(Threadpool, _Begin, _End, _Task);
			}

			threadpool<allocator_t> Threadpool;

		#elif defined(oHAS_TBB)
		
			oScheduler()
			{
				oConcurrency::enable_leak_tracking_threadlocal(false);
				observer = new Observer;
				init = new task_scheduler_init;
				oConcurrency::enable_leak_tracking_threadlocal(true);
			}

			~oScheduler()
			{
				delete init;
				delete observer;
			}

			inline void dispatch(const oTASK& _Task)
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

				// @oooii-tony: This can report a false-positive leak with the Microsoft CRT 
				// leak reporter if task::allocate_root() is called in the middle of a memory
				// state check block. (working with _CrtMemState elsewhere). See oBug_1856
				// for more information.
				// NOTE: I tried to wrap this in a disable-tracking, reenable-after block, but
				// that can cause deadlocks as all of CRT it seems shares the same mutex. Also
				// just setting the state allows for any number of threads to have their 
				// allocs ignored during the disabled period. I tried having 

				oConcurrency::enable_leak_tracking_threadlocal(false);
				task& taskToSpawn = *new(task::allocate_root()) TaskAdapter(_Task);
				oConcurrency::enable_leak_tracking_threadlocal(true);
				task::enqueue(taskToSpawn);
			}

		private:
			class Observer : public task_scheduler_observer
			{
			public:
				Observer() { observe(); }
				void on_scheduler_entry(bool is_worker) override { if (is_worker) ouro::debugger::thread_name("TBB Worker"); }
				void on_scheduler_exit(bool is_worker) override { if (is_worker) oConcurrency::end_thread(); }
			};

			class TaskAdapter : public task
			{
			public:
				TaskAdapter(const oTASK& _TaskAdapter) : Task(_TaskAdapter) {}
				TaskAdapter(oTASK&& _TaskAdapter) { operator=(std::move(_TaskAdapter)); }
				TaskAdapter& operator=(TaskAdapter&& _That) { if (this != &_That) Task = std::move(_That.Task); return *this; }
				task* execute() { Task(); return nullptr; }
			private:
				oTASK Task;
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
		oTASK* task = static_cast<oTASK*>(_pTask);
		(*task)();
		delete task;
	}

	static void oConCRT_Dispatch(const oTASK& _Task)
	{
		oConcurrency::enable_leak_tracking_threadlocal(false);
		oTASK* task = new oTASK(std::move(_Task));
		oConcurrency::enable_leak_tracking_threadlocal(true);
		CurrentScheduler::ScheduleTask(oConCRT_TaskAdapter, task);
	}
#endif

void oConcurrency::dispatch(const oTASK& _Task)
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
	static void parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task)
	{
		oScheduler::Singleton()->parallel_for(_Begin, _End, _Task);
	}
#endif

void oConcurrency::parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task)
{
	#if oIS_TBB_COMPATIBLE
		::parallel_for(_Begin, _End, _Task);
	#else
		serial_for(_Begin, _End, _Task);
	#endif
}

void oConcurrency::begin_thread(const char* _DebuggerName)
{
	ouro::debugger::thread_name(_DebuggerName);
	#if oHAS_TBB
		// Issuing a NOP task causes TBB to allocate bookkeeping for this thread's 
		// memory which otherwise reports as a false-positive memory leak.
		oStd::future<void> f = oStd::async([=] {});
		f.wait();
	#endif
}
namespace oStd {
	namespace future_requirements {

		void thread_at_exit(const oTASK& _AtExit)
		{
			oConcurrency::thread_at_exit(_AtExit);
		}

		class waitable_task_impl : public waitable_task
		{
			task_group_t g;
		public:
			waitable_task_impl(const oTASK& _Task) { g.run(_Task); }
			~waitable_task_impl() { wait(); }
			void wait() override { g.wait(); }
		};

		std::shared_ptr<waitable_task> make_waitable_task(const oTASK& _Task)
		{
			oConcurrency::enable_leak_tracking_threadlocal(false);
			std::shared_ptr<waitable_task> p = std::move(std::make_shared<waitable_task_impl>(_Task));
			oConcurrency::enable_leak_tracking_threadlocal(true);
			return std::move(p);
		}

	} // namespace future_requirements

	namespace condition_variable_requirements {

		void thread_at_exit(const oTASK& _AtExit)
		{
			oConcurrency::thread_at_exit(_AtExit);
		}

	} // namespace condition_variable_requirements

} // namespace oStd
