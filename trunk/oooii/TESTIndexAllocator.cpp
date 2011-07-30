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
#include <oooii/oConcurrentPooledAllocator.h>
#include <oooii/oIndexAllocator.h>
#include <oooii/oTest.h>
#include <oooii/oSTL.h> 

template<typename IndexAllocatorT>
bool IndexAllocatorTest(char* _StrStatus, size_t _SizeofStrStatus)
{
	const size_t CAPACITY = 4;
	const size_t ARENA_BYTES = CAPACITY * IndexAllocatorT::SizeOfIndex;
	void* pBuffer = new char[ARENA_BYTES];
	oTESTB(!!pBuffer, "Failed to allocate buffer of size %u", ARENA_BYTES);
	
	IndexAllocatorT a(pBuffer, ARENA_BYTES);
	oTESTB(a.IsEmpty(), "IndexAllocator did not initialize correctly.");
	oTESTB(a.GetCapacity() == CAPACITY, "Capacity mismatch.");

	unsigned int index[4];
	for (unsigned int i = 0; i < oCOUNTOF(index); i++)
		index[i] = a.Allocate();

	for (unsigned int i = 0; i < oCOUNTOF(index); i++)
		oTESTB(index[i] == i, "Allocation mismatch %u.", i);

	a.Deallocate(index[1]);
	a.Deallocate(index[0]);
	a.Deallocate(index[2]);
	a.Deallocate(index[3]);

	oTESTB(a.IsEmpty(), "A deallocate failed.");
	delete [] a.Deinitialize();

	return true;
}

class TestPooledObj
{
public:
	static const int VALUE = 0xc001c0de;
	TestPooledObj() : dummy(VALUE) {}
	oDECLARE_NEW_DELETE();

	int dummy;
};

// oooii-tony: This number should be greater than 65536 to 
// test/protect against 16-bit limitations such as found in 
// Window's InterlockedSList's depth. I used SList once upon
// a time to avoid worrying about pointer tagging, but I found
// found for a lot of threads sometimes I need a lot of 
// allocation space.
static const size_t NUM_POOLED_OBJECTS = 100000; 
oDEFINE_CONCURRENT_POOLED_NEW_DELETE(TestPooledObj, sObjPool, NUM_POOLED_OBJECTS);

bool PooledAllocatorTest(char* _StrStatus, size_t _SizeofStrStatus)
{
	oTESTB(sObjPool.GetCapacity() == NUM_POOLED_OBJECTS, "Capacity is actually %u, not the expected %u", sObjPool.GetCapacity(), NUM_POOLED_OBJECTS);

	std::vector<TestPooledObj*> objects(NUM_POOLED_OBJECTS);

	for (size_t i = 0; i < NUM_POOLED_OBJECTS; i++)
		objects[i] = new TestPooledObj();
	oTESTB(!sObjPool.IsEmpty(), "Pool has all items allocated, but is considered empty");
	for (size_t i = 0; i < NUM_POOLED_OBJECTS; i++)
	{
		oTESTB(objects[i]->dummy == TestPooledObj::VALUE, "Constructor not properly called");
		delete objects[i];
	}

	oTESTB(sObjPool.IsEmpty(), "Pool is supposed to be empty, but isn't.");
	return true;
}

struct TESTIndexAllocator : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		if (!IndexAllocatorTest<oIndexAllocator>(_StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (!PooledAllocatorTest(_StrStatus, _SizeofStrStatus))
			return FAILURE;

		return SUCCESS;
	}
};

struct TESTIndexAllocatorConcurrent : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		if (!IndexAllocatorTest<oConcurrentIndexAllocator>(_StrStatus, _SizeofStrStatus))
			return FAILURE;

		if (!PooledAllocatorTest(_StrStatus, _SizeofStrStatus))
			return FAILURE;

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTIndexAllocator);
oTEST_REGISTER(TESTIndexAllocatorConcurrent);
