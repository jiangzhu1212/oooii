/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oPlatform/oDebugger.h>
#include <oBasis/oString.h>
#include <oPlatform/Windows/oCRTHeap.h>
#include <oPlatform/oPageAllocator.h>
#include "SoftLink/oWinDbgHelp.h"

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void oDebuggerSetThreadName(const char* _Name, oStd::thread::id _ID)
{
	if (_Name && *_Name && IsDebuggerPresent())
	{
		// http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
		Sleep(10);
		THREADNAME_INFO i;
		i.dwType = 0x1000;
		i.szName = _Name;
		i.dwThreadID = _ID == oStd::thread::id() ? GetCurrentThreadId() : *(DWORD*)&_ID;
		i.dwFlags = 0;
		const static DWORD MS_VC_EXCEPTION = 0x406D1388;
		__try { RaiseException(MS_VC_EXCEPTION, 0, sizeof(i)/sizeof(ULONG_PTR), (ULONG_PTR*)&i); }
		__except(EXCEPTION_EXECUTE_HANDLER) {}
	}
}

// Implement oBasis requirements
void oPlatformSetThreadNameInDebugger(const char* _Name, oStd::thread::id _ID)
{
	oDebuggerSetThreadName(_Name, _ID);
}

void oDebuggerPrint(const char* _String)
{
	oThreadsafeOutputDebugStringA(_String);
}

// Enable this define to get a leak tracker that records the callstack for all
// allocations. This is much slower than the default CRT leak check, but for 
// those stubborn leaks it can be very helpful. NOTE: The custom leak checker
// also reports MUCH later in static deinit than the default one, so there are
// less false-positives.
#define oUSE_CUSTOM_SLOW_BUT_MORE_ROBUST_LEAK_TRACKING

#include "oCRTLeakTracker.h"
void oDebuggerReportCRTLeaksOnExit(bool _Enable)
{
	#ifdef oUSE_CUSTOM_SLOW_BUT_MORE_ROBUST_LEAK_TRACKING
		oCRTLeakTracker::Singleton()->Enable(_Enable);
		oCRTLeakTracker::Singleton()->Report();
	#else
		oCRTEnableAtExitLeakReport(_Enable);
	#endif
}

void oDebuggerBreakOnAllocation(uintptr_t _AllocationID)
{
	oCRTHeapBreakOnAllocation(_AllocationID);
}

/** $(Citation)
	<citation>
		<usage type="Adaptation" />
		<author name="Jochen Kalmback" />
		<description url="http://jpassing.wordpress.com/category/win32/" />
		<license type="BSD" url="http://www.opensource.org/licenses/bsd-license.php" />
	</citation>
	<citation>
		<usage type="Adaptation" />
		<author name="Geoff Evans" />
		<author name="Paul Haile" />
		<description url="http://nocturnal.insomniacgames.com/index.php/Main_Page" />
		<license type="Insomniac Open License (IOL)" url="http://nocturnal.insomniacgames.com/index.php/Insomniac_Open_License" />
	</citation>
*/

// Several of the API assume use of CHAR, not TCHAR
_STATIC_ASSERT(sizeof(CHAR) == sizeof(char));

static void InitContext(CONTEXT& c)
{
	memset(&c, 0, sizeof(c));
	RtlCaptureContext(&c);
}

static void InitStackFrame(STACKFRAME64& s, const CONTEXT& c)
{
  memset(&s, 0, sizeof(s));
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Mode = AddrModeFlat;

	#ifdef _M_IX86
	  s.AddrPC.Offset = c.Eip;
	  s.AddrFrame.Offset = c.Ebp;
	  s.AddrStack.Offset = c.Esp;
	#elif _M_X64
	  s.AddrPC.Offset = c.Rip;
	  s.AddrFrame.Offset = c.Rsp;
	  s.AddrStack.Offset = c.Rsp;
	#elif _M_IA64
	  s.AddrPC.Offset = c.StIIP;
	  s.AddrFrame.Offset = c.IntSp;
	  s.AddrStack.Offset = c.IntSp;
	  s.AddrBStore.Offset = c.RsBSP;
	  s.AddrBStore.Mode = AddrModeFlat;
	#else
		#error "Platform not supported!"
	#endif
}

size_t oDebuggerGetCallstack(unsigned long long* _pSymbols, size_t _NumSymbols, size_t _Offset)
{
	CONTEXT c;
	InitContext(c);

	STACKFRAME64 s;
	InitStackFrame(s, c);

	size_t n = 0;
	while (n < _NumSymbols)
	{
		if (!oWinDbgHelp::Singleton()->CallStackWalk64(GetCurrentThread(), &s, &c))
		{
			break;
		}

		if (s.AddrReturn.Offset == 0 || s.AddrReturn.Offset == 0xffffffffcccccccc)
			break;

		if (_Offset)
		{
			_Offset--;
			continue;
		}

		_pSymbols[n++] = s.AddrReturn.Offset;
	}

	return n;
}

bool oDebuggerTranslateSymbol(oDEBUGGER_SYMBOL* _pSymbol, unsigned long long _Symbol)
{
	if (!_pSymbol)
		return false;

	bool success = true;

	IMAGEHLP_MODULE64 module;
	memset(&module, 0, sizeof(module));
	module.SizeOfStruct = sizeof(module);

	_pSymbol->Address = _Symbol;
	if (oWinDbgHelp::Singleton()->CallSymGetModuleInfo64(_Symbol, &module))
		oStrcpy(_pSymbol->Module, module.ModuleName);
	else
		success = false;

	BYTE buf[sizeof(IMAGEHLP_SYMBOL64) + oCOUNTOF(_pSymbol->Name) * sizeof(TCHAR)];
    IMAGEHLP_SYMBOL64* symbolInfo = (IMAGEHLP_SYMBOL64*)buf;
	memset(buf, 0, sizeof(IMAGEHLP_SYMBOL64));
    symbolInfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	symbolInfo->MaxNameLength = oCOUNTOF(_pSymbol->Name);

	DWORD64 displacement = 0;
	if (oWinDbgHelp::Singleton()->CallSymGetSymFromAddr64(_Symbol, &displacement, symbolInfo))
	{
		// symbolInfo just contains the first 512 characters and doesn't guarantee
		// they will be null-terminated, so copy the buffer and ensure there's some
		// rational terminator

		//oStrcpy(_pSymbol->Name, symbolInfo->Name);
		memcpy(_pSymbol->Name, symbolInfo->Name, sizeof(_pSymbol->Name)-sizeof(TCHAR));
		oStd::ellipsize(_pSymbol->Name);
		_pSymbol->SymbolOffset = static_cast<unsigned int>(displacement);
	}

	else
		success = false;

 	IMAGEHLP_LINE64 line;
	memset(&line, 0, sizeof(line));
	line.SizeOfStruct = sizeof(line);

	DWORD disp;
	if (oWinDbgHelp::Singleton()->CallSymGetLineFromAddr64(_Symbol, &disp, &line))
	{
		oStrcpy(_pSymbol->Filename, line.FileName);
		_pSymbol->Line = line.LineNumber;
		_pSymbol->CharOffset = static_cast<unsigned int>(displacement);
	}

	else
	{
		oStrcpy(_pSymbol->Filename, "Unknown");
		_pSymbol->Line = 0;
		_pSymbol->CharOffset = 0;
	}

	return success;
}

int oDebuggerSymbolSPrintf(char* _Buffer, size_t _SizeofBuffer, unsigned long long _Symbol, const char* _PrefixString, bool* _pIsStdBind)
{
	int rv = 0;
	oDEBUGGER_SYMBOL s;
	oDebuggerTranslateSymbol(&s, _Symbol);
	if (_pIsStdBind && oIsStdBindImplementationDetail(s.Name))
	{
		if (!*_pIsStdBind)
		{
			rv = _snprintf_s(_Buffer, _SizeofBuffer, _TRUNCATE, "%s... std::bind ...\n", _PrefixString);
			*_pIsStdBind = true;
		}
	}

	else
	{
		if (_pIsStdBind)
			*_pIsStdBind = false;

		if (s.Line && s.CharOffset)
			rv = _snprintf_s(_Buffer, _SizeofBuffer, _TRUNCATE, "%s%s!%s() ./%s Line %i + 0x%0x bytes\n", _PrefixString, s.Module.c_str(), s.Name.c_str(), oGetFilebase(s.Filename), s.Line, s.CharOffset);
		else if (s.Line)
			rv = _snprintf_s(_Buffer, _SizeofBuffer, _TRUNCATE, "%s%s!%s() ./%s Line %i\n", _PrefixString, s.Module.c_str(), s.Name.c_str(), oGetFilebase(s.Filename), s.Line);
		else
			rv = _snprintf_s(_Buffer, _SizeofBuffer, _TRUNCATE, "%s%s!%s() ./%s\n", _PrefixString, s.Module.c_str(), s.Name.c_str(), oGetFilebase(s.Filename));
	}

	return rv;
}

// {7118E248-FFAE-4A85-998B-AD7DE3B38CE5}
static const oGUID oDebuggerGuardedGUID = 
{ 0x7118e248, 0xffae, 0x4a85, { 0x99, 0x8b, 0xad, 0x7d, 0xe3, 0xb3, 0x8c, 0xe5 } };

struct GuardedAllocInfo : public oDebuggerAllocationInfo
{
	GuardedAllocInfo(size_t _AllocSize)
		: GuardedGUID(oDebuggerGuardedGUID)
	    , AllocSize(_AllocSize)
	{
		ThreadFreedOn = oInvalid;
		FreedTimer = 0.0;
	}

	oGUID GuardedGUID;
	size_t AllocSize;
};

void* oDebuggerGuardedAlloc(size_t _SizeofData, int _TrailingGuardPages)
{
	// Allocate requested space plus _TrailingGuardPages + 1 (for the leading page) padding
	const size_t PageSize = oPageGetPageSize();

	size_t NonGuardedSizePlusStartPage = oStd::byte_align(_SizeofData, PageSize) + PageSize;
	size_t AllocSize = NonGuardedSizePlusStartPage + _TrailingGuardPages * PageSize;
	void* pData = oPageReserveAndCommit(nullptr, AllocSize);
	oASSERT(pData, "oPageReserveAndCommit failed");

	oStd::memset4(pData, 0xDEB60011, AllocSize);
	// Fill in the alloc info
	new(pData) GuardedAllocInfo(AllocSize);

	// Turn off writing on all pages past the NonGuardedSize
	oPageSetNoAccess(pData, PageSize);
	oPageSetNoAccess(oStd::byte_add(pData, NonGuardedSizePlusStartPage), AllocSize - NonGuardedSizePlusStartPage);

	// Align the allocation such that the range ends on the first guarded page
	return oStd::byte_add(pData, NonGuardedSizePlusStartPage - _SizeofData );
}

void* GetAllocStart(void* _pData)
{
	const size_t PageSize = oPageGetPageSize();
	void* pPageBoundary = oStd::byte_align(_pData, PageSize);
	return oStd::byte_sub(pPageBoundary, pPageBoundary == _pData ? PageSize : 2 * PageSize);
}

void oDebuggerGuardedFree( void* _pData )
{
	const size_t PageSize = oPageGetPageSize();
	void* pAllocStart = GetAllocStart(_pData);

	// Enable this to leak the memory but make it non-accessible
#if 0
	oPageSetReadWrite(pAllocStart, PageSize, true);
	GuardedAllocInfo* pAllocInfo = (GuardedAllocInfo*)pAllocStart;
	pAllocInfo->ThreadFreedOn = GetCurrentThreadId();
	pAllocInfo->FreedTimer = oTimer();

	oPageSetNoAccess(pAllocStart, pAllocInfo->AllocSize);
#else
	oPageDecommit(pAllocStart);
#endif
}

bool oDebuggerGuardedInfo(void* _pData, oDebuggerAllocationInfo* _pInfo)
{
	const size_t PageSize = oPageGetPageSize();
	void* pAllocStart = GetAllocStart(_pData);

	DWORD OldAccess;
	if( !VirtualProtect(pAllocStart, PageSize, PAGE_EXECUTE_READ, &OldAccess) )
		return oErrorSetLast(std::errc::permission_denied, "Couldn't set read access");

	GuardedAllocInfo AllocInfo = *(GuardedAllocInfo*)pAllocStart;
	VirtualProtect(pAllocStart, PageSize, OldAccess, &OldAccess);

	if(oDebuggerGuardedGUID != AllocInfo.GuardedGUID)
		return oErrorSetLast(std::errc::invalid_argument, "Not a guarded allocation");

	*_pInfo = AllocInfo;
	return true;
}
