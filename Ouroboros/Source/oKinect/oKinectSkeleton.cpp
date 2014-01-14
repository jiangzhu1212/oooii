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
#include <oGUI/Windows/oWinWindowing.h>

#ifdef oHAS_KINECT_SDK

using namespace ouro::windows::skeleton;
using namespace std;

static tracking_clipping oKinectGetClipping(const NUI_SKELETON_DATA& _NSD)
{
	tracking_clipping c;
	c.right = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_RIGHT);
	c.left = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_LEFT);
	c.top = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_TOP);
	c.bottom = !!(_NSD.dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_BOTTOM);
	return c;
}

oKinectSkeleton::oKinectSkeleton()
	: LastTrackedTimestamp(0.0)
{
	Invalidate();
	register_source((handle)this, std::bind(&oKinectSkeleton::GetSkeleton, this, std::placeholders::_1));
}

bool oKinectSkeletonCreate(threadsafe oKinectSkeleton** _ppSkeleton)
{
	*_ppSkeleton = new oKinectSkeleton();
	return true;
}

oKinectSkeleton::~oKinectSkeleton()
{
	unregister_source((handle)this);
}

void oKinectSkeleton::Invalidate() threadsafe
{
	lock_t lock(thread_cast<ouro::shared_mutex&>(Mutex));
	oThreadsafe(this)->Skeleton = bone_info(oThreadsafe(this)->Skeleton.source_id);
}

void oKinectSkeleton::Cache(HWND _hWnd, const NUI_SKELETON_DATA& _NSD) threadsafe
{
	if (_NSD.eTrackingState == NUI_SKELETON_TRACKED)
	{
		LastTrackedTimestamp = ouro::timer::now();

		if (!Skeleton.source_id)
		{
			Skeleton.source_id = _NSD.dwTrackingID;
			PostMessage(_hWnd, oWM_USER_CAPTURED, Skeleton.source_id, 0);
		}

		{
			lock_t lock(thread_cast<ouro::shared_mutex&>(Mutex));
			oThreadsafe(Skeleton).clipping = oKinectGetClipping(_NSD);
			std::copy((const float4*)_NSD.SkeletonPositions
				, (const float4*)(_NSD.SkeletonPositions + NUI_SKELETON_POSITION_COUNT)
				, (oThreadsafe(Skeleton).positions.begin()));
		}

		// In most cases the absolute latest skeleton is desired, so leave out 
		// queuing and thread safety of the data for now and just notify the system 
		// that it's worth checking bone data again.
		
		
		PostMessage(_hWnd, oWM_SKELETON, MAKEWPARAM(Skeleton.source_id, 0), (LPARAM)this);
	}

	else
	{
		if (Skeleton.source_id)
		{
			Invalidate();
			PostMessage(_hWnd, oWM_USER_LOST, Skeleton.source_id, 0);
			Skeleton.source_id = 0;
		}
	}
}

void oKinectSkeleton::CheckTrackingTimeout(HWND _hWnd, double _TimeoutThresholdInSeconds) threadsafe
{
	if (Skeleton.source_id && (ouro::timer::now() - LastTrackedTimestamp) > _TimeoutThresholdInSeconds)
	{
		Invalidate();
		PostMessage(_hWnd, oWM_USER_LOST, Skeleton.source_id, 0);
		Skeleton.source_id = 0;
	}
}

void oKinectSkeleton::GetSkeleton(bone_info* _pSkeleton) const threadsafe
{
	ouro::shared_lock<ouro::shared_mutex> lock(thread_cast<ouro::shared_mutex&>(Mutex));
	*_pSkeleton = oThreadsafe(Skeleton);
}

#endif // oHAS_KINECT_SDK
