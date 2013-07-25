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
#include <oStd/assert.h>
#include <oStd/byte.h>
#include <oStd/finally.h>
#include <oStd/fixed_string.h>
#include <oStd/oStdAtomic.h>
#include <oStd/throw.h>
#include <oStd/timer.h>
#include <oConcurrency/tests/oConcurrencyTestRequirements.h>
#include <oConcurrency/basic_threadpool.h>
#include <oConcurrency/threadpool.h>

namespace oConcurrency {
	namespace tests {

template<typename ThreadpoolT> void test_basics(ThreadpoolT& t)
{
	oCHECK(t.joinable(), "Threadpool is not joinable");

	{
		volatile int value = -1;
		t.dispatch([&] { oStd::atomic_increment(&value); });
		t.flush();
		oCHECK(value == 0, "Threadpool did not execute a single task correctly.");
	}

	static const int kNumDispatches = 100;
	{
		int value = 0;
		for (int i = 0; i < kNumDispatches; i++)
			t.dispatch([&] { oStd::atomic_increment(&value); });

		t.flush();
		oCHECK(value == kNumDispatches, "Threadpool did not dispatch correctly, or failed to properly block on flush");
	}
}

template<typename TaskGroupT> static void test_task_group(TaskGroupT& g)
{
	static const int kNumRuns = 100;
	{
		int value = 0;
		for (size_t i = 0; i < kNumRuns; i++)
			g.run([&] { oStd::atomic_increment(&value); });

		g.wait();
		oStd::atomic_thread_fence_read_write();
		oCHECK(value == kNumRuns, "oTaskgroup either failed to run or failed to block on wait (1st run)");
	}

	{
		int value = 0;
		for (int i = 0; i < kNumRuns; i++)
			g.run([&] { oStd::atomic_increment(&value); });

		g.wait();
		oStd::atomic_thread_fence_read_write();
		oCHECK(value == kNumRuns, "oTaskgroup was not able to be reused or either failed to run or failed to block on wait (2nd run)");
	}
}

static void set_value(size_t _Index, size_t* _pArray)
{
	_pArray[_Index] = 1;
}

template<typename ThreadpoolT> static void test_parallel_for(ThreadpoolT& _Threadpool)
{
	static const size_t kFullRange = 100;

	size_t Results[kFullRange];
	memset(Results, 0, kFullRange * sizeof(size_t));

	oConcurrency::detail::parallel_for<16>(_Threadpool, 10, kFullRange - 10, oBIND(set_value, oBIND1, Results));
	for (size_t i = 0; i < 10; i++)
		oCHECK(Results[i] == 0, "wrote out of range at beginning");

	for (size_t i = 10; i < kFullRange-10; i++)
		oCHECK(Results[i] == 1, "bad write in main set_value loop");

	for (size_t i = kFullRange-10; i < kFullRange; i++)
		oCHECK(Results[i] == 0, "wrote out of range at end");
}

template<typename ThreadpoolT> static void TestT()
{
	ThreadpoolT t;
	oStd::finally OSE([&] { t.join(); });

	test_basics(t);
	t.join();
	oCHECK(!t.joinable(), "Threadpool was joined, but remains joinable()");
}

void TESTbasic_threadpool()
{
	TestT<basic_threadpool<std::allocator<oTASK>>>();
}

void TESTthreadpool()
{
	TestT<threadpool<std::allocator<oTASK>>>();
}

void TESTtask_group()
{
	threadpool<std::allocator<oTASK>> t;
	oStd::finally OSE([&] { t.join(); });

	detail::task_group<std::allocator<oTASK>> g(t);
	test_task_group(g);
	
	test_parallel_for(t);
}

namespace RatcliffJobSwarm {

#if 1
	// Settings used in John Ratcliffe's code
	#define FRACTAL_SIZE 2048
	#define SWARM_SIZE 8
	#define MAX_ITERATIONS 65536
#else
	// Super-soak test... thread_pool tests take about ~30 sec each
	#define FRACTAL_SIZE 16384
	#define SWARM_SIZE 64
	#define MAX_ITERATIONS 65536
#endif

#define TILE_SIZE ((FRACTAL_SIZE)/(SWARM_SIZE))

//********************************************************************************
// solves a single point in the mandelbrot set.
//********************************************************************************
static inline unsigned int mandelbrotPoint(unsigned int iterations,double real,double imaginary)
{
	double fx,fy,xs,ys;
	unsigned int count;

  double two(2.0);

	fx = real;
	fy = imaginary;
  count = 0;

  do
  {
    xs = fx*fx;
    ys = fy*fy;
		fy = (two*fx*fy)+imaginary;
		fx = xs-ys+real;
    count++;
	} while ( xs+ys < 4.0 && count < iterations);

	return count;
}

static inline unsigned int solvePoint(unsigned int x,unsigned int y,double x1,double y1,double xscale,double yscale)
{
  return mandelbrotPoint(MAX_ITERATIONS,(double)x*xscale+x1,(double)y*yscale+y1);
}

static void MandelbrotTask(size_t _Index, void* _pData, double _FX, double _FY, double _XScale, double _YScale)
{
	unsigned int Xs = static_cast<unsigned int>(_Index % TILE_SIZE);
	unsigned int Ys = static_cast<unsigned int>(_Index / TILE_SIZE);
	const size_t stride = FRACTAL_SIZE * sizeof(unsigned char);
	const size_t offset = Ys*stride + Xs*sizeof(unsigned char);
	unsigned char* fractal_image = oStd::byte_add((unsigned char*)_pData, offset);
	for (unsigned int y=0; y<SWARM_SIZE; y++)
	{
		unsigned char* pDest = oStd::byte_add(fractal_image, stride*y);
		for (unsigned int x=0; x<SWARM_SIZE; x++)
		{
			unsigned int v = 0xff & solvePoint(x+Xs,y+Ys,_FX,_FY,_XScale,_YScale);
			*pDest++ = (unsigned char)v;
		}
	}
}

#pragma fenv_access (on)

#ifdef _DEBUG
	#define DEBUG_DISCLAIMER "(DEBUG: Non-authoritive) "
#else
	#define DEBUG_DISCLAIMER
#endif

} // namespace RatcliffJobSwarm

void TESTthreadpool_performance(requirements& _Requirements, test_threadpool& _Threadpool)
{
	const unsigned int taskRow = TILE_SIZE;
	const unsigned int taskCount = taskRow*taskRow;
	const double x1 = -0.56017680903960034334758968;
	const double x2 = -0.5540396934395273995800156;
	const double y1 = -0.63815211573948702427222672;
	const double y2 = y1+(x2-x1);
	const double xscale = (x2-x1)/(double)FRACTAL_SIZE;
	const double yscale = (y2-y1)/(double)FRACTAL_SIZE;
	std::vector<unsigned char> fractal;
	fractal.resize(FRACTAL_SIZE*FRACTAL_SIZE);

	const char* n = _Threadpool.name() ? _Threadpool.name() : "(null)";

	oTRACEA("%s::dispatch()...", n);
	oStd::timer t;
	for (unsigned int y=0; y<TILE_SIZE; y++)
		for (unsigned int x=0; x<TILE_SIZE; x++)
			_Threadpool.dispatch(oBIND(RatcliffJobSwarm::MandelbrotTask
			, y*TILE_SIZE + x, fractal.data(), x1, y1, xscale, yscale));

	double scheduling_time = t.seconds();

	_Threadpool.flush();
	double execution_time = t.seconds();

	oTRACEA("%s::parallel_for()...", n);
	t.reset();
	bool DidParallelFor = _Threadpool.parallel_for(0, taskCount
		, oBIND(RatcliffJobSwarm::MandelbrotTask, oBIND1, fractal.data(), x1, y1, xscale, yscale));

	double parallel_for_time = t.seconds();
	const char* DebuggerDisclaimer = _Requirements.is_debugger_attached() ? "(DEBUGGER ATTACHED: Non-authoritive) " : "";

	oStd::sstring st, et, pt;
	oStd::format_duration(st, scheduling_time, true, true);
	oStd::format_duration(et, execution_time, true, true);
	oStd::format_duration(pt, parallel_for_time, true, true);

	_Requirements.report(DEBUG_DISCLAIMER 
		"%s%s: dispatch %s (%s sched), parallel_for %s"
		, DebuggerDisclaimer, n, et.c_str(), st.c_str(), DidParallelFor ? pt.c_str() : "not supported");
}

struct basic_threadpool_impl : test_threadpool
{
	basic_threadpool<std::allocator<oTASK>> t;
	~basic_threadpool_impl() { t.join(); }
	const char* name() const threadsafe override { return "basic_threadpool"; }
	void dispatch(const oTASK& _Task) threadsafe override { return t.dispatch(_Task); }
	bool parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task) threadsafe override { return false; }
	void flush() threadsafe override { t.flush(); }
	void release() threadsafe override { t.join(); }
};

struct threadpool_impl : test_threadpool
{
	threadpool<std::allocator<oTASK>> t;
	~threadpool_impl() { t.join(); }
	const char* name() const threadsafe override { return "threadpool"; }
	void dispatch(const oTASK& _Task) threadsafe override { return t.dispatch(_Task); }
	bool parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task) threadsafe override
	{
		oConcurrency::detail::parallel_for<16>(oThreadsafe(t), _Begin, _End, _Task);
		return true;
	}

	void flush() threadsafe override { t.flush(); }
	void release() threadsafe override { t.join(); }
};

namespace {
	// Implement this inside a TESTMyThreadpool() function.
	template<typename test_threadpool_impl_t> void TESTthreadpool_performance_impl1(requirements& _Requirements)
	{
		test_threadpool_impl_t tp;
		oStd::finally Release([&] { tp.release(); });
		TESTthreadpool_performance(_Requirements, tp);
	}
}

void TESTbasic_threadpool_perf(requirements& _Requirements)
{
	TESTthreadpool_performance_impl1<basic_threadpool_impl>(_Requirements);
}

void TESTthreadpool_perf(requirements& _Requirements)
{
	#ifdef _DEBUG
		oTHROW(permission_denied, "This is slow in debug, and pointless as a benchmark.");
	#else
		TESTthreadpool_performance_impl1<threadpool_impl>(_Requirements);
	#endif
}

	} // namespace tests
} // namespace oConcurrency
