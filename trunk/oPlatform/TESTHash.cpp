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
#include <oBasisTests/oBasisTests.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oSystem.h>

// @oooii-tony: For some reason oBIND'ing oFileLoad directly doesn't work...
// maybe the templates get confused with the various flavors? Anyway, here's an
// exact wrapper just to change the name of the function.
//bool oFileLoad2(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText)
//{
//	return oFileLoad(_ppOutBuffer, _pOutSize, _Allocate, _Path, _AsText);
//}
//
//size_t GetTotalPhysicalMemory()
//{
//	oSYSTEM_HEAP_STATS stats;
//	if (!oSystemGetHeapStats(&stats))
//		return 0;
//	return static_cast<size_t>(stats.TotalPhysical);
//}
//
//struct TESTHash : public oTest
//{
//	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
//	{
//		oBasisTestServices Services;
//		Services.ResolvePath = oBIND(&oTestManager::FindFullPath, oTestManager::Singleton(), oBIND1, oBIND2, oBIND3);
//		Services.AllocateAndLoadBuffer = oBIND(oFileLoad2, oBIND1, oBIND2, malloc, oBIND3, false);
//		Services.FreeLoadedBuffer = free;
//		Services.Rand = rand;
//		Services.GetTotalPhysicalMemory = GetTotalPhysicalMemory;
//
//		oTESTB(oBasisTest_oHash(Services), "%s", oErrorGetLastString());
//		return SUCCESS;
//	}
//};

//oTEST_REGISTER(TESTHash);
