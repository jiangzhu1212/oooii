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
#include "tbb_scheduler.h"
#include <oCore/process_heap.h>
#include <oCore/thread_traits.h>
#include <oBase/task_group.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

namespace ouro {
	namespace tbb {

class context
{
public:
	static context& singleton();

	context()
	{
		Observer = new observer();
		Init = new ::tbb::task_scheduler_init();
	}

	~context()
	{
		delete Init;
		delete Observer;
	}

	inline void dispatch(const std::function<void()>& _Task)
	{
		// Use task::enqueue for tasks with no dependency to ensure the main thread
		// never has to participate in TBB threading and prioritizes tasks that are 
		// issued without dependency as these tend to be tasks that are longer running 
		// and behave more like raw threads.
		//
		// task::enqueue vs task::spawn
		// http://software.intel.com/en-us/blogs/2010/05/04/tbb-30-new-today-version-of-intel-threading-building-blocks/
		// The TBB 3.0 schedule supports task::enqueue, which is effectively a �run me 
		// after the other things already pending� request. Although similar to 
		// spawning a task, an enqueued task is scheduled in a different manner. 
		// Enqueued tasks are valuable when approximately first-in first-out behavior 
		// is important, such as in situations where latency of response is more 
		// important than efficient throughput.

		::tbb::task& taskToSpawn = *new(::tbb::task::allocate_root()) task_adapter(_Task);
		::tbb::task::enqueue(taskToSpawn);
	}

private:
	class observer : public ::tbb::task_scheduler_observer
	{
	public:
		observer() { observe(); }
		void on_scheduler_entry(bool is_worker) override { if (is_worker) core_thread_traits::begin_thread("TBB Worker"); }
		void on_scheduler_exit(bool is_worker) override { if (is_worker) core_thread_traits::end_thread(); }
	};

	class task_adapter : public ::tbb::task
	{
	public:
		task_adapter(const std::function<void()>& _task_adapter) : Task(_task_adapter) {}
		task_adapter(std::function<void()>&& _task_adapter) { operator=(std::move(_task_adapter)); }
		task_adapter& operator=(task_adapter&& _That) { if (this != &_That) Task = std::move(_That.Task); return *this; }
		task* execute() { Task(); return nullptr; }
	private:
		std::function<void()> Task;
		task_adapter(const task_adapter&);
		const task_adapter& operator=(const task_adapter&);
	};

	::tbb::task_scheduler_init* Init;
	observer* Observer;
};

context& context::singleton()
{
	static context* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"tbb::context"
			, process_heap::per_process
			, process_heap::leak_tracked
			, [=](void* _pMemory) { new (_pMemory) context(); }
			, [=](void* _pMemory) { ((context*)_pMemory)->~context(); }
			, &sInstance);
	}
	return *sInstance;
}

const char* name()
{
	return "tbb";
}

void ensure_initialized()
{
	context::singleton();
}

void dispatch(const std::function<void()>& _Task)
{
	context::singleton().dispatch(_Task);
}

void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	::tbb::parallel_for(_Begin, _End, _Task);
}

class task_group_impl : public ouro::task_group
{
	::tbb::task_group g;
public:
	void run(const std::function<void()>& _Task) override { g.run(_Task); }
	void wait() override { g.wait(); }
	~task_group_impl() { wait(); }
};

std::shared_ptr<ouro::task_group> make_task_group()
{
	return std::make_shared<task_group_impl>();
}

	} // namespace tbb
} // namespace ouro
