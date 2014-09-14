// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Declarations of oConcurrency unit tests. These throw on failure.

#pragma once
#include <functional>

namespace ouro { class test_services; namespace tests {

void TESTconcurrent_hash_map(test_services& services);
void TESTconcurrent_queue(test_services& services);
void TESTconcurrent_queue_concrt(test_services& services);
void TESTconcurrent_queue_opt(test_services& services);
void TESTconcurrent_queue_tbb(test_services& services);
void TESTconcurrent_stack(test_services& services);
void TESTcoroutine(test_services& services);
void TESTcountdown_latch(test_services& services);
void TESTdate(test_services& services);
void TESTparallel_for(test_services& services);
void TESTtask_group(test_services& services);
void TESTthreadpool(test_services& services);
void TESTthreadpool_perf(test_services& services);

// Utility functions, do not register these as tests.
struct test_threadpool
{
public:

	// returns the name used to identify this threadpool for test's report.
	virtual const char* name() const = 0;

	// dispatches a single task for execution on any thread. There is no execution
	// order guarantee.
	virtual void dispatch(const std::function<void()>& _Task) = 0;

	// parallel_for basically breaks up some dispatch calls to be executed on 
	// worker threads. If the underlying threadpool does not support parallel_for,
	// this should return false.
	virtual bool parallel_for(size_t begin, size_t end, const std::function<void(size_t index)>& task) = 0;

	// waits for the threadpool to be empty. The threadpool must be reusable after
	// this call (this is not join()).
	virtual void flush() = 0;

	// Release the threadpool reference obtained by enumerate_threadpool below.
	virtual void release() = 0;
};

// This can be used to do an apples-to-apples benchmark with various 
// threadpools, just implement test_threadpool and pass it to this test.
void TESTthreadpool_performance(ouro::test_services& services, test_threadpool& _Threadpool);

// Implement this inside a TESTMyThreadpool() function.
template<typename test_threadpool_impl_t>
void TESTthreadpool_performance_impl(ouro::test_services& services)
{
	struct rel_on_exit
	{
		test_threadpool_impl_t& pool;
		rel_on_exit(test_threadpool_t& tp) : pool(tp) {}
		~rel_on_exit() { pool.release(); }
	};

	test_threadpool_impl_t tp;
	rel_on_exit roe(tp);
	TESTthreadpool_performance(services, tp);
}

}}
