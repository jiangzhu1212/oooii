// $(header)

// Interface for a memory allocator. The interface gets created
// with an arena (raw memory), and the bookkeeping and heap gets
// created within that memory.
#pragma once
#ifndef oAllocator_h
#define oAllocator_h

#include <oBasis/oCallable.h>
#include <oBasis/oInterface.h>
#include <oBasis/oFunction.h>

interface oAllocator : oInterface
{
	struct DESC
	{
		DESC()
			: pArena(0)
			, ArenaSize(0)
		{}

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

	virtual void GetDesc(DESC* _pDesc) = 0;
	virtual void GetStats(STATS* _pStats) = 0;
	virtual const char* GetDebugName() const threadsafe = 0;
	virtual const char* GetType() const threadsafe = 0;
	virtual bool IsValid() = 0;

	virtual void* Allocate(size_t _NumBytes, size_t _Alignment = oDEFAULT_MEMORY_ALIGNMENT) = 0;
	virtual void* Reallocate(void* _Pointer, size_t _NumBytes) = 0;
	virtual void Deallocate(void* _Pointer) = 0;
	virtual size_t GetBlockSize(void* _Pointer) = 0;

	virtual void Reset() = 0;

	virtual void WalkHeap(oFUNCTION<void(void* _Pointer, size_t _Size, bool _Used)> _HeapWalker) = 0;

	// Callable is a type that is to be constructed in memory allocated from this 
	// allocator. Example:
	// MyType* pMyType = pMyAllocator->Construct<MyType>(param1, param2);
	#ifndef oHAS_VARIADIC_TEMPLATES
		oCALLABLE_TEMPLATE0 Callable* Construct(oARG_DECL0) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS0); }
		oCALLABLE_TEMPLATE1 Callable* Construct(oARG_DECL1) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS1); }
		oCALLABLE_TEMPLATE2 Callable* Construct(oARG_DECL2) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS2); }
		oCALLABLE_TEMPLATE3 Callable* Construct(oARG_DECL3) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS3); }
		oCALLABLE_TEMPLATE4 Callable* Construct(oARG_DECL4) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS4); }
		oCALLABLE_TEMPLATE5 Callable* Construct(oARG_DECL5) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS5); }
		oCALLABLE_TEMPLATE6 Callable* Construct(oARG_DECL6) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS6); }
		oCALLABLE_TEMPLATE7 Callable* Construct(oARG_DECL7) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS7); }
		oCALLABLE_TEMPLATE8 Callable* Construct(oARG_DECL8) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS8); }
		oCALLABLE_TEMPLATE9 Callable* Construct(oARG_DECL9) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS9); }
		oCALLABLE_TEMPLATE10 Callable* Construct(oARG_DECL10) { void* m = Allocate(sizeof(Callable)); return new (m) Callable(oARG_PASS10); }
	#else
		#error Implement Construct() with variadic template args
	#endif

	template<typename Callable> void Destroy(Callable* _Pointer) { _Pointer->~Callable(); Deallocate((void*)_Pointer); }
};

#endif
