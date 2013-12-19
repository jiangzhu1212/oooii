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
// Declarations of unit tests of components found in oBasis
#pragma once
#ifndef oBasisTests_h
#define oBasisTests_h

#include <oBase/path.h>
#include <oBasis/oPlatformFeatures.h>
#include <oSurface/surface.h>

struct oBasisTestServices
{
	std::function<bool(ouro::path& _AbsolutePath, const ouro::path& _RelativePath, bool _PathMustExist)> ResolvePath;
	std::function<bool(void** _ppBuffer, size_t* _pSize, const char* _FullPath, bool _AsText)> AllocateAndLoadBuffer;
	std::function<void(void* _pBuffer)> DeallocateLoadedBuffer;
	std::function<int()> Rand;
	std::function<size_t()> GetTotalPhysicalMemory;
};

// oBasisTests follows a pattern: all functions return true if successful, false
// if not successful. In either case oErrorSetLast is used to set a string as
// to what occurred. In success the last error is set to 0.

oAPI bool oBasisTest_oBuffer(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oCompression(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oCoroutine();

// This prints a benchmark string to oErrorGetLastString() even if this function
// returns true, so if client code is careful about passing through and doesn't
// have any other errors, the speed of a successful run of this test will be
// reported.
oAPI bool oBasisTest_oDispatchQueueConcurrent();

// Runs the same test as oBasisTest_oDispatchQueueConcurrent for comparison 
// purposes and also to confirm that a platform implementation of 
// oConcurrency::parallel_for performs correctly.
oAPI bool oBasisTest_oDispatchQueueParallelFor();

oAPI bool oBasisTest_oDispatchQueueGlobal();
oAPI bool oBasisTest_oDispatchQueuePrivate();
oAPI bool oBasisTest_oFilterChain();
oAPI bool oBasisTest_oHash(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oIndexAllocator();
oAPI bool oBasisTest_oINISerialize();
oAPI bool oBasisTest_oJSONSerialize();
oAPI bool oBasisTest_oInt();
oAPI bool oBasisTest_oLinearAllocator();
oAPI bool oBasisTest_oConcurrentIndexAllocator();
oAPI bool oBasisTest_oMath();
oAPI bool oBasisTest_oOBJ(const oBasisTestServices& _Services);
oAPI bool oBasisTest_oOSC();
oAPI bool oBasisTest_oRTTI();
oAPI bool oBasisTest_oString();
oAPI bool oBasisTest_oTaskGroup();
oAPI bool oBasisTest_oThreadpool();
oAPI bool oBasisTest_oThreadpoolTrivial();
oAPI bool oBasisTest_oURI();
oAPI bool oBasisTest_oURIQuerySerialize();
oAPI bool oBasisTest_oXML();
oAPI bool oBasisTest_oXMLSerialize();
oAPI bool oBasisTest_oCountdownLatch();

#endif
