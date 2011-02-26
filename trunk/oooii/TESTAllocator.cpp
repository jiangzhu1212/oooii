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
#include <oooii/oAllocatorTLSF.h>
#include <oooii/oHeap.h>
#include <oooii/oMath.h>
#include <oooii/oRef.h>
#include <oooii/oTest.h>
#include <oooii/oWindows.h>

struct TESTAllocator : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		bool EnoughPhysRamForFullTest = false;
		#ifdef _M_X64
			// Allocating more memory than physically available is possible,
			// but really slowing, so leave some RAM for Windows.
			oHeap::GLOBAL_HEAP_DESC globalHeapDesc;
			oHeap::GetGlobalDesc(&globalHeapDesc);

			// On machines with less memory, it's not a good idea to use all of it
			// because the system would need to page out everything it has to allocate
			// that much memory, which makes the test take many minutes to run.
			const size_t SIZE = __min(globalHeapDesc.TotalPhysical / 2, oGB(6));
			EnoughPhysRamForFullTest = (SIZE <= oGB(4));
		#else
			const size_t SIZE = 500 * 1024 * 1024; // 500 MB
		#endif

		oTestScopedArray<char> arena(SIZE); // bigger than 32-bit's 4 GB limitation

		oAllocator::DESC desc;
		desc.ArenaSize = arena.GetCount();
		desc.pArena = arena.GetPointer();

		oRef<oAllocator> Allocator;
		oTESTB(oAllocatorTLSF::Create("TestAllocator", &desc, &Allocator), "Failed to create a TLSF allocator");

		const size_t NUM_POINTER_TESTS = 1000;
		oTestScopedArray<char*> pointers(NUM_POINTER_TESTS);
		memset(pointers.GetPointer(), 0, sizeof(char*) * pointers.GetCount());
		size_t totalUsed = 0;
		size_t smallestAlloc = ~0u;
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

				float percentUsed = round(100.0f * totalUsed/(float)SIZE);
				float PercentAboutToBeUsed = round(100.0f * (totalUsed + s)/(float)SIZE);

				if (PercentAboutToBeUsed < 97.0f && percentUsed < 97.0f) // TLSF is expected to have ~3% fragmentation
				{
					pointers[i] = (char*)Allocator->Allocate(s);
					oTESTB(pointers[i], "Failed on Allocate %u of %u bytes (total used %0.1f%%)", i, s, percentUsed);
					totalUsed += s;
					smallestAlloc = __min(smallestAlloc, s);
					largestAlloc = __max(largestAlloc, s);
					oTESTB(Allocator->IsValid(), "Heap corrupt on Allocate %u of %u bytes.", i, s);
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
				Allocator->Deallocate(pointers[i]);
				pointers[i] = 0;
				oTESTB(Allocator->IsValid(), "Heap corrupt on Deallocate %u.", i);
			}
		}

		// test really small
		void* p1 = Allocator->Allocate(1);
		oTESTB(Allocator->IsValid(), "Heap corrupt on Allocate of %u.", 1);
		void* p2 = Allocator->Allocate(0);
		oTESTB(Allocator->IsValid(), "Heap corrupt on Allocate of %u.", 0);
		Allocator->Deallocate(p2);
		oTESTB(Allocator->IsValid(), "Heap corrupt on Deallocate");
		Allocator->Deallocate(p1);
		oTESTB(Allocator->IsValid(), "Heap corrupt on Deallocate");

		char RAMused[32];
		oFormatMemorySize(RAMused, SIZE, 1);
		char MAXsize[32];
		oFormatMemorySize(MAXsize, largestAlloc, 1);
		sprintf_s(_StrStatus, _SizeofStrStatus, "%sRAMused: %s, minsize:%u b, maxsize:%s", EnoughPhysRamForFullTest ? "" : "WARNING: system memory not enough to run full test quickly. ", RAMused, smallestAlloc, MAXsize);

		return SUCCESS;
	}
};

TESTAllocator TestAllocator;
