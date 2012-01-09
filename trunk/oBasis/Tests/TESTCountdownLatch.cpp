// $(header)
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oTask.h>
#include <oBasis/oBasis.h>
#include "oBasisTestCommon.h"

static bool TestLatch(int _Count)
{
	int latchCount = _Count;
	int count = 0;
		oCountdownLatch latch("TestLatch", latchCount);
	for (int i = 0; i < latchCount; i++)
		oTaskIssueAsync([&,i]
		{
			oSleep(500);
			latch.Release();
			count++;
		});

	latch.Wait();
	return count == latchCount;
}

bool oBasisTest_oCountdownLatch()
{
	oTESTB(TestLatch(5), "oCountdownLatch failed to wait until the latch was released.");
	oTESTB(TestLatch(1), "oCountdownLatch failed to wait until the latch was released.");
	return true;
}
