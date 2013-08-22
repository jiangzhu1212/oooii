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
#include "oKinectSkeletonStream.h"

#ifdef oHAS_KINECT_SDK
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinRect.h>
#include "oWinKinect10.h"
#include <oKinect/oKinectUtil.h>

static DWORD oKinectGetSkeletonInitFlags(oKINECT_FEATURES _Features)
{
	switch (_Features)
	{
		case oKINECT_DEPTH_SKELETON_SITTING: 
		case oKINECT_COLOR_DEPTH_SKELETON_SITTING: return NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS|NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;
		case oKINECT_DEPTH_SKELETON_STANDING: 
		case oKINECT_COLOR_DEPTH_SKELETON_STANDING: return NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS;
		default: break;
	}

	return 0;
}

static void oKinectFindClosestSkeletons(const NUI_SKELETON_FRAME& _NSF, std::array<int, NUI_SKELETON_MAX_TRACKED_COUNT>& _OutClosestIndices)
{
	static_assert(NUI_SKELETON_MAX_TRACKED_COUNT == 2, "more than two tracked skeletons not yet supported");

	_OutClosestIndices.fill(0);
	USHORT NearestDepth[NUI_SKELETON_MAX_TRACKED_COUNT];
	oFOR(auto& i, NearestDepth)
		i = NUI_IMAGE_DEPTH_MAXIMUM;

	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		if (NUI_SKELETON_NOT_TRACKED != _NSF.SkeletonData[i].eTrackingState)
		{
			LONG x, y;
			USHORT depth;
			NuiTransformSkeletonToDepthImage(_NSF.SkeletonData[i].Position, &x, &y, &depth);

			// Compare depth to previously found item
			if (depth < NearestDepth[0])
			{
				// Move depth and track ID in first place to second place and assign 
				// with the new closer one
				NearestDepth[1] = NearestDepth[0];
				NearestDepth[0] = depth;
				
				_OutClosestIndices[1] = _OutClosestIndices[0];
				_OutClosestIndices[0] = i;
			}
			
			else if (depth < NearestDepth[1])
			{
				// Replace old depth and track ID in second place with the newly found 
				// closer one
				NearestDepth[1] = depth;
				_OutClosestIndices[1] = i;
			}
		}
	}
}

// Converts to a position in pixels in the range of 0,0 to the resolution of the
// depth buffer used to generate the skeleton position.
static int2 oKinectSkeletonToScreen(const Vector4& _SkeletonPosition, const int2& _TargetDimensions, const int2& _DepthBufferResolution)
{
	int2 Screen;
	USHORT depth;
	NuiTransformSkeletonToDepthImage(_SkeletonPosition, (LONG*)&Screen.x, (LONG*)&Screen.y, &depth);
	Screen = Screen * _TargetDimensions / /*_DepthBufferResolution*/int2(320,240); // this seems hard-coded
	return Screen;
}

bool oKinectSkeletonStream::Initialize(INuiSensor* _pSensor, threadsafe oWindow* _pWindow, double _TrackingTimeoutSeconds, oKINECT_FEATURES _KinectFeatures, HANDLE _hEvent) threadsafe
{
	TrackingTimeoutSeconds = _TrackingTimeoutSeconds;
	Window = _pWindow;

	oThreadsafe(ClosestSkeletonIndices).fill(0);

	oFOR(auto& s, oThreadsafe(Skeletons))
	{
		if (!s && !oKinectSkeletonCreate(&s))
			return false; // pass through error
	}

	if (_pSensor->NuiInitializationFlags() & NUI_INITIALIZE_FLAG_USES_SKELETON)
		oVB_RETURN2(_pSensor->NuiSkeletonTrackingEnable(_hEvent, oKinectGetSkeletonInitFlags(_KinectFeatures)));

	oFOR(auto& s, oThreadsafe(Skeletons))
		s->Invalidate();

	return true;
}

oKinectSkeletonStream::oKinectSkeletonStream()
	: TrackingTimeoutSeconds(0.0)
{
}

void oKinectSkeletonStream::CacheNextFrame(INuiSensor* _pSensor) threadsafe
{
	oKinectSkeletonStream* pThis = thread_cast<oKinectSkeletonStream*>(this);

	NUI_SKELETON_FRAME NSF;
	if (SUCCEEDED(_pSensor->NuiSkeletonGetNextFrame(0, &NSF)))
	{
		_pSensor->NuiTransformSmooth(&NSF, nullptr);

		if (Window)
		{
			oFORI(i, NSF.SkeletonData)
				pThis->Skeletons[i]->Cache((HWND)Window->GetNativeHandle(), NSF.SkeletonData[i]);
		}

		TrackClosestSkeletons(_pSensor, NSF);
	}
}

void oKinectSkeletonStream::TrackClosestSkeletons(INuiSensor* _pSensor, const NUI_SKELETON_FRAME& _NSF) threadsafe
{
	static_assert(NUI_SKELETON_MAX_TRACKED_COUNT <= NUI_SKELETON_COUNT, "max tracking larger than skeleton capacity");
	oKinectSkeletonStream* pThis = thread_cast<oKinectSkeletonStream*>(this);
	oKinectFindClosestSkeletons(_NSF, pThis->ClosestSkeletonIndices);
	DWORD TrackingIDs[NUI_SKELETON_MAX_TRACKED_COUNT];
	oFORI(i, TrackingIDs)
		TrackingIDs[i] = _NSF.SkeletonData[pThis->ClosestSkeletonIndices[i]].dwTrackingID;
	_pSensor->NuiSkeletonSetTrackedSkeletons(TrackingIDs);
}

void oKinectSkeletonStream::CheckTrackingTimeouts() threadsafe
{
	oGUI_WINDOW hWnd = nullptr;
	if (Window)
	{
		oFOR(auto& S, oThreadsafe(this)->Skeletons)
			S->CheckTrackingTimeout((HWND)Window->GetNativeHandle(), TrackingTimeoutSeconds);
	}
}

bool oKinectSkeletonStream::GetSkeletonByIndex(int _PlayerIndex, oGUI_BONE_DESC* _pSkeleton) const threadsafe
{
	if (_PlayerIndex < 0 || _PlayerIndex >= NUI_SKELETON_MAX_TRACKED_COUNT)
		return false;
	const int SkeletonIndex = oThreadsafe(ClosestSkeletonIndices)[_PlayerIndex];
	oThreadsafe(Skeletons)[SkeletonIndex]->GetSkeleton(_pSkeleton);
	return _pSkeleton->SourceID != 0;
}

bool oKinectSkeletonStream::GetSkeletonByID(unsigned int _ID, oGUI_BONE_DESC* _pSkeleton) const threadsafe
{
	threadsafe oKinectSkeleton* found = nullptr;
	oKinectSkeletonStream* pThis = thread_cast<oKinectSkeletonStream*>(this);
	for (auto it = std::begin(pThis->Skeletons); it != std::end(pThis->Skeletons); ++it)
	{
		if ((*it)->GetID() == _ID)
		{
			found = *it;
			break;
		}
	}

	if (!found)
		return false;

	found->GetSkeleton(_pSkeleton);
	return true;
}

int oKinectSkeletonStream::GetScreenSpaceBonePositions(
	const oGUI_BONE_DESC& _Skeleton
	, const int2& _TargetPosition
	, const int2& _TargetDimensions
	, const int2& _DepthBufferResolution
	, int2 _BonePositions[NUI_SKELETON_POSITION_COUNT]) const threadsafe
{
	int NumValid = 0;
	oFORI(i, _Skeleton.Positions)
	{
		const float4& P = _Skeleton.Positions[i];

		if (P.w < 0.0f)
			_BonePositions[i] = int2(oDEFAULT, oDEFAULT);
		else
		{
			_BonePositions[i] = _TargetPosition + oKinectSkeletonToScreen((const Vector4&)P, _TargetDimensions, _DepthBufferResolution);
			NumValid++;
		}
	}
	return NumValid;
}

#endif // oHAS_KINECT_SDK
