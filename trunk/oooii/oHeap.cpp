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
#include "pch.h"
#include <oooii/oHeap.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oHash.h>
#include <oooii/oStdAlloc.h>
#include <oooii/oStddef.h>
#include <oooii/oStdio.h>
#include <oooii/oWindows.h>
#include <oooii/oWinPSAPI.h>
#include <map>

void oHeap::Add(STATISTICS* _pStats, const BLOCK_DESC* _pDesc)
{
	_pStats->Blocks++;
	_pStats->Bytes += _pDesc->Size;
	_pStats->Overhead += _pDesc->Overhead;

	if (_pDesc->Used)
	{
		_pStats->BlocksUsed++;
		_pStats->BytesUsed += _pDesc->Size;
	}

	else
	{
		_pStats->BlocksFree++;
		_pStats->BytesFree += _pDesc->Size;
	}

	if (_pDesc->Size <= SMALL_BLOCK_SIZE) _pStats->BlocksSmall++;
	else if (_pDesc->Size >= LARGE_BLOCK_SIZE) _pStats->BlocksLarge++;
	else _pStats->BlocksMedium++;
}

bool oHeap::WalkExternals(WalkerFn _Walker, DescriberFn _Describer, SummarizerFn _Summarizer, void* _pUserData, long _Flags)
{
	HANDLE heaps[128];
	DWORD numHeaps = ::GetProcessHeaps(oCOUNTOF(heaps), heaps);

	const HANDLE hDefault = ::GetProcessHeap();
	const HANDLE hCrt = (HANDLE)_get_heap_handle();

	for (DWORD i = 0; i < numHeaps; i++)
	{
		ULONG heapInfo = 3;
		SIZE_T dummy = 0;
		::HeapQueryInformation(heaps[i], HeapCompatibilityInformation, &heapInfo, sizeof(heapInfo), &dummy);

		const char* desc = "unknown";
		switch (heapInfo)
		{
		case 0: desc = "regular"; break;
		case 1: desc = "fast w/ look-asides"; break;
		case 2: desc = "low-fragmentation (LFH)"; break;
		default: break;
		}

		DESC d;
		sprintf_s(d.Name, "Heap h(%p)", heaps[i]);
		sprintf_s(d.Description, "%s", desc);

		d.Type = EXTERNAL;
		if (heaps[i] == hDefault)
			d.Type = SYSTEM;
		else if (heaps[i] == hCrt)
			d.Type = LIBC;

		(*_Describer)(&d, _pUserData, _Flags);

		STATISTICS s;
		PROCESS_HEAP_ENTRY e;
		e.lpData = 0;
		while (::HeapWalk(heaps[i], &e))
		{
			BLOCK_DESC b;
			b.Address = e.lpData;
			b.Size = e.cbData;
			b.Overhead = e.cbOverhead;
			b.Used = (e.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0;
			(*_Walker)(&b, _pUserData, _Flags);
			Add(&s, &b);
		}

		s.Valid = GetLastError() == ERROR_NO_MORE_ITEMS ? true : false;

		(*_Summarizer)(&s, _pUserData, _Flags);
	}

	// todo: walk internal list looking for type == external since there could be a user pool
	// that is still external because its a wrapper for something else.

	return true;
}

unsigned int oHeap::GetPageSize()
{
	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	return static_cast<unsigned long long>(sysInfo.dwPageSize);
}

void oHeap::SetSmallBlockHeapThreshold(unsigned long long _sbhMaxSize)
{
	_set_sbh_threshold(static_cast<size_t>(_sbhMaxSize));
}

void oHeap::GetProcessDesc(PROCESS_HEAP_DESC* _pDesc)
{
	PROCESS_MEMORY_COUNTERS_EX m;
	memset(&m, 0, sizeof(m));
	m.cb = sizeof(m);
	oVB(oWinPSAPI::Singleton()->GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&m, m.cb));
	_pDesc->NumPageFaults = m.PageFaultCount;
	_pDesc->WorkingSet = m.WorkingSetSize;
	_pDesc->PeakWorkingSet = m.PeakWorkingSetSize;
	_pDesc->PrivateUsage = m.PrivateUsage;
	_pDesc->PageFileUsage = m.PagefileUsage;
	_pDesc->PeakPageFileUsage = m.PeakPagefileUsage;
}

void oHeap::GetGlobalDesc(GLOBAL_HEAP_DESC* _pDesc)
{
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(MEMORYSTATUSEX);
	oVB(GlobalMemoryStatusEx(&ms));
	_pDesc->TotalMemoryUsed = ms.dwMemoryLoad;
	_pDesc->AvailablePhysical = ms.ullAvailPhys;
	_pDesc->TotalPhysical = ms.ullTotalPhys;
	_pDesc->AvailableVirtualProcess = ms.ullAvailVirtual;
	_pDesc->TotalVirtualProcess = ms.ullTotalVirtual;
	_pDesc->AvailablePaged = ms.ullAvailPageFile;
	_pDesc->TotalPaged = ms.ullTotalPageFile;
}

static DWORD GetAccess(oHeap::ACCESS _Access)
{
	switch (_Access)
	{
		case oHeap::READ_WRITE: return PAGE_EXECUTE_READWRITE;
		case oHeap::READ: return PAGE_EXECUTE_READ;
		// @oooii-tony: Not exposed because I'm not sure GUARDED is a cross-platform
		// concept.
		//case oHeap::GUARDED_READ_WRITE: return PAGE_GUARD|PAGE_EXECUTE_READWRITE;
		//case oHeap::GUARDED_READ: return PAGE_GUARD|PAGE_EXECUTE_READ;
		default: oASSUME(0);
	}
}

void* oHeap::PagedAllocate(void* _DesiredPointer, size_t _Size, ACCESS _Access)
{
	oASSERT(_DesiredPointer, "If you're using this API, you probably want to be explicit about the _DesiredPointer you pass in.");
	void* p = VirtualAllocEx(GetCurrentProcess(), _DesiredPointer, _Size, MEM_COMMIT|MEM_RESERVE, GetAccess(_Access));
	if (p != _DesiredPointer)
	{
		oSetLastError(ENOMEM, "VirtualAllocEx return a pointer (0x%p) that is not the same as the requested pointer (0x%p).", _DesiredPointer, p);
		if (p)
			PagedDeallocate(p);
		p = 0;
	}

	return p;
}

void oHeap::PagedDeallocate(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_RELEASE));
}

void oHeap::PagedSetProtection(void* _BaseAddress, size_t _Size, ACCESS _Access)
{
	DWORD oldPermissions = 0;
	oVB(VirtualProtect(_BaseAddress, _Size, GetAccess(_Access), &oldPermissions));
}

template<typename T> struct oStdStaticAllocator
{
	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oStdStaticAllocator)
	oStdStaticAllocator() {}
	template<typename U> oStdStaticAllocator(oStdStaticAllocator<U> const& other) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>(oHeap::StaticAllocate(sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { oHeap::StaticDeallocate(p); }
	inline const oStdStaticAllocator& operator=(const oStdStaticAllocator& other) {}
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oStdStaticAllocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oStdStaticAllocator) { return true; }

struct oProcessStaticHeap
{
	oProcessStaticHeap()
		: hHeap(HeapCreate(0, 100 * 1024, 0))
		, RefCount(1)
	{
	}

	~oProcessStaticHeap()
	{
		HeapDestroy(hHeap);
	}

	static oProcessStaticHeap* Singleton()
	{
		static oProcessStaticHeap* sInstance = 0; // process-global static
		if (!sInstance)
		{
			// Allocate memory at a fixed point at the high end of 
			// the address space so modules/DLLs loaded into the 
			// process can all refer to the exact same block of memory.
			#ifdef _WIN64
				static const uintptr_t HARDCODED_POINTER = 0x7fffff00000;
			#else
				static const uintptr_t HARDCODED_POINTER = 0x7ef00000;
			#endif
			sInstance = static_cast<oProcessStaticHeap*>(VirtualAllocEx(GetCurrentProcess(), (void*)HARDCODED_POINTER, sizeof(oProcessStaticHeap), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));
			
			// if alloc succeeded this is first-instantiation, so run ctor
			if (sInstance)
				new (sInstance) oProcessStaticHeap();
			else // this is another module, so increment ref on the heap
			{
				sInstance = (oProcessStaticHeap*)HARDCODED_POINTER;
				sInstance->RefCount++;
			}
		}

		return sInstance;
	}

	static void Destroy()
	{
		oProcessStaticHeap* pStaticHeap = Singleton();
		if (--pStaticHeap->RefCount == 0)
		{
			pStaticHeap->~oProcessStaticHeap();
			VirtualFreeEx(GetCurrentProcess(), pStaticHeap, 0, MEM_RELEASE);
		}
	}

	void* Allocate(size_t _Size) { return HeapAlloc(hHeap, 0, _Size); }
	void Deallocate(void* _Pointer) { HeapFree(hHeap, 0, _Pointer); }

	bool AllocateShared(const char* _Name, size_t _Size, void** _pPointer);
	void DeallocateShared(void* _Pointer);

	inline size_t Hash(const char* _Name) { return oHash_superfast(_Name, static_cast<unsigned int>(strlen(_Name))); }

	HANDLE hHeap;
	unsigned int RefCount;

	struct ENTRY
	{
		ENTRY()
			: Pointer(0)
			, RefCount(1)
		{}
		
		void* Pointer;
		unsigned int RefCount;
	};

	typedef std::map<size_t, ENTRY, std::less<size_t>, oStdStaticAllocator<std::pair<size_t, ENTRY> > > container_t;
	container_t SharedPointers;
};

bool oProcessStaticHeap::AllocateShared(const char* _Name, size_t _Size, void** _pPointer)
{
	oASSERT(_Name && _Size && _pPointer, "Invalid parameters to oStaticMap::Allocate()");
	size_t h = Hash(_Name);

	container_t::iterator it = SharedPointers.find(h);
	if (it == SharedPointers.end())
	{
		*_pPointer = oHeap::StaticAllocate(_Size);
		ENTRY& e = SharedPointers[h];
		e.Pointer = *_pPointer;
		assert(e.RefCount == 1);
		return true;
	}

	it->second.RefCount++;
	*_pPointer = it->second.Pointer;
	return false;
}

void oProcessStaticHeap::DeallocateShared(void* _Pointer)
{
	for (container_t::iterator it = SharedPointers.begin(); it != SharedPointers.end(); ++it)
	{
		if (it->second.Pointer == _Pointer)
		{
			if (--it->second.RefCount == 0)
			{
				oHeap::StaticDeallocate(it->second.Pointer);
				SharedPointers.erase(it);
				return;
			}
		}
	}
}

void* oHeap::StaticAllocate(size_t _Size)
{
	return oProcessStaticHeap::Singleton()->Allocate(_Size);
}

void oHeap::StaticDeallocate(void* _Pointer)
{
	oProcessStaticHeap::Singleton()->Deallocate(_Pointer);
}

bool oHeap::StaticAllocateShared(const char* _Name, size_t _Size, void** _pPointer)
{
	return oProcessStaticHeap::Singleton()->AllocateShared(_Name, _Size, _pPointer);
}

void oHeap::StaticDeallocateShared(void* _Pointer)
{
	oProcessStaticHeap::Singleton()->DeallocateShared(_Pointer);
}
