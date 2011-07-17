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
// Base implementation for a pooled object allocator.
// NOT THREADSAFE. See oConcurrentPooledAllocator for the 
// threadsafe version.
#pragma once
#ifndef oPooledAllocator_h
#define oPooledAllocator_h

#include <oooii/oIndexAllocator.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <memory>

class oPooledAllocatorBase
{
public:
	enum PoolInitType
	{
		InitElementCount,
		InitBytes,
	};
};

template<typename T, typename IndexAllocatorT, typename Alloc = std::allocator<char> >
class oTPooledAllocator : public oPooledAllocatorBase
{
	const char* DebugName;
	void* Pool;
	Alloc Allocator;
	IndexAllocatorT Indices;
	static const size_t INDEX_SIZE = IndexAllocatorT::SizeOfIndex;
	bool InternallyAllocated;

public:
	typedef T value_t;

	oTPooledAllocator(const char* _DebugName, PoolInitType _Type = InitBytes, size_t _Value = 0, const Alloc& _Allocator = Alloc())
		: DebugName(_DebugName)
		, Pool(0)
		, Allocator(_Allocator)
	{
		if (_Value)
			Initialize(_Type, _Value);
	}
	
	virtual ~oTPooledAllocator()
	{
		if (IsValid())
		{
			oASSERT(IsEmpty(), "oPooledAllocator %s not empty: %u outstanding allocations", DebugName, GetSize());
			Deinitialize();
		}
	}

	inline static size_t CalcArenaSize(size_t _NumElements)
	{
		return oDEFAULT_MEMORY_ALIGNMENT + _NumElements * (INDEX_SIZE + sizeof(T));
	}

	inline static size_t CalcNumElements(size_t _ArenaBytes)
	{
		return (_ArenaBytes - oDEFAULT_MEMORY_ALIGNMENT) / (INDEX_SIZE + sizeof(T));
	}
	
	inline void Initialize(PoolInitType _Type, size_t _Value)
	{
		size_t arenaSize = _Value;
		if (_Type == InitElementCount)
			arenaSize = CalcArenaSize(_Value);
		Initialize(Allocator.allocate(arenaSize), arenaSize);
		InternallyAllocated = true;
	}

	inline void Initialize(void* _pArena, size_t _SizeofArena)
	{
		oASSERT(!IsValid(), "\"%s\": already initialized", DebugName);
		size_t nElements = CalcNumElements(_SizeofArena);
		size_t indexAllocatorOverhead = nElements * INDEX_SIZE;
		Indices.Initialize(_pArena, indexAllocatorOverhead);
		Pool = oByteAdd(_pArena, oByteAlign(indexAllocatorOverhead, oDEFAULT_MEMORY_ALIGNMENT));
		oASSERT(oByteAdd(Pool, sizeof(value_t), Indices.GetCapacity()) < oByteAdd(_pArena, _SizeofArena), "oPooledAllocator not sized correctly.");
		InternallyAllocated = false;
	}

	inline void* Deinitialize() // returns arena as specified in Initialize()
	{
		oASSERT(IsValid(), "\"%s\": Already deinitialized", DebugName);
		oASSERT(IsEmpty(), "\"%s\": There are %u outstanding allocations", DebugName, GetSize());
		Pool = 0;
		void* pArena = Indices.Deinitialize();
		if (InternallyAllocated)
		{
			Allocator.deallocate((char*)pArena, 0);
			pArena = 0;
		}
		return pArena;
	}

	inline void* Allocate() threadsafe
	{
		unsigned int index = Indices.Allocate();
		return (index != oConcurrentIndexAllocator::InvalidIndex) ? oByteAdd(Pool, sizeof(value_t), index) : 0;
	}

	inline void Deallocate(void* _Pointer) threadsafe
	{
		unsigned int index = (unsigned int)oIndexOf((T*)_Pointer, (T*)Pool);
		Indices.Deallocate(index);
	}

	T* Construct() threadsafe { void* m = Allocate(); return new (m) T(); }
	template<typename U> T* Construct(U u) threadsafe { void* m = Allocate(); return new (m) T(u); }
	template<typename U, typename V> T* Construct(U u, V v) threadsafe { void* m = Allocate(); return new (m) T(u, v); }
	template<typename U, typename V, typename W> T* Construct(U u, V v, W w) threadsafe { void* m = Allocate(); return new (m) T(u, v, w); }
	template<typename U, typename V, typename W, typename X> T* Construct(U u, V v, W w, X x) threadsafe { void* m = Allocate(); return new (m) T(u, v, w, x); }
	template<typename U, typename V, typename W, typename X, typename Y> T* Construct(U u, V v, W w, X x, Y y) threadsafe { void* m = Allocate(); return new (m) T(u, v, w, x, y); }
	template<typename U, typename V, typename W, typename X, typename Y, typename Z> T* Construct(U u, V v, W w, X x, Y y, Z z) threadsafe { void* m = Allocate(); return new (m) T(u, v, w, x, y, z); }
	void Destroy(T* _Pointer) threadsafe { _Pointer->~T(); Deallocate((void*)_Pointer); }

	inline errno_t Reset()
	{
		oASSERT(IsValid(), "oPooledAllocator \"%s\": Not yet initialized", DebugName);
		Indices.Reset();
		return 0;
	}
	
	inline const char* GetDebugName() const threadsafe { return DebugName; }
	inline bool IsValid() const { return Indices.IsValid(); }
	inline bool IsEmpty() const { return Indices.IsEmpty(); }
	inline size_t GetCapacity() const threadsafe { return Indices.GetCapacity(); }
	inline size_t GetSize() const { return Indices.GetSize(); } // (SLOW! this loops through entire freelist each call)
	inline size_t GetUsedBytes() const { return GetSize() * sizeof(value_t); }

	Alloc& allocator() { return Allocator; }
	const Alloc& allocator() const { return Allocator; }
};

template<typename T, typename Alloc = std::allocator<char> >
class oPooledAllocator : public oTPooledAllocator<T, oIndexAllocator, Alloc>
{
public:
	oPooledAllocator(const char* _DebugName, PoolInitType _Type = InitBytes, size_t _Value = 0, const Alloc& _Allocator = Alloc())
		: oTPooledAllocator(_DebugName, _Type, _Value, _Allocator)
	{}
};

#define oDEFINE_CONCURRENT_POOLED_NEW_DELETE__(PooledAllocatorClass, PooledClass, PoolInstance, ReserveNumElements) static PooledAllocatorClass<PooledClass> PoolInstance(#PoolInstance, PooledAllocatorClass<PooledClass>::InitElementCount, ReserveNumElements); \
	void* PooledClass::operator new(size_t size) { return PoolInstance.Allocate(); } \
	void PooledClass::operator delete(void* p) { PoolInstance.Deallocate(p); } \
	void* PooledClass::operator new[](size_t size) { oASSERT(false, #PooledAllocatorClass "." #PoolInstance "<" #PooledClass "> new[] should not be called on this object"); return 0; } \
	void PooledClass::operator delete[](void* p) { oASSERT(false, #PooledAllocatorClass "." #PoolInstance "<" #PooledClass "> new[] should not be called on this object"); }

#define oDEFINE_POOLED_NEW_DELETE(PooledClass, PoolInstance, ReserveNumElements) oDEFINE_CONCURRENT_POOLED_NEW_DELETE__(oPooledAllocator, PooledClass, PoolInstance, ReserveNumElements)

#endif
