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
// Encapsulation of the boilerplate lifetime management for the Microsoft 
// Kinect.
#pragma once
#ifndef oKinect_h
#define oKinect_h

#include <oPlatform/oCamera.h>
#include <oPlatform/oWindow.h>

#ifdef oHAS_KINECT_SDK
	#define oKINECT_SDK_MAJOR 1
	#define oKINECT_SDK_MINOR 7
#endif

enum oKINECT_FEATURES
{
	// NOTE: Because skeleton data is derived from depth, some permutations do 
	// not make sense and are thus unavailable.

	oKINECT_NONE,
	oKINECT_COLOR,
	oKINECT_DEPTH,
	oKINECT_COLOR_DEPTH,
	oKINECT_DEPTH_SKELETON_SITTING,
	oKINECT_DEPTH_SKELETON_STANDING,
	oKINECT_COLOR_DEPTH_SKELETON_SITTING,
	oKINECT_COLOR_DEPTH_SKELETON_STANDING,

	oKINECT_FEATURES_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oKINECT_FEATURES)

enum oKINECT_FRAME_TYPE
{
	oKINECT_FRAME_NONE,
	oKINECT_FRAME_COLOR,
	oKINECT_FRAME_DEPTH,

	oKINECT_FRAME_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oKINECT_FRAME_TYPE)

struct oKINECT_DESC
{
	oKINECT_DESC()
		: TrackingTimeoutSeconds(2.0)
		, PitchDegrees(oDEFAULT)
		, Features(oKINECT_COLOR_DEPTH_SKELETON_STANDING)
		, Index(0)
	{}

	double TrackingTimeoutSeconds;

	// [-27,27] where 0 is horizontal. oDEFAULT means (leave it as-is). This can
	// be important because it can take 20+ seconds to set the pitch. It's done
	// asynchronously, but on exit that process needs to complete, and thus 
	// blocks. So if during development an application is opening and shutting a 
	// lot it is recommended to set the pitch to the desired degree and then use
	// oDEFAULT during iteration.
	int PitchDegrees;

	oKINECT_FEATURES Features;

	// Create Kinect by index or by ID. If ID is not empty, it overrides the use
	// of Index.
	int Index;
	ouro::sstring ID;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, oKINECT_DESC)

// {15DCA2B2-72E2-4fd7-A6A4-FCAE7B184D91}
oDEFINE_GUID_I(oKinect, 0x15dca2b2, 0x72e2, 0x4fd7, 0xa6, 0xa4, 0xfc, 0xae, 0x7b, 0x18, 0x4d, 0x91);
interface oKinect : oInterface
{
	virtual void GetDesc(oKINECT_DESC* _pDesc) const threadsafe = 0;

	virtual oGUI_INPUT_DEVICE_STATUS GetStatus() const threadsafe = 0;

	virtual int2 GetDimensions(oKINECT_FRAME_TYPE _Type) const threadsafe = 0;

	virtual void SetPitch(int _Degrees) threadsafe = 0;
	
	virtual bool MapRead(oKINECT_FRAME_TYPE _Type, oSURFACE_DESC* _pDesc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pMapped) const threadsafe = 0;
	virtual void UnmapRead(oKINECT_FRAME_TYPE _Type) const threadsafe = 0;

	// Returns the number of valid bones.
	virtual bool GetSkeletonByIndex(int _PlayerIndex, oGUI_BONE_DESC* _pSkeleton) const threadsafe = 0;
	virtual bool GetSkeletonByID(unsigned int _ID, oGUI_BONE_DESC* _pSkeleton) const threadsafe = 0;
};

// Returns number of Kinects on the system. This can include disconnected 
// Kinects.
int oKinectGetCount();

// If the specified index does not exist, this will return false with last error 
// of no_such_device.
bool oKinectCreate(const oKINECT_DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oKinect** _ppKinect);

#endif
