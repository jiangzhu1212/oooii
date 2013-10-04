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
#include <oPlatform/oTest.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oStreamUtil.h>
#include <oCore/system.h>
#include <oBasis/tests/oBasisTests.h>
#include "oTestIntegration.h"
#include "TESTConcurrencyRequirements.h"

using namespace ouro;

static size_t GetTotalPhysicalMemory()
{
	ouro::system::heap_info hi = ouro::system::get_heap_info();
	return static_cast<size_t>(hi.total_physical);
}

static bool ResolvePath(path& _Path, const char* _RelativePath, bool _PathMustExist, oTest* _pTest)
{
	if (_PathMustExist)	
		return _pTest->BuildPath(_Path, _RelativePath, oTest::DATA, oTest::FileMustExist);
	else return _pTest->BuildPath(_Path, _RelativePath, oTest::DATA);
}

static bool TestSurface(const char* _Name, const ouro::surface::info& _SourceInfo, const ouro::surface::const_mapped_subresource& _SourceMapped, unsigned int _NthImage, int _ColorChannelTolerance, float _MaxRMSError, unsigned int _DiffImageMultiplier, oTest* _pTest)
{
	intrusive_ptr<oImage> image;
	if (!oImageCreate(_Name, _SourceInfo, &image))
		return false; // pass through error
	image->CopyData(_SourceMapped.data, _SourceMapped.row_pitch);
	return _pTest->TestImage(image, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier);
}

static bool AllocateAndLoadSurface(void** _pHandle, ouro::surface::info* _pInfo, ouro::surface::const_mapped_subresource* _pMapped, const char* _URIReference)
{
	intrusive_ptr<oImage> image;
	if (!oImageLoad(_URIReference, &image))
		return false; // pass through error

	*_pInfo = oImageGetSurfaceInfo(image);
	int imageMapSize = ouro::surface::subresource_size(*_pInfo, 0);
	std::vector<char>* pBuf = new std::vector<char>();
	pBuf->resize(imageMapSize);
	*_pMapped = ouro::surface::get_const_mapped_subresource(*_pInfo, 0, 0, pBuf->data());
	image->CopyDataTo((void*)_pMapped->data, _pMapped->row_pitch);
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
	_pServices->ResolvePath = oBIND(ResolvePath, oBIND1, oBIND2, oBIND3, _pTest);
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
			{	case std::errc::permission_denied: r = oTest::SKIPPED; break; \
				case 0: r = oTest::SUCCESS; break; \
				default: r = oTest::FAILURE; break; \
			} \
		} \
		snprintf(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); \
		oTRACE("%s: %s (oErrorGetLast() == %s)", ouro::as_string(r), _StrStatus, oErrorAsString(oErrorGetLast())); \
		return r; \
	} while (false)

#define oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(oBasis_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oTESTB0_BASIS(oCONCAT(oBasisTest_, _BasisTestName)()); \
		} \
	};

#define oTEST_REGISTER_BASIS_TEST(_BasisTestName) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER(oCONCAT(oBasis_, _BasisTestName))

#define oTEST_REGISTER_BASIS_TEST_BUGGED(_BasisTestName, _Bug) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER_BUGGED(oCONCAT(oBasis_, _BasisTestName), _Bug)

#define oTEST_REGISTER_BASIS_TEST_BUGGED32(_BasisTestName, _Bug) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER_BUGGED32(oCONCAT(oBasis_, _BasisTestName), _Bug)

#define oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(oBasis_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oBasisTestServices Services; \
			oInitBasisServices(this, &Services); \
			oTESTB0_BASIS(oCONCAT(oBasisTest_, _BasisTestName)(Services)); \
		} \
	}; \
	oTEST_REGISTER(oCONCAT(oBasis_, _BasisTestName))

oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oAllocatorTLSF);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oCompression);
oTEST_REGISTER_BASIS_TEST(oDispatchQueueGlobal);
oTEST_REGISTER_BASIS_TEST(oDispatchQueuePrivate);
oTEST_REGISTER_BASIS_TEST(oEasing);
oTEST_REGISTER_BASIS_TEST(oFilterChain);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oHash);
oTEST_REGISTER_BASIS_TEST(oINISerialize);
oTEST_REGISTER_BASIS_TEST(oJSON);
oTEST_REGISTER_BASIS_TEST(oJSONSerialize);
oTEST_REGISTER_BASIS_TEST(oInt);
oTEST_REGISTER_BASIS_TEST(oMath);
oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(oOBJ);
oTEST_REGISTER_BASIS_TEST(oOSC);
oTEST_REGISTER_BASIS_TEST(oRTTI);
oTEST_REGISTER_BASIS_TEST(oString);
oTEST_REGISTER_BASIS_TEST(oURI);
oTEST_REGISTER_BASIS_TEST(oURIQuerySerialize);
oTEST_REGISTER_BASIS_TEST(oXMLSerialize);
