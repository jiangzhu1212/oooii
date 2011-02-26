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
#include <oooii/oThreading.h>
#include <oooii/oGUID.h>
#include <oooii/oString.h>
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

// @oooii-will: Reserve pages to virtual memory.  Does not allocate until PagedCommit() 
// is called, but prevents malloc and LocalAlloc from accessing the reserved memory.
void* oHeap::PagedReserve(void* _DesiredPointer, size_t _Size, ACCESS _Access)
{
	oASSERT(_DesiredPointer, "If you're using this API, you probably want to be explicit about the _DesiredPointer you pass in.");
	void* p = VirtualAllocEx(GetCurrentProcess(), _DesiredPointer, _Size, MEM_RESERVE, GetAccess(_Access));
	if (p != _DesiredPointer)
	{
		oSetLastError(ENOMEM, "VirtualAllocEx return a pointer (0x%p) that is not the same as the requested pointer (0x%p).", _DesiredPointer, p);
		if (p)
			PagedUnreserve(p);
		p = 0;
	}

	return p;
}

// @oooii-will: Release reserved virtual memory.  MEM_RELEASE also decommits the memory
// if PagedDecommit() hasn't been called, yet.
void oHeap::PagedUnreserve(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_RELEASE));
}

// @oooii-will: Committing reserved pages to virtual memory.  Attempting to commit a 
// page that is not yet reserved results in an error.
void* oHeap::PagedCommit(void* _DesiredPointer, size_t _Size, ACCESS _Access)
{
	oASSERT(_DesiredPointer, "If you're using this API, you probably want to be explicit about the _DesiredPointer you pass in.");
	void* p = VirtualAllocEx(GetCurrentProcess(), _DesiredPointer, _Size, MEM_COMMIT, GetAccess(_Access));
	if (p != _DesiredPointer)
	{
		oSetLastError(ENOMEM, "VirtualAllocEx return a pointer (0x%p) that is not the same as the requested pointer (0x%p).", _DesiredPointer, p);
		if (p)
			PagedDecommit(p);
		p = 0;
	}

	return p;
}

// @oooii-will: Set committed pages to a reserved state, to be reserved or freed later.
void oHeap::PagedDecommit(void* _Pointer)
{
	oVB(VirtualFreeEx(GetCurrentProcess(), _Pointer, 0, MEM_DECOMMIT));
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

struct oProcessStaticHeap : public oInterface
{
	oProcessStaticHeap()
		: hHeap(HeapCreate(0, 100 * 1024, 0))
	{
		InitializeCriticalSection(&SharedPointerCS);
	}

	~oProcessStaticHeap()
	{
		DeleteCriticalSection(&SharedPointerCS);
	}

private:
	struct oMMapFile
	{
		oProcessStaticHeap* pProcessStaticHeap;
		oGUID guid;
		DWORD processId;
	};
public:

	void Reference() threadsafe
	{
		Refcount.Reference();
	}
	virtual void Release() threadsafe
	{
		if( Refcount.Release() )
		{
			this->~oProcessStaticHeap();
			HeapDestroy(hHeap);
			VirtualFreeEx(GetCurrentProcess(), thread_cast<oProcessStaticHeap*>( this ), 0, MEM_RELEASE);
		}
	}

	static oProcessStaticHeap* Singleton()
	{
		static oRef<oProcessStaticHeap> sInstance = 0; // process-global static
		if (!sInstance)
		{
			static const oGUID heapMMapGuid = { 0x7c5be6d1, 0xc5c2, 0x470e, { 0x85, 0x4a, 0x2b, 0x98, 0x48, 0xf8, 0x8b, 0xa9 } }; // {7C5BE6D1-C5C2-470e-854A-2B9848F88BA9}

			// Filename is "<GUID><CurrentProcessID>"
			static char mmapFileName[128] = {0};
			oToString(mmapFileName, heapMMapGuid);
			sprintf_s(mmapFileName + strlen(mmapFileName), 128 - strlen(mmapFileName), "%u", GetCurrentProcessId());

			// Create a Memory-Mapped File to store the location of the oProcessStaticHeap
			SetLastError( ERROR_SUCCESS );
			HANDLE hMMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(oMMapFile), mmapFileName);
			assert(hMMap && "Fatal: Could not create memory mapped file for oProcessStaticHeap.");

			oMMapFile* memFile = (oMMapFile*)MapViewOfFile(hMMap, FILE_MAP_WRITE, 0, 0, 0);

			if( hMMap && GetLastError() == ERROR_ALREADY_EXISTS ) // File already exists, loop until it's valid.
			{
				while(memFile->processId != GetCurrentProcessId() || memFile->guid != heapMMapGuid)
				{
					UnmapViewOfFile(memFile);
					oSleep(0);
					memFile = (oMMapFile*)MapViewOfFile(hMMap, FILE_MAP_WRITE, 0, 0, 0);
				}

				// @oooii-kevin: Since sInstance is a ref this does the counting
 				sInstance = memFile->pProcessStaticHeap;
			}
			else if( hMMap ) // Created new file, now allocate the oProcessStaticHeap instance.
			{
				// Allocate memory at the highest possible address then store that value in the Memory-Mapped File for other DLLs to access.
				*sInstance.address() = static_cast<oProcessStaticHeap*>(VirtualAllocEx(GetCurrentProcess(), NULL, sizeof(oProcessStaticHeap), MEM_COMMIT|MEM_RESERVE|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE));
				assert(sInstance && "Fatal: VirtualAllocEx failed for oProcessStaticHeap.");
				new (sInstance.c_ptr()) oProcessStaticHeap();

				memFile->pProcessStaticHeap = sInstance;
				memFile->guid = heapMMapGuid;
				memFile->processId = GetCurrentProcessId();
			}

			UnmapViewOfFile(memFile);
		}

		return sInstance;
	}

	void* Allocate(size_t _Size) { return HeapAlloc(hHeap, 0, _Size); }
	void Deallocate(void* _Pointer) { HeapFree(hHeap, 0, _Pointer); }

	bool AllocateSharedMap(const char* _Name, bool _bThreadUnique, size_t _Size, void** _pPointer);
	void AllocateSharedUnmap();
	void DeallocateShared(void* _Pointer);

	inline size_t Hash(const char* _Name) { return oHash_superfast(_Name, static_cast<unsigned int>(strlen(_Name))); }

	HANDLE hHeap;
	oRefCount Refcount;

	struct ENTRY
	{
		ENTRY()
			: Pointer(0)
		{}
		
		void* Pointer;
		char Name[64];
	};

	// @oooii-kevin: We use a raw critical section here because oProcessStaticHeap needs a very low level lock
	// system lock that can't possibly trigger any other complex code. 
	CRITICAL_SECTION SharedPointerCS;

	typedef std::map<size_t, ENTRY, std::less<size_t>, oStdStaticAllocator<std::pair<size_t, ENTRY> > > container_t;
	container_t SharedPointers;
};

bool oProcessStaticHeap::AllocateSharedMap(const char* _Name, bool _bThreadUnique, size_t _Size, void** _pPointer)
{
	EnterCriticalSection(&SharedPointerCS);
	oASSERT(_Name && _Size && _pPointer, "Invalid parameters to oStaticMap::Allocate()");
	size_t h = Hash(_Name);

	container_t::iterator it = SharedPointers.find(h);
	if (it == SharedPointers.end())
	{
		*_pPointer = oHeap::StaticAllocate(_Size);
		ENTRY& e = SharedPointers[h];
		e.Pointer = *_pPointer;
		strcpy_s( e.Name, _Name );

		// @oooii-kevin: Only non-thread unique allocations should reference, as they are the only
		// ones that will Release 
		//if( !_bThreadUnique )
		Reference();

		return true;
	}

	*_pPointer = it->second.Pointer;
	return false;
}
void oProcessStaticHeap::AllocateSharedUnmap()
{
	LeaveCriticalSection(&SharedPointerCS);
}

void oProcessStaticHeap::DeallocateShared(void* _Pointer)
{
	EnterCriticalSection(&SharedPointerCS);
	for (container_t::iterator it = SharedPointers.begin(); it != SharedPointers.end(); ++it)
	{
		if (it->second.Pointer == _Pointer)
		{
			// @oooii-kevin: The order here is critical, we need to tell the heap to deallocate first
			// remove it from the list exit the critical section and then release.  If we release first
			// we risk destroying the heap and then still neading to access it
			oHeap::StaticDeallocate(it->second.Pointer);
			SharedPointers.erase(it);
			LeaveCriticalSection(&SharedPointerCS);
			Release();
			return;
		}
	}

	LeaveCriticalSection(&SharedPointerCS);
}

void* oHeap::StaticAllocate(size_t _Size)
{
	return oProcessStaticHeap::Singleton()->Allocate(_Size);
}

void oHeap::StaticDeallocate(void* _Pointer)
{
	oProcessStaticHeap::Singleton()->Deallocate(_Pointer);
}

bool oHeap::StaticAllocateSharedMap(const char* _Name, bool _bThreadUnique, size_t _Size, void** _pPointer)
{
	return oProcessStaticHeap::Singleton()->AllocateSharedMap(_Name, _bThreadUnique, _Size, _pPointer);
}

void oHeap::StaticAllocateSharedUnmap()
{
	return oProcessStaticHeap::Singleton()->AllocateSharedUnmap();
}

void oHeap::StaticDeallocateShared(void* _Pointer)
{
	oProcessStaticHeap::Singleton()->DeallocateShared(_Pointer);
}
