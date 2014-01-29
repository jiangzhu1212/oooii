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
#include "ouro_scheduler.h"
#include <oCore/process_heap.h>
#include <oCore/thread_traits.h>
#include <oBase/task_group.h>

namespace ouro {
	namespace ouro_scheduler {
class context
{
public:
	typedef process_heap::std_allocator<std::function<void()>> allocator_type;
	typedef threadpool<core_thread_traits, allocator_type> threadpool_type;
	typedef detail::task_group<core_thread_traits, allocator_type> task_group_type;

	static context& singleton();
	context() {}
	~context() { tp.join(); }
	inline threadpool_type& get_threadpool() { return tp; }
	inline void dispatch(const std::function<void()>& _Task) { tp.dispatch(_Task); }
	inline void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task) { ouro::detail::parallel_for<16>(tp, _Begin, _End, _Task); }
private:
	threadpool_type tp;
};

context& context::singleton()
{
	static context* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"ouro::scheduler::context"
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
	return "ouro";
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
	context::singleton().parallel_for(_Begin, _End, _Task);
}

class task_group_impl : public task_group
{
	context::task_group_type g;
public:
	task_group_impl() : g(context::singleton().get_threadpool()) {}
	void run(const std::function<void()>& _Task) override { g.run(_Task); }
	void wait() override { g.wait(); }
	~task_group_impl() { wait(); }
};

std::shared_ptr<ouro::task_group> make_task_group()
{
	return std::make_shared<task_group_impl>();
}

	} // namespace ouro_scheduler
} // namespace ouro
