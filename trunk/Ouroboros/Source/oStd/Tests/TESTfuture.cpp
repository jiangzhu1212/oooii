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
#include <oBasis/oTimer.h>
#include <oStd/finally.h>
#include <oStd/oFor.h>
#include <oStd/macros.h>
#include <oStd/oStdFuture.h>
#include <oStd/throw.h>
#include <oConcurrency/oConcurrency.h>
#include <oPlatform/oProcessStatsMonitor.h>
#include <oStd/tests/oStdTestRequirements.h>

namespace oStd {
	namespace tests {

static void exercise_thread(size_t _Index, int* _pResults, unsigned int _RuntimeMS)
{
	int n = 0;
	oLocalTimeout to(_RuntimeMS);
	while (!to.TimedOut())
		n += rand();

	_pResults[_Index] = n;
}

static bool exercise_all_threads()
{
	const int nTasks = 5 * oStd::thread::hardware_concurrency(); // ensure more work than the number of threads.
	int* results = (int*)_alloca(nTasks * sizeof(int));
	memset(results, -1, nTasks * sizeof(int));

	oConcurrency::parallel_for(0, size_t(nTasks), oBIND(exercise_thread, oBIND1, results, 1500));
	for (int i = 0; i < nTasks; i++)
		oCHECK(results[i] != -1, "Invalid results from parallel_for");
	return true;
}

static bool fail_and_report()
{
	if (1)
		oTHROW(not_supported, "unsupported call");
	return false;
}

static void test_workstealing(requirements& _Requirements)
{
	float CPUavg = 0.0f, CPUpeak = 0.0f;
	oStd::future<bool> Result = oStd::async(exercise_all_threads);

	oTRACE("Waiting for result...");
	bool r = Result.get();
	oCHECK(r, "future returned, but the algo returned the wrong result");

	_Requirements.get_cpu_utilization(&CPUavg, &CPUpeak);
	if (CPUpeak <= 99.0f)
	{
		float CPUpeak2 = 0.0f;
		_Requirements.reset_cpu_utilization();
		oSleep(10000);
		_Requirements.get_cpu_utilization(&CPUavg, &CPUpeak2);
		if (CPUpeak2 > 5.0f)
			oTHROW(permission_denied, "There is too much CPU activity currently on the system to properly judge oStdFuture's workstealing capabilities.");
		else
			oTHROW(protocol_error, "Failed to achieve 100%s CPU utilization. Peaked at %.01f%s", "%%", CPUpeak, "%%");
	}
}

void TESTfuture(requirements& _Requirements)
{
	// Test packaged_task with void return type
	{
		oStd::packaged_task<void(int, int, char*)> test_no_return([&](int _Param1, int _Param2, char*_Param3){});
		test_no_return(1,2,"t");
		test_no_return.get_future().get();
		
		// Test if reset works
		test_no_return.reset();
		test_no_return(3,4,"t");
		test_no_return.get_future().get();
	}

	// Test packaged_task with a return type
	{
		oStd::packaged_task<int(int, const char*, int)> hmmm([&](int _Param1, const char* _Param2, int _Param3)->int{ return _Param1 + _Param3; });
		// Get future before execution
		oStd::future<int> hmmfuture = hmmm.get_future();
		hmmm(10, "a", 20);
		oCHECK(hmmfuture.get() == 30, "Unexpected result1");

		// Test if reset works
		hmmm.reset();
		hmmm(20, "b", 30);
		// Get future after execution
		hmmfuture = hmmm.get_future();
		oCHECK(hmmfuture.get() == 50, "Unexpected result2");
	}

	// Test swapping packaged_tasks
	{
		oStd::packaged_task<int(int,int)> tasktest1([&](int _Param1, int _Param2)->int{ return _Param1 + _Param2; });
		oStd::packaged_task<int(int,int)> tasktest2([&](int _Param1, int _Param2)->int{ return _Param1 - _Param2; });

		oCHECK(tasktest1.valid() && tasktest2.valid(), "oStd::packaged_task should have been valid");

		tasktest1.swap(tasktest2);

		oStd::future<int> tasktest1_future = tasktest1.get_future();
		tasktest1(20, 10);

		tasktest2(20, 10);
		oStd::future<int> tasktest2_future = tasktest2.get_future();

		// tasktest1 should subtract
		oCHECK(tasktest1_future.get() == 10, "Unexpected result3");

		// tasktest2 should add
		oCHECK(tasktest2_future.get() == 30, "Unexpected result4");
	}

	// Test a packaged_task through async with maximum number of arguments
	// (std::bind is apparently limited to 10)
	{
		oStd::future<bool> Result2 = oStd::async((oFUNCTION<bool(int,int,int,int,int,int,int,int,int,int)>)[&](int _Param1,int _Param2,int _Param3,int _Param4,int _Param5,int _Param6,int _Param7,int _Param8,int _Param9,int _Param10)->bool
		{ 
			if (_Param10 == 10)
				return true; 
			else 
				return false; 
		}, 1,2,3,4,5,6,7,8,9,10);

		oCHECK(Result2.get(), "Unexpected result5");
	}

	// test failure
	{
		oErrorSetLast(0, "No error manually set by test");
		oStd::future<bool> FutureToFail = oStd::async(fail_and_report);

		bool ThisShouldFail = true;
		try { ThisShouldFail = FutureToFail.get(); }
		catch (std::exception& e)
		{
			oErrorSetLast(e);
			oCHECK(oErrorGetLast() == std::errc::not_supported, "last errno_t not properly set");
			oCHECK(!strcmp(oErrorGetLastString(), "unsupported call"), "last error string not properly set");
			ThisShouldFail = false;
		}

		oCHECK(ThisShouldFail == false, "Error reporting failed");
	}

	test_workstealing(_Requirements);
};

	} // namespace tests
} // namespace oConcurrency
