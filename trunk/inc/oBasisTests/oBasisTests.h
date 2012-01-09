// $(header)
// Declarations of unit tests of components found in oBasis
#pragma once
#ifndef oBasisTests_h
#define oBasisTests_h

#include <oBasis/oFunction.h>
#include <oBasis/oPlatformFeatures.h>

struct oBasisTestServices
{
	oFUNCTION<bool(char* _ResolvedFullPath, size_t _SizeofResolvedFullPath, const char* _RelativePath)> ResolvePath;
	oFUNCTION<bool(void** _ppBuffer, size_t* _pSize, const char* _FullPath, bool _AsText)> AllocateAndLoadBuffer;
	oFUNCTION<void(void* _pBuffer)> DeallocateLoadedBuffer;
	oFUNCTION<int()> Rand;
	oFUNCTION<size_t()> GetTotalPhysicalMemory;
};

// oBasisTests follows a pattern: all functions return true if successful, false
// if not successful. In either case oErrorSetLast is used to set a string as
// to what occurred. In success the last error is set to oERROR_NONE.

// The oFUNCTION parameter should return how much physical RAM the platform has
// because using up the whole physical memory block can be very slow on paging
// operating systems and impossible on non-paging systems, so this tries to be
// a bit smart about running the test.
oAPI bool oBasisTest_oAllocatorTLSF(const oBasisTestServices& _Services);

// This requires a function to be specified that can return a random value on 
// the range [0,1].
oAPI bool oBasisTest_oAtof(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oBuffer(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oBlockAllocatorFixed();
oAPI bool oBasisTest_oBlockAllocatorGrowable();
oAPI bool oBasisTest_oConcurrentStack();
oAPI bool oBasisTest_oCSV();

// This prints a benchmark string to oErrorGetLastString() even if this function
// returns true, so if client code is careful about passing through and doesn't
// have any other errors, the speed of a successful run of this test will be
// reported.
oAPI bool oBasisTest_oDispatchQueueConcurrent();

// Runs the same test as oBasisTest_oDispatchQueueConcurrent for comparison 
// purposes and also to confirm that a platform implementation of 
// oTaskParallelFor performs correctly.
oAPI bool oBasisTest_oDispatchQueueParallelFor();

oAPI bool oBasisTest_oDispatchQueueGlobal();
oAPI bool oBasisTest_oDispatchQueuePrivate();
oAPI bool oBasisTest_oEightCC();
oAPI bool oBasisTest_oFilterChain();
oAPI bool oBasisTest_oFourCC();
oAPI bool oBasisTest_oHash(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oIndexAllocator();
oAPI bool oBasisTest_oINI();
oAPI bool oBasisTest_oLinearAllocator();
oAPI bool oBasisTest_oConcurrentIndexAllocator();
oAPI bool oBasisTest_oMath();
oAPI bool oBasisTest_oOBJLoader(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oOSC();
oAPI bool oBasisTest_oPath();
oAPI bool oBasisTest_tbbConcurrentQueue();
oAPI bool oBasisTest_concrtConcurrentQueue();
oAPI bool oBasisTest_oConcurrentQueue();
oAPI bool oBasisTest_oString();
oAPI bool oBasisTest_oURI();
oAPI bool oBasisTest_oXML();
oAPI bool oBasisTest_oCountdownLatch();

#endif
