// $(header)
#pragma once
#ifndef oLockFreeRingBuffer_h
#define oLockFreeRingBuffer_h

#include <oooii/oStddef.h>
#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>

template<typename T, typename Alloc = std::allocator<T> >
class oLockFreeRingBuffer
{
	// This structure is for getting the latest valid data where it's ok to skip
	// some entries, but ensure that reading always occurs from a fully valid 
	// entry and writing occurs without any stalling.
	
	T* ElementPool;
	size_t NumElements;
	volatile size_t Write;
	Alloc Allocator;

	oFORCEINLINE void RWB() threadsafe { oReadWriteBarrier(); } // only real native part

public:
	typedef T value_type;

	inline oLockFreeRingBuffer(const Alloc& _Allocator = Alloc(), size_t _Capacity = 3)
		: Allocator(_Allocator)
		, ElementPool(_Allocator.allocate(__max(_Capacity, 3))) // any less than triple-buffered, this won't work at all
		, NumElements(__max(_Capacity, 3))
		, Write(0)
	{
		for (size_t i = 0; i < NumElements; i++)
			Allocator.construct(&ElementPool[i]);
	}

	~oLockFreeRingBuffer()
	{
		Write = 0;
		for (size_t i = 0; i < NumElements; i++)
			Allocator.destroy(&ElementPool[i]);
		Allocator.deallocate(ElementPool, NumElements);
		ElementPool = 0;
		NumElements = 0;
	}

	// Non-safe accessors/mutators for an individual element. This is intended
	// for complex initialization purposes.
	inline void SetElement(unsigned int _Index, T& _Element)
	{
		oASSERT(_Index < NumElements, "");
		ElementPool[_Index] = _Element;
	}

	inline T& GetElement(unsigned int _Index)
	{
		oASSERT(_Index < NumElements, "");
		return ElementPool[_Index];
	}

	inline const T& GetElement(unsigned int _Index) const
	{
		oASSERT(_Index < NumElements, "");
		return ElementPool[_Index];
	}

	inline T& GetWritableElement() threadsafe
	{
		RWB();
		return ElementPool[Write];
	}

	inline void IncrementWritableElement() threadsafe
	{
		RWB();
		Write = (Write + 1) % NumElements;
		RWB();
	}

	// NumBack can get older versions of the element. This must be used with care
	// because going back too far will step on the writable element. Such is the
	// cost of being lock-free. i.e. if you have a triple-buffer, going 2 back
	// would be what's being written, and that would be bad. But if you have a
	// 5-buffer, going back 1 in the history would be valid.
	inline const T& GetReadableElement(unsigned int _NumBack = 0) const threadsafe
	{
		oASSERT(_NumBack < NumElements - 3, "Getting a history that risks be overwritten while being read");
		RWB();
		return ElementPool[(Write - (_NumBack + 1)) % NumElements];
	}

	inline bool IsValid() const threadsafe { return !!ElementPool; }
	inline size_t Capacity() const threadsafe { return NumElements; }
};

#endif
