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
#include <oKinect/oGestureManager.h>
#include <oPlatform/oStreamUtil.h>
#include <oGUI/windows/win_gdi_bitmap.h>
#include <oKinect/oKinectGDI.h>
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oInputMapper.h>
#include <oBasis/oRefCount.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_skeleton.h>

#include "resource.h"

using namespace ouro;
using namespace ouro::windows::gdi;
using namespace std;

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGESTURE_VISUALIZATION)
	oRTTI_ENUM_BEGIN_VALUES(oGESTURE_VISUALIZATION)
		oRTTI_VALUE_CUSTOM(oGESTURE_VIZ_NONE, "None")
		oRTTI_VALUE_CUSTOM(oGESTURE_VIZ_COLOR, "Color")
		oRTTI_VALUE_CUSTOM(oGESTURE_VIZ_TRACKING, "Tracking")
		oRTTI_VALUE_CUSTOM(oGESTURE_VIZ_AIRKEYS, "AirKeys")
		oRTTI_VALUE_CUSTOM(oGESTURE_VIZ_TRACKING_AND_AIRKEYS, "TrackingAndAirKeys")
	oRTTI_ENUM_END_VALUES(oGESTURE_VISUALIZATION)
oRTTI_ENUM_END_DESCRIPTION(oGESTURE_VISUALIZATION)

enum oKINECT_STATUS_DRAW_STATE
{
	oKINECT_STATUS_DRAW_NONE,
	oKINECT_STATUS_DRAW_DISCONNECTED,
	oKINECT_STATUS_DRAW_INITIALIZING_1,
	oKINECT_STATUS_DRAW_INITIALIZING_2,
	oKINECT_STATUS_DRAW_READY,
};

enum oGESTURE_TIMER_EVENT
{
	oGESTURE_TIMER_HEAD_MESSAGE,
	oGESTURE_TIMER_DRAW_INITIALIZING_1,
	oGESTURE_TIMER_DRAW_INITIALIZING_2,
	oGESTURE_TIMER_DRAW_NONE,
};

#define MAKE_TIMER_MSG(_VersionNumber, _GTE, _Index) (((_VersionNumber) << 16) | ((_GTE) << 8) | (_Index))
#define DECODE_TIMER_MSG(_Msg, _pVersionNumber, _pGTE, _pIndex) do \
{	*(_pVersionNumber) = oKINECT_STATUS_DRAW_STATE((_Msg) >> 16); \
	*(_pGTE) = oGESTURE_TIMER_EVENT(((_Msg) >> 8) & 0xff); \
	*(_pIndex) = (_Msg) & 0xff; \
} while (false)

struct oGestureManagerImpl : oGestureManager
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oGestureManagerImpl(const oGESTURE_MANAGER_INIT& _Init, const std::shared_ptr<ouro::window>& _Window, bool* _pSuccess);
	~oGestureManagerImpl();

	void GetDesc(oGESTURE_MANAGER_DESC* _pDesc) override;
	void SetDesc(const oGESTURE_MANAGER_DESC& _Desc) override;
	
	void GetVisualizationDesc(oGESTURE_VISUALIZATION_DESC* _pDesc) override;
	void SetVisualizationDesc(const oGESTURE_VISUALIZATION_DESC& _Desc) override;

	void GetDeviceVisualizationDesc(oGESTURE_DEVICE_VISUALIZATION_DESC* _pDesc) override;
	void SetDeviceVisualizationDesc(const oGESTURE_DEVICE_VISUALIZATION_DESC& _Desc) override;

	bool SetCurrentGestureSet(const char* _KeySetName, const oRTTI* _pInputDynEnumType) override;

	int SetHeadMessageV(int _SkeletonIndex, const char* _Format, va_list _Args) override;

	void GDIDraw(ouro::draw_context_handle _hDC, const int2& _ClientSize) override;

	bool SetCurrentKeyset(const char* _KeysetName);
	bool SetCurrentInputSet(const oRTTI* _pDynamicEnum);

	bool OnFileChange(oSTREAM_EVENT _Event, const uri_string& _ChangedURI) override;

private:

	// Generic elements
	
	std::shared_ptr<ouro::window> Window;
	intrusive_ptr<threadsafe oKinect> Kinect;
	intrusive_ptr<threadsafe oAirKeyboard> AirKeyboard;
	intrusive_ptr<threadsafe oInputMapper> InputMapper;

	typedef ouro::shared_mutex mutex_t;
	typedef ouro::lock_guard<mutex_t> lock_t;
	typedef ouro::shared_lock<mutex_t> lock_shared_t;
	mutex_t KinectMutex;

	oGESTURE_MANAGER_DESC Desc;
	oGESTURE_VISUALIZATION_DESC VizDesc;
	oGESTURE_DEVICE_VISUALIZATION_DESC DeviceVizDesc;
	oKINECT_STATUS_DRAW_STATE KinectDrawState;

	sstring CurrentKeysetName;
	const oRTTI* pCurrentInputSet;
	std::shared_ptr<xml> Keysets;
	std::shared_ptr<xml> Inputs;
	std::array<sstring, 2> ComboMessage;
	sstring NoKinectMessage;

	int hookAirKeys;
	int hookInputMaps;
	int hookEvents;
	int hookActions;

	oRefCount RefCount;


	// GDI-specific elements

	mutex_t DeviceIconMutex;

	scoped_font hFont;
	scoped_pen hBonePen;
	scoped_brush hBoneBrush;
	scoped_brush hBlankBG;

	// timer only executes if the version matches expectation
	oKINECT_STATUS_DRAW_STATE LastSetTimerState;
	int TimerMessageVersion;


	// private APIs

	void HookWindow(bool _Hooked);
	void AttachKinect(bool _Attached, const char* _InstanceName);
	void EnableGesture(bool _Enabled);

	bool GDIDrawKinect(ouro::draw_context_handle _hDC, const int2& _ClientSize);
	void GDIDrawNoKinect(ouro::draw_context_handle _hDC, const int2& _ClientSize);
	void GDIDrawKinectStatus(ouro::draw_context_handle _hDC, const int2& _ClientSize);
	void GDIDrawKinectStatusIcon(ouro::draw_context_handle _hDC, const int2& _ClientSize);
	void GDIDrawNotStatusIcon(ouro::draw_context_handle _hDC, const int2& _ClientSize);

	void OnEvent(const window::basic_event& _Event);
	void OnAction(const ouro::input::action& _Action);
	void OnDeviceChange(const window::basic_event& _Event);

	bool ReloadAirKeySets(const uri_string& _AirKeySets_xml);
	bool ReloadInputs(const uri_string& _Inputs_xml);
};

oGestureManagerImpl::oGestureManagerImpl(const oGESTURE_MANAGER_INIT& _Init, const std::shared_ptr<ouro::window>& _Window, bool* _pSuccess)
	: Window(_Window)
	, VizDesc(_Init.VizDesc)
	, DeviceVizDesc(_Init.DeviceVizDesc)
	, hBonePen(oGDICreatePen(Lime, 2))
	, hBoneBrush(oGDICreateBrush(White))
	, hBlankBG(oGDICreateBrush(Black))
	, KinectDrawState(oKINECT_STATUS_DRAW_INITIALIZING_1)
	, LastSetTimerState(oKINECT_STATUS_DRAW_NONE)
	, TimerMessageVersion(0)
	, pCurrentInputSet(nullptr)
	, NoKinectMessage("Kinect initializing...")
	, hookAirKeys(ouro::invalid)
	, hookInputMaps(ouro::invalid)
	, hookEvents(ouro::invalid)
	, hookActions(ouro::invalid)
{
	*_pSuccess = false;

	if (!Window)
	{
		oErrorSetLast(std::errc::invalid_argument, "A valid window must be specified for the gesture manager");
		return;
	}

	//hKinect = load_icon(IDI_KINECT);
	//hNot = load_icon(IDI_NOT);

	if (!oAirKeyboardCreate(&AirKeyboard))
		return; // pass through error

	if (!oInputMapperCreate(&InputMapper))
		return; // pass through error

	// @tony: todo: make this respect oStream's path stuff.
	// !!! un-hard-code the paths to oPlayer2 stuff !!!
	{
		uri_string dev_uri(ouro::filesystem::dev_path());

		uri_string AirKB = dev_uri;
		sncatf(AirKB, "oooii/Source/oPlayer2/AirKeyboards.xml");
		if (!OnFileChange(oSTREAM_ACCESSIBLE, AirKB))
			return; // pass through error

		uri_string Inputs = dev_uri;
		sncatf(Inputs, "oooii/Source/oPlayer2/Inputs.xml");
		if (!OnFileChange(oSTREAM_ACCESSIBLE, Inputs))
			return; // pass through error
	}

	Desc.GestureEnabled = false;
	SetDesc(_Init.Desc);

	*_pSuccess = true;
}

oGestureManagerImpl::~oGestureManagerImpl()
{
	EnableGesture(false);

	if (DeviceVizDesc.hGestureDevice)
		DestroyIcon((HICON)DeviceVizDesc.hGestureDevice);

	if (DeviceVizDesc.hNotOverlay)
		DestroyIcon((HICON)DeviceVizDesc.hNotOverlay);
}

bool oGestureManagerCreate(const oGESTURE_MANAGER_INIT& _Init, const std::shared_ptr<ouro::window>& _Window, oGestureManager** _ppGestureManager)
{
	bool success = false;
	oCONSTRUCT(_ppGestureManager, oGestureManagerImpl(_Init, _Window, &success));
	return success;
}

void oGestureManagerImpl::HookWindow(bool _Hooked)
{
	if (_Hooked)
	{
		hookAirKeys = AirKeyboard->HookActions(std::bind(&ouro::window::trigger, Window.get(), std::placeholders::_1));
		hookInputMaps = InputMapper->HookActions(std::bind(&ouro::window::trigger, Window.get(), std::placeholders::_1));
		hookEvents = Window->hook_events(std::bind(&oGestureManagerImpl::OnEvent, this, std::placeholders::_1));
		hookActions = Window->hook_actions(std::bind(&oGestureManagerImpl::OnAction, this, std::placeholders::_1));
	}

	else
	{
		#define oSAFE_UNHOOKE(_OBJ, _HookID) do \
		{	if (_HookID != ouro::invalid) \
			{	_OBJ->unhook_events(_HookID); \
				_HookID = ouro::invalid; \
			} \
		} while (false)

		#define oSAFE_UNHOOKA(_OBJ, _HookID) do \
		{	if (_HookID != ouro::invalid) \
				{	_OBJ->unhook_actions(_HookID); \
				_HookID = ouro::invalid; \
			} \
		} while (false)

		#define SAFE_UNHOOKE(_OBJ, _HookID) do \
		{	if (_HookID != ouro::invalid) \
			{	_OBJ->UnhookEvents(_HookID); \
				_HookID = ouro::invalid; \
			} \
		} while (false)

		#define SAFE_UNHOOKA(_OBJ, _HookID) do \
		{	if (_HookID != ouro::invalid) \
				{	_OBJ->UnhookActions(_HookID); \
				_HookID = ouro::invalid; \
			} \
		} while (false)

		oSAFE_UNHOOKE(Window, hookEvents);
		oSAFE_UNHOOKA(Window, hookActions);
		SAFE_UNHOOKA(InputMapper, hookInputMaps);
		SAFE_UNHOOKA(AirKeyboard, hookAirKeys);

		#undef SAFE_UNHOOKE
		#undef SAFE_UNHOOKA
	}
}

void oGestureManagerImpl::AttachKinect(bool _Attached, const char* _InstanceName)
{
	lock_t lock(KinectMutex);

	if (_Attached)
	{
		if (Kinect)
		{
			if (_InstanceName)
				oTRACE("GMGR: ignoring secondary: Kinect %s", _InstanceName);
			return;
		}

		if (oKinectGetCount() < 0)
			return;

		// Minimize video skipping by spinning up a separate thread for Kinect
		// creation.

		Reference();
		async([&]
		{
			intrusive_ptr<threadsafe oKinect> NewKinect;
			oKINECT_DESC kd;
			kd.PitchDegrees = Desc.GestureCameraPitchDegrees;
			bool Result = oKinectCreate(kd, Window, &NewKinect);
			
			if (Result)
			{
				NewKinect->GetDesc(&kd);
				oTRACE("GMGR: connected: Kinect %s", kd.ID.c_str());
			}
			else
				oTRACE("GMGR: connection failed Kinect %s: %s", kd.ID.c_str(), oErrorGetLastString());

			{
				lock_t lock2(KinectMutex);
				
				if (Result)
				{
					NoKinectMessage = "Kinect Initializing...";
					Kinect = NewKinect;
					TimerMessageVersion++;
					KinectDrawState = oKINECT_STATUS_DRAW_READY;
				}
			
				else
					KinectDrawState = oKINECT_STATUS_DRAW_DISCONNECTED;
			}

			Release();
		});
	}

	else if (Kinect)
	{
		oKINECT_DESC kd;
		Kinect->GetDesc(&kd);
		if (!_stricmp(kd.ID, _InstanceName))
		{
			Kinect = nullptr;
			NoKinectMessage = "No Kinect attached.";
			TimerMessageVersion++;
			KinectDrawState = oKINECT_STATUS_DRAW_DISCONNECTED;

			oTRACE("GMGR: disconnected: Kinect %s", kd.ID.c_str());
		}
	}
}

void oGestureManagerImpl::EnableGesture(bool _Enabled)
{
	HookWindow(_Enabled);
	AttachKinect(_Enabled, "?");
}

void oGestureManagerImpl::GetDesc(oGESTURE_MANAGER_DESC* _pDesc)
{
	*_pDesc = Desc;
}

void oGestureManagerImpl::SetDesc(const oGESTURE_MANAGER_DESC& _Desc)
{
	#define CHANGED(_Field) (Desc._Field != _Desc._Field)
	if (CHANGED(GestureEnabled))
	{
		EnableGesture(_Desc.GestureEnabled);
	}

	if (CHANGED(GestureCameraPitchDegrees))
	{
		lock_t lock(KinectMutex);
		if (Kinect && _Desc.GestureCameraPitchDegrees != oDEFAULT)
			Kinect->SetPitch(_Desc.GestureCameraPitchDegrees);
	}

	Desc = _Desc;
	#undef CHANGED
}

void oGestureManagerImpl::GetVisualizationDesc(oGESTURE_VISUALIZATION_DESC* _pDesc)
{
	*_pDesc = VizDesc;
}

void oGestureManagerImpl::SetVisualizationDesc(const oGESTURE_VISUALIZATION_DESC& _Desc)
{
	VizDesc = _Desc;
}

void oGestureManagerImpl::GetDeviceVisualizationDesc(oGESTURE_DEVICE_VISUALIZATION_DESC* _pDesc)
{
	*_pDesc = DeviceVizDesc;
}

void oGestureManagerImpl::SetDeviceVisualizationDesc(const oGESTURE_DEVICE_VISUALIZATION_DESC& _Desc)
{
	lock_t lock(DeviceIconMutex);

	if (DeviceVizDesc.hGestureDevice && DeviceVizDesc.hGestureDevice != _Desc.hGestureDevice)
		DestroyIcon((HICON)DeviceVizDesc.hGestureDevice);

	if (DeviceVizDesc.hNotOverlay && DeviceVizDesc.hNotOverlay != _Desc.hNotOverlay)
		DestroyIcon((HICON)DeviceVizDesc.hNotOverlay);

	DeviceVizDesc = _Desc;
}

bool oGestureManagerImpl::SetCurrentGestureSet(const char* _KeySetName, const oRTTI* _pInputDynEnumType)
{
	if (!SetCurrentKeyset(_KeySetName))
		return false; // pass through error

	if (!SetCurrentInputSet(_pInputDynEnumType))
		return false; // pass through error
	
	return true;
}

bool oGestureManagerImpl::GDIDrawKinect(ouro::draw_context_handle _hDC, const int2& _ClientSize)
{
	lock_shared_t lock(KinectMutex);

	if (Kinect)
	{
		oKINECT_FRAME_TYPE FrameType = oKINECT_FRAME_NONE;

		int KinectDrawFlags = 0;
		int AirKeysDrawFlags = 0;
		switch (VizDesc.Visualization)
		{
			case oGESTURE_VIZ_NONE:
				return true;
			case oGESTURE_VIZ_COLOR:
				FrameType = oKINECT_FRAME_COLOR;
				break;
			case oGESTURE_VIZ_TRACKING:
				FrameType = oKINECT_FRAME_COLOR;
				KinectDrawFlags |= oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_CLIPPING;
				break;
			case oGESTURE_VIZ_AIRKEYS:
				FrameType = oKINECT_FRAME_COLOR;
				AirKeysDrawFlags |= oGDI_AIR_KEY_DRAW_BOX|oGDI_AIR_KEY_DRAW_KEY;
				break;
			case oGESTURE_VIZ_TRACKING_AND_AIRKEYS:
				FrameType = oKINECT_FRAME_COLOR;
				KinectDrawFlags |= oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_CLIPPING;
				AirKeysDrawFlags |= oGDI_AIR_KEY_DRAW_BOX|oGDI_AIR_KEY_DRAW_KEY;
				break;
			oNODEFAULT;
		}

		HDC hDC = (HDC)_hDC;
		const RECT rTarget = oWinRect(ouro::resolve_rect(oRECT(oRECT::pos_size, int2(0,0), _ClientSize), VizDesc.Position, VizDesc.Size, VizDesc.Alignment, true));
			
		scoped_select ScopedSelectBrush(hDC, hBoneBrush);
		scoped_select ScopedSelectPen(hDC, hBonePen);

		oGDIDrawBox(hDC, oWinRect(rTarget.left, rTarget.top, rTarget.right + 1, rTarget.bottom + 1));
		scoped_clip_region SelectClipRegion(hDC, rTarget);
		oGDIDrawKinect(hDC, rTarget, FrameType, KinectDrawFlags, Kinect);

		// Draw boxes and some HUD info
		{
			scoped_select SelFont(hDC, hFont);
			ouro::font_info fd;
			oGDIGetFontDesc(hFont, &fd);

			ouro::input::tracking_skeleton Skeleton;
			int SkelIndex = 0;
			float VerticalOffset = (float)rTarget.top;
			while (Kinect->GetSkeletonByIndex(SkelIndex, &Skeleton))
			{
				if (AirKeyboard)
					AirKeyboard->VisitKeys(std::bind(oGDIDrawAirKey, hDC, std::ref(rTarget), oGDI_AIR_KEY_DRAW_BOX|oGDI_AIR_KEY_DRAW_KEY, std::placeholders::_1, std::placeholders::_2, std::ref(Skeleton)));

				ouro::text_info td;
				td.position = float2((float)rTarget.left, VerticalOffset);
				td.size = oWinRectSize(rTarget);
				td.shadow = Black;
				const float4& h = Skeleton.positions[ouro::input::hip_center];
				mstring text;
				snprintf(text, "HIP: %.02f %.02f %.02f\n", h.x, h.y, h.z);
				oGDIDrawText(hDC, td, text);

				RECT rText = oGDICalcTextRect(hDC, text);
				VerticalOffset += oWinRectH(rText);

				if (!ComboMessage[SkelIndex].empty())
					oGDIDrawBoneText(hDC, rTarget, Skeleton.positions[ouro::input::head], ouro::alignment::bottom_center, int2(0, -75), ouro::alignment::bottom_center, ComboMessage[SkelIndex]);

				SkelIndex++;
			}
		}
	}

	return !!Kinect;
}

void oGestureManagerImpl::GDIDrawNoKinect(ouro::draw_context_handle _hDC, const int2& _ClientSize)
{
	HDC hDC = (HDC)_hDC;

	const RECT rTarget = oWinRect(ouro::resolve_rect(oRECT(oRECT::pos_size, int2(0,0), _ClientSize), VizDesc.Position, VizDesc.Size, VizDesc.Alignment, true));

	{
		scoped_select ScopedSelectBrush(hDC, hBlankBG);
		oGDIDrawBox(hDC, oWinRect(rTarget.left, rTarget.top, rTarget.right + 1, rTarget.bottom + 1));
	}

	scoped_clip_region SelectClipRegion(hDC, rTarget);
	scoped_select ScopedSelectBrush(hDC, hBoneBrush);
	scoped_select ScopedSelectPen(hDC, hBonePen);
	scoped_select ScopedSelFont(hDC, hFont);

	ouro::text_info td;
	td.position = float2(oWinRectPosition(rTarget));
	td.size = oWinRectSize(rTarget);
	td.shadow = Gray;
	td.alignment = ouro::alignment::middle_center;
	oGDIDrawText(hDC, td, NoKinectMessage);
}

void oGestureManagerImpl::GDIDrawKinectStatusIcon(ouro::draw_context_handle _hDC, const int2& _ClientSize)
{
	if (DeviceVizDesc.hGestureDevice)
	{
		oRECT parent(oRECT::pos_size, int2(0,0), _ClientSize);
		oRECT r = ouro::resolve_rect(parent, DeviceVizDesc.Position
			, icon_dimensions((HICON)DeviceVizDesc.hGestureDevice), DeviceVizDesc.Alignment, true);

		HDC hDC = (HDC)_hDC;
		oVB(DrawIconEx(hDC, r.Min.x, r.Min.y, (HICON)DeviceVizDesc.hGestureDevice, r.size().x, r.size().y, 0, nullptr, DI_NORMAL));
	}
}

void oGestureManagerImpl::GDIDrawNotStatusIcon(ouro::draw_context_handle _hDC, const int2& _ClientSize)
{
	if (DeviceVizDesc.hNotOverlay)
	{
		oRECT parent(oRECT::pos_size, int2(0,0), _ClientSize);
		oRECT r = ouro::resolve_rect(parent, DeviceVizDesc.Position
			, icon_dimensions((HICON)DeviceVizDesc.hNotOverlay), DeviceVizDesc.Alignment, true);

		HDC hDC = (HDC)_hDC;
		oVB(DrawIconEx(hDC, r.Min.x, r.Min.y, (HICON)DeviceVizDesc.hNotOverlay, r.size().x, r.size().y, 0, nullptr, DI_NORMAL));
	}
}

void oGestureManagerImpl::GDIDrawKinectStatus(ouro::draw_context_handle _hDC, const int2& _ClientSize)
{
	shared_lock<shared_mutex> lock(DeviceIconMutex);

	switch (KinectDrawState)
	{
		case oKINECT_STATUS_DRAW_DISCONNECTED:
			GDIDrawKinectStatusIcon(_hDC, _ClientSize);
			GDIDrawNotStatusIcon(_hDC, _ClientSize);
			break;

		case oKINECT_STATUS_DRAW_INITIALIZING_1:
			GDIDrawKinectStatusIcon(_hDC, _ClientSize);
			if (LastSetTimerState != oKINECT_STATUS_DRAW_INITIALIZING_1)
			{
				LastSetTimerState = KinectDrawState;
				TimerMessageVersion++;
				Window->start_timer(MAKE_TIMER_MSG(TimerMessageVersion, oGESTURE_TIMER_DRAW_INITIALIZING_2, 0), DeviceVizDesc.BlinkTimeoutMS);
			}
			break;

		case oKINECT_STATUS_DRAW_INITIALIZING_2:
			GDIDrawKinectStatusIcon(_hDC, _ClientSize);
			GDIDrawNotStatusIcon(_hDC, _ClientSize);
			if (LastSetTimerState != oKINECT_STATUS_DRAW_INITIALIZING_2)
			{
				LastSetTimerState = KinectDrawState;
				TimerMessageVersion++;
				Window->start_timer(MAKE_TIMER_MSG(TimerMessageVersion, oGESTURE_TIMER_DRAW_INITIALIZING_1, 0), DeviceVizDesc.BlinkTimeoutMS);
			}
			break;

		case oKINECT_STATUS_DRAW_READY:
			GDIDrawKinectStatusIcon(_hDC, _ClientSize);
			if (LastSetTimerState != oKINECT_STATUS_DRAW_READY)
			{
				LastSetTimerState = KinectDrawState;
				TimerMessageVersion++;
				Window->start_timer(MAKE_TIMER_MSG(TimerMessageVersion, oGESTURE_TIMER_DRAW_NONE, 0), DeviceVizDesc.ShowTimeoutMS);
			}
			break;

		default:
		case oKINECT_STATUS_DRAW_NONE:
			break;
	}
}

void oGestureManagerImpl::GDIDraw(ouro::draw_context_handle _hDC, const int2& _ClientSize)
{
	if (VizDesc.Visualization != oGESTURE_VIZ_NONE && !GDIDrawKinect(_hDC, _ClientSize))
		GDIDrawNoKinect(_hDC, _ClientSize);

	GDIDrawKinectStatus(_hDC, _ClientSize);
}

bool oGestureManagerImpl::SetCurrentKeyset(const char* _KeysetName)
{
	if (!Keysets)
		return oErrorSetLast(std::errc::no_such_file_or_directory, "no current keyset loaded");

	if (!oSTRVALID(_KeysetName))
	{
		oTRACE("Keyset cleared.");
		CurrentKeysetName = "";
		AirKeyboard->SetKeySet(nullptr);
	}

	else
	{
		intrusive_ptr<threadsafe oAirKeySet> KeySet;
		if (oParseAirKeySetsList(*Keysets
			, Keysets->first_child(Keysets->root(), "oAirKeySetList")
			, _KeysetName
			, &KeySet))
		{
			AirKeyboard->SetKeySet(KeySet);
			CurrentKeysetName = _KeysetName;
		}
		else
			return oErrorSetLast(std::errc::no_such_file_or_directory, "not found: %s", _KeysetName);
	}
	
	oTRACE("GMGR: keyset activated: %s", CurrentKeysetName.empty() ? "(none)" : CurrentKeysetName.c_str());
	return true;
}

bool oGestureManagerImpl::SetCurrentInputSet(const oRTTI* _pDynamicEnum)
{
	if (!Inputs)
		return oErrorSetLast(std::errc::no_such_file_or_directory, "no current inputs loaded");

	if (_pDynamicEnum && _pDynamicEnum->GetType() != oRTTI_TYPE_ENUM)
		return oErrorSetLast(std::errc::invalid_argument, "oRTTI must be of an enum");

	intrusive_ptr<threadsafe oInputSet> InputSet;
	pCurrentInputSet = _pDynamicEnum;
	if (pCurrentInputSet)
	{
		if (oParseInputSetList(*Inputs
			, Inputs->first_child(Inputs->root(), "oInputSetList")
			, *pCurrentInputSet
			, &InputSet))
			InputMapper->SetInputSet(InputSet);
	}

	InputMapper->SetInputSet(InputSet);
	sstring typeName;
	oTRACE("GMGR: inputset activated: %s", pCurrentInputSet ? pCurrentInputSet->GetName(typeName) : "(none)");
	return true;
}

int oGestureManagerImpl::SetHeadMessageV(int _SkeletonIndex, const char* _Format, va_list _Args)
{
	if (_SkeletonIndex != 0 && _SkeletonIndex != 1)
	{
		oErrorSetLast(std::errc::invalid_argument, "Invalid _SkeletonIndex %d", _SkeletonIndex);
		return ouro::invalid;
	}

	int l = vsnprintf(ComboMessage[_SkeletonIndex], _Format, _Args);
	//TimerMessageVersion++; // don't respect this because it is separate from
													 // kinect status timer
	Window->start_timer(MAKE_TIMER_MSG(TimerMessageVersion, oGESTURE_TIMER_HEAD_MESSAGE, _SkeletonIndex), Desc.HeadMessageTimeoutMS);
	return l;
}

void oGestureManagerImpl::OnEvent(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case ouro::event_type::sized:
		{
			// Keep the font proportional to the size of the rectangle.
			ouro::display::info di = ouro::display::get_info(oWinGetDisplayId((HWND)_Event.window));
			float2 Ratio = float2(VizDesc.Size) / float2(int2(di.mode.width, di.mode.height));
			float R = min(Ratio);
			ouro::font_info fd;
			fd.point_size = round(R * 50.0f);
			hFont = oGDICreateFont(fd);
			break;
		}

		case ouro::event_type::timer:
		{
			int TimerVersion = ouro::invalid;
			oGESTURE_TIMER_EVENT GTE = oGESTURE_TIMER_HEAD_MESSAGE;
			int SkeletonIndex = ouro::invalid;
			DECODE_TIMER_MSG(_Event.as_timer().context, &TimerVersion, &GTE, &SkeletonIndex);
			
			switch (GTE)
			{
				case oGESTURE_TIMER_HEAD_MESSAGE:
					ComboMessage[SkeletonIndex].clear();
					break;

				case oGESTURE_TIMER_DRAW_INITIALIZING_1:
					if (TimerVersion == TimerMessageVersion)
						KinectDrawState = oKINECT_STATUS_DRAW_INITIALIZING_1;
					break;

				case oGESTURE_TIMER_DRAW_INITIALIZING_2:
					if (TimerVersion == TimerMessageVersion)
						KinectDrawState = oKINECT_STATUS_DRAW_INITIALIZING_2;
					break;

				case oGESTURE_TIMER_DRAW_NONE:
					if (TimerVersion == TimerMessageVersion)
						KinectDrawState = oKINECT_STATUS_DRAW_NONE;
					break;

				default:
					break;
			}

			break;
		}

		case ouro::event_type::input_device_changed:
		{
			OnDeviceChange(_Event);
			break;
		}

		default:
			break;
	}
}

void oGestureManagerImpl::OnAction(const ouro::input::action& _Action)
{
	switch (_Action.action_type)
	{
		case ouro::input::skeleton_acquired:
		{
			sstring text;
			snprintf(text, "GMGR: Skeleton[%d] activated", _Action.device_id);
			AirKeyboard->AddSkeleton(_Action.device_id);
			break;
		}

		case ouro::input::skeleton_lost:
		{
			sstring text;
			snprintf(text, "GMGR: Skeleton[%d] deactivated", _Action.device_id);
			AirKeyboard->RemoveSkeleton(_Action.device_id);
			break;
		}

		case ouro::input::skeleton:
		{
			windows::skeleton::bone_info Skeleton;
			windows::skeleton::get_info((windows::skeleton::handle)_Action.skeleton, &Skeleton);

			ouro::input::tracking_skeleton skel;
			skel.source_id = Skeleton.source_id;
			skel.clipping = *(ouro::input::tracking_clipping*)&Skeleton.clipping;
			std::copy(Skeleton.positions.begin(), Skeleton.positions.begin() + skel.positions.size(), skel.positions.begin());

			AirKeyboard->Update(skel, _Action.timestamp_ms);
			break;
		}

		case ouro::input::key_down:
		case ouro::input::key_up:
		{
			InputMapper->OnAction(_Action);
			break;
		}

		default:
			break;
	}
}

void oGestureManagerImpl::OnDeviceChange(const window::basic_event& _Event)
{
	if (_Event.type == ouro::input::skeleton)
	{
		if (!Desc.GestureEnabled)
		{
			oTRACE("GMGR: ignoring (gesture sys disabled): Kinect %s", _Event.as_input_device().instance_name);
			return;
		}

		//KinectIcon(_Event.AsInputDevice().Status);
		switch (_Event.as_input_device().status)
		{
			case ouro::input::ready:
				AttachKinect(true, _Event.as_input_device().instance_name);
				break;
			case ouro::input::not_connected:
				AttachKinect(false, _Event.as_input_device().instance_name);
				break;
			case ouro::input::initializing:
				NoKinectMessage = "Kinect initializing...";
				KinectDrawState = oKINECT_STATUS_DRAW_INITIALIZING_1;
				break;
			default:
				break;
		}
	}
}

bool oGestureManagerImpl::ReloadAirKeySets(const uri_string& _AirKeySets_xml)
{
	std::shared_ptr<xml> XML;
	try
	{
		intrusive_ptr<threadsafe oAirKeySet> KeySet;
		XML = oXMLLoad(_AirKeySets_xml);
	}

	catch (std::exception& e)
	{
		return oErrorSetLast(e);
	}

	Keysets = XML;
	if (!SetCurrentKeyset(CurrentKeysetName))
		return false; // pass through error
	return true;
}

bool oGestureManagerImpl::ReloadInputs(const uri_string& _Inputs_xml)
{
	std::shared_ptr<xml> XML;
	try
	{
		intrusive_ptr<threadsafe oInputSet> InputSet;
		XML = oXMLLoad(_Inputs_xml);
	}

	catch (std::exception& e)
	{
		return oErrorSetLast(e);
	}

	Inputs = XML;
	if (!SetCurrentInputSet(pCurrentInputSet))
		return false; // pass through error
	return true;
}

bool oGestureManagerImpl::OnFileChange(oSTREAM_EVENT _Event, const uri_string& _ChangedURI)
{
	switch (_Event)
	{
		case oSTREAM_ACCESSIBLE:
		{
			uri URI(_ChangedURI);
			path path = URI.path();
			auto filename = path.filename();

			if (!_stricmp("Inputs.xml", filename))
			{
				if (!ReloadInputs(_ChangedURI))
					return false; // pass through error
			}

			else if (!_stricmp("AirKeyboards.xml", filename))
			{
				if (!ReloadAirKeySets(_ChangedURI))
					return false; // pass through error
			}

			break;
		}

		default:
			break;
	}

	return true;
}
