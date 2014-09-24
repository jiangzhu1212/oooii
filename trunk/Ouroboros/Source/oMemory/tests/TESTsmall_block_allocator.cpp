// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/small_block_allocator.h>
#include <vector>
#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTsmall_block_allocator(test_services& services)
{
	uint16_t sBlockSizes[] = { 8, 16, 64, 256, 8192 };
	uint32_t size = 4;
	void* mem = nullptr;

	small_block_allocator a;

	{
		bool ExpectedFail = false;
		services.report("Expecting failure on invalid argument...");
		try
		{
			a.initialize(mem, size, sBlockSizes, oCOUNTOF(sBlockSizes));
		}
		catch (std::invalid_argument&)
		{
			ExpectedFail = true;
		}

		oTEST(ExpectedFail, "initialize should have thrown on invalid argument");
	}

	std::vector<char> tmp((size+1) * small_block_allocator::chunk_size);
	mem = byte_align(tmp.data(), small_block_allocator::chunk_size);

	{
		bool ExpectedFail = false;
		services.report("Expecting failure on invalid argument...");
		try
		{
			a.initialize(mem, size, sBlockSizes, oCOUNTOF(sBlockSizes));
		}
		catch (std::invalid_argument&)
		{
			ExpectedFail = true;
		}

		oTEST(ExpectedFail, "initialize should have thrown on invalid argument");
	}
	
	size *= small_block_allocator::chunk_size;
	a.initialize(mem, size, sBlockSizes, oCOUNTOF(sBlockSizes));

	void* test = a.allocate(13);
	oTEST(!test, "allocated unspecified block size");

	void* tests[16];

	for (int i = 0; i < oCOUNTOF(tests); i++)
		tests[i] = a.allocate(8192);

	for (int i = 0; i < 12; i++)
		oTEST(tests[i], "expected a valid alloc");

	for (int i = 12; i < oCOUNTOF(tests); i++)
		oTEST(!tests[i], "expected a nullptr");

	test = a.allocate(64);
	oTEST(!test, "expected a nullptr because allocator is full");

	for (int i = 0; i < 3; i++)
	{
		a.deallocate(tests[i]);
		tests[i] = nullptr;
	}

	// that should've freed a chunk
	test = a.allocate(64);
	oTEST(test, "smaller block should have allocated");
	a.deallocate(test);

	// that should've freed a chunk
	test = a.allocate(16);
	oTEST(test, "different block should have allocated");

	for (int i = 6; i < 9; i++)
	{
		a.deallocate(tests[i]);
		tests[i] = nullptr;
	}

	// that should've freed a chunk
	void* test2 = a.allocate(64);
	oTEST(test2, "should have a chunk ready for a new block size");

	a.deallocate(test2);
	a.deallocate(test);

	for (int i = 0; i < oCOUNTOF(tests); i++)
		a.deallocate(tests[i]);

	a.deinitialize();
	services.report("");
}

	}
}
