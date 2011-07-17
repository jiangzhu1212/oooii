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
#include <oooii/oHeap.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oHash.h>
#include <oooii/oPageAllocator.h>
#include <oooii/oStdAlloc.h>
#include <oooii/oStddef.h>
#include <oooii/oStdio.h>
#include <oooii/oWindows.h>
#include <oooii/oThreading.h>
#include <oooii/oGUID.h>
#include <oooii/oString.h>
#include <map>
#include "oWinPSAPI.h"

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

void oHeap::SetSmallBlockHeapThreshold(unsigned long long _sbhMaxSize)
{
	oASSERT(false, "Functionality no longer exists in vcrt10.");
//	_set_sbh_threshold(static_cast<size_t>(_sbhMaxSize));
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

unsigned int oHeap::GetSystemAllocationGranularity()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwAllocationGranularity;
}