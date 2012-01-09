// $(header)
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
