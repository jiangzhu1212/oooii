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
#include <oConcurrency/block_allocator.h>
#include <oStd/assert.h>
#include <oStd/throw.h>
#include <vector>

namespace oConcurrency {
	namespace tests {

struct TestObj
{
	TestObj(bool* _pDestroyed)
		: Value(0xc001c0de)
		, pDestroyed(_pDestroyed)
	{
		*pDestroyed = false;
	}

	~TestObj()
	{
		*pDestroyed = true;
	}

	size_t Value;
	bool* pDestroyed;
};

void TESTblock_allocator()
{
	static const int NumBlocks = 2000;
	static const int NumBlocksPerChunk = 10;
	std::unique_ptr<oConcurrency::block_allocator_t<TestObj>> pAllocator(new oConcurrency::block_allocator_t<TestObj>(NumBlocksPerChunk));
	try 
	{
		bool destroyed[NumBlocks];
		memset(destroyed, 0, sizeof(destroyed));

		TestObj* tests[NumBlocks];
		memset(tests, 0xaa, sizeof(tests));

		oConcurrency::parallel_for(0, NumBlocks, [&](size_t _Index)
		{
			tests[_Index] = pAllocator->construct(&destroyed[_Index]);
			tests[_Index]->Value = _Index;
		});

		size_t expectedChunks = NumBlocks / NumBlocksPerChunk;
		if ((NumBlocks % NumBlocksPerChunk) != 0)
			expectedChunks++;

		// Because of concurrency, it could be that two or more threads come to the 
		// conclusion that all chunks are exhausted and BOTH decide to create new 
		// chunks. If that happens on the last iteration of the test, then more than
		// expected chunks might be created.
		size_t currentNumChunks = pAllocator->num_chunks();
		oCHECK(currentNumChunks >= expectedChunks, "Unexpected number of chunks allocated");

		pAllocator->shrink(2);
		oCHECK(pAllocator->num_chunks() == currentNumChunks, "Unexpected number of chunks allocated (after false shrink try)"); // shouldn't modify because there is 100% allocation

		oTRACE("oConcurrency::parallel_for { pAllocator->Destroy }");

		oConcurrency::parallel_for(0, NumBlocks, [&](size_t _Index)
		{
			pAllocator->destroy(tests[_Index]);
		});

		pAllocator->shrink(2);
		oCHECK(pAllocator->num_chunks() == 2, "Unexpected number of chunks allocated (after real shrink)");

		pAllocator->reserve(5 * pAllocator->num_blocks_per_chunk());
		oCHECK(pAllocator->num_chunks() == 5, "Unexpected number of chunks allocated (after grow)");

		pAllocator->shrink(0);
		oCHECK(pAllocator->num_chunks() == 0, "Unexpected number of chunks allocated (after 2nd shrink)");

		oTRACE("oConcurrency::parallel_for { if (odd) Destroy }");

		oConcurrency::parallel_for(0, NumBlocks, [&](size_t _Index)
		{
			tests[_Index] = pAllocator->construct(&destroyed[_Index]);
			tests[_Index]->Value = _Index;
			if (_Index & 0x1)
				pAllocator->destroy(tests[_Index]);
		});

		oTRACE("oConcurrency::parallel_for { Destroy remaining evens }");

		for (size_t i = 0; i < NumBlocks; i++)
		{
			if (i & 0x1)
				oCHECK(destroyed[i], "Destruction did not occur for allocation %d", i);
			else
			{
				oCHECK(tests[i]->Value == i, "Constructor did not occur for allocation %d", i);
				pAllocator->destroy(tests[i]);
			}
		}
	}

	catch (std::exception&)
	{
		pAllocator->clear();
	}
}

	} // namespace test
} // namespace oConcurrency
