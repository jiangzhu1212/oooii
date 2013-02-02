/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include "oDQConcurrent.h"

namespace RatcliffJobSwarm { bool RunDispatchQueueTest(const char* _Name, threadsafe oDispatchQueue* _pDispatchQueue); }

oTest::RESULT TESTDispatchQueueBase::Run(char* _StrStatus, size_t _SizeofStrStatus)
{
	oRef<threadsafe oDispatchQueue> q;
	oTESTB(CreateQueue(100000, &q), "Failed to create dispatch queue");
	oTESTB(RatcliffJobSwarm::RunDispatchQueueTest(q->GetDebugName(), q), "dispatch queue failed");
	oPrintf(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); // pass through benchmark report
	return SUCCESS;
}

static bool IsPrime(size_t n)
{
	bool bPrime = true;
	for (size_t i = 2; i < n; i++)
	{
		if (0 == n % i)
		{
			bPrime = false;
			break;
		}
	}

	return bPrime;
}

static bool RunDispatchQueueTestTrivial(threadsafe oDispatchQueue* _pDispatchQueue, bool Results[1024])
{
	for (size_t i = 0; i < oCOUNTOF(Results); ++i)
	{
		_pDispatchQueue->Dispatch([=]
		{
			Results[i] = IsPrime(i);
		});
	}

	_pDispatchQueue->Join();

	for (size_t i = 0; i < oCOUNTOF(Results); ++i)
	{
		if (IsPrime(i) != Results[i])
			return oErrorSetLast(oERROR_GENERIC, "Parallel results don't match single-threaded results");
	}

	// @oooii-tony: We need something better, or at least more hidden than this...
	// This needs to never be a pattern someone tries to emulate.

	// Asynchronous tasks that may be servicing this queue hold references to the 
	// queue.  This ensures the object has gone out of scope before returning
	while (_pDispatchQueue->Reference() > 2)
	{
		_pDispatchQueue->Release();
	}
	_pDispatchQueue->Release();

	oErrorSetLast(oERROR_NONE);
	return true;
}

oTest::RESULT TESTDispatchQueueTrivialBase::Run(char* _StrStatus, size_t _SizeofStrStatus)
{
	bool Results[1024];
	oRef<threadsafe oDispatchQueue> q;
	oTESTB(CreateQueue(100000, &q), "Failed to create dispatch queue");
	oTESTB(RunDispatchQueueTestTrivial(q, Results), "dispatch queue failed");
	oPrintf(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); // pass through benchmark report
	return SUCCESS;
}
