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
// Declarations of oConcurrency unit tests. These throw an oStd::standard_error 
// on failure.
#pragma once
#ifndef oConcurrencyTests_h
#define oConcurrencyTests_h

#include <oConcurrency/tests/oConcurrencyTestRequirements.h>
#include <oStd/finally.h>

namespace oConcurrency {
	namespace tests {

		void TESTbasic_threadpool();
		void TESTbasic_threadpool_perf(requirements& _Requirements);
		void TESTblock_allocator();
		void TESTconcurrent_index_allocator();
		void TESTconcurrent_linear_allocator();
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
		void TESTthreadpool_perf(requirements& _Requirements);

		// Utility functions, do not register these as tests.

		// This can be used to do an apples-to-apples benchmark with various 
		// threadpools, just implement test_threadpool and pass it to this test.
		void TESTthreadpool_performance(requirements& _Requirements, test_threadpool& _Threadpool);

		// Implement this inside a TESTMyThreadpool() function.
		template<typename test_threadpool_impl_t> void TESTthreadpool_performance_impl(requirements& _Requirements)
		{
			test_threadpool_impl_t tp;
			oStd::finally Release([&] { tp.release(); });
			TESTthreadpool_performance(_Requirements, tp);
		}

	} // namespace tests
} // namespace oConcurrency

#endif
