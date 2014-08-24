/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBase/concurrent_growable_object_pool.h>
#include <oBase/assert.h>
#include <oBase/concurrency.h>
#include <oCore/windows/win_crt_leak_tracker.h>
#include <oBase/throw.h>
#include <vector>

// @tony: hack: 
#include <oBase/finally.h>
#include "../../../Include/oCore/windows/win_crt_leak_tracker.h"

namespace ouro {
	namespace tests {

struct test_obj
{
	test_obj(bool* _pDestroyed)
		: Value(0xc001c0de)
		, pDestroyed(_pDestroyed)
	{
		*pDestroyed = false;
	}

	~test_obj()
	{
		*pDestroyed = true;
	}

	size_t Value;
	bool* pDestroyed;
};

void TESTconcurrent_growable_object_pool()
{
	static const int NumBlocks = 2000;
	static const int NumBlocksPerChunk = 10;
 	concurrent_growable_object_pool<test_obj> Allocator(NumBlocksPerChunk);
	try 
	{
		bool destroyed[NumBlocks];
		memset(destroyed, 0, sizeof(destroyed));

		test_obj* tests[NumBlocks];
		memset(tests, 0xaa, sizeof(tests));

		ouro::windows::crt_leak_tracker::enable(false);
		finally reenable([&] { ouro::windows::crt_leak_tracker::enable(true); });

		ouro::parallel_for(0, NumBlocks, [&](size_t _Index)
		{
			tests[_Index] = Allocator.create(&destroyed[_Index]);
			tests[_Index]->Value = _Index;
		});

		size_t expectedChunks = NumBlocks / NumBlocksPerChunk;
		if ((NumBlocks % NumBlocksPerChunk) != 0)
			expectedChunks++;

		// Because of concurrency, it could be that two or more threads come to the 
		// conclusion that all chunks are exhausted and BOTH decide to create new 
		// chunks. If that happens on the last iteration of the test, then more than
		// expected chunks might be created.
		size_t currentNumChunks = Allocator.num_chunks();
		oCHECK(currentNumChunks >= expectedChunks, "Unexpected number of chunks allocated");

		Allocator.shrink(2);
		oCHECK(Allocator.num_chunks() == currentNumChunks, "Unexpected number of chunks allocated (after false shrink try)"); // shouldn't modify because there is 100% allocation

		oTRACE("ouro::parallel_for { Allocator.destroy }");

		ouro::parallel_for(0, NumBlocks, [&](size_t _Index)
		{
			Allocator.destroy(tests[_Index]);
		});

		Allocator.shrink(2);
		oCHECK(Allocator.num_chunks() == 1, "Unexpected number of chunks allocated (after real shrink)");

		Allocator.grow(5 * Allocator.chunk_capacity());
		oCHECK(Allocator.num_chunks() == 5, "Unexpected number of chunks allocated (after grow)");

		Allocator.shrink(0);
		oCHECK(Allocator.num_chunks() == 0, "Unexpected number of chunks allocated (after 2nd shrink)");

		oTRACE("ouro::parallel_for { if (odd) destroy }");

		ouro::parallel_for(0, NumBlocks, [&](size_t _Index)
		{
			tests[_Index] = Allocator.create(&destroyed[_Index]);
			tests[_Index]->Value = _Index;
			if (_Index & 0x1)
				Allocator.destroy(tests[_Index]);
		});

		oTRACE("ouro::parallel_for { Destroy remaining evens }");

		for (size_t i = 0; i < NumBlocks; i++)
		{
			if (i & 0x1)
				oCHECK(destroyed[i], "Destruction did not occur for allocation %d", i);
			else
			{
				oCHECK(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
				Allocator.destroy(tests[i]);
			}
		}
	}

	catch (std::exception&)
	{
	}
}

	} // namespace test
} // namespace ouro
