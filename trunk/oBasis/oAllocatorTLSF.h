// $(header)
#pragma once
#ifndef AllocatorTLSF_Impl_h
#define AllocatorTLSF_Impl_h

#include <oBasis/oAllocatorTLSF.h>
#include <oBasis/oRefCount.h>

interface oAllocatorTLSF;
struct AllocatorTLSF_Impl : public oAllocator
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
	oDEFINE_TRIVIAL_QUERYINTERFACE(oAllocatorTLSF);

	AllocatorTLSF_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess);
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
	void WalkHeap(oFUNCTION<void(void* _Pointer, size_t _Size, bool _Used)> _HeapWalker) override;

	DESC Desc;
	STATS Stats;
	oRefCount RefCount;
	void* hPool;
	char DebugName[64];
};

#endif
