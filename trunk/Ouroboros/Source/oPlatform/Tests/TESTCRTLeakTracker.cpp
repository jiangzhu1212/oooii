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
#include "../oCRTLeakTracker.h"

// @tony: This test has removed so many platform dependencies, it proves 
// that refactoring oCRTLeakTracker into a better location/API may be worthwhile.

// @tony: If this test can be moved to oBasis, unify this with 
// oBasisTestCommon.h (maybe make that more public?) (_P for Platform)
#define oTESTB0_P(test) do { if (!(test)) return false; } while(false) // pass through error
#define oTESTB_P(test, msg, ...) do { if (!(test)) return oErrorSetLast(std::errc::protocol_error, msg, ## __VA_ARGS__); } while(false)

using namespace ouro;
using namespace windows::crt_leak_tracker;

bool TESTcrt_leak_tracker()
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

		oTESTB_P(!report(), "Outstanding leaks detected at start of test");

		char* pCharAlloc = new char;
		oTESTB_P(report(), "Tracker failed to detect char leak");
		delete pCharAlloc;

		oStd::future<void> check = oStd::async([=] {});
		check.wait();
		check = oStd::future<void>(); // zero-out the future because it makes an alloc

		oTESTB_P(!report(), "Tracker erroneously detected leak from a different thread");

		char* pCharAllocThreaded = nullptr;

		check = oStd::async([&]() { pCharAllocThreaded = new char; });

		check.wait();
		oTESTB_P(report(), "Tracker failed to detect char leak from different thread");
		delete pCharAllocThreaded;

		oErrorSetLast(0, "");
		return true;
	#else
		oErrorSetLast(std::errc::permission_denied, "Leak tracker not available in release builds");
		return true;
	#endif
}

struct PLATFORM_oCRTLeakTracker : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTESTB(TESTcrt_leak_tracker(), "%s", oErrorGetLastString());
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oCRTLeakTracker);