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
#pragma once

#ifndef KinectDisplay_h
#define KinectDisplay_h

#ifdef oHAS_KINECT_SDK

#include <NuiApi.h>
#include <oPlatform/oKinect.h>

// Dimensions for displaying the depth image
#define oDEPTH_WIDTH	640
#define oDEPTH_HEIGHT	480
#define oDEPTH_RESOLUTION	NUI_IMAGE_RESOLUTION_640x480

class oKinectDisplay
{
public:
	oKinectDisplay();
	~oKinectDisplay();
	typedef oFUNCTION<void(const RGBQUAD &_Surface, int2 _FrameSize, int _RowSize)> DepthSurfaceUpdate;

	void DrawBones(const Vector4 &_BonePosition, int _Size);
	void CreateDepthDisplayImage(const NUI_IMAGE_FRAME &_ImageFrame);
	void RegisterObserver(const oCameraOnFrameFn& _OnFrame) { Observers.push_back(_OnFrame); }
	void RemoveObservers();
private:	
	void PaintPixel(float2 _Pixel2D, int _OffsetX, int _OffsetY, int _MaxDistFromOrigin, RGBQUAD _Color);
	void CreatePoint(float2 _CenterPt, int _Radius, RGBQUAD _Color);
	//void DrawFPS();
	void ClearBones();

	double DepthFramesTotal;
	double LastDepthFPStime;
	double LastDepthFramesTotal;

	// Color array for copying to oWindow
	RGBQUAD RGBWk[oDEPTH_WIDTH*oDEPTH_HEIGHT];
	// Color array for displaying joints over the depth image
	RGBQUAD GesturePointMap[oDEPTH_WIDTH*oDEPTH_HEIGHT];
	std::vector<oCameraOnFrameFn> Observers;
};

#endif

#endif