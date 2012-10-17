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
#include <oBasis/oConcurrentIndexAllocator.h>
#include <oBasis/oIndexAllocator.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>
#include <vector>

template<typename IndexAllocatorT>
bool IndexAllocatorTest()
{
	const size_t CAPACITY = 4;
	const size_t ARENA_BYTES = CAPACITY * IndexAllocatorT::SizeOfIndex;
	void* pBuffer = new char[ARENA_BYTES];
	if (!pBuffer)
		return oErrorSetLast(oERROR_AT_CAPACITY, "Failed to allocate buffer of size %u", ARENA_BYTES);

	IndexAllocatorT a(pBuffer, ARENA_BYTES);

	struct OnExit
	{
		IndexAllocatorT& IA;
		OnExit(IndexAllocatorT& _IA) : IA(_IA) {}
		~OnExit() { delete [] IA.Deinitialize(); }
	};

	OnExit onExit(a);

	if (!a.IsEmpty())
		return oErrorSetLast(oERROR_GENERIC, "IndexAllocator did not initialize correctly.");
	if (a.GetCapacity() != CAPACITY)
		return oErrorSetLast(oERROR_GENERIC, "Capacity mismatch.");

	unsigned int index[4];
	for (unsigned int i = 0; i < oCOUNTOF(index); i++)
		index[i] = a.Allocate();

	for (unsigned int i = 0; i < oCOUNTOF(index); i++)
		if (index[i] != i)
			return oErrorSetLast(oERROR_GENERIC, "Allocation mismatch %u.", i);

	a.Deallocate(index[1]);
	a.Deallocate(index[0]);
	a.Deallocate(index[2]);
	a.Deallocate(index[3]);

	if (!a.IsEmpty())
		return oErrorSetLast(oERROR_GENERIC, "A deallocate failed.");

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

bool oBasisTest_oIndexAllocator()
{
	return IndexAllocatorTest<oIndexAllocator>();
}

bool oBasisTest_oConcurrentIndexAllocator()
{
	return IndexAllocatorTest<oConcurrentIndexAllocator>();
}
