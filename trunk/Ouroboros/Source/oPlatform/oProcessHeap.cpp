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
#include <oPlatform/oProcessHeap.h>
#include <oBasis/oHash.h>
#include <oBasis/oInterface.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oThreadX.h>
#include <oPlatform/Windows/oWindows.h>
#include <algorithm>
#include <map>
#include "oCRTLeakTracker.h"
#include "SoftLink/oWinDbgHelp.h"

struct oProcessHeapContext : oInterface
{
public:
	// All methods on this object must be virtual with the one exception being the 
	// Singleton accessor. This ensures that anytime a method is called the 
	// underlying code that is executed runs in the allocating module.  If they 
	// are not virtual, we run the risk that the code (and objects on the 
	// underlying implementation) will not match in one module vs another.
	virtual void* Allocate(size_t _Size) = 0;
	virtual bool Find(const oGUID& _GUID, bool _IsThreadLocal, void** _pPointer) = 0;
	virtual bool FindOrAllocate(const oGUID& _GUID, bool _IsThreadLocal, bool _IsLeakTracked, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, const char* _DebugName, void** _pPointer) = 0;
	virtual void Deallocate(void* _Pointer) = 0;
	virtual void ReportLeaks() = 0;
	virtual void Lock() = 0;
	virtual void Unlock() = 0;

	static oProcessHeapContext* Singleton();
};

void* oProcessHeapAllocate(size_t _Size)
{
	return oProcessHeapContext::Singleton()->Allocate(_Size);
}

bool oProcessHeapFindOrAllocate(const oGUID& _GUID, bool _IsThreadLocal, bool _IsLeakTracked, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, const char* _DebugName, void** _pPointer)
{
	return oProcessHeapContext::Singleton()->FindOrAllocate(_GUID, _IsThreadLocal, _IsLeakTracked, _Size, _PlacementConstructor, _DebugName, _pPointer);
}

bool oProcessHeapFind(const oGUID& _GUID, bool _IsThreadLocal, void** _pPointer)
{
	return oProcessHeapContext::Singleton()->Find(_GUID, _IsThreadLocal, _pPointer);
}

void oProcessHeapDeallocate(void* _Pointer)
{
	oProcessHeapContext::Singleton()->Deallocate(_Pointer);
}

void oProcessHeapEnsureRunning()
{
	oProcessHeapContext::Singleton()->Lock();
	oProcessHeapContext::Singleton()->Unlock();
}

struct oProcessHeapContextImpl : oProcessHeapContext
{
	static const size_t PROCESS_HEAP_SIZE = oKB(100);
	static const size_t STACK_TRACE_DEPTH = 64;
	
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
			: Pointer(nullptr)
		{
		}

		void* Pointer;
		oStd::thread::id InitThreadID; // threadID of init, and where deinit will probably take place
		oGUID GUID;
		char DebugName[64];
		bool IsThreadLocal;
		bool IsTracked;
		unsigned long long StackTrace[STACK_TRACE_DEPTH];
		size_t NumStackEntries;
		SRWLOCK EntryLock;
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

	//code relies on this being node bases, meaning nodes don't get moved in memory as they are moved. so don't change this to an unordered_map
	typedef std::map<size_t, ENTRY, std::less<size_t>, oProcessHeapAllocator<std::pair<size_t, ENTRY> > > container_t;
	container_t* pSharedPointers;

	static bool IsTBBEntry(container_t::const_iterator it);
	static bool ShouldConsider(container_t::const_iterator it);

	static oProcessHeapContextImpl* sAtExistInstance;

public:
	oDEFINE_NOOP_QUERYINTERFACE();
	oProcessHeapContextImpl()
		: hHeap(GetProcessHeap())
	{
		InitializeCriticalSection(&SharedPointerCS);

		pSharedPointers = new(HeapAlloc(hHeap, 0, sizeof(container_t))) container_t();
		sAtExistInstance = this;
		atexit(AtExit);
		IsValid = true;
		
		// DbgHelp is needed by the most base functionality that goes through static
		// init such as heap and process heap leak reporting as well as traces. The
		// lib also has the nasty habit of simply terminating the process if not 
		// initialized and a call is made, so force this to stay around as long as 
		// the idea of the process is around.
		oWinDbgHelp::Singleton()->Reference();

		char moduleName[_MAX_PATH];
		oVERIFY(oModuleGetName(moduleName, oModuleGetCurrent()));
		char buf[oKB(1)];
		char syspath[_MAX_PATH];
		oPrintf(buf, "%s(%d): {%s} %s ProcessHeap initialized at 0x%p\n", __FILE__, __LINE__, oGetFilebase(moduleName), oSystemGetPath(syspath, oSYSPATH_EXECUTION), this);
		oThreadsafeOutputDebugStringA(buf);
	}

	int Reference() threadsafe override
	{
		return RefCount.Reference();
	}

	void Release() threadsafe override
	{
		if (RefCount.Release())
		{
			IsValid = false;
			
			pSharedPointers->~container_t();
			oMemset4(pSharedPointers, 0xfeeefeee, sizeof(container_t));
			HeapFree(hHeap, 0, pSharedPointers);
			pSharedPointers = nullptr;
			
			// thread_cast is safe because we're shutting down
			DeleteCriticalSection(thread_cast<LPCRITICAL_SECTION>( &SharedPointerCS ));

			this->~oProcessHeapContextImpl();
			VirtualFreeEx(GetCurrentProcess(), thread_cast<oProcessHeapContextImpl*>(this), 0, MEM_RELEASE);
		}
	}

	inline size_t Hash(const oGUID& _GUID, bool _IsThreadLocal)
	{
		unsigned int h = oHash_superfast(&_GUID, sizeof(oGUID));
		if (_IsThreadLocal)
		{
			oStd::thread::id id = oStd::this_thread::get_id();
			h = oHash_superfast(&id, sizeof(oStd::thread::id), h);
		}

		return h;
	}

	void* Allocate(size_t _Size) override { return HeapAlloc(hHeap, 0, _Size); }
	bool FindOrAllocate(const oGUID& _GUID, bool _IsThreadLocal, bool _IsLeakTracked, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, const char* _DebugName, void** _pPointer) override;
	bool Find(const oGUID& _GUID, bool _IsThreadLocal, void** _pPointer) override;
	void Deallocate(void* _Pointer) override;
	void ReportLeaks() override;

	virtual void Lock() override;
	virtual void Unlock() override;

	static void CreatePrimodialSingletons();
	static void DestroyPrimodialSingletons();

	static void AtExit();
};

static void oProcessHeapOutputLeakReportFooter(size_t _NumLeaks)
{
	char buf[256];
	char syspath[_MAX_PATH];
	char moduleName[_MAX_PATH];
	oVERIFY(oModuleGetName(moduleName, oModuleGetCurrent()));
	oPrintf(buf, "========== Process Heap Leak Report: %u Leaks %s ==========\n", _NumLeaks, oSystemGetPath(syspath, oSYSPATH_EXECUTION));
	OutputDebugStringA(buf);
}

bool oProcessHeapContextImpl::IsValid = false;
oProcessHeapContextImpl* oProcessHeapContextImpl::sAtExistInstance = nullptr;
void oProcessHeapContextImpl::AtExit()
{
	DestroyPrimodialSingletons();

	if (IsValid)
		oProcessHeapContextImpl::Singleton()->ReportLeaks();
	else
		oProcessHeapOutputLeakReportFooter(0);
	
	// Release here and not in dtor because being a singleton this needs 
	// oProcessHeap to be valid
	oWinDbgHelp::Singleton()->Release();
}

bool oProcessHeapContextImpl::IsTBBEntry(container_t::const_iterator it)
{
	char buf[256];
	bool IsStdBind = false;
	for (size_t i = 0; i < it->second.NumStackEntries; i++)
	{
		bool WasStdBind = IsStdBind;
		oDebuggerSymbolSPrintf(buf, it->second.StackTrace[i], "  ", &IsStdBind);
		if (strstr(buf, "oStd::async") || strstr(buf, "tbbD!tbb::") || strstr(buf, "DEPRECATED_oTaskIssueAsync"))
			return true;
		if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
			i += 5;
	}

	return false;
}

// Sometimes this report will run before all static deinit is done, falsely
// reporting leaks. At the time of this macro though, the system was leak-free
// and this report is guaranteed to be pretty late in the pipe.
//#define oIGNORE_SAME_THREAD_FALSE_LEAKS

bool oProcessHeapContextImpl::ShouldConsider(container_t::const_iterator it)
{
	// @oooii-tony: I just cannot get TBB to play ball. It lets threads 
	// disappear whenever with no user callback. According to another guy 
	// who likes to build software in teams and produce products, freeing
	// any threadlocal resources just doesn't seem possible...
	// http://software.intel.com/en-us/forums/showthread.php?t=68825
	// So for now, just don't report anything that comes from TBB.

	// @oooii-tony: I've decided to blacklist all oThreadlocalMallocs because 
	// there are several 3rd-party problem-makers, not just TBB, so for now that
	// seems to clean all the leaks. However running code in different orders
	// makes different things appear, so don't remove this quite yet, we may need
	// it yet again...
	//if (IsTBBEntry(it))
	//	return false;

	// Special-case oWinDbgHelp since we're using it so very late for IsTBBEntry
	// and printing callstacks.
	if (!oStrcmp("oWinDbgHelp", it->second.DebugName))
	{
		oInterface* i = static_cast<oInterface*>(it->second.Pointer);
		int r = i->Reference() - 1;
		i->Release();
		if (r > 1)
			return true;
		else
			return false;
	}

	#ifdef oIGNORE_SAME_THREAD_FALSE_LEAKS
		if (it->second.InitThreadID == oStd::this_thread::get_id())
			return false;
	#endif

	return it->second.IsTracked;
}

void oProcessHeapContextImpl::ReportLeaks()
{
	// freeing of singletons is done with atexit(). So ignore leaks that were 
	// created on this thread because they will potentially be freed after this
	// report. The traces for singleton lifetimes should indicate threadID of 
	// freeing, so if there are any after this report that don't match the threadID
	// of this report, that would be bad.

	// do a pre-scan to see if it's worth printing anything to the log

	unsigned int nLeaks = 0;
	unsigned int nIgnoredLeaks = 0;
	for (container_t::const_iterator it = pSharedPointers->begin(); it != pSharedPointers->end(); ++it)
	{
		if (ShouldConsider(it))
			nLeaks++;
		else
			nIgnoredLeaks++;
	}

	char moduleName[_MAX_PATH];
	oVERIFY(oModuleGetName(moduleName, oModuleGetCurrent()));
	
	char buf[oKB(1)];
	
	if (nLeaks)
	{
		char syspath[_MAX_PATH];
		oPrintf(buf, "========== Process Heap Leak Report %s (Module %s) ==========\n", oSystemGetPath(syspath, oSYSPATH_EXECUTION), oGetFilebase(moduleName));
		OutputDebugStringA(buf);
		for (container_t::const_iterator it = pSharedPointers->begin(); it != pSharedPointers->end(); ++it)
		{
			if (ShouldConsider(it))
			{
				const ENTRY& e = it->second;

				char TLBuf[128];
				oPrintf(TLBuf, " (thread_local in thread 0x%x%s)", *(unsigned int*)&e.InitThreadID, oThreadGetMainID() == e.InitThreadID ? " (main)" : "");
				char GUIDStr[128];
				oPrintf(buf, "%s %s%s\n", oToString(GUIDStr, e.GUID), oSAFESTRN(e.DebugName), e.IsThreadLocal ? TLBuf : "");
				OutputDebugStringA(buf); // use non-threadsafe version because that could alloc the mutex

				bool IsStdBind = false;
				for (size_t i = 0; i < e.NumStackEntries; i++)
				{
					bool WasStdBind = IsStdBind;
					oDebuggerSymbolSPrintf(buf, e.StackTrace[i], "  ", &IsStdBind);
					if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
						i += 5;
					OutputDebugStringA(buf); // use non-threadsafe version because that could alloc the mutex
				}
			}
		}
	}

	oProcessHeapOutputLeakReportFooter(nLeaks);

	// For ignored leaks, release those refs on the oProcessHeap
	for (unsigned int i = 0; i < nIgnoredLeaks; i++)
		Release();
}

bool oProcessHeapContextImpl::Find(const oGUID& _GUID, bool _IsThreadLocal, void** _pPointer)
{
	oCRTASSERT(_pPointer, "oProcessHeap::Find(): Invalid parameter");
	size_t h = Hash(_GUID, _IsThreadLocal);
	*_pPointer = nullptr;
	EnterCriticalSection(&SharedPointerCS);
	container_t::iterator it = pSharedPointers->find(h);
	if (it != pSharedPointers->end())
		*_pPointer = it->second.Pointer;
	LeaveCriticalSection(&SharedPointerCS);
	return !!*_pPointer;
}

bool oProcessHeapContextImpl::FindOrAllocate(const oGUID& _GUID, bool _IsThreadLocal, bool _IsLeakTracked, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, const char* _DebugName, void** _pPointer)
{
	bool Allocated = false;
	oCRTASSERT(_Size && _pPointer, "oProcessHeap::FindOrAllocate(): Invalid parameter");
	size_t h = Hash(_GUID, _IsThreadLocal);

	EnterCriticalSection(&SharedPointerCS);

	container_t::iterator it = pSharedPointers->find(h);
	if (it == pSharedPointers->end())
	{
		ENTRY& e = (*pSharedPointers)[h];
		InitializeSRWLock(&e.EntryLock);
		//the constructor for this new object could call back into FindOrAllocate when creating its members. this can cause a deadlock. so release the primary lock before
		//	constructing the new object and lock a shared mutex just for that entry.
		AcquireSRWLockExclusive(&e.EntryLock);
		LeaveCriticalSection(&SharedPointerCS);

		*_pPointer = Allocate(_Size);
		if (_PlacementConstructor)
			_PlacementConstructor(*_pPointer);

		e.Pointer = *_pPointer;
		e.InitThreadID = oStd::this_thread::get_id();
		e.GUID = _GUID;
		e.IsThreadLocal = _IsThreadLocal;
		e.IsTracked = _IsLeakTracked;
		*e.DebugName = 0;
		if (_DebugName)
		{
			memcpy_s(e.DebugName, sizeof(e.DebugName), _DebugName, oMin(strlen(_DebugName)+1, sizeof(e.DebugName)));
			oAddTruncationElipse(e.DebugName);
		}

		e.NumStackEntries = 0;
		memset(e.StackTrace, 0, sizeof(e.StackTrace));

		static bool captureCallstack = true;
		if (captureCallstack)
		{
			e.NumStackEntries = oDebuggerGetCallstack(e.StackTrace, 3);
		}

		Reference();
		Allocated = true;

		ReleaseSRWLockExclusive(&e.EntryLock);
	}
	else
	{
		AcquireSRWLockShared(&it->second.EntryLock); //may exist but not be constructed yet.
		*_pPointer = it->second.Pointer;
		ReleaseSRWLockShared(&it->second.EntryLock);
		LeaveCriticalSection(&SharedPointerCS);
	}
	
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
	static oProcessHeapContext* sInstance;
	if (!sInstance)
	{
		static const oGUID heapMMapGuid = { 0x7c5be6d1, 0xc5c2, 0x470e, { 0x85, 0x4a, 0x2b, 0x98, 0x48, 0xf8, 0x8b, 0xa9 } }; // {7C5BE6D1-C5C2-470e-854A-2B9848F88BA9}

		// Filename is "<GUID><CurrentProcessID>"
		static char mmapFileName[128] = {0};
		oToString(mmapFileName, heapMMapGuid);
		oPrintf(mmapFileName + strlen(mmapFileName), 128 - strlen(mmapFileName), "%u", GetCurrentProcessId());

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
				::Sleep(0);
				memFile = (oProcessHeapContextImpl::MMAPFILE*)MapViewOfFile(hMMap, FILE_MAP_WRITE, 0, 0, 0);
			}

			sInstance = memFile->pProcessStaticHeap;
		}

		// Created new file, now allocate the oProcessHeapContext instance.
		else if (hMMap) 
		{
			// Allocate memory at the highest possible address then store that value 
			// in the Memory-Mapped File for other DLLs to access.
			*(&sInstance) = static_cast<oProcessHeapContextImpl*>(VirtualAllocEx(GetCurrentProcess(), 0, sizeof(oProcessHeapContextImpl), MEM_COMMIT|MEM_RESERVE|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE));
			oCRTASSERT(sInstance, "VirtualAllocEx failed for oProcessHeapContext.");
			new (sInstance) oProcessHeapContextImpl();

			memFile->pProcessStaticHeap = sInstance;
			memFile->guid = heapMMapGuid;
			memFile->processId = GetCurrentProcessId();

			oProcessHeapContextImpl::CreatePrimodialSingletons();
		}

		UnmapViewOfFile(memFile);
	}

	return sInstance;
}

void oProcessHeapContextImpl::CreatePrimodialSingletons()
{
	// Because all allocations from start to end should be tracked, start tracking
	// ASAP.
	oCRTLeakTracker::Singleton();

	// Because threads could start up during static init, ensure thread_local
	// support API such as oAtThreadExit and oThreadlocalMalloc are ready.
	void oThreadlocalRegistryCreate();
	oThreadlocalRegistryCreate();
}

void oProcessHeapContextImpl::DestroyPrimodialSingletons()
{
	void oThreadlocalRegistryDestroy();
	oThreadlocalRegistryDestroy();
}

void oProcessHeapContextImpl::Lock()
{
	EnterCriticalSection(&SharedPointerCS);
}

void oProcessHeapContextImpl::Unlock()
{
	LeaveCriticalSection(&SharedPointerCS);
}
