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
// Declarations of oConcurrency unit tests. These throw an oStd::standard_error 
// on failure.
#pragma once
#ifndef oConcurrencyTests_h
#define oConcurrencyTests_h

#include <oBase/finally.h>

namespace ouro { class test_services; }

namespace oConcurrency {
	namespace tests {

		void TESTbasic_threadpool();
		void TESTbasic_threadpool_perf(ouro::test_services& _Services);
		void TESTblock_allocator();
		void TESTconcurrent_queue();
		void TESTconcurrent_queue_concrt();
		void TESTconcurrent_queue_opt();
		void TESTconcurrent_queue_tbb();
		void TESTconcurrent_stack();
		void TESTconcurrent_worklist();
		void TESTcoroutine();
		void TESTcountdown_latch();
		void TESTfixed_block_allocator();
		void TESTindex_allocator();
		void TESTparallel_for();
		void TESTtask_group();
		void TESTthreadpool();
		void TESTthreadpool_perf(ouro::test_services& _Services);

		// Utility functions, do not register these as tests.

		struct test_threadpool
		{
		public:

			// returns the name used to identify this threadpool for test's report.
			virtual const char* name() const = 0;

			// dispatches a single task for execution on any thread. There is no execution
			// order guarantee.
			virtual void dispatch(const oTASK& _Task) = 0;

			// parallel_for basically breaks up some dispatch calls to be executed on 
			// worker threads. If the underlying threadpool does not support parallel_for,
			// this should return false.
			virtual bool parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task) = 0;

			// waits for the threadpool to be empty. The threadpool must be reusable after
			// this call (this is not join()).
			virtual void flush() = 0;

			// Release the threadpool reference obtained by enumerate_threadpool below.
			virtual void release() = 0;
		};

		// This can be used to do an apples-to-apples benchmark with various 
		// threadpools, just implement test_threadpool and pass it to this test.
		void TESTthreadpool_performance(ouro::test_services& _Services, test_threadpool& _Threadpool);

		// Implement this inside a TESTMyThreadpool() function.
		template<typename test_threadpool_impl_t> void TESTthreadpool_performance_impl(ouro::test_services& _Services)
		{
			test_threadpool_impl_t tp;
			ouro::finally Release([&] { tp.release(); });
			TESTthreadpool_performance(_Services, tp);
		}

	} // namespace tests
} // namespace oConcurrency

#endif
