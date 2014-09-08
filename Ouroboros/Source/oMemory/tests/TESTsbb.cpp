// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/sbb.h>
#include <oBase/finally.h>
#include <stdlib.h>
#include <vector>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTsbb_trivial(test_services& services)
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
	oTEST0(!ExpectedFailBiggerThanArena);

//   0    0x7fffffffffffffff
//   1    
// 1   1
//1 1 1 1

	void* a = sbb_malloc(sbb, 1);

//   0   0x17ffffffffffffff
//   0
// 0   1
//0 1 1 1

	oTEST0(a == arena);
	void* b = sbb_malloc(sbb, 1);

//   0    0x13ffffffffffffff
//   0    
// 0   1
//0 0 1 1

	oTEST0(b == ((char*)arena + kMinBlockSize));
	void* c = sbb_malloc(sbb, 17);

//   0    0x03ffffffffffffff
//   0    
// 0   0
//0 0 1 1

	oTEST0(c);
	void* ExpectedFailOOM = sbb_malloc(sbb, 1);
	oTEST0(!ExpectedFailOOM);

	sbb_free(sbb, b);

//   0   0x07ffffffffffffff
//   0
// 0   0
//0 1 1 1

	sbb_free(sbb, c);

//   0    0x17ffffffffffffff
//   0    
// 0   1
//0 1 1 1

	sbb_free(sbb, a);

//   0  0x7fffffffffffffff  
//   1    
// 1   1
//1 1 1 1

	void* d = sbb_malloc(sbb, kArenaSize);
	oTEST0(d);
	sbb_free(sbb, d);
}

void TESTsbb(test_services& services)
{
	TESTsbb_trivial(services);

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
	services.report("About to test an invalid case, an exception may be caught by the debugger. CONTINUE.");
	try
	{
		sbb_create(arena, kBadArenaSize, kBadMinBlockSize, bookkeeping);
		ExpectedFailSucceeded = false;
	}

	catch (...) {}
	oTEST0(ExpectedFailSucceeded);

	services.report("About to test an invalid case, an exception may be caught by the debugger. CONTINUE.");
	try
	{
		sbb_create(arena, kArenaSize, kBadMinBlockSize, bookkeeping);
		ExpectedFailSucceeded = false;
	}

	catch (...) {}
	oTEST0(ExpectedFailSucceeded);

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
	oTEST0(FullBlock);
}

	} // namespace tests
} // namespace ouro
