// $(header)
// Threadsafe implementation of oIndexAllocator using atomics.
#pragma once
#ifndef oConcurrentIndexAllocator_h
#define oConcurrentIndexAllocator_h

#include <oBasis/oIndexAllocator.h>

class oConcurrentIndexAllocator : public oIndexAllocatorBase
{
public:
	oConcurrentIndexAllocator();
	oConcurrentIndexAllocator(void* _pArena, size_t _SizeofArena); // call deinitialize explicitly to free arena
	virtual ~oConcurrentIndexAllocator();
	unsigned int Allocate() threadsafe; // return an index reserved until it is made available by deallocate
	void Deallocate(unsigned int _Index) threadsafe; // make index available again
};

#endif
