// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declarations of oBase unit tests. These throw on failure.
#pragma once
#ifndef oBaseTests_h
#define oBaseTests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTaaboxf();
		void TESTcompression(test_services& _Services);
		void TESTconcurrent_growable_object_pool();
		void TESTconcurrent_hash_map(test_services& _Services);
		void TESTconcurrent_queue();
		void TESTconcurrent_queue_concrt();
		void TESTconcurrent_queue_opt();
		void TESTconcurrent_queue_tbb();
		void TESTconcurrent_stack();
		void TESTcoroutine();
		void TESTcountdown_latch();
		void TESTdate(test_services& _Services);
		void TESTequal();
		void TESTfilter_chain();
		void TESTfixed_block_allocator();
		void TESTfourcc();
		void TESTfuture(test_services& _Services);
		void TESThash_map(test_services& _Services);
		void TESTosc();
		void TESTparallel_for();
		void TESTpath();
		void TESTtask_group();
		void TESTthreadpool();
		void TESTthreadpool_perf(ouro::test_services& _Services);
		void TESTuri();

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
			virtual bool parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task) = 0;

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
} // namespace ouro

#endif
