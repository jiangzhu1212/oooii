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
// Encapsulate dealing with the skeleton Kinect generates. Like the NUI API, the 
// sensor's and event's lifetime is not managed by this object, only the 
// skeleton's lifetime.
#pragma once
#ifndef oKinectSkeletonStream_h
#define oKinectSkeletonStream_h

#ifdef oHAS_KINECT_SDK

#include <oKinect/oKinect.h>
#include <oBase/surface.h>
#include <oConcurrency/mutex.h>
#include "oKinectSkeleton.h"
#include <NuiApi.h>
#include <vector>

class oKinectSkeletonStream
{
	// NOTE: Because of some lifetime issues, this object needs to be ref-counted,
	// but right now is a bad example of an oInterface since this does not have a 
	// pure interface + implementation class. Do not emulate this pattern.

public:
	oKinectSkeletonStream();

	// Call this any time the sensor gets its initialize called.
	bool Initialize(INuiSensor* _pSensor, threadsafe oWindow* _pWindow, double _TrackingTimeoutSeconds, oKINECT_FEATURES _KinectFeatures, HANDLE _hEvent) threadsafe;

	void CacheNextFrame(INuiSensor* _pSensor) threadsafe;

	// This should be called regardless of whether there was an event or not since
	// it is checking for a time of no events to trigger a deactivated action.
	void CheckTrackingTimeouts() threadsafe;

	bool GetSkeletonByIndex(int _PlayerIndex, oGUI_BONE_DESC* _pSkeleton) const threadsafe;
	bool GetSkeletonByID(unsigned int _ID, oGUI_BONE_DESC* _pSkeleton) const threadsafe;

private:
	double TrackingTimeoutSeconds;
	ouro::intrusive_ptr<threadsafe oWindow> Window;

	std::array<int, NUI_SKELETON_MAX_TRACKED_COUNT> ClosestSkeletonIndices;
	std::array<ouro::intrusive_ptr<threadsafe oKinectSkeleton>, NUI_SKELETON_COUNT> Skeletons;

	void TrackClosestSkeletons(INuiSensor* _pSensor, const NUI_SKELETON_FRAME& _NSF) threadsafe;

	int GetScreenSpaceBonePositions(
		const oGUI_BONE_DESC& _Skeleton
		, const int2& _TargetPosition
		, const int2& _TargetDimensions
		, const int2& _DepthBufferResolution
		, int2 _BonePositions[NUI_SKELETON_POSITION_COUNT]) const threadsafe;
};

#endif
#endif
