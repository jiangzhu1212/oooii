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
#include <oKinect/oKinectGDI.h>
#include <oKinect/oKinectUtil.h>

#ifdef oHAS_KINECT_SDK

#include <oPlatform/Windows/oWinRect.h>
#include <NuiApi.h>

static const int2 kNoDraw = int2(oDEFAULT, oDEFAULT);

static void oGDIDrawKinectBone(HDC _hDC, const int2& _SSBonePos0, const int2& _SSBonePos1)
{
	if (any(_SSBonePos0 != kNoDraw) && any(_SSBonePos1 != kNoDraw))
		oVB(oGDIDrawLine(_hDC, _SSBonePos0, _SSBonePos1));
}

static void oGDIDrawKinectJoint(HDC _hDC, const int2& _SSBonePos, int _Radius)
{
	if (any(_SSBonePos != kNoDraw))
	{
		RECT r = oWinRectDilate(oWinRectWH(_SSBonePos, int2(0, 0)), _Radius);
		oVB(oGDIDrawEllipse(_hDC, r));
	}
}

static void oGDIDrawKinectSkeleton(HDC _hDC, int _BoneRadius, int2 _ScreenSpaceBonePositions[oGUI_BONE_COUNT])
{
	// Render Torso
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HEAD], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_CENTER]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_CENTER], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_LEFT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_CENTER], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_CENTER], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SPINE]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SPINE], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_CENTER]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_CENTER], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_LEFT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_CENTER], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_RIGHT]);

	// Left Arm
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_LEFT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ELBOW_LEFT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ELBOW_LEFT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_WRIST_LEFT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_WRIST_LEFT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HAND_LEFT]);

	// Right Arm
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ELBOW_RIGHT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ELBOW_RIGHT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_WRIST_RIGHT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HAND_RIGHT]);

	// Left Leg
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_LEFT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_KNEE_LEFT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_KNEE_LEFT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ANKLE_LEFT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ANKLE_LEFT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_FOOT_LEFT]);

	// Right Leg
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_HIP_RIGHT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_KNEE_RIGHT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_KNEE_RIGHT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ANKLE_RIGHT]);
	oGDIDrawKinectBone(_hDC, _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_ANKLE_RIGHT], _ScreenSpaceBonePositions[NUI_SKELETON_POSITION_FOOT_RIGHT]);

	for (int i = 0; i < oGUI_BONE_COUNT; i++)
		oGDIDrawKinectJoint(_hDC, _ScreenSpaceBonePositions[i], _BoneRadius);
}

static void oGDIDrawClipping(HDC _hDC, const RECT& _rTarget, const oGUI_TRACKING_CLIPPING& _Clipping)
{
	const int2 Offset = oWinRectPosition(_rTarget);
	const int2 Dimensions = oWinRectSize(_rTarget);
	const int2 ClippingDimensions = int2(float2(Dimensions) * 0.04f);

	oGDIScopedObject<HBRUSH> Hatch(CreateHatchBrush(HS_BDIAGONAL, oGDIGetBrushColor(oGDIGetBrush(_hDC))));
	oGDIScopedSelect SelectHatch(_hDC, Hatch);
	oGDIScopedBkMode TransparentBk(_hDC, TRANSPARENT);
	oGDIScopedSelect SelectNoPen(_hDC, GetStockObject(NULL_PEN));

	if (_Clipping.Left)
	{
		const RECT r = { 0, 0, ClippingDimensions.x, Dimensions.y };
		oGDIDrawBox(_hDC, oWinRectTranslate(r, Offset));
	}

	if (_Clipping.Top)
	{
		const RECT r = { 0, 0, Dimensions.x, ClippingDimensions.y };
		oGDIDrawBox(_hDC, oWinRectTranslate(r, Offset));
	}

	if (_Clipping.Right)
	{
		const RECT r = { 0, 0, ClippingDimensions.x, Dimensions.y };
		oGDIDrawBox(_hDC, oWinRectTranslate(r, Offset + int2(oWinRectW(_rTarget) - oWinRectW(r), 0)));
	}

	if (_Clipping.Bottom)
	{
		const RECT r = { 0, 0, Dimensions.x, ClippingDimensions.y };
		oGDIDrawBox(_hDC, oWinRectTranslate(r, Offset + int2(0, oWinRectH(_rTarget) - oWinRectH(r))));
	}
}

static void oGDIDrawKinectBoneNames(HDC _hDC, const RECT& _rTarget, int2 _ScreenSpaceBonePositions[oGUI_BONE_COUNT])
{
	oGUI_TEXT_DESC td;
	td.Size = int2(300, 50);
	td.Alignment = oGUI_ALIGNMENT_TOP_LEFT;
	COLORREF rgb = oGDIGetBrushColor(oGDIGetBrush(_hDC));
	td.Foreground = 0xff000000 | rgb;
	td.Shadow = oStd::Black;
	for (int i = 0; i < oGUI_BONE_COUNT; i++)
	{
		const char* Name = oStd::as_string(oGUI_BONE(i)) + 14; // skip "oGUI_BONE_"
		td.Position = _ScreenSpaceBonePositions[i] + int2(5,5);
		oGDIDrawText(_hDC, td, Name);
	}
}

void oGDIDrawBoneText(
	HDC _hDC
	, const RECT& _rTarget
	, const float4& _BonePosition
	, oGUI_ALIGNMENT _Anchor
	, const int2& _Offset
	, oGUI_ALIGNMENT _Alignment
	, const char* _Text
	)
{
	const int2 Size = oWinRectSize(_rTarget);

	int2 SSBone = oKinectSkeletonToScreen(
		_BonePosition
		, oWinRectPosition(_rTarget)
		, Size
		, int2(320,240));

	RECT rText = oGDICalcTextRect(_hDC, _Text);
	RECT r = oWinRectResolve(SSBone + int2(0, -2 * oWinRectH(rText)), oWinRectSize(rText), _Anchor);

	oGUI_TEXT_DESC td;
	td.Position = oWinRectPosition(r);
	td.Size = oWinRectSize(r);
	td.Alignment = _Alignment;
	COLORREF rgb = oGDIGetBrushColor(oGDIGetBrush(_hDC));
	td.Foreground = 0xff000000 | rgb;
	td.Shadow = oStd::Black;
	oGDIDrawText(_hDC, td, _Text);
}

void oGDIDrawKinect(HDC _hDC, const RECT& _rTarget, oKINECT_FRAME_TYPE _Type, int _Flags, const threadsafe oKinect* _pKinect)
{
	int PenWidth = 0;
	oGDIGetPenColor(oGDIGetPen(_hDC), &PenWidth);
	static const int kJointThickness = PenWidth * 2;

	// Ensure thick lines aren't drawn out-of-bounds by insetting _rTarget
	const RECT rInset = oWinRectDilate(_rTarget, -kJointThickness);

	if (_Type != oKINECT_FRAME_NONE)
	{
		oSURFACE_DESC sd;
		oSURFACE_CONST_MAPPED_SUBRESOURCE m;
		if (_pKinect->MapRead(_Type, &sd, &m))
		{
			oV(oGDIStretchBits(_hDC, _rTarget, sd.Dimensions.xy(), sd.Format, m.pData, m.RowPitch));
			_pKinect->UnmapRead(_Type);
		}
	}

	if (_Flags & (oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_CLIPPING|oGDI_KINECT_DRAW_BONE_NAMES))
	{
		const int2 DepthDimensions = _pKinect->GetDimensions(oKINECT_FRAME_DEPTH);
		int2 ScreenSpaceBonePositions[oGUI_BONE_COUNT];

		oGUI_BONE_DESC Skeleton;
		for (int i = 0; i < NUI_SKELETON_MAX_TRACKED_COUNT; i++)
		{
			if (_pKinect->GetSkeletonByIndex(i, &Skeleton))
			{
				if (_Flags & (oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_BONE_NAMES))
				{
					oKinectCalcScreenSpacePositions(Skeleton, oWinRectPosition(_rTarget), oWinRectSize(_rTarget), DepthDimensions, ScreenSpaceBonePositions);
					// clip positions so that a thick line doesn't draw outside the bounds.
					oFOR(auto& p, ScreenSpaceBonePositions)
						p = oWinClip(rInset, p);
				}

				if (_Flags & oGDI_KINECT_DRAW_SKELETON)
					oGDIDrawKinectSkeleton(_hDC, kJointThickness, ScreenSpaceBonePositions);

				if (_Flags & oGDI_KINECT_DRAW_CLIPPING)
					oGDIDrawClipping(_hDC, _rTarget, Skeleton.Clipping);

				if (_Flags & oGDI_KINECT_DRAW_BONE_NAMES)
					oGDIDrawKinectBoneNames(_hDC, _rTarget, ScreenSpaceBonePositions);
			}
		}
	}
}

void oGDIDrawAirKey(HDC _hDC, const RECT& _rTarget, int _Flags, const oAIR_KEY& _Key, oGUI_ACTION _LastAction, const oGUI_BONE_DESC& _Skeleton)
{
	oAABoxf Bounds = _Key.Bounds;
	if (_Key.Origin != oGUI_BONE_INVALID)
		oTranslate(Bounds, _Skeleton.Positions[_Key.Origin].xyz());

	float3 Min = Bounds.Min;
	float3 Max = Bounds.Max;

	int2 SSMin = oKinectSkeletonToScreen(float4(Min.x, Min.y, Max.z, 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

	// Choose box based on nearest corners.
	int2 SSMax = oKinectSkeletonToScreen(float4(Max, 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

	int2 SSCenter = oKinectSkeletonToScreen(float4(Bounds.center(), 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

	oStd::sstring text;
	oGUI_TEXT_DESC td;
	td.Position = SSMin + int2(2,2);
	td.Size = (SSMax - SSMin) - int2(4,4);
	td.Shadow = oStd::Black;
	
	if (_Flags & oGDI_AIR_KEY_DRAW_MIN)
	{
		snprintf(text, "%.02f %.02f %.02f", Min.x, Min.y, Min.z);
		td.Alignment = oGUI_ALIGNMENT_TOP_LEFT;
		oGDIDrawText(_hDC, td, text.c_str());
	}

	if (_Flags & oGDI_AIR_KEY_DRAW_MAX)
	{
		snprintf(text, "%.02f %.02f %.02f", Max.x, Max.y, Max.z);
		td.Alignment = oGUI_ALIGNMENT_BOTTOM_RIGHT;
		oGDIDrawText(_hDC, td, text.c_str());
	}

	if (_Flags & oGDI_AIR_KEY_DRAW_KEY)
	{
		if (_LastAction == oGUI_ACTION_KEY_DOWN)
			td.Foreground = oStd::OOOiiGreen;
		td.Alignment = oGUI_ALIGNMENT_MIDDLE_CENTER;
		oGDIDrawText(_hDC, td, oStd::as_string(_Key.Key));
	}

	if (_Flags & oGDI_AIR_KEY_DRAW_BOX)
	{
		float3 Corners[8];
		oDecompose(Bounds, Corners);

		int2 SSCorners[8];

		oFORI(i, SSCorners)
			SSCorners[i] = oKinectSkeletonToScreen(float4(Corners[i].x, Corners[i].y, Corners[i].z, 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

		// Far face (minZ face)
		oGDIDrawLine(_hDC, SSCorners[0], SSCorners[1]);
		oGDIDrawLine(_hDC, SSCorners[0], SSCorners[2]);
		oGDIDrawLine(_hDC, SSCorners[1], SSCorners[3]);
		oGDIDrawLine(_hDC, SSCorners[2], SSCorners[3]);

		// Near face (maxZ face)
		oGDIDrawLine(_hDC, SSCorners[7], SSCorners[6]);
		oGDIDrawLine(_hDC, SSCorners[7], SSCorners[5]);
		oGDIDrawLine(_hDC, SSCorners[6], SSCorners[4]);
		oGDIDrawLine(_hDC, SSCorners[5], SSCorners[4]);

		// top 2
		oGDIDrawLine(_hDC, SSCorners[2], SSCorners[6]);
		oGDIDrawLine(_hDC, SSCorners[3], SSCorners[7]);

		// bottom 2
		oGDIDrawLine(_hDC, SSCorners[0], SSCorners[4]);
		oGDIDrawLine(_hDC, SSCorners[1], SSCorners[5]);
	}

	else
	{
		RECT r = oWinRect(SSMin, SSMax);
		oGDIScopedSelect NoFill(_hDC, GetStockObject(NULL_BRUSH));
		oGDIDrawBox(_hDC, r);
	}
}

#else 

void oGDIDrawBoneText(
	HDC _hDC
	, const RECT& _rTarget
	, const float4& _BonePosition
	, oGUI_ALIGNMENT _Anchor
	, const int2& _Offset
	, oGUI_ALIGNMENT _Alignment
	, const char* _Text
	)
{
}
	
void oGDIDrawKinect(HDC _hDC, const RECT& _rTarget, oKINECT_FRAME_TYPE _Type, int _Flags, const threadsafe oKinect* _pKinect)
{
}

void oGDIDrawAirKey(HDC _hDC, const RECT& _rTarget, int _Flags, const oAIR_KEY& _Key, oGUI_ACTION _LastAction, const oGUI_BONE_DESC& _Skeleton)
{
}

#endif // oHAS_KINECT_SDK
