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
#include <oBase/finally.h>
#include <oBase/timer.h>
#include <oBasis/oSurfaceResize.h>
#include <oBasis/oTimer.h>
#include "oBasisTestCommon.h"
#include <oBasis/tests/oBasisTests.h>
#include <vector>

using namespace ouro;

static bool oBasisTest_oSurfaceResize_TestSize(const oBasisTestServices& _Services, const ouro::surface::info& _SourceInfo, const ouro::surface::const_mapped_subresource& _SourceMapped, ouro::surface::filter::value _Filter, const int3& _NewSize, unsigned int _NthImage)
{
	ouro::surface::info destInfo = _SourceInfo;
	destInfo.dimensions = _NewSize;
	int destMapSize = ouro::surface::total_size(destInfo, 0);
	std::vector<char> destMapData;
	destMapData.resize(destMapSize);
	ouro::surface::mapped_subresource destMap = ouro::surface::get_mapped_subresource(destInfo, 0, 0, destMapData.data());

	{
		scoped_timer timer("resize time");
		ouro::surface::resize(_SourceInfo, _SourceMapped, destInfo, &destMap, _Filter);
	}

	return _Services.TestSurface("oSurfaceResize result", destInfo, destMap, _NthImage, oDEFAULT, -1.0f, oDEFAULT);
}

static bool oBasisTest_oSurfaceResize_TestFilter(const oBasisTestServices& _Services, const ouro::surface::info& _SourceInfo, const ouro::surface::const_mapped_subresource& _SourceMapped, ouro::surface::filter::value _Filter, unsigned int _NthImage)
{
	if (!oBasisTest_oSurfaceResize_TestSize(_Services, _SourceInfo, _SourceMapped, _Filter, _SourceInfo.dimensions * int3(2,2,1), _NthImage))
		return false; // pass through error
	if (!oBasisTest_oSurfaceResize_TestSize(_Services, _SourceInfo, _SourceMapped, _Filter, _SourceInfo.dimensions / int3(2,2,1), _NthImage+1))
		return false; // pass through error
	return true;
}

bool oBasisTest_oSurfaceResize(const oBasisTestServices& _Services)
{
	static const char* testImage = "file://DATA/Test/Textures/lena_1.png";

	void* hSurface = nullptr;
	ouro::surface::info SourceInfo;
	ouro::surface::const_mapped_subresource SourceMapped;
	if (!_Services.AllocateAndLoadSurface(&hSurface, &SourceInfo, &SourceMapped, testImage))
		return false; // pass through error

	ouro::finally ose([&] { _Services.DeallocateSurface(hSurface); });

	unsigned int NthImage = 0;
	for (int i = 0; i < ouro::surface::filter::filter_count; i++, NthImage += 2)
	{
		if (!oBasisTest_oSurfaceResize_TestFilter(_Services, SourceInfo, SourceMapped, ouro::surface::filter::value(i), NthImage))
			return false; // pass through error
	}

	oErrorSetLast(0);
	return true;
}
