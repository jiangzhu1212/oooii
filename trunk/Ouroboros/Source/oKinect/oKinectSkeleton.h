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
#ifndef oKinectSkeleton_h
#define oKinectSkeleton_h

#ifdef oHAS_KINECT_SDK

#include <oBasis/oRefCount.h>

#undef interface
#undef INTERFACE_DEFINED
#include <windows.h>
#include <NuiApi.h>
#include <oCore/windows/win_skeleton.h>
#include <oCore/mutex.h>

class oKinectSkeleton
{
public:
	typedef Vector4 bone_position_t;

	oKinectSkeleton();
	~oKinectSkeleton();

	void Reference() threadsafe { RefCount.Reference(); }
	void Release() threadsafe { if (RefCount.Release()) delete this; }

	// Parse skeleton data into activated/deactivated events as well as changes to
	// ID and bone position updates.
	void Cache(HWND _hWnd, const NUI_SKELETON_DATA& _NSD) threadsafe;

	// Marks bones as invalid
	void Invalidate() threadsafe;

	// If no new skeleton data has been received for the specified time, send
	// an oWM_USER_LOST message.
	void CheckTrackingTimeout(HWND _hWnd, double _TimeoutThresholdInSeconds) threadsafe;

	// Copies the most recent skeleton data to the specified pointer.
	void GetSkeleton(ouro::windows::skeleton::bone_info* _pSkeleton) const threadsafe;

	// Returns the skeleton ID
	inline DWORD GetID() const threadsafe { return Skeleton.source_id; }

private:
	double LastTrackedTimestamp;
	oRefCount RefCount;
	typedef ouro::shared_mutex mutex_t;
	typedef std::lock_guard<mutex_t> lock_t;
	mutable mutex_t Mutex;
	ouro::windows::skeleton::bone_info Skeleton;
};

inline void intrusive_ptr_add_ref(const threadsafe oKinectSkeleton* p) { const_cast<threadsafe oKinectSkeleton*>(p)->Reference(); }
inline void intrusive_ptr_release(const threadsafe oKinectSkeleton* p) { const_cast<threadsafe oKinectSkeleton*>(p)->Release(); }

bool oKinectSkeletonCreate(threadsafe oKinectSkeleton** _ppSkeleton);

#endif // oHAS_KINECT_SDK
#endif
