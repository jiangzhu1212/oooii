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
#include <oCore/scheduler.h>
#include <oBasis/oDispatchQueueConcurrent.h>
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
	const char* name() const override { return ouro::scheduler::name(); }
	void dispatch(const oTASK& _Task) override { t->Dispatch(_Task); }
	bool parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task) override { return false; }
	void flush() override { t->Flush(); }
	void release() override { if (t->Joinable()) t->Join(); t = nullptr; }
};

void TESToDispatchQueueConcurrent(ouro::test_services& _Services)
{
	oConcurrency::tests::TESTthreadpool_performance_impl<oDQC_impl>(_Services);
}

using namespace oConcurrency::tests; // @tony: macros should be more explicit but other refactor is going on right now
oTEST_THROWS_REGISTER(oCONCAT(PLATFORM_, oDispatchQueueConcurrent), oCONCAT(TEST, oDispatchQueueConcurrent));
