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
#include <oStd/finally.h>
#include <oStd/timer.h>
#include <oBasis/oSurfaceResize.h>
#include <oBasis/oTimer.h>
#include "oBasisTestCommon.h"
#include <oBasis/tests/oBasisTests.h>
#include <vector>

static bool oBasisTest_oSurfaceResize_TestSize(const oBasisTestServices& _Services, const oSURFACE_DESC& _SourceDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SourceMapped, oSURFACE_FILTER _Filter, const int3& _NewSize, unsigned int _NthImage)
{
	oSURFACE_DESC destDesc;
	destDesc = _SourceDesc;
	destDesc.Dimensions = _NewSize;
	int destMapSize = oSurfaceSubresourceCalcSize(destDesc, 0);
	std::vector<char> destMapData;
	destMapData.resize(destMapSize);
	oSURFACE_MAPPED_SUBRESOURCE destMap;
	oSurfaceCalcMappedSubresource(destDesc, 0, 0, destMapData.data(), &destMap);

	{
		oStd::scoped_timer timer("resize time");
		oSurfaceResize(_SourceDesc, _SourceMapped, destDesc, &destMap, _Filter);
	}

	return _Services.TestSurface("oSurfaceResize result", destDesc, destMap, _NthImage, oDEFAULT, -1.0f, oDEFAULT);
}

static bool oBasisTest_oSurfaceResize_TestFilter(const oBasisTestServices& _Services, const oSURFACE_DESC& _SourceDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SourceMapped, oSURFACE_FILTER _Filter, unsigned int _NthImage)
{
	if (!oBasisTest_oSurfaceResize_TestSize(_Services, _SourceDesc, _SourceMapped, _Filter, _SourceDesc.Dimensions * int3(2,2,1), _NthImage))
		return false; // pass through error
	if (!oBasisTest_oSurfaceResize_TestSize(_Services, _SourceDesc, _SourceMapped, _Filter, _SourceDesc.Dimensions / int3(2,2,1), _NthImage+1))
		return false; // pass through error
	return true;
}

bool oBasisTest_oSurfaceResize(const oBasisTestServices& _Services)
{
	static const char* testImage = "file://DATA/Test/Textures/lena_1.png";

	void* hSurface = nullptr;
	oSURFACE_DESC SourceDesc;
	oSURFACE_CONST_MAPPED_SUBRESOURCE SourceMapped;
	if (!_Services.AllocateAndLoadSurface(&hSurface, &SourceDesc, &SourceMapped, testImage))
		return false; // pass through error

	oStd::finally ose([&] { _Services.DeallocateSurface(hSurface); });

	unsigned int NthImage = 0;
	for (int i = 0; i < oSURFACE_FILTER_COUNT; i++, NthImage += 2)
	{
		if (!oBasisTest_oSurfaceResize_TestFilter(_Services, SourceDesc, SourceMapped, oSURFACE_FILTER(i), NthImage))
			return false; // pass through error
	}

	oErrorSetLast(0);
	return true;
}
