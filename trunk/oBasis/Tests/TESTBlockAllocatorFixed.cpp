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
#include <oBasis/oBlockAllocatorGrowable.h>
#include "oBasisTestCommon.h"
#include <oBasis/oAlgorithm.h>
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oMemory.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oTask.h>
#include <vector>

struct TestObj
{
	TestObj(bool* _pDestructed)
		: Value(0xc001c0de)
		, pDestructed(_pDestructed)
	{
		*pDestructed = false;
	}

	~TestObj()
	{
		*pDestructed = true;
	}

	int Value;
	bool* pDestructed;
};

static bool oBasisTest_oBlockAllocatorFixed_Allocate()
{
	static const size_t NumBlocks = 20;
	static const size_t BlockSize = sizeof(TestObj);
	std::vector<char> scopedArena(oBlockAllocatorFixed::CalculateRequiredSize(BlockSize, NumBlocks));
	oBlockAllocatorFixed* pAllocator = reinterpret_cast<oBlockAllocatorFixed*>(oGetData(scopedArena));
	pAllocator->Initialize(BlockSize, NumBlocks);
	oTESTB(NumBlocks == pAllocator->CountAvailable(BlockSize), "There should be %u available blocks (after init)", NumBlocks);

	void* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = pAllocator->Allocate(BlockSize);
		oTESTB(tests[i], "TestObj %u should have been allocated", i);
	}

	void* shouldBeNull = pAllocator->Allocate(BlockSize);
	oTESTB(!shouldBeNull, "Allocation should have failed");
	oTESTB(0 == pAllocator->CountAvailable(BlockSize), "There should be 0 available blocks");

	for (size_t i = 0; i < NumBlocks; i++)
		pAllocator->Deallocate(BlockSize, NumBlocks, tests[i]);

	oTESTB(NumBlocks == pAllocator->CountAvailable(BlockSize), "There should be %u available blocks (after deallocate)", NumBlocks);
	
	oErrorSetLast(oERROR_NONE, "");
	return true;
}

static bool oBasisTest_oBlockAllocatorFixed_Create()
{
	static const size_t NumBlocks = 20;
	std::vector<char> scopedArena(oBlockAllocatorFixedT<TestObj>::CalculateRequiredSize(NumBlocks));
	oBlockAllocatorFixedT<TestObj>* pAllocator = reinterpret_cast<oBlockAllocatorFixedT<TestObj>*>(oGetData(scopedArena));
	pAllocator->Initialize(NumBlocks);

	bool testDestructed[NumBlocks];
	TestObj* tests[NumBlocks];
	for (size_t i = 0; i < NumBlocks; i++)
	{
		tests[i] = pAllocator->Create(&testDestructed[i]);
		oTESTB(tests[i], "TestObj %u should have been allocated", i);
		oTESTB(tests[i]->Value == 0xc001c0de, "TestObj %u should have been constructed", i);
		oTESTB(tests[i]->pDestructed && false == *tests[i]->pDestructed, "TestObj %u should have been constructed", i);
	}

	for (size_t i = 0; i < NumBlocks; i++)
	{
		pAllocator->Destroy(NumBlocks, tests[i]);
		oTESTB(testDestructed[i] == true, "TestObj %u should have been destroyed", i);
	}

	oTESTB(NumBlocks == pAllocator->CountAvailable(), "There should be %u available blocks (after deallocate)", NumBlocks);

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

static bool oBasisTest_oBlockAllocatorFixed_Concurrency()
{
	static const int NumBlocks = 10000;
	std::vector<char> scopedArena(oBlockAllocatorFixedT<TestObj>::CalculateRequiredSize(NumBlocks));
	oBlockAllocatorFixedT<TestObj>* pAllocator = reinterpret_cast<oBlockAllocatorFixedT<TestObj>*>(oGetData(scopedArena));
	pAllocator->Initialize(NumBlocks);

	bool destructed[NumBlocks];
	memset(destructed, 0, sizeof(destructed));

	TestObj* tests[NumBlocks];
	oMemset4(tests, 0xdeadc0de, sizeof(tests));

	oCountdownLatch latch("Sync", NumBlocks);

	for (int i = 0; i < NumBlocks; i++)
#ifdef oBug_1938
		oTaskIssueSerial([&,i]
#else
		oTaskIssueAsync([&,i]
#endif
		{
			tests[i] = pAllocator->Create(&destructed[i]);
			tests[i]->Value = i;
			if (i & 0x1)
				pAllocator->Destroy(NumBlocks, tests[i]);

			latch.Release();
		});

	latch.Wait();

	oTESTB((NumBlocks/2) == pAllocator->CountAvailable(), "Allocation/Destroys did not occur correctly");

	for (int i = 0; i < NumBlocks; i++)
	{
		if (i & 0x1)
			oTESTB(destructed[i], "Destruction did not occur for allocation %d", i);
		else
			oTESTB(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
	}
	
	oErrorSetLast(oERROR_NONE, "");
	return true;
}

bool oBasisTest_oBlockAllocatorFixed()
{
	if (!oBasisTest_oBlockAllocatorFixed_Allocate())
		return false;

	if (!oBasisTest_oBlockAllocatorFixed_Create())
		return false;

	if (!oBasisTest_oBlockAllocatorFixed_Concurrency())
		return false;
	oBug_1938_EXIT();
	oErrorSetLast(oERROR_NONE, "");
	return true;
}
