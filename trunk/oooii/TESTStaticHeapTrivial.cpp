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
#include "pch.h"
#include <oooii/oRef.h>
#include <oooii/oBuffer.h>
#include <oooii/oHeap.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oTest.h>

struct TESTStaticHeapTrivial : public oTest
{
	struct TestStaticContext
	{
		TestStaticContext()
			: Counter(1234)
		{}

		int Counter;
	};

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		const char* NAME = "TestBuffer";
		TestStaticContext* c = oHeap::StaticConstructShared<TestStaticContext>(NAME, false);
		oTESTB(c && c->Counter == 1234, "Failed to construct context");
		c->Counter = 4321;

		c = oHeap::StaticConstructShared<TestStaticContext>(NAME, false);
		oTESTB(c && c->Counter == 4321, "Failed to attach context");

		oHeap::StaticDeallocateShared(c);
		oHeap::StaticDeallocateShared(c);
		return SUCCESS;
	}
};

TESTStaticHeapTrivial TestStaticHeapTrivial;
