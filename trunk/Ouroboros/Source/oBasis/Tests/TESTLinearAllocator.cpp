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
#include <oBasis/oLinearAllocator.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oStdFuture.h>
#include <oBasis/oTask.h>
#include <oBasis/oRef.h>
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

static size_t* AllocAndAssign(oLinearAllocator* _pAllocator, int _Int)
{
	size_t* p = (size_t*)_pAllocator->Allocate(1024);
	if (p)
		*p = _Int;
	return p;
}

static bool oBasisTest_oLinearAllocator_Concurrency()
{
	static const size_t nAllocs = 100;

	std::vector<char> buffer(sizeof(oLinearAllocator) + oKB(90), 0);
	oLinearAllocator* pAllocator = reinterpret_cast<oLinearAllocator*>(oGetData(buffer));
	pAllocator->Initialize(buffer.size());

	void* ptr[nAllocs];
	memset(ptr, 0, nAllocs);

	std::vector<oStd::future<size_t*>> FuturePointers;
	
	for (int i = 0; i < nAllocs; i++)
	{
		oStd::future<size_t*> f = oStd::async(AllocAndAssign, pAllocator, i);
		FuturePointers.push_back(std::move(f));
	}

	// Concurrency scheduling makes predicting where the nulls will land 
	// difficult, so just count up the nulls for a total rather than assuming 
	// where they might be.
	size_t nNulls = 0, nFailures = 0;
	for (size_t i = 0; i < nAllocs; i++)
	{
		size_t* p = FuturePointers[i].get();
		if (p)
		{
			if (*p != i)
				nFailures++;
		}
		else
			nNulls++;
	}

	oTESTB(nFailures == 0, "There were %d failures", nFailures);
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
