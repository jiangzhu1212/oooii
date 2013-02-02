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
#include <oPlatform/oTest.h>
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oRef.h>
#include "oDQConcurrent.h"

namespace RatcliffJobSwarm { bool RunDispatchQueueTest(const char* _Name, threadsafe oDispatchQueue* _pDispatchQueue); }

struct PLATFORM_DQC_Generic : TESTDispatchQueueBase
{
	bool CreateQueue(size_t _InitialTaskCapacity, threadsafe oDispatchQueue** _ppQueue) override
	{
		return oDispatchQueueCreateConcurrentGeneric("Generic", _InitialTaskCapacity, (threadsafe oDispatchQueueConcurrent**)_ppQueue);
	}
};

struct PLATFORM_DQC_GenericTrivial : TESTDispatchQueueTrivialBase
{
	bool CreateQueue(size_t _InitialTaskCapacity, threadsafe oDispatchQueue** _ppQueue) override
	{
		return oDispatchQueueCreateConcurrentGeneric("Generic", _InitialTaskCapacity, (threadsafe oDispatchQueueConcurrent**)_ppQueue);
	}
};
oTEST_REGISTER(PLATFORM_DQC_GenericTrivial);
oTEST_REGISTER(PLATFORM_DQC_Generic);
