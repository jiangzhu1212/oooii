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
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oTask.h>
#include <oBasis/oTimer.h>
#include <oBasis/oStdFuture.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oTest.h>

#include <oPlatform/oProcess.h>
#include <oPlatform/oSystem.h>

void ExerciseThread(size_t _Index, int* _pResults, unsigned int _RuntimeMS)
{
	int n = 0;
	oLocalTimeout to(_RuntimeMS);
	while (!to.TimedOut())
		n += rand();

	_pResults[_Index] = n;
}

bool ExerciseAllThreads()
{
	static const int nTasks = 50; // ensure more work than the number of threads.
	int results[nTasks];
	memset(results, 0, sizeof(results));

	oTaskParallelFor(0, size_t(oCOUNTOF(results)), oBIND(ExerciseThread, oBIND1, results, 500));
	for (size_t i = 0; i < oCOUNTOF(results); i++)
	{
		if (results[i] == oInvalid)
			return oErrorSetLast(oERROR_GENERIC, "Invalid results from thread exec %d", i);
	}

	return true;
}

void MonitorCPUUsage(bool* _pDone, double* _pPeakUsage)
{
	unsigned long long PreviousSystemTime = 0, PreviousProcessTime = 0;
	oProcessCalculateCPUUsage(oProcessGetCurrentID(), &PreviousSystemTime, &PreviousProcessTime);
	while (!*_pDone)
	{
		double usage = oProcessCalculateCPUUsage(oProcessGetCurrentID(), &PreviousSystemTime, &PreviousProcessTime);
		*_pPeakUsage = __max(*_pPeakUsage, usage);
		oSleep(1000);
	}
}

bool oBasisTest_oStdFuture()
{
	bool done = false;
	double peak = 0.0;
	oStd::thread t(MonitorCPUUsage, &done, &peak);
	oOnScopeExit joinThread([&] { done = true; t.join(); } );

	oStd::future<bool> Result = oStd::async(ExerciseAllThreads);

	bool r = false;
	oTRACE("Waiting for result...");
	if (!Result.get(&r))
		return Result.set_last_error();

	if (!r)
		return Result.set_last_error();

	if (peak <= 99.0)
	{
		oSleep(5000);
		if (peak > 5.0)
			return oErrorSetLast(oERROR_REFUSED, "There is too much CPU activity currently on the system to properly judge oStdFuture's workstealing capabilities.");
		else
			return oErrorSetLast(oERROR_GENERIC, "Failed to achieve 100%% CPU utilization. Peaked at %.01f%%", peak);
	}

	oErrorSetLast(oERROR_NONE);
	return true;
};
