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
#include <oKinect/oKinect.h>
#include <oBase/invalid.h>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oKINECT_FEATURES)
	oRTTI_ENUM_BEGIN_VALUES(oKINECT_FEATURES)
	oRTTI_VALUE_CUSTOM(oKINECT_NONE, "none")
	oRTTI_VALUE_CUSTOM(oKINECT_COLOR, "color")
	oRTTI_VALUE_CUSTOM(oKINECT_DEPTH, "depth")
	oRTTI_VALUE_CUSTOM(oKINECT_COLOR_DEPTH, "color_depth")
	oRTTI_VALUE_CUSTOM(oKINECT_DEPTH_SKELETON_SITTING, "depth_skeleton_sitting")
	oRTTI_VALUE_CUSTOM(oKINECT_DEPTH_SKELETON_STANDING, "depth_skeleton_standing")
	oRTTI_VALUE_CUSTOM(oKINECT_COLOR_DEPTH_SKELETON_SITTING, "color_depth_skeleton_sitting")
	oRTTI_VALUE_CUSTOM(oKINECT_COLOR_DEPTH_SKELETON_STANDING, "color_depth_skeleton_standing")
	oRTTI_ENUM_END_VALUES(oKINECT_FEATURES)
	oRTTI_ENUM_VALIDATE_COUNT(oKINECT_FEATURES, oKINECT_FEATURES_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oKINECT_FEATURES)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oKINECT_FRAME_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oKINECT_FRAME_TYPE)
	oRTTI_VALUE_CUSTOM(oKINECT_FRAME_NONE, "none")
	oRTTI_VALUE_CUSTOM(oKINECT_FRAME_COLOR, "color")
	oRTTI_VALUE_CUSTOM(oKINECT_FRAME_DEPTH, "depth")
	oRTTI_ENUM_END_VALUES(oKINECT_FRAME_TYPE)
	oRTTI_ENUM_VALIDATE_COUNT(oKINECT_FRAME_TYPE, oKINECT_FRAME_TYPE_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oKINECT_FRAME_TYPE)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oKINECT_DESC)
	oRTTI_COMPOUND_ABSTRACT(oKINECT_DESC)
	oRTTI_COMPOUND_VERSION(oKINECT_DESC, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oKINECT_DESC)
		oRTTI_COMPOUND_ATTR(oKINECT_DESC, TrackingTimeoutSeconds, oRTTI_OF(double), "TrackingTimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oKINECT_DESC, PitchDegrees, oRTTI_OF(int), "PitchDegrees", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oKINECT_DESC, Features, oRTTI_OF(oKINECT_FEATURES), "Features", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oKINECT_DESC, Index, oRTTI_OF(int), "Index", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oKINECT_DESC, ID, oRTTI_OF(ouro_sstring), "ID", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oKINECT_DESC)
oRTTI_COMPOUND_END_DESCRIPTION(oKINECT_DESC)

#ifdef oHAS_KINECT_SDK

#include <oKinect/oKinectUtil.h>
#include "oKinectInternal.h"
#include "oWinKinect10.h"
#include "oKinectManager.h"

#undef interface
#include <oCore/windows/win_util.h>

using namespace ouro;
using namespace std;

static const NUI_IMAGE_RESOLUTION kResolution = NUI_IMAGE_RESOLUTION_640x480;

bool oKinectImpl::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;
	if (_InterfaceID == oGUID_oInterface || _InterfaceID == oGUID_oKinect)
	{
		Reference();
		*_ppInterface = this;
	}

	else if (_InterfaceID == (const oGUID&)__uuidof(INuiSensor))
	{
		NUISensor->AddRef();
		*_ppInterface = NUISensor;
	}

	return !!*_ppInterface;
}

bool oKinectImpl::Reinitialize()
{
	if (Desc.ID.empty())
	{
		HRESULT hr = oWinKinect10::Singleton().SafeNuiCreateSensorByIndex(Desc.Index, &NUISensor); 
		if (hr == E_NUI_BADIINDEX)
			return oErrorSetLast(errc::no_such_device);
		else if (FAILED(hr))
			throw windows::error(hr);

		oASSERT(NUISensor->NuiInstanceIndex() == Desc.Index, "");
		Desc.ID = NUISensor->NuiDeviceConnectionId();
		oTRACE("oKinect 0x%p reinitialized with ConnectionId %s", this, Desc.ID.c_str());
	}

	else
	{
		static_assert(sizeof(wchar_t) == sizeof(remove_pointer<BSTR>::type), "BSTR size mismatch");
		swstring wID = Desc.ID;
		HRESULT hr = oWinKinect10::Singleton().SafeNuiCreateSensorById(wID, &NUISensor); 
		if (hr == E_NUI_BADIINDEX)
			return oErrorSetLast(errc::no_such_device);
		else if (FAILED(hr))
			throw windows::error(hr);
		Desc.Index = NUISensor->NuiInstanceIndex();
	}

	ouro::input::status s = oKinectImpl::GetStatus();
	if (ouro::input::ready != s)
		return oErrorSetLast(oKinectGetErrcFromStatus(s), oKinectGetErrcStringFromStatus(s));

	const DWORD InitFlags = oKinectGetInitFlags(Desc.Features);
	oV(NUISensor->NuiInitialize(InitFlags));
	try
	{
		if (InitFlags & NUI_INITIALIZE_FLAG_USES_COLOR)
		{
			if (!Color)
				oVERIFY(oKinectCreateSurface(NUI_IMAGE_TYPE_COLOR, kResolution, &Color));
			if (!hColorStream)
				oV(NUISensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, kResolution, 0, oKINECT_MAX_CACHED_FRAMES, hEvents[oKINECT_EVENT_COLOR_FRAME], &hColorStream));
		}

		if ((InitFlags & NUI_INITIALIZE_FLAG_USES_DEPTH) || (InitFlags & NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX))
		{
			const NUI_IMAGE_TYPE Type = (InitFlags & NUI_INITIALIZE_FLAG_USES_SKELETON) 
				? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX 
				: NUI_IMAGE_TYPE_DEPTH;

			if (!Depth)
				oVERIFY(oKinectCreateSurface(Type, kResolution, &Depth));
			if (!hDepthStream)
				oV(NUISensor->NuiImageStreamOpen(Type, kResolution, 0, oKINECT_MAX_CACHED_FRAMES, hEvents[oKINECT_EVENT_DEPTH_FRAME], &hDepthStream));
		}

		if (InitFlags & NUI_INITIALIZE_FLAG_USES_SKELETON)
			Skeletons.Initialize(NUISensor, Window, Desc.TrackingTimeoutSeconds, Desc.Features, hEvents[oKINECT_EVENT_SKELETON]);
	}

	catch (exception& e)
	{
		return oErrorSetLast(e);
	}

	// @tony: We could roll all event handling into the single window thread
	// with this style API... but it would alter some of the setup in the util 
	// code. It's worth a look.
	// DWORD dwEvent = MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);
	EventThread = move(thread(&oKinectImpl::OnEvent, this));
	PitchThread = move(thread(&oKinectImpl::OnPitch, this));

	oKinectImpl::SetPitch(Desc.PitchDegrees);

	Running = true;
	return true;
}

oKinectImpl::oKinectImpl(const oKINECT_DESC& _Desc, const shared_ptr<window>& _Window, bool* _pSuccess)
	: Desc(_Desc)
	, Window(_Window)
	, hColorStream(nullptr)
	, hDepthStream(nullptr)
	, NewPitch(oDEFAULT)
	, Running(false)
{
	*_pSuccess = false;

	version v = oWinKinect10::Singleton().GetVersion();

	if (v.major != oKINECT_SDK_MAJOR || v.minor != oKINECT_SDK_MINOR)
	{
		sstring v1, v2;

		if (v.major == 0)
			oErrorSetLast(errc::protocol_error, "oKinect requires the Kinect Runtime %s to be installed.", to_string2(v1, version(oKINECT_SDK_MAJOR, oKINECT_SDK_MINOR)));
		else
			oErrorSetLast(errc::protocol_error, "oKinect requires %s. Version %s is currently installed.", to_string2(v1, version(oKINECT_SDK_MAJOR, oKINECT_SDK_MINOR)), to_string2(v2, v));
		return;
	}

	oFORI(i, hEvents)
		hEvents[i] = CreateEvent(nullptr, true, false, nullptr);

	if (!Reinitialize())
		return; // pass through error

	oKinectManager::Singleton()->Register(this);
	*_pSuccess = true;
}

void oKinectImpl::Shutdown()
{
	Running = false;
	lock_unique_t Lock(PitchMutex);
	PitchCV.notify_all();
	Lock.unlock();

	SetEvent(hEvents[oKINECT_EVENT_WAKEUP]);
	PitchThread.join();
	EventThread.join();

	oWinKinect10::Singleton().SafeNuiShutdown(NUISensor);

	NUISensor = nullptr;
	hColorStream = nullptr;
	hDepthStream = nullptr;

	oFORI(i, hEvents)
		ResetEvent(hEvents[i]);
}

oKinectImpl::~oKinectImpl()
{
	oKinectManager::Singleton()->Unregister(this);

	Shutdown();

	oFORI(i, hEvents)
		CloseHandle(hEvents[i]);
}

void oKinectImpl::GetDesc(oKINECT_DESC* _pDesc) const threadsafe
{
	*_pDesc = oThreadsafe(Desc);
}

int2 oKinectImpl::GetDimensions(oKINECT_FRAME_TYPE _Type) const threadsafe
{
	surface::info inf;

	oKinectImpl* pThis = thread_cast<oKinectImpl*>(this);

	switch (_Type)
	{
		case oKINECT_FRAME_COLOR:
			if (pThis->Color)
				inf = pThis->Color->get_info();
			break;
		case oKINECT_FRAME_DEPTH:
			if (pThis->Depth)
				inf = pThis->Depth->get_info();
		default: break;
	}
	return inf.dimensions.xy();
}

bool oKinectImpl::GetSkeletonByIndex(int _PlayerIndex, ouro::input::tracking_skeleton* _pSkeleton) const threadsafe
{
	windows::skeleton::bone_info Skeleton;
	if (Skeletons.GetSkeletonByIndex(_PlayerIndex, &Skeleton))
	{
		_pSkeleton->source_id = Skeleton.source_id;
		_pSkeleton->clipping = *(ouro::input::tracking_clipping*)&Skeleton.clipping;
		copy(Skeleton.positions.begin(), Skeleton.positions.begin() + _pSkeleton->positions.size(), _pSkeleton->positions.begin());
	}

	return false;
}

bool oKinectImpl::GetSkeletonByID(unsigned int _ID, ouro::input::tracking_skeleton* _pSkeleton) const threadsafe
{
	windows::skeleton::bone_info Skeleton;
	if (Skeletons.GetSkeletonByID(_ID, &Skeleton))
	{
		_pSkeleton->source_id = Skeleton.source_id;
		_pSkeleton->clipping = *(ouro::input::tracking_clipping*)&Skeleton.clipping;
		copy(Skeleton.positions.begin(), Skeleton.positions.begin() + _pSkeleton->positions.size(), _pSkeleton->positions.begin());
	}

	return false;
}

ouro::input::status oKinectImpl::GetStatus() const threadsafe
{
	return oKinectStatusFromHR(thread_cast<INuiSensor*>(NUISensor.c_ptr())->NuiStatus());
}

void oKinectImpl::SetPitch(int _Degrees) threadsafe
{
	if (_Degrees != oDEFAULT)
	{
		lock_t lock(thread_cast<mutex_t&>(PitchMutex));
		NewPitch = _Degrees;
		thread_cast<condition_variable&>(PitchCV).notify_all();
	}
}

void oKinectImpl::OnPitch()
{
	{
		mstring Name;
		snprintf(Name, "oKinect[%d].SetPitch", Desc.Index);
		oConcurrency::begin_thread(Name);
	}

	while (Running)
	{
		lock_unique_t lock(PitchMutex);
		while (Running && (NewPitch < NUI_CAMERA_ELEVATION_MINIMUM || NewPitch > NUI_CAMERA_ELEVATION_MAXIMUM))
			PitchCV.wait(lock);

		if (!Running)
			break;

		int Pitch = NewPitch;
		Desc.PitchDegrees = Pitch;
		NewPitch = oDEFAULT;
		lock.unlock();

		if (NUISensor && NUISensor->NuiStatus() == S_OK)
		{
			oTRACE("Kinect[%d] setting pitch to %d degrees (takes 20+ sec)...", Desc.Index, Pitch);
			NUISensor->NuiCameraElevationSetAngle(Pitch);
			oTRACE("Kinect[%d] pitch set to %d.", Desc.Index, Pitch);
		}

		else
			oTRACE("Kinect[%d] set pitch to %d degrees ignored because Kinect is not ready.", Desc.Index, Pitch);
	}

	oConcurrency::end_thread();
}

bool oKinectImpl::MapRead(oKINECT_FRAME_TYPE _Type, surface::info* _pInfo, surface::const_mapped_subresource* _pMapped) const threadsafe
{
	int2 ByteDimensions;

	oKinectImpl* pThis = thread_cast<oKinectImpl*>(this);

	switch (_Type)
	{
		case oKINECT_FRAME_COLOR:
			if (!pThis->Color)
				return oErrorSetLast(errc::operation_not_supported, "color support not configured");
			*_pInfo = pThis->Color->get_info();
			pThis->Color->map_const(0, _pMapped, &ByteDimensions);
			break;
		case oKINECT_FRAME_DEPTH:
			if (!pThis->Depth)
				return oErrorSetLast(errc::operation_not_supported, "depth support not configured");
			*_pInfo = pThis->Depth->get_info();
			pThis->Depth->map_const(0, _pMapped, &ByteDimensions);
			break;
		default:
			return oErrorSetLast(errc::operation_not_supported, "_Type %d not supported", _Type);
	}

	return true;
}

void oKinectImpl::UnmapRead(oKINECT_FRAME_TYPE _Type) const threadsafe
{
	oKinectImpl* pThis = thread_cast<oKinectImpl*>(this);

	switch (_Type)
	{
		case oKINECT_FRAME_COLOR:
			if (!pThis->Color)
				oTHROW(operation_not_supported, "color image not configured");
			pThis->Color->unmap_const(0);
			break;
		case oKINECT_FRAME_DEPTH:
			if (!pThis->Depth)
				oTHROW(operation_not_supported, "depth image not configured");
			pThis->Depth->unmap_const(0);
			break;
		default:
			throw runtime_error("bad frame type");
	}
}

void oKinectImpl::OnEvent()
{
	{
		mstring Name;
		snprintf(Name, "oKinect[%d]", Desc.Index);
		oConcurrency::begin_thread(Name);
	}

	while (Running)
	{
		DWORD EventIndex = WaitForMultipleObjects(oCOUNTOF(hEvents), hEvents, FALSE, 100);
		if (EventIndex != WAIT_TIMEOUT)
		{
			try
			{
				// Multiple events can be set for the same wake, so check each individually

				if (WAIT_OBJECT_0 == WaitForSingleObject(hEvents[oKINECT_EVENT_COLOR_FRAME], 0))
					oKinectUpdate(NUISensor, hColorStream, Color.get());

				if (WAIT_OBJECT_0 == WaitForSingleObject(hEvents[oKINECT_EVENT_DEPTH_FRAME], 0))
					oKinectUpdate(NUISensor, hDepthStream, Depth.get());

				if (WAIT_OBJECT_0 == WaitForSingleObject(hEvents[oKINECT_EVENT_SKELETON], 0))
					Skeletons.CacheNextFrame(NUISensor);
			}

			catch (exception& e)
			{
				oTRACE("oKinect[%d] ptr=0x%p exception caught: %s", Desc.Index, this, e.what());
				oFORI(i, hEvents)
					ResetEvent(hEvents[i]);
			}
		}

		Skeletons.CheckTrackingTimeouts();
	}

	oConcurrency::end_thread();
}

int oKinectGetCount()
{
	int Count = ouro::invalid;
	NuiGetSensorCount(&Count);
	return Count;
}

bool oKinectCreate(const oKINECT_DESC& _Desc, const shared_ptr<window>& _Window, threadsafe oKinect** _ppKinect)
{
	bool success = false;
	oCONSTRUCT(_ppKinect, oKinectImpl(_Desc, _Window, &success));
	return success;
}

#else

int oKinectGetCount()
{
	oErrorSetLast(errc::no_such_device, "library not compiled with Kinect support");
	return ouro::invalid;
}

bool oKinectCreate(const oKINECT_DESC& _Desc, const shared_ptr<window>& _Window, threadsafe oKinect** _ppKinect)
{
	return oErrorSetLast(errc::no_such_device, "library not compiled with Kinect support");
}

#endif // oHAS_KINECT_SDK
