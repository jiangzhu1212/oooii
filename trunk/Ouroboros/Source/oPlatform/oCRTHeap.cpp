/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/Windows/oCRTHeap.h>
#include <oPlatform/oProcessHeap.h>
#include <crtdbg.h>
#include "oStaticMutex.h"

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

bool oCRTHeapIsValidPointer(void* _Pointer)
{
	return !!_CrtIsValidHeapPointer(_Pointer);
}

void* oCRTHeapGetPointer(struct _CrtMemBlockHeader* _pMemBlockHeader)
{
	return (void*)(_pMemBlockHeader+1);
}

void* oCRTHeapGetNextPointer(void* _Pointer)
{
	return oCRTHeapGetPointer(pHdr(_Pointer)->pBlockHeaderNext);
}

size_t oCRTHeapGetSize(void* _Pointer)
{
	return _Pointer ? pHdr(_Pointer)->nDataSize : 0;
}

bool oCRTHeapIsFree(void* _Pointer)
{
	return pHdr(_Pointer)->nBlockUse == _FREE_BLOCK;
}

bool oCRTHeapIsNormal(void* _Pointer)
{
	return pHdr(_Pointer)->nBlockUse == _NORMAL_BLOCK;
}

bool oCRTHeapIsCRT(void* _Pointer)
{
	return pHdr(_Pointer)->nBlockUse == _CRT_BLOCK;
}

bool oCRTHeapIsIgnore(void* _Pointer)
{
	return pHdr(_Pointer)->nBlockUse == _IGNORE_BLOCK;
}

bool oCRTHeapIsClient(void* _Pointer)
{
	return pHdr(_Pointer)->nBlockUse == _CLIENT_BLOCK;
}

const char* oCRTHeapGetAllocationFilename(void* _Pointer)
{
	return pHdr(_Pointer)->szFileName;
}

unsigned int oCRTHeapGetAllocationLine(void* _Pointer)
{
	return static_cast<unsigned int>(pHdr(_Pointer)->nLine);
}

uintptr_t oCRTHeapGetAllocationID(void* _Pointer)
{
	return static_cast<uintptr_t>(pHdr(_Pointer)->lRequest);
}

void oCRTHeapBreakOnAllocation(uintptr_t _AllocationID)
{
	_CrtSetBreakAlloc((long)_AllocationID);
}

// {50063038-C83F-4C13-9DC3-9D2F299AD4CB}
static const oGUID guid = { 0x50063038, 0xc83f, 0x4c13, { 0x9d, 0xc3, 0x9d, 0x2f, 0x29, 0x9a, 0xd4, 0xcb } };
oDEFINE_STATIC_MUTEX(oMutex, sCRTReportLeakMutex, GUID);

void oCRTHeapEnableAtExitLeakReport(bool _Enable)
{
	oLockGuard<oStaticMutex<oMutex, sCRTReportLeakMutex_t>> Lock(sCRTReportLeakMutex);

	int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(_Enable ? (flags | _CRTDBG_LEAK_CHECK_DF) : (flags &~ _CRTDBG_LEAK_CHECK_DF));
}

bool oCRTHeapEnableMemoryTracking(bool _Enable)
{
	oLockGuard<oStaticMutex<oMutex, sCRTReportLeakMutex_t>> Lock(sCRTReportLeakMutex);
	int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(_Enable ? (flags | _CRTDBG_ALLOC_MEM_DF) : (flags &~ _CRTDBG_ALLOC_MEM_DF));
	return (flags & _CRTDBG_ALLOC_MEM_DF) == _CRTDBG_ALLOC_MEM_DF;
}
