// $(header)
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oRef.h>
#include "oBasisTestCommon.h"
#include "RatcliffJobSwarmTest.h"

bool oBasisTest_oDispatchQueueConcurrent()
{
	oRef<threadsafe oDispatchQueueConcurrent> q;
	oTESTB(oDispatchQueueCreateConcurrent("Test Concurrent Dispatch Queue", 100000, &q), "Failed to create concurrent dispatch queue");
	oTESTB(RatcliffJobSwarm::RunDispatchQueueTest("oDispatchQueueConcurrent", q), "concurrent dispatch queue failed");
	//oErrorSetLast(oERROR_NONE); // Allow pass-thru of RunDispatchQueueTest result
	return true;
}
