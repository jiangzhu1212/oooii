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
#ifndef AllocatorTLSF_Impl_h
#define AllocatorTLSF_Impl_h

#include <oooii/oAllocatorTLSF.h>
#include <oooii/oRefCount.h>

struct AllocatorTLSF_Impl : public oAllocatorTLSF
{
	// Always allocate memory for this struct in the arena specified by the user
	void* operator new(size_t _Size) { return 0; }
	void* operator new[](size_t si_Size) { return 0; }
	void operator delete(void* _Pointer) {}
	void operator delete[](void* _Pointer) {}
public:
	void* operator new(size_t _Size, void* _pMemory) { return _pMemory; }
	void operator delete(void* _Pointer, void* _pMemory) {}

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oAllocatorTLSF>());

	AllocatorTLSF_Impl(const char* _DebugName, const DESC* _pDesc, bool* _pSuccess);
	~AllocatorTLSF_Impl();

	void GetDesc(DESC* _pDesc) override;
	void GetStats(STATS* _pStats) override;
	const char* GetDebugName() const threadsafe override;
	const char* GetType() const threadsafe override;
	bool IsValid() override;
	void* Allocate(size_t _NumBytes, size_t _Alignment = oDEFAULT_MEMORY_ALIGNMENT) override;
	void* Reallocate(void* _Pointer, size_t _NumBytes) override;
	void Deallocate(void* _Pointer) override;
	size_t GetBlockSize(void* _Pointer) override;
	void Reset() override;
	void WalkHeap(WalkerFn _Walker, void* _pUserData, long _Flags = 0) override;

	DESC Desc;
	STATS Stats;
	oRefCount RefCount;
	void* hPool;
	char DebugName[64];
};

#endif
