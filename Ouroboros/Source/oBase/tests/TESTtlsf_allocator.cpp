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
#include <oBase/assert.h>
#include <oBase/compiler_config.h>
#include <oBase/fixed_string.h>
#include <oBase/macros.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <oBase/tlsf_allocator.h>
#include <oHLSL/oHLSLMath.h>
#include <vector>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTtlsf_allocator(test_services& _Services)
{
	bool EnoughPhysRamForFullTest = true;

	#if o64BIT == 1
		// Allocating more memory than physically available is possible,
		// but really slowing, so leave some RAM for the operating system.

		// On machines with less memory, it's not a good idea to use all of it
		// because the system would need to page out everything it has to allocate
		// that much memory, which makes the test take many minutes to run.
		size_t ArenaSize = __min(_Services.total_physical_memory() / 2, oMB(4500));
		EnoughPhysRamForFullTest = (ArenaSize > oGB(4));
	#else
		const size_t ArenaSize = oMB(500);
	#endif

	sstring strArenaSize;
	format_bytes(strArenaSize, ArenaSize, 2);
	oTRACE("Allocating %s arena using CRT... (SLOW! OS has to defrag virtual memory to get a linear run of this size)", strArenaSize.c_str());

	#if oENABLE_TRACES
		timer t;
	#endif
	std::vector<char> arena(ArenaSize); // _ArenaSize should be bigger than 32-bit's 4 GB limitation
	oTRACE("Allocation took %.02f seconds", t.seconds());

	tlsf_allocator a(arena.data(), arena.size());

	const size_t NUM_POINTER_TESTS = 1000;
	std::vector<char*> pointers(NUM_POINTER_TESTS);
	memset(pointers.data(), 0, sizeof(char*) * pointers.size());

	size_t totalUsed = 0;
	size_t smallestAlloc = size_t(-1);
	size_t largestAlloc = 0;

	for (size_t numRuns = 0; numRuns < 1; numRuns++)
	{
		totalUsed = 0;

		// allocate some pointers
		for (size_t i = 0; i < NUM_POINTER_TESTS; i++)
		{
			size_t s = 0;
			size_t r = rand();
			size_t limitation = r % 3;
			switch (limitation)
			{
				default:
				case 0:
					s = r; // small
					break;
				case 1:
					s = r * 512; // med
					break;
				case 2:
					s = r * 8 * 1024; // large
					break;
			}

			float percentUsed = round(100.0f * totalUsed/(float)ArenaSize);
			float PercentAboutToBeUsed = round(100.0f * (totalUsed + s)/(float)ArenaSize);

			if (PercentAboutToBeUsed < 97.0f && percentUsed < 97.0f) // TLSF is expected to have ~3% fragmentation
			{
				pointers[i] = (char*)a.allocate(s);
				oCHECK(pointers[i], "Failed on allocate %u of %u bytes (total used %0.1f%%)", i, s, percentUsed);
				totalUsed += s;
				smallestAlloc = __min(smallestAlloc, s);
				largestAlloc = __max(largestAlloc, s);
				oCHECK(a.valid(), "Heap corrupt on allocate %u of %u bytes.", i, s);
			}
		}

		// shuffle pointer order
		for (size_t i = 0; i < NUM_POINTER_TESTS; i++)
		{
			size_t r = rand() % NUM_POINTER_TESTS;
			char* p = pointers[i];
			pointers[i] = pointers[r];
			pointers[r] = p;
		}

		// free out of order
		for (size_t i = 0; i < NUM_POINTER_TESTS; i++)
		{
			a.deallocate(pointers[i]);
			pointers[i] = 0;
			oCHECK(a.valid(), "Heap corrupt on deallocate %u.", i);
		}
	}

	// test really small
	void* p1 = a.allocate(1);
	oCHECK(a.valid(), "Heap corrupt on allocate of %u.", 1);
	void* p2 = a.allocate(0);
	oCHECK(a.valid(), "Heap corrupt on allocate of %u.", 0);
	a.deallocate(p2);
	oCHECK(a.valid(), "Heap corrupt on deallocate");
	a.deallocate(p1);
	oCHECK(a.valid(), "Heap corrupt on deallocate");

	// Fill out statistics and report
	sstring RAMused, MINsize, MAXsize;
	format_bytes(RAMused, ArenaSize, 2);
	format_bytes(MINsize, smallestAlloc, 2);
	format_bytes(MAXsize, largestAlloc, 2);
	
	_Services.report("%sRAMused: %s, minsize:%s, maxsize:%s"
		, EnoughPhysRamForFullTest ? "" : "WARNING: system memory not enough to run full test quickly. "
		, RAMused.c_str(), MINsize.c_str(), MAXsize.c_str());
}
	} // namespace tests
} // namespace ouro

