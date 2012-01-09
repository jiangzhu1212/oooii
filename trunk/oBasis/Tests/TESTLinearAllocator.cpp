// $(header)
#include <oBasis/oLinearAllocator.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oTask.h>
#include "oBasisTestCommon.h"
#include <vector>

static bool oBasisTest_oLinearAllocator_Allocate()
{
	std::vector<char> buffer(1024, 0xcc);
	oLinearAllocator* pAllocator = reinterpret_cast<oLinearAllocator*>(oGetData(buffer));
	pAllocator->Initialize(buffer.size());

	static const size_t kAllocSize = 30;

	char* c1 = pAllocator->Allocate<char>(kAllocSize);
	oTESTB(c1, "Allocation failed (1)");
	char* c2 = pAllocator->Allocate<char>(kAllocSize);
	oTESTB(c2, "Allocation failed (2)");
	char* c3 = pAllocator->Allocate<char>(kAllocSize);
	oTESTB(c3, "Allocation failed (3)");
	char* c4 = pAllocator->Allocate<char>(kAllocSize);
	oTESTB(c4, "Allocation failed (4)");

	memset(c1, 0, kAllocSize);
	memset(c3, 0, kAllocSize);
	oTESTB(!memcmp(c2, c4, kAllocSize), "Allocation failed (5)");

	char* c5 = pAllocator->Allocate<char>(1024);
	oTESTB(!c5, "Too large an allocation should have failed, but succeeded");

	size_t nFree = 1024 - oByteAlign(sizeof(oLinearAllocator), oDEFAULT_MEMORY_ALIGNMENT);
	nFree -= 4 * oByteAlign(kAllocSize, oDEFAULT_MEMORY_ALIGNMENT);

	oTESTB(pAllocator->GetBytesAvailable() == nFree, "Bytes available is incorrect");

	pAllocator->Reset();

	char* c6 = pAllocator->Allocate<char>(880);
	oTESTB(c6, "Should've been able to allocate a large allocation after reset");

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

static bool oBasisTest_oLinearAllocator_Concurrency()
{
	static const size_t nAllocs = 100;

	std::vector<char> buffer(sizeof(oLinearAllocator) + oKB(90), 0);
	oLinearAllocator* pAllocator = reinterpret_cast<oLinearAllocator*>(oGetData(buffer));
	pAllocator->Initialize(buffer.size());

	void* ptr[nAllocs];
	memset(ptr, 0, nAllocs);

	oCountdownLatch latch("Sync", nAllocs);
	
	for (size_t i = 0; i < nAllocs; i++)
	oTaskIssueAsync([&,i]
	{
		ptr[i] = pAllocator->Allocate(1024);
		if (ptr[i])
			*(size_t*)ptr[i] = i;
		latch.Release();
	});

	latch.Wait();

	// Concurrency scheduling makes predicting where the nulls will land 
	// difficult, so just count up the nulls for a total rather than assuming 
	// where they might be.
	size_t nNulls = 0;
	for (size_t i = 0; i < nAllocs; i++)
	{
		if (ptr[i])
			oTESTB(*(size_t*)ptr[i] == i, "Async allocation failed (%u)", i);
		else
			nNulls++;
	}

	oTESTB(nNulls == 10, "There should have been 10 failed allocations");

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

bool oBasisTest_oLinearAllocator()
{
	if (!oBasisTest_oLinearAllocator_Allocate())
		return false;

	if (!oBasisTest_oLinearAllocator_Concurrency())
		return false;

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
