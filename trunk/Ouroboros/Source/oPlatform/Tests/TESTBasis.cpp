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
#include <oPlatform/oTest.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oSystem.h>
#include <oBasisTests/oBasisTests.h>

static size_t GetTotalPhysicalMemory()
{
	oSYSTEM_HEAP_STATS stats;
	if (!oSystemGetHeapStats(&stats))
		return 0;
	return static_cast<size_t>(stats.TotalPhysical);
}

static bool ResolvePath(char* _ResolvedFullPath, size_t _SizeofResolvedFullPath, const char* _RelativePath, bool _PathMustExist, oTest* _pTest)
{
	if (_PathMustExist)	
		return _pTest->BuildPath(_ResolvedFullPath, _SizeofResolvedFullPath, _RelativePath, oTest::DATA, oTest::FileMustExist);
	else return _pTest->BuildPath(_ResolvedFullPath, _SizeofResolvedFullPath, _RelativePath, oTest::DATA);
}

static bool TestSurface(const char* _Name, const oSURFACE_DESC& _SourceDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SourceMapped, unsigned int _NthImage, int _ColorChannelTolerance, float _MaxRMSError, unsigned int _DiffImageMultiplier, oTest* _pTest)
{
	oRef<oImage> image;
	if (!oImageCreate(_Name, _SourceDesc, &image))
		return false; // pass through error
	image->CopyData(_SourceMapped.pData, _SourceMapped.RowPitch);
	return _pTest->TestImage(image, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier);
}

static bool AllocateAndLoadSurface(void** _pHandle, oSURFACE_DESC* _pDesc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pMapped, const char* _URIReference)
{
	oRef<oImage> image;
	if (!oImageLoad(_URIReference, &image))
		return false; // pass through error

	oImageGetSurfaceDesc(image, _pDesc);
	int imageMapSize = oSurfaceSubresourceCalcSize(*_pDesc, 0);
	std::vector<char>* pBuf = new std::vector<char>();
	pBuf->resize(imageMapSize);
	oSurfaceCalcMappedSubresource(*_pDesc, 0, 0, pBuf->data(), _pMapped);
	image->CopyDataTo((void*)_pMapped->pData, _pMapped->RowPitch);
	*_pHandle = pBuf;
	return true;
}

static void DeallocateSurface(void* _Handle)
{
	std::vector<char>* pBuf = static_cast<std::vector<char>*>(_Handle);
	delete pBuf;
}

static void oInitBasisServices(oTest* _pTest, oBasisTestServices* _pServices)
{
	_pServices->ResolvePath = oBIND(ResolvePath, oBIND1, oBIND2, oBIND3, oBIND4, _pTest);
	_pServices->AllocateAndLoadBuffer = oBIND(oStreamLoad, oBIND1, oBIND2, malloc, free, oBIND3, oBIND4);
	_pServices->DeallocateLoadedBuffer = free;
	_pServices->Rand = rand;
	_pServices->GetTotalPhysicalMemory = GetTotalPhysicalMemory;
	_pServices->AllocateAndLoadSurface = AllocateAndLoadSurface;
	_pServices->DeallocateSurface = DeallocateSurface;
	_pServices->TestSurface = oBIND(TestSurface, oBIND1, oBIND2, oBIND3, oBIND4, oBIND5, oBIND6, oBIND7, _pTest);
}

// Tests from oBasis follow a common form, so as a convenience and 
// centralization, use this macro to get through the boilerplate bridging from
// that test call to oTest's infrastructure.

#define oTESTB0_BASIS(expr) do \
	{	RESULT r = oTest::SUCCESS; \
		if (!(expr)) \
		{	switch (oErrorGetLast()) \
			{	case oERROR_REFUSED: r = oTest::SKIPPED; break; \
				case oERROR_LEAKS: r = oTest::LEAKS; break; \
				case oERROR_NONE: r = oTest::SUCCESS; break; \
				default: r = oTest::FAILURE; break; \
			} \
		} \
		oPrintf(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); \
		oTRACE("%s: %s (oErrorGetLast() == %s)", oAsString(r), _StrStatus, oAsString(oErrorGetLast())); \
		return r; \
	} while (false)

#define oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(BASIS_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oTESTB0_BASIS(oCONCAT(oBasisTest_, _BasisTestName)()); \
		} \
	};

#define oTEST_REGISTER_BASIS_TEST(_BasisTestName) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER(oCONCAT(BASIS_, _BasisTestName))

#define oTEST_REGISTER_BASIS_TEST_BUGGED(_BasisTestName, _Bug) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER_BUGGED(oCONCAT(BASIS_, _BasisTestName), _Bug)

#define oTEST_REGISTER_BASIS_TEST_BUGGED32(_BasisTestName, _Bug) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER_BUGGED32(oCONCAT(BASIS_, _BasisTestName), _Bug)

#define oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(BASIS_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oBasisTestServices Services; \
			oInitBasisServices(this, &Services); \
			oTESTB0_BASIS(oCONCAT(oBasisTest_, _BasisTestName)(Services)); \
		} \
	}; \
	oTEST_REGISTER(oCONCAT(BASIS_, _BasisTestName))

oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oAllocatorTLSF);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oAtof);
oTEST_REGISTER_BASIS_TEST(oBlockAllocatorFixed);
oTEST_REGISTER_BASIS_TEST(oBlockAllocatorGrowable);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oCompression);
oTEST_REGISTER_BASIS_TEST(oConcurrentIndexAllocator);
oTEST_REGISTER_BASIS_TEST(oConcurrentQueue);
//oTEST_REGISTER_BASIS_TEST(tbbConcurrentQueue);
//oTEST_REGISTER_BASIS_TEST(concrtConcurrentQueue);
oTEST_REGISTER_BASIS_TEST(oConcurrentStack);
oTEST_REGISTER_BASIS_TEST(oCountdownLatch);
oTEST_REGISTER_BASIS_TEST(oCSV);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oDate);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueConcurrent);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueGlobal);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueParallelFor);
oTEST_REGISTER_BASIS_TEST(oDispatchQueuePrivate);
oTEST_REGISTER_BASIS_TEST(oEightCC);
oTEST_REGISTER_BASIS_TEST(oFilterChain);
oTEST_REGISTER_BASIS_TEST(oFourCC);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oHash);
oTEST_REGISTER_BASIS_TEST(oIndexAllocator);
oTEST_REGISTER_BASIS_TEST(oINI);
oTEST_REGISTER_BASIS_TEST(oInt);
oTEST_REGISTER_BASIS_TEST(oLinearAllocator);
oTEST_REGISTER_BASIS_TEST(oMath);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oOBJ);
oTEST_REGISTER_BASIS_TEST(oOSC);
oTEST_REGISTER_BASIS_TEST(oPath);
oTEST_REGISTER_BASIS_TEST(oRTTI);
oTEST_REGISTER_BASIS_TEST(oStdFuture);
oTEST_REGISTER_BASIS_TEST(oSurface);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oSurfaceResize);
oTEST_REGISTER_BASIS_TEST(oURI);
oTEST_REGISTER_BASIS_TEST(oXML);
