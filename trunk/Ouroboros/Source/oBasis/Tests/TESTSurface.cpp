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
#include <oBasis/oSurface.h>
#include <oBasis/oError.h>
#include <oStd/macros.h>
#include "oBasisTestCommon.h"

#define oTESTB2(test) do { if (!(test)) return false; } while(false)

bool Test_oSurfaceMipCalcRowPitch(int _Depth, int _ArraySize)
{
	// This tests whether the row pitch remains the same per mip level regardless of depth or number of slices
	oSURFACE_DESC desc;
	desc.Dimensions = int3(512,512, _Depth);
	desc.ArraySize = _ArraySize;
	desc.Format = oSURFACE_R8G8B8A8_UNORM;

	int RowPitches[10] = { 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4 };

	desc.Layout = oSURFACE_LAYOUT_IMAGE;
	oFORI(mipLevel, RowPitches)
	{
		oTESTB(oSurfaceMipCalcRowPitch(desc, 0) == RowPitches[mipLevel], "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_IMAGE failed", _Depth, _ArraySize);
		desc.Dimensions = oSurfaceMipCalcDimensionsNPOT(desc.Format, desc.Dimensions, 1);
	}
	desc.Dimensions = int3(512,512,_Depth);

	desc.Layout = oSURFACE_LAYOUT_TIGHT;
	oFORI(mipLevel, RowPitches)
		oTESTB(oSurfaceMipCalcRowPitch(desc, (int)mipLevel) == RowPitches[mipLevel], "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_TIGHT failed", _Depth, _ArraySize);

	desc.Layout = oSURFACE_LAYOUT_BELOW;
	oFORI(mipLevel, RowPitches)
		oTESTB(oSurfaceMipCalcRowPitch(desc, (int)mipLevel) == RowPitches[0], "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_BELOW failed", _Depth, _ArraySize);

	desc.Layout = oSURFACE_LAYOUT_RIGHT;
	oFORI(mipLevel, RowPitches)
		oTESTB(oSurfaceMipCalcRowPitch(desc, (int)mipLevel) == (RowPitches[0]+RowPitches[1]), "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_RIGHT failed", _Depth, _ArraySize);

	return true;
}

bool Test_oSurfaceMipCalcDepthPitch(int _Depth, int _ArraySize)
{
	// This tests whether the depth pitch remains the same per mip level regardless of depth or number of slices
	oSURFACE_DESC desc;
	desc.Dimensions = int3(512,512,_Depth);
	desc.ArraySize = _ArraySize;
	desc.Format = oSURFACE_R8G8B8A8_UNORM;

	int DepthPitches[10] = { 0x100000, 0x40000, 0x10000, 0x4000, 0x1000, 0x400, 0x100, 0x40, 0x10, 0x4 };

	desc.Layout = oSURFACE_LAYOUT_IMAGE;
	oFORI(mipLevel, DepthPitches)
	{
		oTESTB(oSurfaceMipCalcDepthPitch(desc, 0) == DepthPitches[mipLevel], "Test_oSurfaceMipCalcDepthPitch() oSURFACE_LAYOUT_IMAGE failed");
		desc.Dimensions = oSurfaceMipCalcDimensionsNPOT(desc.Format, desc.Dimensions, 1);
	}
	desc.Dimensions = int3(512,512,_Depth);

	desc.Layout = oSURFACE_LAYOUT_TIGHT;
	oFORI(mipLevel, DepthPitches)
		oTESTB(oSurfaceMipCalcDepthPitch(desc, (int)mipLevel) == DepthPitches[mipLevel], "Test_oSurfaceMipCalcDepthPitch() oSURFACE_LAYOUT_TIGHT failed");

	desc.Layout = oSURFACE_LAYOUT_BELOW;
	oFORI(mipLevel, DepthPitches)
		oTESTB(oSurfaceMipCalcDepthPitch(desc, (int)mipLevel) == DepthPitches[mipLevel] << mipLevel, "oSurfaceMipCalcDepthPitch() oSURFACE_LAYOUT_BELOW failed");

	desc.Layout = oSURFACE_LAYOUT_RIGHT;
	oFORI(mipLevel, DepthPitches)
		oTESTB(oSurfaceMipCalcDepthPitch(desc, (int)mipLevel) == (DepthPitches[mipLevel] << mipLevel) + ((DepthPitches[mipLevel] >> 1) << mipLevel), "oSurfaceMipCalcDepthPitch() oSURFACE_LAYOUT_RIGHT failed");

	return true;
}

bool Test_oSurfaceSliceCalcPitch(int _ArraySize)
{
	oSURFACE_DESC desc;
	desc.Dimensions = int3(512,512,1);
	desc.ArraySize = _ArraySize;
	desc.Format = oSURFACE_R8G8B8A8_UNORM;

	if (_ArraySize > 1)
	{
		// This tests whether the slice pitch remains the same regardless of number of slices
		oSURFACE_DESC desc1 = desc;
		desc1.ArraySize = 1;

		desc.Layout = oSURFACE_LAYOUT_IMAGE;
		oTESTB(oSurfaceSliceCalcPitch(desc) == 0x00100000, "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == oSurfaceMipCalcDepthPitch(desc), "oSurfaceSliceCalcPitch() failed");
		
		desc.Layout = oSURFACE_LAYOUT_TIGHT;
		oTESTB(oSurfaceSliceCalcPitch(desc) == 0x00155800, "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == (oSurfaceCalcSize(desc) / _ArraySize), "oSurfaceSliceCalcPitch()==(oSurfaceCalcSize()/slices) failed");

		desc.Layout = oSURFACE_LAYOUT_BELOW;
		oTESTB(oSurfaceSliceCalcPitch(desc) == 0x00180000, "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == (oSurfaceCalcSize(desc) / _ArraySize), "oSurfaceSliceCalcPitch()==(oSurfaceCalcSize()/slices) failed");

		desc.Layout = oSURFACE_LAYOUT_RIGHT;
		oTESTB(oSurfaceSliceCalcPitch(desc) == 0x00180000, "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == (oSurfaceCalcSize(desc) / _ArraySize), "oSurfaceSliceCalcPitch()==(oSurfaceCalcSize()/slices) failed");
		return true;
	}

	// This tests whether the slice pitch is the same as the depth pitch multiplied by the depth
	desc.Layout = oSURFACE_LAYOUT_IMAGE;
	for(int depthLevel=0; depthLevel<10; depthLevel++)
	{
		desc.Dimensions.z = (int)depthLevel + 1;
		oTESTB(oSurfaceSliceCalcPitch(desc) == (oSurfaceMipCalcDepthPitch(desc, 0) * desc.Dimensions.z), "oSurfaceSliceCalcPitch()==(oSurfaceMipCalcDepthPitch()*depth) failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == oSurfaceCalcSize(desc), "oSurfaceSliceCalcPitch()==oSurfaceCalcSize() failed");
	}

	// The rest below tests the slice pitches for a number of depth levels, and 3d textures don't support more than one slice
	desc.Layout = oSURFACE_LAYOUT_TIGHT;
	int SlicePitches[15] = { 0x00155800, 0x00255800, 0x00355800, 0x00495800, 0x00595800, 0x006d5800, 0x007d5800, 0x00925800, 0x00a25800, 0x00b65800, 0x00c65800, 0x00db5800, 0x00eb5800, 0x00ff5800, 0x010f5800 };
	oFORI(depthLevel, SlicePitches)
	{
		desc.Dimensions.z = (int)depthLevel + 1;
		oTESTB(oSurfaceSliceCalcPitch(desc) == SlicePitches[depthLevel], "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == oSurfaceCalcSize(desc), "oSurfaceSliceCalcPitch()==oSurfaceCalcSize() failed");
	}

	desc.Layout = oSURFACE_LAYOUT_BELOW;
	int SlicePitchesVertical[15] = { 0x00180000, 0x00280000, 0x00380000, 0x00500000, 0x00600000, 0x00780000, 0x00880000, 0x00a00000, 0x00b00000, 0x00c80000, 0x00d80000, 0x00f00000, 0x01000000, 0x01180000, 0x01280000 };
	oFORI(depthLevel, SlicePitchesVertical)
	{
		desc.Dimensions.z = (int)depthLevel + 1;
		oTESTB(oSurfaceSliceCalcPitch(desc) == SlicePitchesVertical[depthLevel], "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == oSurfaceCalcSize(desc), "oSurfaceSliceCalcPitch()==oSurfaceCalcSize() failed");
	}

	desc.Layout = oSURFACE_LAYOUT_RIGHT;
	int SlicePitchesHorizontal[15] = { 0x00180000, 0x00300000, 0x00480000, 0x00600000, 0x00780000, 0x00900000, 0x00a80000, 0x00c00000, 0x00d80000, 0x00f00000, 0x01080000, 0x01200000, 0x01380000, 0x01500000, 0x01680000 };
	oFORI(depthLevel, SlicePitchesHorizontal)
	{
		desc.Dimensions.z = (int)depthLevel + 1;
		oTESTB(oSurfaceSliceCalcPitch(desc) == SlicePitchesHorizontal[depthLevel], "oSurfaceSliceCalcPitch() failed");
		oTESTB(oSurfaceSliceCalcPitch(desc) == oSurfaceCalcSize(desc), "oSurfaceSliceCalcPitch()==oSurfaceCalcSize() failed");
	}

	return true;
}

bool Test_oSurface()
{
	for (int i=1; i <= 16; ++i)
	{
		if (!Test_oSurfaceMipCalcRowPitch(i, 1)) return false;
		if (!Test_oSurfaceMipCalcRowPitch(1, i)) return false;
		if (!Test_oSurfaceMipCalcDepthPitch(i, 1)) return false;
		if (!Test_oSurfaceMipCalcDepthPitch(1, i)) return false;
		if (!Test_oSurfaceSliceCalcPitch(i)) return false;
	}

	oSURFACE_DESC desc;
	desc.Dimensions = int3(512,512,511);
	desc.ArraySize = 1;
	desc.Format = oSURFACE_R8G8B8A8_UNORM;
	desc.Layout = oSURFACE_LAYOUT_TIGHT;
	oTESTB(oSurfaceMipCalcOffset(desc, 1)==0x1ff00000, "");

	int3 MipDimensions[] = 
	{ 
		int3(1,1,1), 
		int3(2,2,2), int3(1,2,2), int3(2,1,2), int3(2,2,1), int3(1,1,2), int3(1,2,1), int3(2,1,1), 
		int3(3,3,3), int3(1,3,3), int3(3,1,3), int3(3,3,1), int3(1,1,3), int3(1,3,1), int3(3,1,1), 
		int3(4,4,4),
		int3(16,16,16),
		int3(1024,1024,1024), int3(1,1024,1024), int3(1024,1,1024), int3(1024,1024,1), int3(1,1,1024), int3(1,1024,1), int3(1,1,1024),
	};
	int NumMips[] = 
	{ 
		1, 
		2, 2, 2, 2, 2, 2, 2, 
		2, 2, 2, 2, 2, 2, 2, 
		3,
		5,
		11, 11, 11, 11, 11, 11, 11
	};

	oFORI(i, MipDimensions)
		oTESTB(oSurfaceCalcNumMips(oSURFACE_LAYOUT_TIGHT, MipDimensions[i]) == NumMips[i], "oSurfaceCalcNumMips(.., int3(%d,%d,%d))==%d failed", MipDimensions[i].x, MipDimensions[i].y, MipDimensions[i].z, NumMips[i]);

	oErrorSetLast(0, "");
	return true;
}

bool oBasisTest_oSurface()
{
	oTESTB2(Test_oSurface());
	return true;
}
    