// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/thread_traits.h>
#include <oCore/debugger.h>
#include <oCore/process_heap.h>
#include <oBase/throw.h>

namespace ouro {

void core_thread_traits::begin_thread(const char* _ThreadName)
{
	debugger::thread_name(_ThreadName);
}

void core_thread_traits::update_thread()
{
}

void core_thread_traits::end_thread()
{
	process_heap::exit_thread();
}

void core_thread_traits::at_thread_exit(const std::function<void()>& _AtExit)
{
	oTHROW0(operation_not_supported);
}

}
