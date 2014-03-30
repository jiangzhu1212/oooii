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
// Utilities for drawing Kinect visualizations with GDI.
#pragma once
#ifndef oKinectGDI_h
#define oKinectGDI_h

#include <oBasis/oAirKeyboard.h>
#include <oKinect/oKinect.h>
#include <oGUI/Windows/win_gdi.h>

static const int oGDI_KINECT_DRAW_SKELETON = 1;
static const int oGDI_KINECT_DRAW_CLIPPING = 2;
static const int oGDI_KINECT_DRAW_BONE_NAMES = 4;

// Given a bone position, draw text in screen space at the specified offset 
// from the specified anchor point of the text's bounding box.
void oGDIDrawBoneText(
	HDC _hDC
	, const RECT& _rTarget
	, const float4& _BonePosition
	, ouro::alignment::value _Anchor
	, const int2& _Offset
	, ouro::alignment::value _Alignment
	, const char* _Text
	);

// Draws a rectangle filled with one of the frame types with an optional 
// skeleton overlay based on the current state of the specified Kinect device.
void oGDIDrawKinect(HDC _hDC, const RECT& _rTarget, oKINECT_FRAME_TYPE _Type, int _Flags, const threadsafe oKinect* _pKinect);

static const int oGDI_AIR_KEY_DRAW_RECT = 0;
static const int oGDI_AIR_KEY_DRAW_BOX = 1;
static const int oGDI_AIR_KEY_DRAW_KEY = 2;
static const int oGDI_AIR_KEY_DRAW_MIN = 4;
static const int oGDI_AIR_KEY_DRAW_MAX = 8;

void oGDIDrawAirKey(HDC _hDC, const RECT& _rTarget, int _Flags, const oAIR_KEY& _Key, ouro::input::action_type _LastAction, const ouro::input::tracking_skeleton& _Skeleton);

#endif
