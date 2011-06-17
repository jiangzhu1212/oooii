// $(header)
#include <oooii/oProcessHeap.h>
#include <oooii/oStdAlloc.h>
#include <oooii/oHash.h>
#include <oooii/oInterface.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oString.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>
#include <algorithm>
#include <map>

struct oProcessHeapContext : oInterface
{
public:
	// All methods on this object must be virtual with the one
	// exception being the Singleton accessor.  This ensures that
	// anytime a method is called the underlying code that is executed
	// runs in the allocating module.  If they are not virtual, we run the
	// risk that the code (and objects on the underlying implementation) will
	// not match in one module vs another.
	virtual void* Allocate(size_t _Size) = 0;
	virtual bool FindOrAllocate(const char* _Name, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, void** _pPointer) = 0;
	virtual void Deallocate(void* _Pointer) = 0;
	virtual void ReportLeaks() = 0;
	virtual void Lock() = 0;
	virtual void UnLock() = 0;

	static oProcessHeapContext* Singleton();
};

void* oProcessHeap::Allocate(size_t _Size)
{
	return oProcessHeapContext::Singleton()->Allocate(_Size);
}

bool oProcessHeap::FindOrAllocate(const char* _Name, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, void** _pPointer)
{
	return oProcessHeapContext::Singleton()->FindOrAllocate(_Name, _Size, _PlacementConstructor, _pPointer);
}

void oProcessHeap::Deallocate(void* _Pointer)
{
	oProcessHeapContext::Singleton()->Deallocate(_Pointer);
}

void oProcessHeap::Lock()
{
	oProcessHeapContext::Singleton()->Lock();
}

void oProcessHeap:: UnLock()
{
	oProcessHeapContext::Singleton()->UnLock();
}

template<typename T> struct oStdStaticAllocator
{
	// Below we'll use a hash, but we want all its allocations to come out of the
	// specific process heap.

	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oStdStaticAllocator)
		oStdStaticAllocator() {}
	template<typename U> oStdStaticAllocator(oStdStaticAllocator<U> const& other) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>(oProcessHeap::Allocate(sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { oProcessHeap::Deallocate(p); }
	inline const oStdStaticAllocator& operator=(const oStdStaticAllocator& other) {}
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oStdStaticAllocator)
	oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oStdStaticAllocator) { return true; }

struct oProcessHeapContextImpl : oProcessHeapContext
{
	static const size_t PROCESS_HEAP_SIZE = oKB(100);
	
	struct MMAPFILE
	{
		oProcessHeapContext* pProcessStaticHeap;
		oGUID guid;
		DWORD processId;
	};

protected:


	struct ENTRY
	{
		ENTRY()
			: Pointer(0)
		{
			*Name = 0;
		}

		void* Pointer;
		char Name[64];
	};

	struct MatchesEntry
	{
		MatchesEntry(void* _Pointer) : Pointer(_Pointer) {}
		bool operator()(const std::pair<size_t, ENTRY>& _MapEntry) { return _MapEntry.second.Pointer == Pointer; }
		void* Pointer;
	};

	// This is a low-level, platform-specific implementation object, so use the
	// platform mutex directly to avoid executing any other more complex code.
	CRITICAL_SECTION SharedPointerCS;

	HANDLE hHeap;
	oRefCount RefCount;
	static bool IsValid;

	typedef std::map<size_t, ENTRY, std::less<size_t>, oStdStaticAllocator<std::pair<size_t, ENTRY> > > container_t;
	container_t* pSharedPointers;

public:
	oDEFINE_NOOP_QUERYINTERFACE();
	oProcessHeapContextImpl()
		: hHeap(HeapCreate(0, PROCESS_HEAP_SIZE, 0))
	{
		InitializeCriticalSection(&SharedPointerCS);
		pSharedPointers = new container_t;
		atexit(AtExit);
		IsValid = true;
	}

	void Reference() threadsafe override
	{
		RefCount.Reference();
	}

	void Release() threadsafe override
	{
		if (RefCount.Release())
		{
			IsValid = false;
			delete pSharedPointers;
			// thread_cast is safe because we're shutting down
			DeleteCriticalSection(thread_cast<LPCRITICAL_SECTION>( &SharedPointerCS ));
			this->~oProcessHeapContextImpl();
			HeapDestroy(hHeap);
			VirtualFreeEx(GetCurrentProcess(), thread_cast<oProcessHeapContextImpl*>(this), 0, MEM_RELEASE);
		}
	}

	inline size_t Hash(const char* _Name) { return oHash_superfast(_Name, static_cast<unsigned int>(strlen(_Name))); }

	void* Allocate(size_t _Size) override { return HeapAlloc(hHeap, 0, _Size); }
	bool FindOrAllocate(const char* _Name, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, void** _pPointer) override;
	void Deallocate(void* _Pointer) override;
	void ReportLeaks() override;

	virtual void Lock() override;
	virtual void UnLock() override;

	static void AtExit();
};

bool oProcessHeapContextImpl::IsValid = false;
void oProcessHeapContextImpl::AtExit()
{
	if (IsValid)
	{
		oProcessHeapContextImpl::Singleton()->ReportLeaks();
	}

	else
	{
		OutputDebugStringA("========== Process Heap Leak Report: 0 Leaks ==========\n");
	}
}

void oProcessHeapContextImpl::ReportLeaks()
{
	char buf[oKB(1)];
	OutputDebugStringA("========== Process Heap Leak Report ==========\n");
	for (container_t::const_iterator it = pSharedPointers->begin(); it != pSharedPointers->end(); ++it)
	{
		sprintf_s(buf, "%s\n", it->second.Name);
		OutputDebugStringA(buf);
	}

	sprintf_s(buf, "========== Process Heap Leak Report: %u Leaks ==========\n", pSharedPointers->size());
	OutputDebugStringA(buf);
}


bool oProcessHeapContextImpl::FindOrAllocate(const char* _Name, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, void** _pPointer)
{
	bool Allocated = false;
	oCRTASSERT(_Name && _Size && _pPointer, "oProcessHeap::FindOrAllocate(): Invalid parameter");
	size_t h = Hash(_Name);

	EnterCriticalSection(&SharedPointerCS);

	container_t::iterator it = pSharedPointers->find(h);
	if (it == pSharedPointers->end())
	{
		*_pPointer = oProcessHeap::Allocate(_Size);
		_PlacementConstructor(*_pPointer);

		ENTRY& e = (*pSharedPointers)[h];
		e.Pointer = *_pPointer;
		oCRTASSERT(strlen(_Name) < (oCOUNTOF(e.Name)-1), "oProcessHeap::FindOrAllocate(): _Name must be less than 64 characters long.");
		strcpy_s(e.Name, _Name);

		Reference();
		Allocated = true;
	}

	else
		*_pPointer = it->second.Pointer;
	
	LeaveCriticalSection(&SharedPointerCS);
	return Allocated;
}

void oProcessHeapContextImpl::Deallocate(void* _Pointer)
{
	EnterCriticalSection(&SharedPointerCS);

	if( !IsValid ) // We are shutting down, so this allocation must be from pSharedPointers itself, so just free it
	{
		HeapFree(hHeap, 0, _Pointer);
		LeaveCriticalSection(&SharedPointerCS);
		return;
	}

	container_t::iterator it = std::find_if(pSharedPointers->begin(), pSharedPointers->end(), MatchesEntry(_Pointer));
	if (it == pSharedPointers->end())
		HeapFree(hHeap, 0, _Pointer);
	else
	{
		// @oooii-kevin: The order here is critical, we need to tell the heap to 
		// deallocate first remove it from the list exit the critical section and 
		// then release. If we release first we risk destroying the heap and then 
		// still needing to access it.
		void* p = it->second.Pointer;
		HeapFree(hHeap, 0, p);
		pSharedPointers->erase(it);
		LeaveCriticalSection(&SharedPointerCS);
		Release();
		return;
	}

	LeaveCriticalSection(&SharedPointerCS);
}


oProcessHeapContext* oProcessHeapContext::Singleton()
{
	static oRef<oProcessHeapContext> sInstance;
	if (!sInstance)
	{
		static const oGUID heapMMapGuid = { 0x7c5be6d1, 0xc5c2, 0x470e, { 0x85, 0x4a, 0x2b, 0x98, 0x48, 0xf8, 0x8b, 0xa9 } }; // {7C5BE6D1-C5C2-470e-854A-2B9848F88BA9}

		// Filename is "<GUID><CurrentProcessID>"
		static char mmapFileName[128] = {0};
		oToString(mmapFileName, heapMMapGuid);
		sprintf_s(mmapFileName + strlen(mmapFileName), 128 - strlen(mmapFileName), "%u", GetCurrentProcessId());

		// Create a memory-mapped File to store the location of the oProcessHeapContext
		SetLastError(ERROR_SUCCESS);
		HANDLE hMMap = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(oProcessHeapContextImpl::MMAPFILE), mmapFileName);
		oCRTASSERT(hMMap, "Could not create memory mapped file for oProcessHeapContext.");

		oProcessHeapContextImpl::MMAPFILE* memFile = (oProcessHeapContextImpl::MMAPFILE*)MapViewOfFile(hMMap, FILE_MAP_WRITE, 0, 0, 0);

		if (hMMap && GetLastError() == ERROR_ALREADY_EXISTS) // File already exists, loop until it's valid.
		{
			while(memFile->processId != GetCurrentProcessId() || memFile->guid != heapMMapGuid)
			{
				UnmapViewOfFile(memFile);
				oSleep(0);
				memFile = (oProcessHeapContextImpl::MMAPFILE*)MapViewOfFile(hMMap, FILE_MAP_WRITE, 0, 0, 0);
			}

			sInstance = memFile->pProcessStaticHeap;
		}

		// Created new file, now allocate the oProcessHeapContext instance.
		else if (hMMap) 
		{
			// Allocate memory at the highest possible address then store that value 
			// in the Memory-Mapped File for other DLLs to access.
			*sInstance.address() = static_cast<oProcessHeapContextImpl*>(VirtualAllocEx(GetCurrentProcess(), 0, sizeof(oProcessHeapContextImpl), MEM_COMMIT|MEM_RESERVE|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE));
			oCRTASSERT(sInstance, "VirtualAllocEx failed for oProcessHeapContext.");
			new (sInstance.c_ptr()) oProcessHeapContextImpl();

			memFile->pProcessStaticHeap = sInstance;
			memFile->guid = heapMMapGuid;
			memFile->processId = GetCurrentProcessId();
		}

		UnmapViewOfFile(memFile);
	}

	return sInstance;
}

void oProcessHeapContextImpl::Lock()
{
	EnterCriticalSection(&SharedPointerCS);
}

void oProcessHeapContextImpl::UnLock()
{
	LeaveCriticalSection(&SharedPointerCS);
}