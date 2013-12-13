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
#include <oPlatform/oTest.h>
#include <oStd/future.h>
#include <oBase/finally.h>
#include <oCore/windows/win_crt_leak_tracker.h>

#include "../../test_services.h"

using namespace ouro;
using namespace windows::crt_leak_tracker;

namespace ouro {
	namespace tests {

void TESTwin_crt_leak_tracker(test_services& _Services)
{
	#ifdef _DEBUG
		oTRACE("THIS TESTS THE LEAK REPORTING CODE, SO THIS WILL INTENTIONALLY REPORT LEAKS IN THE OUTPUT AS PART OF THAT TEST.");
	
		finally OSEDisableTracking;

		if (!enabled())
		{
			OSEDisableTracking = finally([&] { enable(false); });
			enable(true);
		}

		reset();

		oCHECK(!report(), "Outstanding leaks detected at start of test");

		char* pCharAlloc = new char;
		oCHECK(report(), "Tracker failed to detect char leak");
		delete pCharAlloc;

		oStd::future<void> check = oStd::async([=] {});
		check.wait();
		check = oStd::future<void>(); // zero-out the future because it makes an alloc

		oCHECK(!report(), "Tracker erroneously detected leak from a different thread");

		char* pCharAllocThreaded = nullptr;

		check = oStd::async([&]() { pCharAllocThreaded = new char; });

		check.wait();
		oCHECK(report(), "Tracker failed to detect char leak from different thread");
		delete pCharAllocThreaded;
	#else
		_Services.report("Leak tracker not available in release builds");
	#endif
}

	} // namespace tests
} // namespace ouro
