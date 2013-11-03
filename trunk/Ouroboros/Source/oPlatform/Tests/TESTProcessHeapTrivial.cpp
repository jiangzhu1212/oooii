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
#include <oCore/process_heap.h>
#include "../../test_services.h"

namespace ouro {
	namespace tests {

struct TestStaticContext
{
	TestStaticContext()
		: Counter(1234)
	{}

	int Counter;

	static void Ctor(void* _Memory) { new (_Memory) TestStaticContext(); }
};

void TESTprocess_heap(/*test_services& _Services*/)
{
	TestStaticContext* c = 0;
	bool allocated = process_heap::find_or_allocate(
		sizeof(TestStaticContext)
		, typeid(TestStaticContext).name()
		, process_heap::per_process
		, process_heap::leak_tracked
		, TestStaticContext::Ctor
		, nullptr
		, (void**)&c);
	
	oCHECK(allocated && c && c->Counter == 1234, "Failed to construct context");
	
	c->Counter = 4321;
	allocated = process_heap::find_or_allocate(
		sizeof(TestStaticContext)
		, typeid(TestStaticContext).name()
		, process_heap::per_process
		, process_heap::leak_tracked
		, TestStaticContext::Ctor
		, nullptr
		, (void**)&c);
	oCHECK(!allocated && c && c->Counter == 4321, "Failed to attach context");

	process_heap::deallocate(c);
}

	} // namespace tests
} // namespace ouro

#include <oBasis/oBuffer.h>
#include <oBasis/oLockedPointer.h>
#include <oPlatform/oTest.h>

struct PLATFORM_oProcessHeapTrivial : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		try { ouro::tests::TESTprocess_heap(); }
		catch (std::exception&) { return FAILURE; }
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oProcessHeapTrivial);
