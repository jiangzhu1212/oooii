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
#include <oGUI/Windows/oGDI.h>
#include <oPlatform/Windows/oWinSkeleton.h>
#include <oKinect/oKinectGDI.h>
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oInputMapper.h>

#include "../Source/oStd/win.h"
#include "resource.h"

using namespace ouro;
using namespace oStd;

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

	void GDIDraw(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize) override;

	bool SetCurrentKeyset(const char* _KeysetName);
	bool SetCurrentInputSet(const oRTTI* _pDynamicEnum);

	bool OnFileChange(oSTREAM_EVENT _Event, const uri_string& _ChangedURI) override;

private:

	// Generic elements
	
	std::shared_ptr<ouro::window> Window;
	intrusive_ptr<threadsafe oKinect> Kinect;
	intrusive_ptr<threadsafe oAirKeyboard> AirKeyboard;
	intrusive_ptr<threadsafe oInputMapper> InputMapper;

	shared_mutex KinectMutex;

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

	shared_mutex DeviceIconMutex;

	oGDIScopedObject<HFONT> hFont;
	oGDIScopedObject<HPEN> hBonePen;
	oGDIScopedObject<HBRUSH> hBoneBrush;
	oGDIScopedObject<HBRUSH> hBlankBG;

	// timer only executes if the version matches expectation
	oKINECT_STATUS_DRAW_STATE LastSetTimerState;
	int TimerMessageVersion;


	// private APIs

	void HookWindow(bool _Hooked);
	void AttachKinect(bool _Attached, const char* _InstanceName);
	void EnableGesture(bool _Enabled);

	bool GDIDrawKinect(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize);
	void GDIDrawNoKinect(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize);
	void GDIDrawKinectStatus(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize);
	void GDIDrawKinectStatusIcon(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize);
	void GDIDrawNotStatusIcon(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize);

	void OnEvent(const oGUI_EVENT_DESC& _Event);
	void OnAction(const oGUI_ACTION_DESC& _Action);
	void OnDeviceChange(const oGUI_EVENT_DESC& _Event);

	bool ReloadAirKeySets(const uri_string& _AirKeySets_xml);
	bool ReloadInputs(const uri_string& _Inputs_xml);
};

oGestureManagerImpl::oGestureManagerImpl(const oGESTURE_MANAGER_INIT& _Init, const std::shared_ptr<ouro::window>& _Window, bool* _pSuccess)
	: Window(_Window)
	, VizDesc(_Init.VizDesc)
	, DeviceVizDesc(_Init.DeviceVizDesc)
	, hBonePen(oGDICreatePen(OOOiiGreen, 2))
	, hBoneBrush(oGDICreateBrush(White))
	, hBlankBG(oGDICreateBrush(Black))
	, KinectDrawState(oKINECT_STATUS_DRAW_INITIALIZING_1)
	, LastSetTimerState(oKINECT_STATUS_DRAW_NONE)
	, TimerMessageVersion(0)
	, pCurrentInputSet(nullptr)
	, NoKinectMessage("Kinect initializing...")
	, hookAirKeys(oInvalid)
	, hookInputMaps(oInvalid)
	, hookEvents(oInvalid)
	, hookActions(oInvalid)
{
	*_pSuccess = false;

	if (!Window)
	{
		oErrorSetLast(std::errc::invalid_argument, "A valid window must be specified for the gesture manager");
		return;
	}

	//hKinect = oGDILoadIcon(IDI_KINECT);
	//hNot = oGDILoadIcon(IDI_NOT);

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
		hookAirKeys = AirKeyboard->HookActions(oBIND(&ouro::window::trigger, Window.get(), oBIND1));
		hookInputMaps = InputMapper->HookActions(oBIND(&ouro::window::trigger, Window.get(), oBIND1));
		hookEvents = Window->hook_events(oBIND(&oGestureManagerImpl::OnEvent, this, oBIND1));
		hookActions = Window->hook_actions(oBIND(&oGestureManagerImpl::OnAction, this, oBIND1));
	}

	else
	{
		#define oSAFE_UNHOOKE(_OBJ, _HookID) do \
		{	if (_HookID != oInvalid) \
			{	_OBJ->unhook_events(_HookID); \
				_HookID = oInvalid; \
			} \
		} while (false)

		#define oSAFE_UNHOOKA(_OBJ, _HookID) do \
		{	if (_HookID != oInvalid) \
				{	_OBJ->unhook_actions(_HookID); \
				_HookID = oInvalid; \
			} \
		} while (false)

		#define SAFE_UNHOOKE(_OBJ, _HookID) do \
		{	if (_HookID != oInvalid) \
			{	_OBJ->UnhookEvents(_HookID); \
				_HookID = oInvalid; \
			} \
		} while (false)

		#define SAFE_UNHOOKA(_OBJ, _HookID) do \
		{	if (_HookID != oInvalid) \
				{	_OBJ->UnhookActions(_HookID); \
				_HookID = oInvalid; \
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
	lock_guard<shared_mutex> lock(KinectMutex);

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
				lock_guard<shared_mutex> lock2(KinectMutex);
				
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
		lock_guard<shared_mutex> lock(KinectMutex);
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
	lock_guard<shared_mutex> lock(DeviceIconMutex);

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

bool oGestureManagerImpl::GDIDrawKinect(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)
{
	shared_lock<shared_mutex> lock(KinectMutex);

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
		const RECT rTarget = oWinRect(oGUIResolveRect(oRECT(oRECT::pos_size, int2(0,0), _ClientSize), VizDesc.Position, VizDesc.Size, VizDesc.Alignment, true));
			
		oGDIScopedSelect ScopedSelectBrush(hDC, hBoneBrush);
		oGDIScopedSelect ScopedSelectPen(hDC, hBonePen);

		oGDIDrawBox(hDC, oWinRect(rTarget.left, rTarget.top, rTarget.right + 1, rTarget.bottom + 1));
		oGDIScopedClipRegion SelectClipRegion(hDC, rTarget);
		oGDIDrawKinect(hDC, rTarget, FrameType, KinectDrawFlags, Kinect);

		// Draw boxes and some HUD info
		{
			oGDIScopedSelect SelFont(hDC, hFont);
			oGUI_FONT_DESC fd;
			oGDIGetFontDesc(hFont, &fd);

			oGUI_BONE_DESC Skeleton;
			int SkelIndex = 0;
			float VerticalOffset = (float)rTarget.top;
			while (Kinect->GetSkeletonByIndex(SkelIndex, &Skeleton))
			{
				if (AirKeyboard)
					AirKeyboard->VisitKeys(oBIND(oGDIDrawAirKey, hDC, oBINDREF(rTarget), oGDI_AIR_KEY_DRAW_BOX|oGDI_AIR_KEY_DRAW_KEY, oBIND1, oBIND2, oBINDREF(Skeleton)));

				oGUI_TEXT_DESC td;
				td.Position = float2((float)rTarget.left, VerticalOffset);
				td.Size = oWinRectSize(rTarget);
				td.Shadow = Black;
				const float4& h = Skeleton.Positions[oGUI_BONE_HIP_CENTER];
				mstring text;
				snprintf(text, "HIP: %.02f %.02f %.02f\n", h.x, h.y, h.z);
				oGDIDrawText(hDC, td, text);

				RECT rText = oGDICalcTextRect(hDC, text);
				VerticalOffset += oWinRectH(rText);

				if (!ComboMessage[SkelIndex].empty())
					oGDIDrawBoneText(hDC, rTarget, Skeleton.Positions[oGUI_BONE_HEAD], oGUI_ALIGNMENT_BOTTOM_CENTER, int2(0, -75), oGUI_ALIGNMENT_BOTTOM_CENTER, ComboMessage[SkelIndex]);

				SkelIndex++;
			}
		}
	}

	return !!Kinect;
}

void oGestureManagerImpl::GDIDrawNoKinect(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)
{
	HDC hDC = (HDC)_hDC;

	const RECT rTarget = oWinRect(oGUIResolveRect(oRECT(oRECT::pos_size, int2(0,0), _ClientSize), VizDesc.Position, VizDesc.Size, VizDesc.Alignment, true));

	{
		oGDIScopedSelect ScopedSelectBrush(hDC, hBlankBG);
		oGDIDrawBox(hDC, oWinRect(rTarget.left, rTarget.top, rTarget.right + 1, rTarget.bottom + 1));
	}

	oGDIScopedClipRegion SelectClipRegion(hDC, rTarget);
	oGDIScopedSelect ScopedSelectBrush(hDC, hBoneBrush);
	oGDIScopedSelect ScopedSelectPen(hDC, hBonePen);
	oGDIScopedSelect ScopedSelFont(hDC, hFont);

	oGUI_TEXT_DESC td;
	td.Position = float2(oWinRectPosition(rTarget));
	td.Size = oWinRectSize(rTarget);
	td.Shadow = Gray;
	td.Alignment = oGUI_ALIGNMENT_MIDDLE_CENTER;
	oGDIDrawText(hDC, td, NoKinectMessage);
}

void oGestureManagerImpl::GDIDrawKinectStatusIcon(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)
{
	if (DeviceVizDesc.hGestureDevice)
	{
		oRECT parent(oRECT::pos_size, int2(0,0), _ClientSize);
		oRECT r = oGUIResolveRect(parent, DeviceVizDesc.Position
			, oGDIGetIconSize((HICON)DeviceVizDesc.hGestureDevice), DeviceVizDesc.Alignment, true);

		HDC hDC = (HDC)_hDC;
		oVB(DrawIconEx(hDC, r.Min.x, r.Min.y, (HICON)DeviceVizDesc.hGestureDevice, r.size().x, r.size().y, 0, nullptr, DI_NORMAL));
	}
}

void oGestureManagerImpl::GDIDrawNotStatusIcon(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)
{
	if (DeviceVizDesc.hNotOverlay)
	{
		oRECT parent(oRECT::pos_size, int2(0,0), _ClientSize);
		oRECT r = oGUIResolveRect(parent, DeviceVizDesc.Position
			, oGDIGetIconSize((HICON)DeviceVizDesc.hNotOverlay), DeviceVizDesc.Alignment, true);

		HDC hDC = (HDC)_hDC;
		oVB(DrawIconEx(hDC, r.Min.x, r.Min.y, (HICON)DeviceVizDesc.hNotOverlay, r.size().x, r.size().y, 0, nullptr, DI_NORMAL));
	}
}

void oGestureManagerImpl::GDIDrawKinectStatus(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)
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

void oGestureManagerImpl::GDIDraw(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)
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
		return oInvalid;
	}

	int l = vsnprintf(ComboMessage[_SkeletonIndex], _Format, _Args);
	//TimerMessageVersion++; // don't respect this because it is separate from
													 // kinect status timer
	Window->start_timer(MAKE_TIMER_MSG(TimerMessageVersion, oGESTURE_TIMER_HEAD_MESSAGE, _SkeletonIndex), Desc.HeadMessageTimeoutMS);
	return l;
}

void oGestureManagerImpl::OnEvent(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_SIZED:
		{
			// Keep the font proportional to the size of the rectangle.
			ouro::display::info di = ouro::display::get_info(oWinGetDisplayId((HWND)_Event.hWindow));
			float2 Ratio = float2(VizDesc.Size) / float2(int2(di.mode.width, di.mode.height));
			float R = min(Ratio);
			oGUI_FONT_DESC fd;
			fd.PointSize = oInt(round(R * 50.0f));
			hFont = oGDICreateFont(fd);
			break;
		}

		case oGUI_TIMER:
		{
			int TimerVersion = oInvalid;
			oGESTURE_TIMER_EVENT GTE = oGESTURE_TIMER_HEAD_MESSAGE;
			int SkeletonIndex = oInvalid;
			DECODE_TIMER_MSG(_Event.AsTimer().Context, &TimerVersion, &GTE, &SkeletonIndex);
			
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

		case oGUI_INPUT_DEVICE_CHANGED:
		{
			OnDeviceChange(_Event);
			break;
		}

		default:
			break;
	}
}

void oGestureManagerImpl::OnAction(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_SKELETON_ACQUIRED:
		{
			sstring text;
			snprintf(text, "GMGR: Skeleton[%d] activated", _Action.DeviceID);
			AirKeyboard->AddSkeleton(_Action.DeviceID);
			break;
		}

		case oGUI_ACTION_SKELETON_LOST:
		{
			sstring text;
			snprintf(text, "GMGR: Skeleton[%d] deactivated", _Action.DeviceID);
			AirKeyboard->RemoveSkeleton(_Action.DeviceID);
			break;
		}

		case oGUI_ACTION_SKELETON:
		{
			oGUI_BONE_DESC Skeleton;
			oWinGetSkeletonDesc((HSKELETON)_Action.hSkeleton, &Skeleton);
			AirKeyboard->Update(Skeleton, _Action.TimestampMS);
			break;
		}

		case oGUI_ACTION_KEY_DOWN:
		case oGUI_ACTION_KEY_UP:
		{
			InputMapper->OnAction(_Action);
			break;
		}

		default:
			break;
	}
}

void oGestureManagerImpl::OnDeviceChange(const oGUI_EVENT_DESC& _Event)
{
	if (_Event.Type == oGUI_INPUT_DEVICE_SKELETON)
	{
		if (!Desc.GestureEnabled)
		{
			oTRACE("GMGR: ignoring (gesture sys disabled): Kinect %s", _Event.AsInputDevice().InstanceName);
			return;
		}

		//KinectIcon(_Event.AsInputDevice().Status);
		switch (_Event.AsInputDevice().Status)
		{
			case oGUI_INPUT_DEVICE_READY:
				AttachKinect(true, _Event.AsInputDevice().InstanceName);
				break;
			case oGUI_INPUT_DEVICE_NOT_CONNECTED:
				AttachKinect(false, _Event.AsInputDevice().InstanceName);
				break;
			case oGUI_INPUT_DEVICE_INITIALIZING:
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
