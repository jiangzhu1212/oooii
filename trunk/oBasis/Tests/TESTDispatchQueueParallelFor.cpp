// $(header)
#include <oBasis/oTask.h>
#include "oBasisTestCommon.h"
#include "RatcliffJobSwarmTest.h"

bool oBasisTest_oDispatchQueueParallelFor()
{
	oTESTB(RatcliffJobSwarm::RunParallelForTest("oDispatchQueueParallelFor"), "Ratcliff parallel for test failed");
	//oErrorSetLast(oERROR_NONE); // Allow pass-thru of RunDispatchQueueTest result
	return true;
}
