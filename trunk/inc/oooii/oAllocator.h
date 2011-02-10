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
#pragma once
#ifndef oAllocator_h
#define oAllocator_h

#include <oooii/oInterface.h>

interface oAllocator : public oInterface
{
	enum TYPE
	{
		TLSF,
	};

	struct DESC
	{
		DESC()
			: Type(TLSF)
			, pArena(0)
			, ArenaSize(0)
		{}

		TYPE Type;
		void* pArena;
		size_t ArenaSize;
	};

	struct STATS
	{
		size_t NumAllocations;
		size_t BytesAllocated;
		size_t PeakBytesAllocated;
		size_t BytesFree;
	};

	struct BLOCK_DESC
	{
		void* Address;
		size_t Size;
		bool Used;
	};

	// The implementation class is created from memory inside the specified arena
	// where you could c-cast pArena to oAllocator and it would work.
	static bool Create(const char* _DebugName, const DESC* _pDesc, oAllocator** _ppAllocator);

	virtual void GetDesc(DESC* _pDesc) = 0;
	virtual void GetStats(STATS* _pStats) = 0;
	virtual bool IsValid() = 0;

	virtual void* Allocate(size_t _NumBytes, size_t _Alignment = oDEFAULT_MEMORY_ALIGNMENT) = 0;
	virtual void* Reallocate(void* _Pointer, size_t _NumBytes) = 0;
	virtual void Deallocate(void* _Pointer) = 0;
	virtual size_t GetBlockSize(void* _Pointer) = 0;

	virtual void Reset() = 0;

	typedef void (*WalkerFn)(const BLOCK_DESC* pDesc, void* _pUser, long _Flags);
	virtual void WalkHeap(WalkerFn _Walker, void* _pUserData, long _Flags = 0) = 0;

	// An alternative for using placement new with memory from this oAllocator.
	template<typename T> T* Construct() { void* m = Allocate(sizeof(T)); return new (m) T(); }
	template<typename T, typename U> T* Construct(U u) { void* m = Allocate(sizeof(T)); return new (m) T(u); }
	template<typename T, typename U, typename V> T* Construct(U u, V v) { void* m = Allocate(sizeof(T)); return new (m) T(u, v); }
	template<typename T, typename U, typename V, typename W> T* Construct(U u, V v, W w) { void* m = Allocate(sizeof(T)); return new (m) T(u, v, w); }
	template<typename T, typename U, typename V, typename W, typename X> T* Construct(U u, V v, W w, X x) { void* m = Allocate(sizeof(T)); return new (m) T(u, v, w, x); }
	template<typename T, typename U, typename V, typename W, typename X, typename Y> T* Construct(U u, V v, W w, X x, Y y) { void* m = Allocate(sizeof(T)); return new (m) T(u, v, w, x, y); }
	template<typename T, typename U, typename V, typename W, typename X, typename Y, typename Z> T* Construct(U u, V v, W w, X x, Y y, Z z) { void* m = Allocate(sizeof(T)); return new (m) T(u, v, w, x, y, z); }
	template<typename T> void Destroy(T* _Pointer) { _Pointer->~T(); Deallocate((void*)_Pointer); }
};

#endif
