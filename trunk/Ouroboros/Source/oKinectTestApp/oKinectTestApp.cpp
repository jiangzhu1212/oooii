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
#include <oGUI/Windows/oGDI.h>
#include <oCore/windows/win_skeleton.h>
#include <oGUI/Windows/oWinRect.h>
#include <oPlatform/oStream.h>
#include <oPlatform/oStreamUtil.h>
#include <oGUI/msgbox.h>
#include <oGUI/window.h>
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oInputMapper.h>
#include <oBase/color.h>
#include "resource.h"

#include <oConcurrency/mutex.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace ouro;
using namespace oConcurrency;

typedef std::array<sstring, 2> head_messages_t;

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
		// @tony: TODO: There was a big refactor of how oWindow worked, so 
		// these shouldn't need to all be on the same thread anymore. Revisit moving
		// each window to its own thread once all integration is done.
		while (1)
		{
			int count = 0;
			oFOR(auto& w, KinectWindows)
			{
				if (w.Window && w.Running)
				{
					w.Window->flush_messages();
					count++;

					OnPaint((HWND)w.Window->native_handle(), w.Window->client_size(), w.hFont, w.Kinect, w.ComboMessage);
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

		std::shared_ptr<window> Window;
		intrusive_ptr<threadsafe oKinect> Kinect;
		intrusive_ptr<threadsafe oInputMapper> InputMapper;
		
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

	intrusive_ptr<threadsafe oStreamMonitor> StreamMonitor;
	intrusive_ptr<threadsafe oAirKeyboard> AirKeyboard;

	bool Ready;

	void UpdateStatusBar(window* _pWindow, const input::tracking_skeleton& _Skeleton, const char* _GestureName);
	void UpdateStatusBar(window* _pWindow, const char* _GestureName);

	void MainEventHook(const window::basic_event& _Event, int _Index);
	void MainActionHook(const input::action& _Action, int _Index);

	// @tony: Maybe each window should get its own air keyboard / input?
	void BroadcastActions(const input::action& _Action)
	{
		// pass through to oWindows.
		oFOR(auto& w, KinectWindows)
			if (w.Window)
			w.Window->trigger(_Action);
	}

	void OnFileChange(oSTREAM_EVENT _Event, const uri_string& _ChangedURI);

	void OnPaint(HWND _hWnd
		, const int2& _ClientSize
		, HFONT hFont
		, threadsafe oKinect* _pKinect
		, const head_messages_t& _HeadMessages);
};

oKinectTestApp::oKinectTestApp()
	: hKinectPen(oGDICreatePen(Lime, 2))
	, hKinectBrush(oGDICreateBrush(White))
	, Ready(false)
{
	int nKinects = oKinectGetCount();
	if (nKinects && nKinects != oInvalid)
	{
		window::init winit;

		// hide while everything is initialized
		winit.icon = (icon_handle)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, 0);
		winit.client_drag_to_move = true;
		winit.shape.state = window_state::hidden;
		winit.shape.style = window_style::sizable_with_statusbar;
		winit.shape.client_size = int2(640, 480);

		KinectWindows.resize(nKinects);
		for (int i = 0; i < nKinects; i++)
		{
			auto& w = KinectWindows[i];
			winit.title = "New Window";

			winit.on_event = std::bind(&oKinectTestApp::MainEventHook, this, std::placeholders::_1, i);
			winit.on_action = std::bind(&oKinectTestApp::MainActionHook, this, std::placeholders::_1, i);

			w.Window = window::make(winit);

			int SectionWidths[4] = { kCoordSpace, kCoordSpace, kCoordSpace, oInvalid };
			w.Window->set_num_status_sections(SectionWidths);

			input::tracking_skeleton s;
			UpdateStatusBar(w.Window.get(), s, "<Gesture>");
		}

		intrusive_ptr<threadsafe oKinect> Kinect;
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

				mstring Name;
				snprintf(Name, "Kinect[%d] ID=%s", kd.Index, kd.ID.c_str());

				KinectWindows[kd.Index].Window->set_title(Name);
			}
			else
				KinectWindows[kd.Index].Window = nullptr;

			kd.Index++;
		}

		oFOR(auto& w, KinectWindows)
			if (w.Window)
				w.Window->show();
	}
	
	else if (nKinects == 0)
		msgbox(msg_type::info, nullptr, "oKinect Test App", "No Kinects attached.");

	else if (nKinects == oInvalid)
		msgbox(msg_type::error, nullptr, "oKinect Test App"
			, "This application was not built with Kinect support. It will need to be recompiled against the Kinect SDK to support Kinect features.");

	// Register input handlers

	uri_string dev_uri(filesystem::dev_path());
	
	{
		oVERIFY(oAirKeyboardCreate(&AirKeyboard));
		AirKeyboard->HookActions(std::bind(&oKinectTestApp::BroadcastActions, this, std::placeholders::_1));
		uri_string AirKB = dev_uri;
		sncatf(AirKB, "Ouroboros/Source/oKinectTestApp/AirKeyboards.xml");
		OnFileChange(oSTREAM_ACCESSIBLE, AirKB);
	}

	{
		oFOR(auto& w, KinectWindows)
		{
			oVERIFY(oInputMapperCreate(&w.InputMapper));
			w.InputMapper->HookActions(std::bind(&oKinectTestApp::BroadcastActions, this, std::placeholders::_1));
		}

		uri_string Inputs = dev_uri;
		sncatf(Inputs, "Ouroboros/Source/oKinectTestApp/Inputs.xml");
		OnFileChange(oSTREAM_ACCESSIBLE, Inputs);
	}

	{
		oSTREAM_MONITOR_DESC smd;
		smd.Monitor = dev_uri;
		sncatf(smd.Monitor, "Ouroboros/Source/oKinectTestApp/*.xml");
		smd.TraceEvents = false;
		smd.WatchSubtree = false;
		oVERIFY(oStreamMonitorCreate(smd, std::bind(&oKinectTestApp::OnFileChange, this, std::placeholders::_1, std::placeholders::_2), &StreamMonitor));
	}
	
	Ready = true;
}

void oKinectTestApp::UpdateStatusBar(window* _pWindow, const input::tracking_skeleton& _Skeleton, const char* _GestureName)
{
	// this doesn't differentiate between multiple skeletons yet...
	#define kCoordFormat "%c: %.02f %.02f %.02f"
	const float4& H = _Skeleton.positions[input::head];
	const float4& L = _Skeleton.positions[input::hand_left];
	const float4& R = _Skeleton.positions[input::hand_right];
	_pWindow->set_status_text(0, kCoordFormat, 'H', H.x, H.y, H.z);
	_pWindow->set_status_text(1, kCoordFormat, 'L', L.x, L.y, L.z);
	_pWindow->set_status_text(2, kCoordFormat, 'R', R.x, R.y, R.z);
	UpdateStatusBar(_pWindow, _GestureName);
}

void oKinectTestApp::UpdateStatusBar(window* _pWindow, const char* _GestureName)
{
	if (oSTRVALID(_GestureName))
		_pWindow->set_status_text(3, "%c: %s", 'G', _GestureName);
}

void oKinectTestApp::OnPaint(HWND _hWnd
	, const int2& _ClientSize
	, HFONT _hFont
	, threadsafe oKinect* _pKinect
	, const head_messages_t& _HeadMessages)
{
	const RECT rTarget = oWinRectWH(int2(0,0), _ClientSize);
	oGDIScopedOffscreen hDC(_hWnd);
	oGDIScopedSelect ScopedSelectBrush(hDC, hKinectBrush);
	oGDIScopedSelect ScopedSelectPen(hDC, hKinectPen);
	oGDIDrawKinect(hDC, rTarget, oKINECT_FRAME_COLOR, oGDI_KINECT_DRAW_SKELETON|oGDI_KINECT_DRAW_CLIPPING, _pKinect);

	// Draw boxes and some HUD info
	{
		oGDIScopedSelect SelFont(hDC, _hFont);
		ouro::font_info fd;
		oGDIGetFontDesc(_hFont, &fd);

		input::tracking_skeleton Skeleton;
		int SkelIndex = 0;
		float VerticalOffset = 0.0f;
		while (_pKinect->GetSkeletonByIndex(SkelIndex, &Skeleton))
		{
			AirKeyboard->VisitKeys(std::bind(oGDIDrawAirKey, (HDC)hDC, std::ref(rTarget), oGDI_AIR_KEY_DRAW_BOX|oGDI_AIR_KEY_DRAW_KEY, std::placeholders::_1, std::placeholders::_2, std::ref(Skeleton)));

			ouro::text_info td;
			td.position = float2(0.0f, VerticalOffset);
			td.size = _ClientSize;
			td.shadow = Black;
			const float4& h = Skeleton.positions[input::hip_center];
			const float4& hr = Skeleton.positions[input::ankle_right];
			mstring text;
			snprintf(text, "HIP: %.02f %.02f %.02f\nRANKLE: %.02f %.02f %.02f", h.x, h.y, h.z, hr.x, hr.y, hr.z);
			oGDIDrawText(hDC, td, text);

			RECT rText = oGDICalcTextRect(hDC, text);
			VerticalOffset += oWinRectH(rText);

			if (!_HeadMessages[SkelIndex].empty())
				oGDIDrawBoneText(hDC, rTarget, Skeleton.positions[input::head], alignment::bottom_center, int2(0, -75), alignment::bottom_center, _HeadMessages[SkelIndex]);

			SkelIndex++;
		}
	}
}

void oKinectTestApp::MainEventHook(const window::basic_event& _Event, int _Index)
{
	if (!Ready)
		return;

	oKinectWindow& kw = KinectWindows[_Index];
	if (!kw.Window)
		return;

	switch (_Event.type)
	{
		case event_type::sized:
		{
			display::info di = display::get_info(kw.Window->display_id());
			float2 Ratio = float2(_Event.as_shape().shape.client_size) / float2(int2(di.mode.width, di.mode.height));
			float R = max(Ratio);
			ouro::font_info fd;
			fd.point_size = oInt(round(R * 35.0f));
			kw.hFont = oGDICreateFont(fd);
			break;
		}

		case event_type::input_device_changed:
		{
			oTRACE("%s %s status change, now: %s", as_string(_Event.type), _Event.as_input_device().instance_name, as_string(_Event.as_input_device().status));
			break;
		}

		case event_type::timer:
		{
			kw.ComboMessage[_Event.as_timer().context].clear();
			break;
		}

		case event_type::closing:
			kw.Running = false;
			break;

		default:
			break;
	}
}

void oKinectTestApp::MainActionHook(const input::action& _Action, int _Index)
{
	if (!Ready)
		return;

	oKinectWindow& kw = KinectWindows[_Index];
	if (!kw.Window)
		return;

	switch (_Action.action_type)
	{
		case input::skeleton_acquired:
		{
			sstring text;
			snprintf(text, "Skeleton[%d] activated", _Action.device_id);
			UpdateStatusBar(kw.Window.get(), text);
			AirKeyboard->AddSkeleton(_Action.device_id);
			break;
		}

		case input::skeleton_lost:
		{
			sstring text;
			snprintf(text, "Skeleton[%d] deactivated", _Action.device_id);
			UpdateStatusBar(kw.Window.get(), text);
			AirKeyboard->RemoveSkeleton(_Action.device_id);
			break;
		}

		case input::control_activated:
		{
			#ifdef oUSE_MEDIA_INPUT
				switch (_Action.action_code)
				{
					case oMEDIA_INPUT_PLAY_PAUSE:
						kw.Playing = !kw.Playing;
						UpdateStatusBar(kw.Window.get(), kw.Playing ? "Playing" : "Paused");
						kw.ComboMessage[0] = kw.Playing ? "Playing" : "Paused";
						kw.Window->start_timer(0, 2000);
						break;
					case oMEDIA_INPUT_TRACK_NEXT:
						UpdateStatusBar(kw.Window.get(), "Next");
						kw.ComboMessage[0] = "Next";
						kw.Window->start_timer(0, 2000);
						break;
					case oMEDIA_INPUT_TRACK_PREV:
						UpdateStatusBar(kw.Window.get(), "Prev");
						kw.ComboMessage[0] = "Prev";
						kw.Window->start_timer(0, 2000);
						break;
				}
			#else
				switch (_Action.action_code)
				{
					case oSIMPLE_INPUT_WAVE:
						kw.ComboMessage[0] = "Wave";
						kw.Window->start_timer(0, 2000);
						break;
					default:
						break;
				}
			#endif

			break;
		}

		case input::key_down:
		case input::key_up:
		{
 			kw.InputMapper->OnAction(_Action);
			oTRACE("%s: %s", as_string(_Action.action_type), as_string(_Action.key));
			break;
		}

		case input::skeleton:
		{
			windows::skeleton::bone_info Skeleton;
			windows::skeleton::get_info((windows::skeleton::handle)_Action.skeleton, &Skeleton);
			
			input::tracking_skeleton skel;
			skel.source_id = Skeleton.source_id;
			skel.clipping = *(input::tracking_clipping*)&Skeleton.clipping;
			std::copy(Skeleton.positions.begin(), Skeleton.positions.begin() + skel.positions.size(), skel.positions.begin());
			
			AirKeyboard->Update(skel, _Action.timestamp_ms);

			// this doesn't differentiate between multiple skeletons yet...
			UpdateStatusBar(kw.Window.get(), skel, nullptr);
			break;
		}

		default:
			break;
	}
}

void oKinectTestApp::OnFileChange(oSTREAM_EVENT _Event, const uri_string& _ChangedURI)
{
	if (_Event == oSTREAM_ACCESSIBLE)
	{
		if (strstr(_ChangedURI, "Inputs.xml"))
		{
			try 
			{
				intrusive_ptr<threadsafe oInputSet> InputSet;
				std::shared_ptr<xml> XML = oXMLLoad(_ChangedURI);
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
					msgbox(msg_type::error, nullptr, "oKinect Test App", "Failed to parse input set list file %s.\n%s", _ChangedURI.c_str(), oErrorGetLastString());
			}

			catch (std::exception& e)
			{
				msgbox(msg_type::error, nullptr, "oKinect Test App", "Failed to reload %s.\n%s", _ChangedURI.c_str(), e.what());
			}
		}

		else if (strstr(_ChangedURI, "AirKeyboards.xml"))
		{
			AirKeyboard->SetKeySet(nullptr);

			try
			{
				intrusive_ptr<threadsafe oAirKeySet> KeySet;
				std::shared_ptr<xml> XML = oXMLLoad(_ChangedURI);
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
				msgbox(msg_type::error, nullptr, "oKinect Test App", "Failed to reload %s.\n%s", _ChangedURI.c_str(), e.what());
			}
		}
	}
}

int main(int argc, const char* argv[])
{
	oKinectTestApp App;
	App.Run();
	return 0;
}
