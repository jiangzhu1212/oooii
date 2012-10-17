/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oBasis/oTask.h>
#include <oBasis/oStdFuture.h>
#include "../oCRTLeakTracker.h"

// @oooii-tony: This test has removed so many platform dependencies, it proves 
// that refactoring oCRTLeakTracker into a better location/API may be worthwhile.

// @oooii-tony: If this test can be moved to oBasis, unify this with 
// oBasisTestCommon.h (maybe make that more public?) (_P for Platform)
#define oTESTB0_P(test) do { if (!(test)) return false; } while(false) // pass through error
#define oTESTB_P(test, msg, ...) do { if (!(test)) return oErrorSetLast(oERROR_GENERIC, msg, ## __VA_ARGS__); } while(false)

bool oPlatformTest_oCRTLeakTracker()
{
	#ifdef _DEBUG
		oTRACE("THIS TESTS THE LEAK REPORTING CODE, SO THIS WILL INTENTIONALLY REPORT LEAKS IN THE OUTPUT AS PART OF THAT TEST.");
	
		oCRTLeakTracker* pTracker = oCRTLeakTracker::Singleton();

		oOnScopeExit OSEDisableTracking;

		if (!pTracker->IsEnabled())
		{
			OSEDisableTracking = oOnScopeExit([&] { pTracker->Enable(false); });
			pTracker->Enable(true);
		}

		pTracker->Reset();

		oTESTB_P(!pTracker->ReportLeaks(), "Outstanding leaks detected at start of test");

		char* pCharAlloc = new char;
		oTESTB_P(pTracker->ReportLeaks(), "Tracker failed to detect char leak");
		delete pCharAlloc;

		oStd::future<void> check = oStd::async([=] {});
		check.wait();
		check = oStd::future<void>(); // zero-out the future because it makes an alloc

		oTESTB_P(!pTracker->ReportLeaks(), "Tracker erroneously detected leak from a different thread");

		char* pCharAllocThreaded = nullptr;

		check = oStd::async([&]() { pCharAllocThreaded = new char; });

		check.wait();
		oTESTB_P(pTracker->ReportLeaks(), "Tracker failed to detect char leak from different thread");
		delete pCharAllocThreaded;

		oErrorSetLast(oERROR_NONE);
		return true;
	#else
		oErrorSetLast(oERROR_REFUSED, "Leak tracker not available in release builds");
		return true;
	#endif
}

struct TESTCRTLeakTracker : public oTest
{
	// Test is wrapped in RunTest so we can properly unwind on failure
	RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus)
	{
		#ifdef _DEBUG
			oCRTLeakTracker* pTracker = oCRTLeakTracker::Singleton();
			oTESTB(!pTracker->ReportLeaks(), "Outstanding leaks detected at start of test");

			char* pCharAlloc = new char;
			oTESTB(pTracker->ReportLeaks(), "Tracker failed to detect char leak");
			delete pCharAlloc;

			oStd::future<void> check = oStd::async([=] {});
			check.wait();
			check = oStd::future<void>(); // zero-out the future because it makes an alloc
		
			oTESTB(!pTracker->ReportLeaks(), "Tracker erroneously detected leak from a different thread");

			char* pCharAllocThreaded = nullptr;

			check = oStd::async([&]() { pCharAllocThreaded = new char; });

			check.wait();
			oTESTB(pTracker->ReportLeaks(), "Tracker failed to detect char leak from different thread");
			delete pCharAllocThreaded;
		#else
			oPrintf(_StrStatus, _SizeofStrStatus, "Not available in Release");
		#endif
		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTESTB(oPlatformTest_oCRTLeakTracker(), "%s", oErrorGetLastString());
		oPrintf(_StrStatus, _SizeofStrStatus, "%s: %s", oAsString(oErrorGetLast()), oErrorGetLastString());
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTCRTLeakTracker);