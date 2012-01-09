// $(header)
// Simple allocator that has a fixed arena of memory and increments an index 
// with each allocation until all memory is used up. There is no Deallocate(), 
// but Reset() will set the index back to zero. This is useful for containers
// such as std::maps that are built up and searched and whose data is simple
// (i.e. no ref counting or handles/pointers that need to be cleaned up). Use of 
// this allocator can alleviate long destructor times in such containers where
// there's not a lot of use to the destruction because of the simple types.
//
// This uses the oversized-allocation pattern where the full arena is allocated
// and this class is overlaid on top of it to manage the buffer.
//
// Allocation is O(1) and a single CAS is used to enable concurrency.

#pragma once
#ifndef oLinearAllocator_h
#define oLinearAllocator_h

#include <oBasis/oByte.h>
#include <oBasis/oStdAllocator.h>
#include <oBasis/oStdAtomic.h>
#include <oBasis/oThreadsafe.h>

class oLinearAllocator
{
public:
	oLinearAllocator()
		: Head(nullptr)
		, End(nullptr)
	{}

	inline void Initialize(size_t _ArenaSize)
	{
		End = oByteAdd(this, _ArenaSize);
		Reset();
	}

	inline void* Allocate(size_t _Size) threadsafe
	{
		void* New, *Old;
		do
		{
			Old = Head;
			New = oByteAdd(Old, _Size);
			if (New > End)
				return nullptr;
			New = oByteAlign(New, oDEFAULT_MEMORY_ALIGNMENT);
		} while (!oStd::atomic_compare_exchange(&Head, New, Old));
		return Old;
	}

	template<typename T> T* Allocate(size_t _Size = sizeof(T)) threadsafe { return reinterpret_cast<T*>(Allocate(_Size)); }
	
	void Reset() threadsafe { oStd::atomic_exchange(&Head, begin()); }

	bool Contains(void* _Pointer) const threadsafe { return oInRange(_Pointer, begin(), Head); }

	size_t GetBytesAvailable() const threadsafe { ptrdiff_t diff = oByteDiff(End, Head); return diff > 0 ? diff : 0; }
	
private:
	void* Head;
	void* End;

	void* begin() threadsafe const { return (void*)(this+1); }
};

template<typename T> struct oStdLinearAllocator
{
	// Use an initial buffer or when that is exhausted fall back to system malloc.
	// Deallocate noops on the first allocations, but will free memory allocated
	// by the system heaps. This also keeps a pointer that records how many bytes
	// were allocated from platform Malloc so that the arena size can be adjusted.

	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oStdLinearAllocator)
	oStdLinearAllocator(void* _Arena, size_t _SizeofArena, size_t* _pPlatformBytesAllocated = nullptr, void* (*_PlatformMalloc)(size_t _Size) = malloc, void (*_PlatformFree)(void* _Pointer) = free)
		: pAllocator(reinterpret_cast<oLinearAllocator*>(_Arena))
		, PlatformMalloc(_PlatformMalloc)
		, PlatformFree(_PlatformFree)
		, pPlatformBytesAllocated(_pPlatformBytesAllocated)
	{
		pAllocator->Initialize(_SizeofArena);
		if (pPlatformBytesAllocated)
			*pPlatformBytesAllocated = 0;
	}

	~oStdLinearAllocator()
	{
	}
	
	template<typename U> oStdLinearAllocator(oStdLinearAllocator<U> const& _That)
		: pAllocator(_That.pAllocator)
		, PlatformMalloc(_That.PlatformMalloc)
		, PlatformFree(_That.PlatformFree)
		, pPlatformBytesAllocated(_That.pPlatformBytesAllocated)
	{}
	
	inline pointer allocate(size_type count, const_pointer hint = 0)
	{
		const size_t nBytes = sizeof(T) * count;

		void* p = pAllocator->Allocate(nBytes);
		if (!p)
		{
			p = PlatformMalloc(nBytes);
			if (p && pPlatformBytesAllocated)
				*pPlatformBytesAllocated += nBytes;
		}
		return static_cast<pointer>(p);
	}
	
	inline void deallocate(pointer p, size_type count)
	{
		if (!pAllocator->Contains(p))
			PlatformFree(p);
	}
	
	inline void Reset() { pAllocator->Reset(); }
	inline const oStdLinearAllocator& operator=(const oStdLinearAllocator& _That)
	{
		pAllocator = _That.pAllocator;
		PlatformMalloc = _That.PlatformMalloc;
		PlatformFree = _That.PlatformFree;
		pPlatformBytesAllocated = _That.pPlatformBytesAllocated;
		return *this;
	}
	
	oLinearAllocator* pAllocator;
	void* (*PlatformMalloc)(size_t _Size);
	void (*PlatformFree)(void* _Pointer);
	size_t* pPlatformBytesAllocated;
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oStdLinearAllocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oStdLinearAllocator) { return a.pAllocator == b.pAllocator; }

#endif
