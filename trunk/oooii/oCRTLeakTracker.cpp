// $(header)
#include "oCRTLeakTracker.h"
#include <oooii/oDebugger.h>
#include <oooii/oString.h>
#include <assert.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#endif

// _____________________________________________________________________________
// Copy-paste from $(VSInstallDir)crt\src\dbgint.h, to avoid including CRT 
// source code
#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
	struct _CrtMemBlockHeader * pBlockHeaderNext;
	struct _CrtMemBlockHeader * pBlockHeaderPrev;
	char *                      szFileName;
	int                         nLine;
	#ifdef _WIN64
		/* These items are reversed on Win64 to eliminate gaps in the struct
		* and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
		* maintained in the debug heap.
		*/
		int                         nBlockUse;
		size_t                      nDataSize;
	#else  /* _WIN64 */
		size_t                      nDataSize;
		int                         nBlockUse;
	#endif  /* _WIN64 */
	long                        lRequest;
	unsigned char               gap[nNoMansLandSize];
	/* followed by:
	*  unsigned char           data[nDataSize];
	*  unsigned char           anotherGap[nNoMansLandSize];
	*/
} _CrtMemBlockHeader;

#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)

// _____________________________________________________________________________

_CrtMemBlockHeader* GetHead()
{
	// New blocks are added to the head of the list
	void* p = malloc(1);
	_CrtMemBlockHeader* hdr = pHdr(p);
	free(p);
	return hdr;
}

oCRTMallocTracker::oCRTMallocTracker()
		: InInternalProcesses(false)
		, Enabled(false)
		, OriginalAllocHook(0)
{
	oDebugger::Reference();

	sInstanceForDeferredRelease = this;
	Reference(); // keep an extra references to ourselves
	atexit(AtExit); // then free it at the very end
}

oCRTMallocTracker::~oCRTMallocTracker()
{
	ReportLeaks();
	_CrtSetAllocHook(OriginalAllocHook);
	oDebugger::Release();
}

oCRTMallocTracker* oCRTMallocTracker::sInstanceForDeferredRelease = 0;
void oCRTMallocTracker::AtExit()
{
	if (sInstanceForDeferredRelease)
		sInstanceForDeferredRelease->Release();
}

void oCRTMallocTracker::Enable(bool _Enabled)
{
	if (_Enabled)
		OriginalAllocHook = _CrtSetAllocHook(MallocHook);
	else
	{
		_CrtSetAllocHook(OriginalAllocHook);
		OriginalAllocHook = 0;
		Allocations.clear();
	}

	Enabled = _Enabled;
}

bool oCRTMallocTracker::IsEnabled() const
{
	return Enabled;
}

void oCRTMallocTracker::Report(bool _Report)
{
	ReportEnabled = _Report;
}

void oCRTMallocTracker::OnAllocation(void* _Address, size_t _Size, long _RequestNumber, const char* _Path, unsigned int _Line, bool _IsReallocation)
{
	if (InInternalProcesses || (_IsReallocation && !_CrtIsValidHeapPointer(_Address))) return;
	InInternalProcesses = true;

	allocations_t::iterator it = Allocations.find(_RequestNumber);

	if (_IsReallocation)
		Allocations.erase(it);
	else
		assert(it == Allocations.end() && "Address already tracked as allocated");

	ALLOCATION_DESC d;
	d.Address = _Address;
	memset(d.StackTrace, 0, sizeof(d.StackTrace));
	d.NumStackEntries = static_cast<unsigned int>(oDebugger::GetCallstack(d.StackTrace, oCOUNTOF(d.StackTrace), STACK_TRACE_OFFSET));
	d.Size = _Size;
	d.Line = _Line;
	d.RequestNumber = _RequestNumber;
	strcpy_s(d.Path, oSAFESTR(_Path));
	Allocations[_RequestNumber] = d;

	InInternalProcesses = false;
}

void oCRTMallocTracker::OnDeallocation(void* _Address)
{
	if (InInternalProcesses || !_CrtIsValidHeapPointer(_Address)) return;
	InInternalProcesses = true;

	allocations_t::iterator it = Allocations.find(pHdr(_Address)->lRequest);
	if (it != Allocations.end()) // there may be existing allocs before tracking was enabled, so ignore those
		Allocations.erase(it);

	InInternalProcesses = false;
}

void oCRTMallocTracker::ReportLeaks()
{
	if (ReportEnabled)
	{
		char buf[oKB(2)];
		char totalLeakMemSize[oKB(1)];
		unsigned long long totalLeakBytes = 0;

		if (!Allocations.empty())
		{
			oDebugger::Print("========== Leak Report ==========\n");

			for (allocations_t::const_iterator it = Allocations.begin(); it != Allocations.end(); ++it)
			{
				const ALLOCATION_DESC& d = it->second;
				totalLeakBytes += d.Size;

				char memsize[64];
				oFormatMemorySize(memsize, d.Size, 2);
				// TODO: Add callstack dump

				if (d.Path && *d.Path)
					sprintf_s(buf, "%s(%u) : {%d} %s\n", d.Path, d.Line, d.RequestNumber, memsize);
				else
					sprintf_s(buf, "<no filename> : {%d} %s (probably a call to ::new(size_t))\n", d.RequestNumber, memsize);

				oDebugger::Print(buf);

				oDebugger::SYMBOL s;
				for (size_t i = 0; i < d.NumStackEntries; i++)
				{
					oDebugger::TranslateSymbol(&s, d.StackTrace[i]);
					sprintf_s(buf, "  %s(%u) : %s%s\n", s.Filename, s.Line, s.Name, i == d.NumStackEntries-1 ? "\n" : "");
					oDebugger::Print(buf);
				}
			}
		}

		oFormatMemorySize(totalLeakMemSize, totalLeakBytes, 2);
		sprintf_s(buf, "========== Leak Report: %u Leaks %s ==========\n", Allocations.size(), totalLeakMemSize);
		oDebugger::Print(buf);
	}
}

int oCRTMallocTracker::CallOriginalHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line)
{
	if (_UserData && pHdr(_UserData)->nBlockUse == _IGNORE_BLOCK)
	{
		oCRTMallocTracker& Tracker = *Singleton();
		if (Tracker.OriginalAllocHook != 0)
		{
			Tracker.OriginalAllocHook(_AllocationType, _UserData, _Size, _BlockType, _RequestNumber, _Path, _Line);
			return true;
		}
	}

	return false;
}

int oCRTMallocTracker::MallocHook(int _AllocationType, void* _UserData, size_t _Size, int _BlockType, long _RequestNumber, const unsigned char* _Path, int _Line)
{
	if (_BlockType != _CRT_BLOCK && !CallOriginalHook(_AllocationType, _UserData, _Size, _BlockType, _RequestNumber, _Path, _Line))
	{
		oCRTMallocTracker* pTracker = Singleton();
		if (oIsValidSingletonPointer(pTracker))
		{
			switch (_AllocationType)
			{
				case _HOOK_REALLOC:
				case _HOOK_ALLOC:
					pTracker->OnAllocation(_UserData, _Size, _RequestNumber, (const char*)_Path, static_cast<unsigned int>(_Line), _AllocationType == _HOOK_REALLOC);
					break;

				case _HOOK_FREE:
					pTracker->OnDeallocation(_UserData);
					break;

				default:
					break;
			}
		}
	}

	return true;
}
