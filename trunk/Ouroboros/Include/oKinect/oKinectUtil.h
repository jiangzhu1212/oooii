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
// Helpful utilities for using the Kinect API.
#pragma once
#ifndef oKinectUtil_h
#define oKinectUtil_h

#ifdef oHAS_KINECT_SDK

#include <oKinect/oKinect.h>
#include <oBasis/oSurface.h>
#include <oPlatform/Windows/oWindows.h>
#include <NuiApi.h>

// From Kinect SDK 1.7 KinectExplorer NuiImageBuffer.cpp
static const unsigned short oKINECT_MIN_DEPTH = 400;
static const unsigned short oKINECT_MAX_DEPTH = 16383;
static const int oKINECT_MAX_CACHED_FRAMES = 2;

// Converts a features enum to the proper init flags
DWORD oKinectGetInitFlags(oKINECT_FEATURES _Features);

// Converts a status-specific HR code to an oKINECT_STATUS enum
oGUI_INPUT_DEVICE_STATUS oKinectStatusFromHR(HRESULT _hrNuiStatus);

// Convert between NUI and oGUI enums for bones
NUI_SKELETON_POSITION_INDEX oKinectFromBone(oGUI_BONE _Bone);
oGUI_BONE oKinectToBone(NUI_SKELETON_POSITION_INDEX _BoneIndex);

// Returns a std::errc fit for using oErrorSetLast() from a status.
std::errc::errc oKinectGetErrcFromStatus(oGUI_INPUT_DEVICE_STATUS _Status);
const char* oKinectGetErrcStringFromStatus(oGUI_INPUT_DEVICE_STATUS _Status);

// Gets the latest (rather than just the next) frame from the specified sensor.
// Once done with the contents of _pLatest, call 
// _pSensor->NuiImageStreamReleaseFrame(_hStream, _pLatest) as the default
// NuiImageStreamGetNextFrame normally requires.
void oKinectGetLatestFrame(INuiSensor* _pSensor, HANDLE _hStream, DWORD _Timeout, NUI_IMAGE_FRAME* _pLatest);

ouro::surface::format oKinectGetFormat(NUI_IMAGE_TYPE _Type);

void oKinectGetDesc(NUI_IMAGE_TYPE _Type, NUI_IMAGE_RESOLUTION _Resolution, ouro::surface::info* _pInfo);
bool oKinectCreateSurface(NUI_IMAGE_TYPE _Type, NUI_IMAGE_RESOLUTION _Resolution, std::shared_ptr<ouro::surface::buffer>* _ppSurface);

// This must be pure depth, not depth+index. This adjusts the specified value
// so that there is more contrast when rendering visuals of the depth value.
unsigned char oKinectGetDepthIntensity(unsigned short _Depth);

// Returns a color based on player index and depth intensity fit for visual
// display.
RGBQUAD oKinectGetColoredDepth(unsigned short _DepthAndIndex);

// Copies the specified NUI frame to the specified properly allocated 
// destination surface.
void oKinectCopyBits(const NUI_IMAGE_FRAME& _NIF, ouro::surface::mapped_subresource& _Destination);

// Uses the above utility code to copy the latest from from the specified stream
// and fills the properly allocated surface with the image contents. This 
// returns the FrameNumber (NUI_IMAGE_FRAME::dwFrameNumber).
unsigned int oKinectUpdate(INuiSensor* _pSensor, HANDLE _hStream, ouro::surface::buffer* _pSurface);

// Converts to a position in pixels for the specified target dimensions. If the 
// specified position is invalid (w < 0) then the return value will be 
// int2(oDEFAULT, oDEFAULT).
int2 oKinectSkeletonToScreen(
	const float4& _CameraSpacePosition
	, const int2& _TargetPosition
	, const int2& _TargetDimensions
	, const int2& _DepthBufferResolution);

// Returns number of valid bones. The value int2(oDEFAULT, oDEFAULT) is set to
// any invalid bone.
int oKinectCalcScreenSpacePositions(
	const oGUI_BONE_DESC& _Skeleton
	, const int2& _TargetPosition
	, const int2& _TargetDimensions
	, const int2& _DepthBufferResolution
	, int2 _ScreenSpacePositions[oGUI_BONE_COUNT]);

#endif // oHAS_KINECT_SDK

// Converts the _OriginBone to 0,0,0 and makes all positions relative to that.
void oKinectCalcBoneSpacePositions(oGUI_BONE _OriginBone, oGUI_BONE_DESC& _Skeleton);

#endif
