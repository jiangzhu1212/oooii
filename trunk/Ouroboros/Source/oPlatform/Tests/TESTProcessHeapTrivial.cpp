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
#include <oBasis/oBuffer.h>
#include <oBasis/oRef.h>
#include <oBasis/oLockedPointer.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oTest.h>

// {0BC98B60-0F99-4688-A028-F49B0271A237}
static const oGUID GUIDTestBuffer = { 0xbc98b60, 0xf99, 0x4688, { 0xa0, 0x28, 0xf4, 0x9b, 0x2, 0x71, 0xa2, 0x37 } };

struct PLATFORM_oProcessHeapTrivial : public oTest
{
	struct TestStaticContext
	{
		TestStaticContext()
			: Counter(1234)
		{}

		int Counter;

		static void Ctor(void* _Memory) { new (_Memory) TestStaticContext(); }
	};

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		TestStaticContext* c = 0;
		bool allocated = oProcessHeapFindOrAllocate(GUIDTestBuffer, false, true, sizeof(TestStaticContext), TestStaticContext::Ctor, "TestBuffer", (void**)&c);
		oTESTB(allocated && c && c->Counter == 1234, "Failed to construct context");
		c->Counter = 4321;

		allocated = oProcessHeapFindOrAllocate(GUIDTestBuffer, false, true, sizeof(TestStaticContext), TestStaticContext::Ctor, "TestBuffer", (void**)&c);
		oTESTB(!allocated && c && c->Counter == 4321, "Failed to attach context");

		oProcessHeapDeallocate(c);
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oProcessHeapTrivial);
