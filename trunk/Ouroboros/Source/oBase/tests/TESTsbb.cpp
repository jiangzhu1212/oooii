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
#include <oBase/sbb.h>
#include <oBase/finally.h>
#include <oBase/throw.h>
#include <stdlib.h>
#include <vector>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTsbb_trivial()
{
	const size_t kArenaSize = 64;
	const size_t kMinBlockSize = 16;
	const size_t kBookkeepingSize = sbb_bookkeeping_size(kArenaSize, kMinBlockSize);
	
	char* bookkeeping = new char[kBookkeepingSize];
	finally FreeBookkeeping([&] { if (bookkeeping) delete [] bookkeeping; });

	char* arena = new char[kArenaSize];
	finally FreeArena([&] { if (arena) delete [] arena; });

	sbb_t sbb = sbb_create(arena, kArenaSize, kMinBlockSize, bookkeeping);
	finally DestroySbb([&] { if (sbb) sbb_destroy(sbb); });

	void* ExpectedFailBiggerThanArena = sbb_malloc(sbb, 65);
	oCHECK0(!ExpectedFailBiggerThanArena);

//   0    011111111111...
//   1    0x7fffffffffffffff
// 1   1
//1 1 1 1

	void* a = sbb_malloc(sbb, 1);

//   0    011101111111...
//   1    0x77ffffffffffffff
// 1   1
//0 1 1 1

	oCHECK0(a);
	void* b = sbb_malloc(sbb, 1);

//   0    010100111111...
//   1    0x53ffffffffffffff
// 0   1
//0 0 1 1

	oCHECK0(b);
	void* c = sbb_malloc(sbb, 17);

//   0    000000111111...
//   0    0x03ffffffffffffff
// 0   0
//0 0 1 1

	oCHECK0(c);
	void* ExpectedFailOOM = sbb_malloc(sbb, 1);
	oCHECK0(!ExpectedFailOOM);

	sbb_free(sbb, b);
//   0    011001111111...
//   1    0x67ffffffffffffff
// 1   0
//0 1 1 1

	sbb_free(sbb, c);
//   0    011101111111...
//   1    0x77ffffffffffffff
// 1   1
//0 1 1 1

	sbb_free(sbb, a);
//   0    011011111111...
//   1    0x7fffffffffffffff
// 1   1
//1 1 1 1

	void* d = sbb_malloc(sbb, kArenaSize);
	oCHECK0(d);
	sbb_free(sbb, d);
}

void TESTsbb(test_services& services)
{
	TESTsbb_trivial();

	const size_t kBadArenaSize = 123445;
	const size_t kBadMinBlockSize = 7;
	const size_t kArenaSize = 512 * 1024 * 1024;
	const size_t kMinBlockSize = 16;
	const size_t kMaxAllocSize = 10 * 1024 * 1024;

	const size_t kBookkeepingSize = sbb_bookkeeping_size(kArenaSize, kMinBlockSize);

	char* bookkeeping = new char[kBookkeepingSize];
	finally FreeBookkeeping([&] { if (bookkeeping) delete [] bookkeeping; });

	char* arena = new char[kArenaSize];
	finally FreeArena([&] { if (arena) delete [] arena; });

	bool ExpectedFailSucceeded = true;
	try
	{
		sbb_create(arena, kBadArenaSize, kBadMinBlockSize, bookkeeping);
		ExpectedFailSucceeded = false;
	}

	catch (...) {}
	oCHECK0(ExpectedFailSucceeded);

	try
	{
		sbb_create(arena, kArenaSize, kBadMinBlockSize, bookkeeping);
		ExpectedFailSucceeded = false;
	}

	catch (...) {}
	oCHECK0(ExpectedFailSucceeded);

	sbb_t sbb = sbb_create(arena, kArenaSize, kMinBlockSize, bookkeeping);
	finally DestroySbb([&] { if (sbb) sbb_destroy(sbb); });

	static const size_t kNumIterations = 1000;

	std::vector<void*> pointers(kNumIterations);
	std::fill(std::begin(pointers), std::end(pointers), nullptr);
	for (size_t i = 0; i < kNumIterations; i++)
	{
		const size_t r = services.rand();
		const size_t amt = __min(kMaxAllocSize, r);
		pointers[i] = sbb_malloc(sbb, amt);
	}

	const size_t count = services.rand() % kNumIterations;
	for (size_t i = 0; i < count; i++)
	{
		const size_t j = services.rand() % kNumIterations;
		sbb_free(sbb, pointers[j]);
		pointers[j] = nullptr;
	}

	for (size_t i = 0; i < kNumIterations; i++)
	{
		if (pointers[i])
			continue;

		const size_t r = services.rand();
		const size_t amt = __min(kMaxAllocSize, r);
		pointers[i] = sbb_malloc(sbb, amt);
	}

	for (size_t i = 0; i < kNumIterations; i++)
	{
		sbb_free(sbb, pointers[i]);
	}

	void* FullBlock = sbb_malloc(sbb, kArenaSize);
	oCHECK0(FullBlock);
}

	} // namespace tests
} // namespace ouro
