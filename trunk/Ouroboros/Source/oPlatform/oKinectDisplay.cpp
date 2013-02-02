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
#include "oKinectDisplay.h"
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oBasis/oFor.h>

#ifdef oHAS_KINECT_SDK

static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

oKinectDisplay::oKinectDisplay()
	: DepthFramesTotal(0)
	, LastDepthFPStime(0)
	, LastDepthFramesTotal(0)
{
}

oKinectDisplay::~oKinectDisplay()
{
}

// Font & Text to display in oWindow
//struct UI_TEXT
//{
//	oGDIScopedObject<HFONT> hFont;
//	oGUI_TEXT_DESC TextDesc;
//	unsigned int ExtraHook;
//	oStringL Text;
//
//	UI_TEXT()
//		: ExtraHook(oInvalid)
//	{
//		oGUI_FONT_DESC fd;
//		fd.Italic = true;
//		fd.PointSize = 24.0f;
//		hFont = oGDICreateFont(fd);
//		oVERIFY(hFont);
//
//		TextDesc.Position = int2(0, 0);
//		TextDesc.Alignment = oGUI_ALIGNMENT_MIDDLE_CENTER;
//		TextDesc.Shadow = std::Black;
//	}
//
//	void SetText(const char* _Format, va_list _Args) { oVPrintf(Text, _Format, _Args); }
//	inline void SetText(const char* _Format, ...) { va_list Args; va_start(Args, _Format); SetText(_Format, Args); va_end(Args); }
//
//	void Draw(HWND _hWnd)
//	{
//		RECT rParent;
//		GetClientRect(_hWnd, &rParent);
//		TextDesc.Size = oWinRectSize(rParent);
//
//		oGDIScopedGetDC hDC(_hWnd);
//		oGDIScopedSelect SelectFont(hDC, hFont);
//		oVERIFY(oGDIDrawText(hDC, TextDesc, Text));
//	}
//
//} uiText, fpsText;

//void oKinectDisplay::DrawFPS()
//{
//	// Once per second, update the depth FPS
//	double t = oTimer();
//	if ( (t - LastDepthFPStime) > 1.0 )
//	{
//		double fps = ((DepthFramesTotal - LastDepthFramesTotal) + 0.5) / (t - LastDepthFPStime);
//		LastDepthFramesTotal = DepthFramesTotal;
//		LastDepthFPStime = t;
//
//		char fpsStr[64];
//		oPrintf(fpsStr, "%i fps ", (int)fps);
//		fpsText.SetText(fpsStr);
//	}
//
//	//if(DepthWindow)
//	//{
//	//	HWND hWnd = nullptr;
//	//	DepthWindow->QueryInterface(oGetGUID<oGUI_WINDOW>(), &hWnd);
//	//	fpsText.Draw(hWnd);
//	//}
//}

void oKinectDisplay::RemoveObservers()
{
	Observers.clear();
}

// Mark a pixel on the GesturePointMap, ignoring if it is too far from the point's origin
void oKinectDisplay::PaintPixel(float2 _Pixel2D, int _OffsetX, int _OffsetY, int _MaxDistFromOrigin, RGBQUAD _Color)
{
	int pointIdx = (int)(((_Pixel2D.y + _OffsetY) * oDEPTH_WIDTH) + (_Pixel2D.x + _OffsetX));
	float2 thisPt2D = float2((float)(_Pixel2D.x + _OffsetX), (float)(_Pixel2D.y + _OffsetY));

	if(pointIdx > 0 && pointIdx < (oDEPTH_WIDTH*oDEPTH_HEIGHT) && distance(thisPt2D, _Pixel2D) < _MaxDistFromOrigin)
		GesturePointMap[pointIdx] = _Color;
}

// Draw a square on the depth map
void oKinectDisplay::CreatePoint(float2 _CenterPt, int _Radius, RGBQUAD _Color)
{
	for(int xIdx=0; xIdx < _Radius; xIdx++)
	{
		for(int yIdx=0; yIdx < _Radius; yIdx++)
		{
			PaintPixel(_CenterPt, xIdx, yIdx, _Radius, _Color);
			PaintPixel(_CenterPt, xIdx, -yIdx, _Radius, _Color);
			PaintPixel(_CenterPt, -xIdx, yIdx, _Radius, _Color);
			PaintPixel(_CenterPt, -xIdx, -yIdx, _Radius, _Color);
		}
	}
}

void oKinectDisplay::ClearBones()
{
	memset(&GesturePointMap, 0, oDEPTH_WIDTH*oDEPTH_HEIGHT*sizeof(RGBQUAD));
}

void oKinectDisplay::DrawBones(const Vector4& _BonePosition, int _Size)
{
	POINT points;
	USHORT depth;
	NuiTransformSkeletonToDepthImage(_BonePosition, &points.x, &points.y, &depth);

	RGBQUAD color = {255, 0, 0, 255};
	points.x = (points.x * oDEPTH_WIDTH) / 320;
	points.y = (points.y * oDEPTH_HEIGHT) / 240; 

	// add a dot at the location
	CreatePoint(float2((float)points.x, (float)points.y), _Size, color);
}

RGBQUAD Nui_ShortToQuad_Depth( USHORT s )
{
	USHORT RealDepth = NuiDepthPixelToDepth(s);
	USHORT Player    = NuiDepthPixelToPlayerIndex(s);
	//ZONES Player    = (ZONES)userInfo[NuiDepthPixelToPlayerIndex(s)].zoneId;

	// transform 13-bit depth information into an 8-bit intensity appropriate
	// for display (we disregard information in most significant bit)
	BYTE intensity = (BYTE)~(RealDepth >> 4);

	// tint the intensity by dividing by per-player values
	RGBQUAD color;
	color.rgbRed   = intensity >> g_IntensityShiftByPlayerR[Player];
	color.rgbGreen = intensity >> g_IntensityShiftByPlayerG[Player];
	color.rgbBlue  = intensity >> g_IntensityShiftByPlayerB[Player];

	return color;
}

void oKinectDisplay::CreateDepthDisplayImage(const NUI_IMAGE_FRAME &_ImageFrame)
{
	++DepthFramesTotal;
	DWORD frameWidth, frameHeight;
	NuiImageResolutionToSize(_ImageFrame.eResolution, frameWidth, frameHeight);
	INuiFrameTexture * pTexture = _ImageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if (0 != LockedRect.Pitch)
	{
		DWORD frameWidth, frameHeight;

		NuiImageResolutionToSize(_ImageFrame.eResolution, frameWidth, frameHeight);

		// draw the bits to the bitmap
		RGBQUAD * rgbrun = RGBWk;
		RGBQUAD * rgbGesturePointMap = GesturePointMap;
		USHORT * pBufferRun = (USHORT *)LockedRect.pBits;

		// end pixel is start + width*height - 1
		USHORT * pBufferEnd = pBufferRun + (frameWidth * frameHeight);

		assert(frameWidth * frameHeight <= ARRAYSIZE(RGBWk));

		while (pBufferRun < pBufferEnd)
		{
			// If we marked this point as a joint, make it a different color than what it currently wants to draw
			if(rgbGesturePointMap->rgbReserved)
				*rgbrun = Nui_ShortToQuad_Depth(*pBufferRun + 1);
			else
				*rgbrun = Nui_ShortToQuad_Depth(*pBufferRun);

			rgbrun->rgbReserved = (BYTE)255;

			++pBufferRun;
			++rgbrun;
			++rgbGesturePointMap;
		}

		oSURFACE_DESC Desc;
		Desc.Dimensions = int3(frameWidth, frameHeight, 1);
		Desc.Format = oSURFACE_B8G8R8A8_UNORM;
		Desc.Layout = oSURFACE_LAYOUT_IMAGE;
		Desc.NumSlices = 1;

		oSURFACE_CONST_MAPPED_SUBRESOURCE cmsr;
		cmsr.pData = RGBWk;
		cmsr.RowPitch = oSurfaceMipCalcRowSize(Desc.Format, Desc.Dimensions);
		cmsr.DepthPitch = oSurfaceMipCalcDepthPitch(Desc);

		oFOR(auto OnFrame, Observers)
			OnFrame(Desc, cmsr);
	}
	else
	{
		oASSERT(false, "Buffer length of received texture is not valid");
	}

	pTexture->UnlockRect(0);
	ClearBones();
	//DrawFPS();
}

#else
bool SquelchWarningAboutNoObjects = true;
#endif