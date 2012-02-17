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

bool oBasisTest_oBlockAllocatorGrowable()
{
	static const int NumBlocks = 2000;
	static const int NumBlocksPerChunk = 10;
	oBlockAllocatorGrowableT<TestObj>* pAllocator = new oBlockAllocatorGrowableT<TestObj>(NumBlocksPerChunk);
	oOnScopeExit onScopeExit([&] { pAllocator->Reset(); delete pAllocator; });
	
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
			latch.Release();
		});

	latch.Wait();
	size_t expectedChunks = NumBlocks / NumBlocksPerChunk;
	if ((NumBlocks % NumBlocksPerChunk) != 0)
		expectedChunks++;

	// Because of concurrency, it could be that two or more threads come to the 
	// conclusion that all chunks are exhausted and BOTH decide to create new 
	// chunks. If that happens on the last iteration of the test, then more than
	// expected chunks might be created.
	size_t currentNumChunks = pAllocator->GetNumChunks();
	oTESTB(currentNumChunks >= expectedChunks, "Unexpected number of chunks allocated");

	pAllocator->Shrink(2);
	oTESTB(pAllocator->GetNumChunks() == currentNumChunks, "Unexpected number of chunks allocated (after false shrink try)"); // shouldn't modify because there is 100% allocation

	latch.Reset(NumBlocks);
	for (int i = 0; i < NumBlocks; i++)
	#ifdef oBug_1938
		oTaskIssueSerial([&,i]
	#else
		oTaskIssueAsync([&,i]
	#endif
		{
			pAllocator->Deallocate(tests[i]);
			latch.Release();
		});

	latch.Wait();

	pAllocator->Shrink(2);
	oTESTB(pAllocator->GetNumChunks() == 2, "Unexpected number of chunks allocated (after real shrink)");

	pAllocator->Grow(5);
	oTESTB(pAllocator->GetNumChunks() == 5, "Unexpected number of chunks allocated (after grow)");

	pAllocator->Shrink();
	oTESTB(pAllocator->GetNumChunks() == 0, "Unexpected number of chunks allocated (after 2nd shrink)");

	latch.Reset(NumBlocks);
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
				pAllocator->Destroy(tests[i]);

			latch.Release();
		});

	latch.Wait();
	
	for (int i = 0; i < NumBlocks; i++)
	{
		if (i & 0x1)
			oTESTB(destructed[i], "Destruction did not occur for allocation %d", i);
		else
		{
			oTESTB(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
			pAllocator->Destroy(tests[i]);
		}
	}

	oBug_1938_EXIT();
	oErrorSetLast(oERROR_NONE, "");
	return true;
}
