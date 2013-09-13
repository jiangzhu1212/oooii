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
#include <oKinect/oKinectGDI.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/Windows/oWinSkeleton.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oStream.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oWindow.h>
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oInputMapper.h>
#include <oStd/color.h>
#include "resource.h"

#include <oConcurrency/mutex.h>
using namespace oConcurrency;

typedef std::array<oStd::sstring, 2> head_messages_t;

#define oUSE_MEDIA_INPUT

#ifdef oUSE_MEDIA_INPUT

enum oMEDIA_INPUT
{
	// Main messages
	oMEDIA_INPUT_PLAY_PAUSE,
	oMEDIA_INPUT_TRACK_NEXT,
	oMEDIA_INPUT_TRACK_PREV,

	// Components
	oMEDIA_INPUT_TRACK_NEXT_1,
	oMEDIA_INPUT_TRACK_NEXT_2,
	oMEDIA_INPUT_TRACK_PREV_1,
	oMEDIA_INPUT_TRACK_PREV_2,

	oMEDIA_INPUT_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oMEDIA_INPUT)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oMEDIA_INPUT)
	oRTTI_ENUM_BEGIN_VALUES(oMEDIA_INPUT)
		oRTTI_VALUE(oMEDIA_INPUT_PLAY_PAUSE)
		oRTTI_VALUE(oMEDIA_INPUT_TRACK_NEXT)
		oRTTI_VALUE(oMEDIA_INPUT_TRACK_PREV)
		oRTTI_VALUE(oMEDIA_INPUT_TRACK_NEXT_1)
		oRTTI_VALUE(oMEDIA_INPUT_TRACK_NEXT_2)
		oRTTI_VALUE(oMEDIA_INPUT_TRACK_PREV_1)
		oRTTI_VALUE(oMEDIA_INPUT_TRACK_PREV_2)
	oRTTI_ENUM_END_VALUES(oMEDIA_INPUT)
	oRTTI_ENUM_VALIDATE_COUNT(oMEDIA_INPUT, oMEDIA_INPUT_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oMEDIA_INPUT)

#else

enum oSIMPLE_INPUT
{
	oSIMPLE_INPUT_WAVE,

	oSIMPLE_INPUT_WAVE_LEFT_1,
	oSIMPLE_INPUT_WAVE_LEFT_2,
	oSIMPLE_INPUT_WAVE_RIGHT_1,
	oSIMPLE_INPUT_WAVE_RIGHT_2,

	oSIMPLE_INPUT_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oSIMPLE_INPUT)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oSIMPLE_INPUT)
	oRTTI_ENUM_BEGIN_VALUES(oSIMPLE_INPUT)
		oRTTI_VALUE(oSIMPLE_INPUT_WAVE)
		oRTTI_VALUE(oSIMPLE_INPUT_WAVE_LEFT_1)
		oRTTI_VALUE(oSIMPLE_INPUT_WAVE_LEFT_2)
		oRTTI_VALUE(oSIMPLE_INPUT_WAVE_RIGHT_1)
		oRTTI_VALUE(oSIMPLE_INPUT_WAVE_RIGHT_2)
	oRTTI_ENUM_END_VALUES(oSIMPLE_INPUT)
	oRTTI_ENUM_VALIDATE_COUNT(oSIMPLE_INPUT, oSIMPLE_INPUT_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oSIMPLE_INPUT)

#endif

class oKinectTestApp
{
public:
	oKinectTestApp();

	void Run()
	{
		// @oooii-tony: TODO: There was a big refactor of how oWindow worked, so 
		// these shouldn't need to all be on the same thread anymore. Revisit moving
		// each window to its own thread once all integration is done.
		while (1)
		{
			int count = 0;
			oFOR(auto& w, KinectWindows)
			{
				if (w.Window && w.Running)
				{
					w.Window->FlushMessages();
					count++;

					OnPaint((HWND)w.Window->GetNativeHandle(), w.Window->GetClientSize(), w.hFont, w.Kinect, w.ComboMessage);
				}
			}

			if (!count)
				break;
		}
	}

private:
	static const int kCoordSpace = 90;

	struct oKinectWindow
	{
		oKinectWindow()
			: Playing(true)
			, Running(true)
		{}

		oKinectWindow(oKinectWindow&& _That) { operator=(std::move(_That)); }
		oKinectWindow& operator=(oKinectWindow&& _That)
		{
			if (this != &_That)
			{
				Window = std::move(_That.Window);
				Kinect = std::move(_That.Kinect);
				InputMapper = std::move(_That.InputMapper);
				LastInput = std::move(_That.LastInput);
				ComboMessage = std::move(_That.ComboMessage);
				Playing = std::move(_That.Playing);
				hFont = std::move(_That.hFont);
			}

			return *this;
		}

		oStd::ref<oWindow> Window;
		oStd::ref<threadsafe oKinect> Kinect;
		oStd::ref<threadsafe oInputMapper> InputMapper;
		
		#ifdef oUSE_MEDIA_INPUT
			oMEDIA_INPUT LastInput;
		#else	
			oSIMPLE_INPUT LastInput;
		#endif

		head_messages_t ComboMessage; // for each tracked skeleton

		bool Playing;
		bool Running;

		oGDIScopedObject<HFONT> hFont;
	private:
		oKinectWindow(const oKinectWindow&);
		const oKinectWindow& operator=(const oKinectWindow&);
	};

	std::vector<oKinectWindow> KinectWindows;

	oGDIScopedObject<HPEN> hKinectPen;
	oGDIScopedObject<HBRUSH> hKinectBrush;

	oStd::ref<threadsafe oStreamMonitor> StreamMonitor;
	oStd::ref<threadsafe oAirKeyboard> AirKeyboard;

	bool Ready;

	void UpdateStatusBar(threadsafe oWindow* _pWindow, const oGUI_BONE_DESC& _Skeleton, const char* _GestureName);
	void UpdateStatusBar(threadsafe oWindow* _pWindow, const char* _GestureName);

	void MainEventHook(const oGUI_EVENT_DESC& _Event, int _Index);
	void MainActionHook(const oGUI_ACTION_DESC& _Action, int _Index);

	// @oooii-tony: Maybe each window should get its own air keyboard / input?
	void BroadcastActions(const oGUI_ACTION_DESC& _Action)
	{
		// pass through to oWindows.
		oFOR(auto& w, KinectWindows)
			if (w.Window)
			w.Window->Trigger(_Action);
	}

	void OnFileChange(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI);

	void OnPaint(HWND _hWnd
		, const int2& _ClientSize
		, HFONT hFont
		, threadsafe oKinect* _pKinect
		, const head_messages_t& _HeadMessages);
};

oKinectTestApp::oKinectTestApp()
	: hKinectPen(oGDICreatePen(oStd::OOOiiGreen, 2))
	, hKinectBrush(oGDICreateBrush(oStd::White))
	, Ready(false)
{
	int nKinects = oKinectGetCount();
	if (nKinects && nKinects != oInvalid)
	{
		oWINDOW_INIT init;

		// hide while everything is initialized
		init.hIcon = (oGUI_ICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, 0);
		init.ClientDragToMove = true;
		init.Shape.State = oGUI_WINDOW_HIDDEN;
		init.Shape.Style = oGUI_WINDOW_SIZABLE_WITH_STATUSBAR;
		init.Shape.ClientSize = int2(640, 480);

		KinectWindows.resize(nKinects);
		for (int i = 0; i < nKinects; i++)
		{
			auto& w = KinectWindows[i];
			init.Title = "New Window";

			init.EventHook = oBIND(&oKinectTestApp::MainEventHook, this, oBIND1, i);
			init.ActionHook = oBIND(&oKinectTestApp::MainActionHook, this, oBIND1, i);

			oVERIFY(oWindowCreate(init, &w.Window));

			int SectionWidths[4] = { kCoordSpace, kCoordSpace, kCoordSpace, oInvalid };
			w.Window->SetNumStatusSections(SectionWidths);

			oGUI_BONE_DESC s;
			UpdateStatusBar(w.Window, s, "<Gesture>");
		}

		oStd::ref<threadsafe oKinect> Kinect;
		oKINECT_DESC kd;

		kd.PitchDegrees = oDEFAULT;

		while (kd.Index < nKinects)
		{
			if (!oKinectCreate(kd, KinectWindows[kd.Index].Window, &Kinect))
			{
				if (oErrorGetLast() == std::errc::no_such_device)
					break;

				oTRACE("Kinect[%d] failed: %s", kd.Index, oErrorGetLastString());
			}

			if (Kinect)
			{
				KinectWindows[kd.Index].Kinect = Kinect;

				oKINECT_DESC kd;
				Kinect->GetDesc(&kd);

				oStd::mstring Name;
				snprintf(Name, "Kinect[%d] ID=%s", kd.Index, kd.ID.c_str());

				KinectWindows[kd.Index].Window->SetTitle(Name);
			}
			else
				KinectWindows[kd.Index].Window = nullptr;

			kd.Index++;
		}

		oFOR(auto& w, KinectWindows)
			if (w.Window)
				w.Window->Show();
	}
	
	else if (nKinects == 0)
		oMsgBox(oMSGBOX_DESC(oMSGBOX_INFO, "oKinect Test App"), "No Kinects attached.");

	else if (nKinects == oInvalid)
		oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, "oKinect Test App")
			, "This application was not built with Kinect support. It will need to be recompiled against the Kinect SDK to support Kinect features.");

	// Register input handlers

	oStd::uri_string dev_uri;
	oVERIFY(oSystemGetURI(dev_uri, oSYSPATH_DEV));
	
	{
		oVERIFY(oAirKeyboardCreate(&AirKeyboard));
		AirKeyboard->HookActions(oBIND(&oKinectTestApp::BroadcastActions, this, oBIND1));
		oStd::uri_string AirKB = dev_uri;
		oStrAppendf(AirKB, "Ouroboros/Source/oKinectTestApp/AirKeyboards.xml");
		OnFileChange(oSTREAM_ACCESSIBLE, AirKB);
	}

	{
		oFOR(auto& w, KinectWindows)
		{
			oVERIFY(oInputMapperCreate(&w.InputMapper));
			w.InputMapper->HookActions(oBIND(&oKinectTestApp::BroadcastActions, this, oBIND1));
		}

		oStd::uri_string Inputs = dev_uri;
		oStrAppendf(Inputs, "Ouroboros/Source/oKinectTestApp/Inputs.xml");
		OnFileChange(oSTREAM_ACCESSIBLE, Inputs);
	}

	{
		oSTREAM_MONITOR_DESC smd;
		smd.Monitor = dev_uri;
		oStrAppendf(smd.Monitor, "Ouroboros/Source/oKinectTestApp/*.xml");
		smd.TraceEvents = false;
		smd.WatchSubtree = false;
		oVERIFY(oStreamMonitorCreate(smd, oBIND(&oKinectTestApp::OnFileChange, this, oBIND1, oBIND2), &StreamMonitor));
	}
	
	Ready = true;
}

void oKinectTestApp::UpdateStatusBar(threadsafe oWindow* _pWindow, const oGUI_BONE_DESC& _Skeleton, const char* _GestureName)
{
	// this doesn't differentiate between multiple skeletons yet...
	#define kCoordFormat "%c: %.02f %.02f %.02f"
	const float4& H = _Skeleton.Positions[oGUI_BONE_HEAD];
	const float4& L = _Skeleton.Positions[oGUI_BONE_HAND_LEFT];
	const float4& R = _Skeleton.Positions[oGUI_BONE_HAND_RIGHT];
	_pWindow->SetStatusText(0, kCoordFormat, 'H', H.x, H.y, H.z);
	_pWindow->SetStatusText(1, kCoordFormat, 'L', L.x, L.y, L.z);
	_pWindow->SetStatusText(2, kCoordFormat, 'R', R.x, R.y, R.z);
	UpdateStatusBar(_pWindow, _GestureName);
}

void oKinectTestApp::UpdateStatusBar(threadsafe oWindow* _pWindow, const char* _GestureName)
{
	if (oSTRVALID(_GestureName))
		_pWindow->SetStatusText(3, "%c: %s", 'G', _GestureName);
}

void oKinectTestApp::OnPaint(HWND _hWnd
	, const int2& _ClientSize
	, HFONT _hFont
	, threadsafe oKinect* _pKinect
	, const head_messages_t& _HeadMessages)
{
	const RECT rTarget = oWinRectWH(int2(0,0), _ClientSize);
	oGDIScopedOffscreen hDC(_hWnd);
	oGDIScopedSelect SelectBrush(hDC, hKinectBrush);
	oGDIScopedSelect SelectPen(hDC, hKinectPen);
	oGDIDrawKinect(hDC, rTarget, oKINECT_FRAME_COLOR, oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_CLIPPING, _pKinect);

	// Draw boxes and some HUD info
	{
		oGDIScopedSelect SelFont(hDC, _hFont);
		oGUI_FONT_DESC fd;
		oGDIGetFontDesc(_hFont, &fd);

		oGUI_BONE_DESC Skeleton;
		int SkelIndex = 0;
		float VerticalOffset = 0.0f;
		while (_pKinect->GetSkeletonByIndex(SkelIndex, &Skeleton))
		{
			AirKeyboard->VisitKeys(oBIND(oGDIDrawAirKey, (HDC)hDC, oBINDREF(rTarget), oGDI_AIR_KEY_DRAW_BOX|oGDI_AIR_KEY_DRAW_KEY, oBIND1, oBIND2, oBINDREF(Skeleton)));

			oGUI_TEXT_DESC td;
			td.Position = float2(0.0f, VerticalOffset);
			td.Size = _ClientSize;
			td.Shadow = oStd::Black;
			const float4& h = Skeleton.Positions[oGUI_BONE_HIP_CENTER];
			const float4& hr = Skeleton.Positions[oGUI_BONE_ANKLE_RIGHT];
			oStd::mstring text;
			snprintf(text, "HIP: %.02f %.02f %.02f\nRANKLE: %.02f %.02f %.02f", h.x, h.y, h.z, hr.x, hr.y, hr.z);
			oGDIDrawText(hDC, td, text);

			RECT rText = oGDICalcTextRect(hDC, text);
			VerticalOffset += oWinRectH(rText);

			if (!_HeadMessages[SkelIndex].empty())
				oGDIDrawBoneText(hDC, rTarget, Skeleton.Positions[oGUI_BONE_HEAD], oGUI_ALIGNMENT_BOTTOM_CENTER, int2(0, -75), oGUI_ALIGNMENT_BOTTOM_CENTER, _HeadMessages[SkelIndex]);

			SkelIndex++;
		}
	}
}

void oKinectTestApp::MainEventHook(const oGUI_EVENT_DESC& _Event, int _Index)
{
	if (!Ready)
		return;

	oKinectWindow& kw = KinectWindows[_Index];
	if (!kw.Window)
		return;

	switch (_Event.Type)
	{
		case oGUI_SIZED:
		{
			int DIndex = kw.Window->GetDisplayIndex();
			oDISPLAY_DESC dd;
			oDisplayEnum(DIndex, &dd);
			float2 Ratio = float2(_Event.AsShape().Shape.ClientSize) / float2(dd.Mode.Size);
			float R = max(Ratio);
			oGUI_FONT_DESC fd;
			fd.PointSize = oInt(round(R * 35.0f));
			kw.hFont = oGDICreateFont(fd);
			break;
		}

		case oGUI_INPUT_DEVICE_CHANGED:
		{
			oTRACE("%s %s status change, now: %s", oStd::as_string(_Event.Type), _Event.AsInputDevice().InstanceName, oStd::as_string(_Event.AsInputDevice().Status));
			break;
		}

		case oGUI_TIMER:
		{
			kw.ComboMessage[_Event.AsTimer().Context].clear();
			break;
		}

		case oGUI_CLOSING:
			kw.Running = false;
			break;

		default:
			break;
	}
}

void oKinectTestApp::MainActionHook(const oGUI_ACTION_DESC& _Action, int _Index)
{
	if (!Ready)
		return;

	oKinectWindow& kw = KinectWindows[_Index];
	if (!kw.Window)
		return;

	switch (_Action.Action)
	{
		case oGUI_ACTION_SKELETON_ACQUIRED:
		{
			oStd::sstring text;
			snprintf(text, "Skeleton[%d] activated", _Action.DeviceID);
			UpdateStatusBar(kw.Window, text);
			AirKeyboard->AddSkeleton(_Action.DeviceID);
			break;
		}

		case oGUI_ACTION_SKELETON_LOST:
		{
			oStd::sstring text;
			snprintf(text, "Skeleton[%d] deactivated", _Action.DeviceID);
			UpdateStatusBar(kw.Window, text);
			AirKeyboard->RemoveSkeleton(_Action.DeviceID);
			break;
		}

		case oGUI_ACTION_CONTROL_ACTIVATED:
		{
			#ifdef oUSE_MEDIA_INPUT
				switch (_Action.ActionCode)
				{
					case oMEDIA_INPUT_PLAY_PAUSE:
						kw.Playing = !kw.Playing;
						UpdateStatusBar(kw.Window, kw.Playing ? "Playing" : "Paused");
						kw.ComboMessage[0] = kw.Playing ? "Playing" : "Paused";
						kw.Window->SetTimer(0, 2000);
						break;
					case oMEDIA_INPUT_TRACK_NEXT:
						UpdateStatusBar(kw.Window, "Next");
						kw.ComboMessage[0] = "Next";
						kw.Window->SetTimer(0, 2000);
						break;
					case oMEDIA_INPUT_TRACK_PREV:
						UpdateStatusBar(kw.Window, "Prev");
						kw.ComboMessage[0] = "Prev";
						kw.Window->SetTimer(0, 2000);
						break;
				}
			#else
				switch (_Action.ActionCode)
				{
					case oSIMPLE_INPUT_WAVE:
						kw.ComboMessage[0] = "Wave";
						kw.Window->SetTimer(0, 2000);
						break;
					default:
						break;
				}
			#endif

			break;
		}

		case oGUI_ACTION_KEY_DOWN:
		case oGUI_ACTION_KEY_UP:
		{
 			kw.InputMapper->OnAction(_Action);
			oTRACE("%s: %s", oStd::as_string(_Action.Action), oStd::as_string(_Action.Key));
			break;
		}

		case oGUI_ACTION_SKELETON:
		{
			oGUI_BONE_DESC Skeleton;
			oWinGetSkeletonDesc((HSKELETON)_Action.hSkeleton, &Skeleton);
			AirKeyboard->Update(Skeleton, _Action.TimestampMS);

			// this doesn't differentiate between multiple skeletons yet...
			UpdateStatusBar(kw.Window, Skeleton, nullptr);
			break;
		}

		default:
			break;
	}
}

void oKinectTestApp::OnFileChange(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI)
{
	if (_Event == oSTREAM_ACCESSIBLE)
	{
		if (strstr(_ChangedURI, "Inputs.xml"))
		{
			try 
			{
				oStd::ref<threadsafe oInputSet> InputSet;
				std::shared_ptr<oStd::xml> XML = oXMLLoad(_ChangedURI);
				if (oParseInputSetList(*XML
					, XML->first_child(XML->root(), "oInputSetList")
					#ifdef oUSE_MEDIA_INPUT
						, oRTTI_OF(oMEDIA_INPUT)
					#else
						, oRTTI_OF(oSIMPLE_INPUT)
					#endif
					, &InputSet))
				{
					oFOR(auto& w, KinectWindows)
						w.InputMapper->SetInputSet(InputSet);
					oTRACE("%s reloaded successfully.", _ChangedURI.c_str());
				}

				else
					oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, "oKinect Test App"), "Failed to parse input set list file %s.\n%s", _ChangedURI.c_str(), oErrorGetLastString());
			}

			catch (std::exception& e)
			{
				oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, "oKinect Test App"), "Failed to reload %s.\n%s", _ChangedURI.c_str(), e.what());
			}
		}

		else if (strstr(_ChangedURI, "AirKeyboards.xml"))
		{
			AirKeyboard->SetKeySet(nullptr);

			try
			{
				oStd::ref<threadsafe oAirKeySet> KeySet;
				std::shared_ptr<oStd::xml> XML = oXMLLoad(_ChangedURI);
				if (oParseAirKeySetsList(*XML
					, XML->first_child(XML->root(), "oAirKeySetList")
					#ifdef oUSE_MEDIA_INPUT
						, "MediaKeys"
					#else
						, "WaveBoxes"
					#endif
					, &KeySet))
				{
					AirKeyboard->SetKeySet(KeySet);
					oTRACE("%s reloaded successfully.", _ChangedURI.c_str());
				}

			}
			catch (std::exception& e)
			{
				oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, "oKinect Test App"), "Failed to reload %s.\n%s", _ChangedURI.c_str(), e.what());
			}
		}
	}
}

oMAINA()
{
	oKinectTestApp App;
	App.Run();
	return 0;
}
