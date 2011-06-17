// $(header)

// Allocates indices, which can be used for fixed-size pool 
// management. The allocated arena will contain a linked list 
// of free indices, so it must be sized to contain the number 
// of indices desired, so NumIndices * sizeof(unsigned int).
// NOT THREADSAFE. See oConcurrentIndexAllocator for a 
// threadsafe implementation.
#pragma once
#ifndef oIndexAllocator_h
#define oIndexAllocator_h

#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

class oIndexAllocatorBase : oNoncopyable
{
protected:
	void* Arena;
	size_t ArenaBytes;
	unsigned int Freelist;
	void InternalReset();
	oIndexAllocatorBase();
	oIndexAllocatorBase(void* _pArena, size_t _SizeofArena);
	virtual ~oIndexAllocatorBase();
public:
	static const unsigned int InvalidIndex;
	static const size_t SizeOfIndex = sizeof(unsigned int);
	void Initialize(void* _pArena, size_t _SizeofArena);
	void* Deinitialize(); // returns arena as specified in initialize
	void Reset(); // deallocate all indices
	inline bool IsValid() const { return Arena != 0; }
	inline bool IsEmpty() const { return GetSize() == 0; } // (SLOW! see GetSize())
	virtual size_t GetSize() const; // number of indices allocated (SLOW! this loops through entire freelist each call)
	size_t GetCapacity() const threadsafe;
};

class oIndexAllocator : public oIndexAllocatorBase
{
public:
	oIndexAllocator();
	oIndexAllocator(void* _pArena, size_t _SizeofArena); // call deinitialized explicitly to free arena
	virtual ~oIndexAllocator();

	virtual size_t GetSize() const override;
	inline unsigned int Allocate() { unsigned int allocatedIndex = Freelist; Freelist = static_cast<unsigned int*>(Arena)[Freelist]; return allocatedIndex; }
	inline void Deallocate(unsigned int _Index) { static_cast<unsigned int*>(Arena)[_Index] = Freelist; Freelist = _Index; }
};

#endif
