// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include <oConcurrency/future.h>
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
		oCHECK(!report(), "Tracker failed to detect char delete");

		ouro::future<void> check = ouro::async([=] {});
		check.wait();
		check = ouro::future<void>(); // zero-out the future because it makes an alloc

		oCHECK(!report(), "Tracker erroneously detected leak from a different thread");

		char* pCharAllocThreaded = nullptr;

		check = ouro::async([&]() { pCharAllocThreaded = new char; });

		check.wait();
		oCHECK(report(), "Tracker failed to detect char leak from different thread");
		delete pCharAllocThreaded;
	#else
		_Services.report("Leak tracker not available in release builds");
	#endif
}

	}
}
