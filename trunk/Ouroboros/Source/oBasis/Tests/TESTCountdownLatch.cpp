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
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oFor.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oStdFuture.h>
#include <oBasis/oThread.h>
#include "oBasisTestCommon.h"
#include <vector>

static bool TestLatch(int _Count)
{
	int latchCount = _Count;
	int count = 0;
	oMutex Mutex;
	oCountdownLatch latch("TestLatch", latchCount);
	
	// NOTE: This pattern is only for testing purposes of countdown latch. In 
	// applications, it's better to async() a task that will then do other tasks
	// and wait on the future of that first async() call... such as async()'ing a 
	// oTaskParallelFor. Again, this pattern here is to purposefully exacerbate 
	// and test oCountdownLatch, thus sync'ing on the futures wouldn't do much 
	// good.

	std::vector<oStd::future<void> > Futures;

	for (int i = 0; i < _Count; i++)
	{
		oStd::future<void> f = oStd::async([&,i]
		{
			oSleep(100 * (i + 1)); // stagger the sleeps to simulate doing work that takes a variable amount of time
			{
				// Lock before incrementing count because there are several threads may be accessing this at the same time
				oLockGuard<oMutex> Lock(Mutex);
				count++;
			}
			latch.Release();
		});

		Futures.push_back(std::move(f));
	}

	latch.Wait();

	oFOR(oStd::future<void>& f, Futures)
		f.wait();

	return count == latchCount;
}

bool oBasisTest_oCountdownLatch()
{
	oTESTB(TestLatch(5), "oCountdownLatch failed to wait until the latch was released.");
	oTESTB(TestLatch(1), "oCountdownLatch failed to wait until the latch was released.");
	oErrorSetLast(oERROR_NONE, "");
	return true;
}
