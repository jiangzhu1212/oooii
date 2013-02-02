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
// Declarations of unit tests of components found in oBasis
#pragma once
#ifndef oBasisTests_h
#define oBasisTests_h

#include <oBasis/oFunction.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oSurface.h>

struct oBasisTestServices
{
	oFUNCTION<bool(char* _ResolvedFullPath, size_t _SizeofResolvedFullPath, const char* _RelativePath, bool _PathMustExist)> ResolvePath;
	oFUNCTION<bool(void** _ppBuffer, size_t* _pSize, const char* _FullPath, bool _AsText)> AllocateAndLoadBuffer;
	oFUNCTION<void(void* _pBuffer)> DeallocateLoadedBuffer;
	oFUNCTION<int()> Rand;
	oFUNCTION<size_t()> GetTotalPhysicalMemory;
	
	// Fill pointers with values loaded from the specified _URIReference. _pHandle
	// receives an opaque state object that will hold the memory that is pointed
	// at by *_pMapped and should have DeallocateSurface() called on it when 
	// finished.
	oFUNCTION<bool(void** _pHandle, oSURFACE_DESC* _pDesc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pMapped, const char* _URIReference)> AllocateAndLoadSurface;
	oFUNCTION<void(void* _Handle)> DeallocateSurface;

	oFUNCTION<bool(const char* _Name, const oSURFACE_DESC& _SourceDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SourceMapped, unsigned int _NthImage, int _ColorChannelTolerance, float _MaxRMSError, unsigned int _DiffImageMultiplier)> TestSurface;
};

// oBasisTests follows a pattern: all functions return true if successful, false
// if not successful. In either case oErrorSetLast is used to set a string as
// to what occurred. In success the last error is set to oERROR_NONE.

// The oFUNCTION parameter should return how much physical RAM the platform has
// because using up the whole physical memory block can be very slow on paging
// operating systems and impossible on non-paging systems, so this tries to be
// a bit smart about running the test.
oAPI bool oBasisTest_oAllocatorTLSF(const oBasisTestServices& _Services);

// This requires a function to be specified that can return a random value on 
// the range [0,1].
oAPI bool oBasisTest_oAtof(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oBuffer(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oBlockAllocatorFixed();
oAPI bool oBasisTest_oBlockAllocatorGrowable();
oAPI bool oBasisTest_oCompression(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oConcurrentStack();
oAPI bool oBasisTest_oCSV();
oAPI bool oBasisTest_oDate(const oBasisTestServices& _Services);

// This prints a benchmark string to oErrorGetLastString() even if this function
// returns true, so if client code is careful about passing through and doesn't
// have any other errors, the speed of a successful run of this test will be
// reported.
oAPI bool oBasisTest_oDispatchQueueConcurrent();

// Runs the same test as oBasisTest_oDispatchQueueConcurrent for comparison 
// purposes and also to confirm that a platform implementation of 
// oTaskParallelFor performs correctly.
oAPI bool oBasisTest_oDispatchQueueParallelFor();

oAPI bool oBasisTest_oDispatchQueueGlobal();
oAPI bool oBasisTest_oDispatchQueuePrivate();
oAPI bool oBasisTest_oEightCC();
oAPI bool oBasisTest_oFilterChain();
oAPI bool oBasisTest_oFourCC();
oAPI bool oBasisTest_oHash(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oIndexAllocator();
oAPI bool oBasisTest_oINI();
oAPI bool oBasisTest_oInt();
oAPI bool oBasisTest_oLinearAllocator();
oAPI bool oBasisTest_oConcurrentIndexAllocator();
oAPI bool oBasisTest_oMath();
oAPI bool oBasisTest_oOBJ(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oOSC();
oAPI bool oBasisTest_oPath();
oAPI bool oBasisTest_oRTTI();
oAPI bool oBasisTest_tbbConcurrentQueue();
oAPI bool oBasisTest_concrtConcurrentQueue();
oAPI bool oBasisTest_oConcurrentQueue();
oAPI bool oBasisTest_oString();
oAPI bool oBasisTest_oSurface();
oAPI bool oBasisTest_oSurfaceResize(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oURI();
oAPI bool oBasisTest_oXML();
oAPI bool oBasisTest_oCountdownLatch();

#endif
