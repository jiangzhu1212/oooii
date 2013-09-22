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
#include <oConcurrency/concurrent_linear_allocator.h>
#include <oBase/byte.h>
#include <oBase/macros.h>
#include <oStd/future.h>
#include <oBase/throw.h>
#include <vector>

using namespace ouro;

namespace oConcurrency {
	namespace tests {

static void test_basics()
{
	std::vector<char> buffer(1024, 0xcc);
	oConcurrency::concurrent_linear_allocator* pAllocator = reinterpret_cast<oConcurrency::concurrent_linear_allocator*>(&buffer[0]);
	pAllocator->initialize(buffer.size());

	static const size_t kAllocSize = 30;

	char* c1 = pAllocator->allocate<char>(kAllocSize);
	oCHECK(c1, "Allocation failed (1)");
	char* c2 = pAllocator->allocate<char>(kAllocSize);
	oCHECK(c2, "Allocation failed (2)");
	char* c3 = pAllocator->allocate<char>(kAllocSize);
	oCHECK(c3, "Allocation failed (3)");
	char* c4 = pAllocator->allocate<char>(kAllocSize);
	oCHECK(c4, "Allocation failed (4)");

	memset(c1, 0, kAllocSize);
	memset(c3, 0, kAllocSize);
	oCHECK(!memcmp(c2, c4, kAllocSize), "Allocation failed (5)");

	char* c5 = pAllocator->allocate<char>(1024);
	oCHECK(!c5, "Too large an allocation should have failed, but succeeded");

	size_t nFree = 1024 - byte_align(sizeof(oConcurrency::concurrent_linear_allocator), oDEFAULT_MEMORY_ALIGNMENT);
	nFree -= 4 * byte_align(kAllocSize, oDEFAULT_MEMORY_ALIGNMENT);

	oCHECK(pAllocator->bytes_available() == nFree, "Bytes available is incorrect");

	pAllocator->reset();

	char* c6 = pAllocator->allocate<char>(880);
	oCHECK(c6, "Should've been able to allocate a large allocation after reset");
}

static size_t* AllocAndAssign(oConcurrency::concurrent_linear_allocator* _pAllocator, int _Int)
{
	size_t* p = (size_t*)_pAllocator->allocate(1024);
	if (p)
		*p = _Int;
	return p;
}

static void test_concurrency()
{
	static const size_t nAllocs = 100;

	std::vector<char> buffer(sizeof(oConcurrency::concurrent_linear_allocator) + oKB(90), 0);
	oConcurrency::concurrent_linear_allocator* pAllocator = reinterpret_cast<oConcurrency::concurrent_linear_allocator*>(&buffer[0]);
	pAllocator->initialize(buffer.size());

	void* ptr[nAllocs];
	memset(ptr, 0, nAllocs);

	std::vector<oStd::future<size_t*>> FuturePointers;
	
	for (int i = 0; i < nAllocs; i++)
	{
		oStd::future<size_t*> f = oStd::async(AllocAndAssign, pAllocator, i);
		FuturePointers.push_back(std::move(f));
	}

	// Concurrency scheduling makes predicting where the nulls will land 
	// difficult, so just count up the nulls for a total rather than assuming 
	// where they might be.
	size_t nNulls = 0, nFailures = 0;
	for (size_t i = 0; i < nAllocs; i++)
	{
		size_t* p = FuturePointers[i].get();
		if (p)
		{
			if (*p != i)
				nFailures++;
		}
		else
			nNulls++;
	}

	oCHECK(nFailures == 0, "There were %d failures", nFailures);
	oCHECK(nNulls == 10, "There should have been 10 failed allocations");
}

void TESTconcurrent_linear_allocator()
{
	test_basics();
	test_concurrency();
}

	} // namespace tests
} // namespace oConcurrency
