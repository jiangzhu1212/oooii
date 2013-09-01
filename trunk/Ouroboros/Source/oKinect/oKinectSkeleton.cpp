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
#include "oKinectSkeleton.h"
#include <oPlatform/Windows/oWinSkeleton.h>
#include <oPlatform/Windows/oWinWindowing.h>

#ifdef oHAS_KINECT_SDK

using namespace oConcurrency;

static oGUI_TRACKING_CLIPPING oKinectGetClipping(const NUI_SKELETON_DATA& _NSD)
{
	oGUI_TRACKING_CLIPPING Clipping;
	Clipping.Right = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_RIGHT);
	Clipping.Left = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_LEFT);
	Clipping.Top = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_TOP);
	Clipping.Bottom = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_BOTTOM);
	return Clipping;
}

oKinectSkeleton::oKinectSkeleton()
	: LastTrackedTimestamp(0.0)
{
	Invalidate();
	oWinRegisterSkeletonSource((HSKELETON)this, oBIND(&oKinectSkeleton::GetSkeleton, this, oBIND1));
}

bool oKinectSkeletonCreate(threadsafe oKinectSkeleton** _ppSkeleton)
{
	*_ppSkeleton = new oKinectSkeleton();
	return true;
}

oKinectSkeleton::~oKinectSkeleton()
{
	oWinUnregisterSkeletonSource((HSKELETON)this);
}

void oKinectSkeleton::Invalidate() threadsafe
{
	lock_guard<shared_mutex> lock(Mutex);
	oThreadsafe(this)->Skeleton = oGUI_BONE_DESC(oThreadsafe(this)->Skeleton.SourceID);
}

void oKinectSkeleton::Cache(HWND _hWnd, const NUI_SKELETON_DATA& _NSD) threadsafe
{
	if (_NSD.eTrackingState == NUI_SKELETON_TRACKED)
	{
		LastTrackedTimestamp = oTimer();

		if (!Skeleton.SourceID)
		{
			Skeleton.SourceID = _NSD.dwTrackingID;
			PostMessage(_hWnd, oWM_USER_CAPTURED, Skeleton.SourceID, 0);
		}

		{
			lock_guard<shared_mutex> lock(Mutex);
			oThreadsafe(Skeleton).Clipping = oKinectGetClipping(_NSD);
			std::copy((const float4*)_NSD.SkeletonPositions
				, (const float4*)(_NSD.SkeletonPositions + NUI_SKELETON_POSITION_COUNT)
				, (oThreadsafe(Skeleton).Positions.begin()));
		}

		// In most cases the absolute latest skeleton is desired, so leave out 
		// queuing and thread safety of the data for now and just notify the system 
		// that it's worth checking bone data again.
		
		
		PostMessage(_hWnd, oWM_SKELETON, MAKEWPARAM(Skeleton.SourceID, 0), (LPARAM)this);
	}

	else
	{
		if (Skeleton.SourceID)
		{
			Invalidate();
			PostMessage(_hWnd, oWM_USER_LOST, Skeleton.SourceID, 0);
			Skeleton.SourceID = 0;
		}
	}
}

void oKinectSkeleton::CheckTrackingTimeout(HWND _hWnd, double _TimeoutThresholdInSeconds) threadsafe
{
	if (Skeleton.SourceID && (oTimer() - LastTrackedTimestamp) > _TimeoutThresholdInSeconds)
	{
		Invalidate();
		PostMessage(_hWnd, oWM_USER_LOST, Skeleton.SourceID, 0);
		Skeleton.SourceID = 0;
	}
}

void oKinectSkeleton::GetSkeleton(oGUI_BONE_DESC* _pSkeleton) const threadsafe
{
	shared_lock<shared_mutex> lock(Mutex);
	*_pSkeleton = oThreadsafe(Skeleton);
}

#endif // oHAS_KINECT_SDK
