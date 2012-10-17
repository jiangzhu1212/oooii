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
