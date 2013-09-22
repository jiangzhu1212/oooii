/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include "TESTConcurrencyRequirements.h"
#include "oTestIntegration.h"
#include <oConcurrency/tests/oConcurrencyTests.h>

struct oDQC_impl : oConcurrency::tests::test_threadpool
{
	oDQC_impl()
	{
		oVERIFY(oDispatchQueueCreateConcurrent("platform threadpool", 100000, &t));
	}
	ouro::intrusive_ptr<threadsafe oDispatchQueueConcurrent> t;
	~oDQC_impl() { if (t) t->Join(); }
	const char* name() const threadsafe override { return oConcurrency::task_scheduler_name(); }
	void dispatch(const oTASK& _Task) threadsafe override { t->Dispatch(_Task); }
	bool parallel_for(size_t _Begin, size_t _End, const oINDEXED_TASK& _Task) threadsafe override { return false; }
	void flush() threadsafe override { t->Flush(); }
	void release() threadsafe override { t->Join(); t = nullptr; }
};

void TESToDispatchQueueConcurrent(oConcurrency::tests::requirements& _Requirements)
{
	oConcurrency::tests::TESTthreadpool_performance_impl<oDQC_impl>(_Requirements);
}

using namespace oConcurrency::tests; // @oooii-tony: macros should be more explicit but other refactor is going on right now
oTEST_THROWS_REGISTER(oCONCAT(PLATFORM_, oDispatchQueueConcurrent), oCONCAT(TEST, oDispatchQueueConcurrent));
