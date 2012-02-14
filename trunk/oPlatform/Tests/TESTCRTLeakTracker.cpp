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
#include <oPlatform\oTest.h>
#include <oPlatform\oEvent.h>
#include <oBasis\oTask.h>
#include "..\oCRTLeakTracker.h"

struct TESTCRTLeakTracker : public oTest
{
	oEvent AllocEvent;

	// Test is wrapped in RunTest so we can properly unwind on failure
	RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus)
	{
#ifdef _DEBUG
		oCRTLeakTracker* pTracker = oCRTLeakTracker::Singleton();
		oTESTB(!pTracker->ReportLeaks(), "Outstanding leaks detected at start of test");

		char* pCharAlloc = new char;
		oTESTB(pTracker->ReportLeaks(), "Tracker failed to detect char leak");
		delete pCharAlloc;

		oTaskIssueAsync(
			[&]()
		{
			AllocEvent.Set();
		});
		AllocEvent.Wait();
		oTESTB(!pTracker->ReportLeaks(), "Tracker erroneously detected leak from a different thread");
		AllocEvent.Reset();

		char* pCharAllocThreaded = nullptr;

		oTaskIssueAsync(
			[&]()
			{
				pCharAllocThreaded = new char;
				AllocEvent.Set();
			});
		AllocEvent.Wait();
		oTESTB(pTracker->ReportLeaks(), "Tracker failed to detect char leak from different thread");
		delete pCharAllocThreaded;
#else
		sprintf_s(_StrStatus, _SizeofStrStatus, "Not available in Release");
#endif
		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oCRTLeakTracker* pTracker = oCRTLeakTracker::Singleton();
		bool HadToEnable = !pTracker->IsEnabled();

		if(HadToEnable)
			pTracker->Enable( true );

		pTracker->Reset();

		RESULT res = RunTest(_StrStatus, _SizeofStrStatus);

		if(HadToEnable)
			pTracker->Enable(false);

		return res;
	}
};

oTEST_REGISTER(TESTCRTLeakTracker);