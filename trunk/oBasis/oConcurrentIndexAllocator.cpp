// $(header)
#include "oIndexAllocatorInternal.h"
#include <oBasis/oConcurrentIndexAllocator.h>
#include <oBasis/oAssert.h>
#include <oBasis/oStdAtomic.h>

oConcurrentIndexAllocator::oConcurrentIndexAllocator() : oIndexAllocatorBase() {}
oConcurrentIndexAllocator::oConcurrentIndexAllocator(void* _pArena, size_t _SizeofArena) : oIndexAllocatorBase(_pArena, _SizeofArena) {}

unsigned int oConcurrentIndexAllocator::Allocate() threadsafe
{
	unsigned int oldI, newI, allocatedIndex;
	do
	{	// Tagging guards against ABA by including a monotonically increasing version counter
		oldI = Freelist;
		allocatedIndex = oldI >> TAG_BITS;
		if (allocatedIndex == TAGGED_INVALIDINDEX)
			return InvalidIndex;
		newI = (static_cast<unsigned int*>(Arena)[allocatedIndex] << TAG_BITS) | (TAG_MASK&(oldI+1));
	} while (!oStd::atomic_compare_exchange(&Freelist, newI, oldI));

	return allocatedIndex;
}

void oConcurrentIndexAllocator::Deallocate(unsigned int _Index) threadsafe
{
	unsigned int oldI, newI;
	do
	{
		oldI = Freelist;
		static_cast<unsigned int*>(Arena)[_Index] = oldI >> TAG_BITS;
		newI = (_Index << TAG_BITS) | (TAG_MASK&(oldI+1));
	} while (!oStd::atomic_compare_exchange(&Freelist, newI, oldI));
}

oConcurrentIndexAllocator::~oConcurrentIndexAllocator()
{
	if (IsValid())
	{
		oASSERT(IsEmpty(), "Index allocator not empty on destruction");
	}
}