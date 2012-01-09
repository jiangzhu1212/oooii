// $(header)
#include <oPlatform/oTest.h>
#include <oPlatform/oDispatchQueueConcurrentWTP.h>

namespace RatcliffJobSwarm { bool RunDispatchQueueTest(const char* _Name, threadsafe oDispatchQueue* _pDispatchQueue); }

struct TESTDQConcurrentWTP : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oRef<threadsafe oDispatchQueueConcurrent> q;
		oTESTB(oDispatchQueueCreateConcurrentWTP("Test Concurrent Dispatch Queue WTP", 100000, &q), "Failed to create Windows threadpool dispatch queue");
		oTESTB(RatcliffJobSwarm::RunDispatchQueueTest("oDispatchQueueConcurrentWTP", q), "Windows threadpool dispatch queue failed");
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); // pass through benchmark report
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTDQConcurrentWTP);
