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
#include <oKinect/oKinectUtil.h>
#include <oKinect/oKinectGDI.h>

#ifdef oHAS_KINECT_SDK

#include <oCore/windows/win_error.h>
#include <oGUI/windows/win_gdi.h>
#include <oGUI/windows/win_gdi_bitmap.h>
#include <oGUI/windows/win_gdi_draw.h>
#include <oGUI/Windows/oWinRect.h>
#include <NuiApi.h>

using namespace ouro;
using namespace ouro::windows::gdi;

static const int2 kNoDraw = int2(oDEFAULT, oDEFAULT);

static void oGDIDrawKinectBone(HDC _hDC, const int2& _SSBonePos0, const int2& _SSBonePos1)
{
	if (any(_SSBonePos0 != kNoDraw) && any(_SSBonePos1 != kNoDraw))
		draw_line(_hDC, _SSBonePos0, _SSBonePos1);
}

static void oGDIDrawKinectJoint(HDC _hDC, const int2& _SSBonePos, int _Radius)
{
	if (any(_SSBonePos != kNoDraw))
	{
		RECT r = oWinRectDilate(oWinRectWH(_SSBonePos, int2(0, 0)), _Radius);
		draw_ellipse(_hDC, r);
	}
}

static void oGDIDrawKinectSkeleton(HDC _hDC, int _BoneRadius, int2 _ScreenSpaceBonePositions[ouro::input::bone_count])
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

	for (int i = 0; i < ouro::input::bone_count; i++)
		oGDIDrawKinectJoint(_hDC, _ScreenSpaceBonePositions[i], _BoneRadius);
}

static void oGDIDrawClipping(HDC _hDC, const RECT& _rTarget, const ouro::input::tracking_clipping& _Clipping)
{
	const int2 Offset = oWinRectPosition(_rTarget);
	const int2 Dimensions = oWinRectSize(_rTarget);
	const int2 ClippingDimensions = int2(float2(Dimensions) * 0.04f);

	scoped_brush Hatch(CreateHatchBrush(HS_BDIAGONAL, brush_color(current_brush(_hDC))));
	scoped_select SelectHatch(_hDC, Hatch);
	scoped_bk_mode TransparentBk(_hDC, TRANSPARENT);
	scoped_select SelectNoPen(_hDC, GetStockObject(NULL_PEN));

	if (_Clipping.left)
	{
		const RECT r = { 0, 0, ClippingDimensions.x, Dimensions.y };
		draw_box(_hDC, oWinRectTranslate(r, Offset));
	}

	if (_Clipping.top)
	{
		const RECT r = { 0, 0, Dimensions.x, ClippingDimensions.y };
		draw_box(_hDC, oWinRectTranslate(r, Offset));
	}

	if (_Clipping.right)
	{
		const RECT r = { 0, 0, ClippingDimensions.x, Dimensions.y };
		draw_box(_hDC, oWinRectTranslate(r, Offset + int2(oWinRectW(_rTarget) - oWinRectW(r), 0)));
	}

	if (_Clipping.bottom)
	{
		const RECT r = { 0, 0, Dimensions.x, ClippingDimensions.y };
		draw_box(_hDC, oWinRectTranslate(r, Offset + int2(0, oWinRectH(_rTarget) - oWinRectH(r))));
	}
}

static void oGDIDrawKinectBoneNames(HDC _hDC, const RECT& _rTarget, int2 _ScreenSpaceBonePositions[ouro::input::bone_count])
{
	ouro::text_info td;
	td.size = int2(300, 50);
	td.alignment = ouro::alignment::top_left;
	COLORREF rgb = brush_color(current_brush(_hDC));
	td.foreground = 0xff000000 | rgb;
	td.shadow = black;
	for (int i = 0; i < ouro::input::bone_count; i++)
	{
		const char* Name = ouro::as_string(ouro::input::skeleton_bone(i)) + 14; // skip "oGUI_BONE_"
		td.position = _ScreenSpaceBonePositions[i] + int2(5,5);
		draw_text(_hDC, td, Name);
	}
}

void oGDIDrawBoneText(
	HDC _hDC
	, const RECT& _rTarget
	, const float4& _BonePosition
	, ouro::alignment::value _Anchor
	, const int2& _Offset
	, ouro::alignment::value _Alignment
	, const char* _Text
	)
{
	const int2 Size = oWinRectSize(_rTarget);

	int2 SSBone = oKinectSkeletonToScreen(
		_BonePosition
		, oWinRectPosition(_rTarget)
		, Size
		, int2(320,240));

	RECT rText = calc_text_rect(_hDC, _Text);
	RECT r = oWinRectResolve(SSBone + int2(0, -2 * oWinRectH(rText)), oWinRectSize(rText), _Anchor);

	ouro::text_info td;
	td.position = oWinRectPosition(r);
	td.size = oWinRectSize(r);
	td.alignment = _Alignment;
	COLORREF rgb = brush_color(current_brush(_hDC));
	td.foreground = 0xff000000 | rgb;
	td.shadow = black;
	draw_text(_hDC, td, _Text);
}

void oGDIDrawKinect(HDC _hDC, const RECT& _rTarget, oKINECT_FRAME_TYPE _Type, int _Flags, const threadsafe oKinect* _pKinect)
{
	int PenWidth = 0;
	pen_color(current_pen(_hDC), &PenWidth);
	static const int kJointThickness = PenWidth * 2;

	// Ensure thick lines aren't drawn out-of-bounds by insetting _rTarget
	const RECT rInset = oWinRectDilate(_rTarget, -kJointThickness);

	if (_Type != oKINECT_FRAME_NONE)
	{
		ouro::surface::info inf;
		ouro::surface::const_mapped_subresource m;
		if (_pKinect->MapRead(_Type, &inf, &m))
		{
			ouro::windows::gdi::stretch_bits(_hDC, _rTarget, inf.dimensions.xy(), inf.format, m.data);
			_pKinect->UnmapRead(_Type);
		}
	}

	if (_Flags & (oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_CLIPPING|oGDI_KINECT_DRAW_BONE_NAMES))
	{
		const int2 DepthDimensions = _pKinect->GetDimensions(oKINECT_FRAME_DEPTH);
		int2 ScreenSpaceBonePositions[ouro::input::bone_count];

		ouro::input::tracking_skeleton Skeleton;
		for (int i = 0; i < NUI_SKELETON_MAX_TRACKED_COUNT; i++)
		{
			if (_pKinect->GetSkeletonByIndex(i, &Skeleton))
			{
				if (_Flags & (oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_BONE_NAMES))
				{
					oKinectCalcScreenSpacePositions(Skeleton, oWinRectPosition(_rTarget), oWinRectSize(_rTarget), DepthDimensions, ScreenSpaceBonePositions);
					// clip positions so that a thick line doesn't draw outside the bounds.
					for (auto& p : ScreenSpaceBonePositions)
						p = oWinClip(rInset, p);
				}

				if (_Flags & oGDI_KINECT_DRAW_SKELETON)
					oGDIDrawKinectSkeleton(_hDC, kJointThickness, ScreenSpaceBonePositions);

				if (_Flags & oGDI_KINECT_DRAW_CLIPPING)
					oGDIDrawClipping(_hDC, _rTarget, Skeleton.clipping);

				if (_Flags & oGDI_KINECT_DRAW_BONE_NAMES)
					oGDIDrawKinectBoneNames(_hDC, _rTarget, ScreenSpaceBonePositions);
			}
		}
	}
}

void oGDIDrawAirKey(HDC _hDC, const RECT& _rTarget, int _Flags, const oAIR_KEY& _Key, ouro::input::action_type _LastAction, const ouro::input::tracking_skeleton& _Skeleton)
{
	aaboxf Bounds = _Key.Bounds;
	if (_Key.Origin != ouro::input::invalid_bone)
		ouro::translate(Bounds, _Skeleton.positions[_Key.Origin].xyz());

	float3 Min = Bounds.Min;
	float3 Max = Bounds.Max;

	int2 SSMin = oKinectSkeletonToScreen(float4(Min.x, Min.y, Max.z, 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

	// Choose box based on nearest corners.
	int2 SSMax = oKinectSkeletonToScreen(float4(Max, 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

	int2 SSCenter = oKinectSkeletonToScreen(float4(Bounds.center(), 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

	sstring text;
	ouro::text_info td;
	td.position = SSMin + int2(2,2);
	td.size = (SSMax - SSMin) - int2(4,4);
	td.shadow = black;
	
	if (_Flags & oGDI_AIR_KEY_DRAW_MIN)
	{
		snprintf(text, "%.02f %.02f %.02f", Min.x, Min.y, Min.z);
		td.alignment = ouro::alignment::top_left;
		draw_text(_hDC, td, text.c_str());
	}

	if (_Flags & oGDI_AIR_KEY_DRAW_MAX)
	{
		snprintf(text, "%.02f %.02f %.02f", Max.x, Max.y, Max.z);
		td.alignment = ouro::alignment::bottom_right;
		draw_text(_hDC, td, text.c_str());
	}

	if (_Flags & oGDI_AIR_KEY_DRAW_KEY)
	{
		if (_LastAction == ouro::input::key_down)
			td.foreground = lime;
		td.alignment = ouro::alignment::middle_center;
		draw_text(_hDC, td, ouro::as_string(_Key.Key));
	}

	if (_Flags & oGDI_AIR_KEY_DRAW_BOX)
	{
		float3 Corners[8];
		ouro::decompose(Bounds, Corners);

		int2 SSCorners[8];

		oFORI(i, SSCorners)
			SSCorners[i] = oKinectSkeletonToScreen(float4(Corners[i].x, Corners[i].y, Corners[i].z, 1.0f), oWinRectPosition(_rTarget), oWinRectSize(_rTarget), int2(320,240));

		// Far face (minZ face)
		draw_line(_hDC, SSCorners[0], SSCorners[1]);
		draw_line(_hDC, SSCorners[0], SSCorners[2]);
		draw_line(_hDC, SSCorners[1], SSCorners[3]);
		draw_line(_hDC, SSCorners[2], SSCorners[3]);

		// Near face (maxZ face)
		draw_line(_hDC, SSCorners[7], SSCorners[6]);
		draw_line(_hDC, SSCorners[7], SSCorners[5]);
		draw_line(_hDC, SSCorners[6], SSCorners[4]);
		draw_line(_hDC, SSCorners[5], SSCorners[4]);

		// top 2
		draw_line(_hDC, SSCorners[2], SSCorners[6]);
		draw_line(_hDC, SSCorners[3], SSCorners[7]);

		// bottom 2
		draw_line(_hDC, SSCorners[0], SSCorners[4]);
		draw_line(_hDC, SSCorners[1], SSCorners[5]);
	}

	else
	{
		RECT r = oWinRect(SSMin, SSMax);
		scoped_select NoFill(_hDC, GetStockObject(NULL_BRUSH));
		draw_box(_hDC, r);
	}
}

#else 

void oGDIDrawBoneText(
	HDC _hDC
	, const RECT& _rTarget
	, const float4& _BonePosition
	, ouro::alignment::value _Anchor
	, const int2& _Offset
	, ouro::alignment::value _Alignment
	, const char* _Text
	)
{
}
	
void oGDIDrawKinect(HDC _hDC, const RECT& _rTarget, oKINECT_FRAME_TYPE _Type, int _Flags, const threadsafe oKinect* _pKinect)
{
}

void oGDIDrawAirKey(HDC _hDC, const RECT& _rTarget, int _Flags, const oAIR_KEY& _Key, ouro::input::value _LastAction, const ouro::input::tracking_skeleton& _Skeleton)
{
}

#endif // oHAS_KINECT_SDK
