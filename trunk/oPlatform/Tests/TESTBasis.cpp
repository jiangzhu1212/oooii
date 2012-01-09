// $(header)
#include <oPlatform/oTest.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oSystem.h>
#include <oBasisTests/oBasisTests.h>

// @oooii-tony: For some reason oBIND'ing oFileLoad directly doesn't work...
// maybe the templates get confused with the various flavors? Anyway, here's an
// exact wrapper just to change the name of the function.
static bool oFileLoad2(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText)
{
	return oFileLoad(_ppOutBuffer, _pOutSize, _Allocate, _Path, _AsText);
}

static size_t GetTotalPhysicalMemory()
{
	oSYSTEM_HEAP_STATS stats;
	if (!oSystemGetHeapStats(&stats))
		return 0;
	return static_cast<size_t>(stats.TotalPhysical);
}

static void oInitBasisServices(oBasisTestServices* _pServices)
{
	_pServices->ResolvePath = oBIND(&oTestManager::FindFullPath, oTestManager::Singleton(), oBIND1, oBIND2, oBIND3);
	_pServices->AllocateAndLoadBuffer = oBIND(oFileLoad2, oBIND1, oBIND2, malloc, oBIND3, oBIND4);
	_pServices->DeallocateLoadedBuffer = free;
	_pServices->Rand = rand;
	_pServices->GetTotalPhysicalMemory = GetTotalPhysicalMemory;
}

// Tests from oBasis follow a common form, so as a convenience and 
// centralization, use this macro to get through the boilerplate bridging from
// that test call to oTest's infrastructure.
#define oTEST_REGISTER_BASIS_TEST(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(BASIS_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oTESTB(oCONCAT(oBasisTest_, _BasisTestName)(), "%s", oErrorGetLastString()); \
			sprintf_s(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); \
			return SUCCESS; \
		} \
	}; \
	oTEST_REGISTER(oCONCAT(BASIS_, _BasisTestName))

#define oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(BASIS_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oBasisTestServices Services; \
			oInitBasisServices(&Services); \
			oTESTB(oCONCAT(oBasisTest_, _BasisTestName)(Services), "%s", oErrorGetLastString()); \
			sprintf_s(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); \
			return SUCCESS; \
		} \
	}; \
	oTEST_REGISTER(oCONCAT(BASIS_, _BasisTestName))


oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oAllocatorTLSF);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oAtof);
oTEST_REGISTER_BASIS_TEST(oBlockAllocatorFixed);
oTEST_REGISTER_BASIS_TEST(oBlockAllocatorGrowable);
oTEST_REGISTER_BASIS_TEST(oConcurrentIndexAllocator);
oTEST_REGISTER_BASIS_TEST(oConcurrentQueue);
//oTEST_REGISTER_BASIS_TEST(tbbConcurrentQueue);
//oTEST_REGISTER_BASIS_TEST(concrtConcurrentQueue);
oTEST_REGISTER_BASIS_TEST(oConcurrentStack);
oTEST_REGISTER_BASIS_TEST(oCountdownLatch);
oTEST_REGISTER_BASIS_TEST(oCSV);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueConcurrent);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueGlobal);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueParallelFor);
oTEST_REGISTER_BASIS_TEST(oDispatchQueuePrivate);
oTEST_REGISTER_BASIS_TEST(oEightCC);
oTEST_REGISTER_BASIS_TEST(oFilterChain);
oTEST_REGISTER_BASIS_TEST(oFourCC);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oHash);
oTEST_REGISTER_BASIS_TEST(oIndexAllocator);
oTEST_REGISTER_BASIS_TEST(oINI);
oTEST_REGISTER_BASIS_TEST(oLinearAllocator);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oOBJLoader);
oTEST_REGISTER_BASIS_TEST(oOSC);
oTEST_REGISTER_BASIS_TEST(oPath);
oTEST_REGISTER_BASIS_TEST(oURI);
oTEST_REGISTER_BASIS_TEST(oXML);
