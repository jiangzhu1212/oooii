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

#include <oBasis/oRTTI.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/oGUIMenu.h>
#include <oGUI/msgbox.h>
#include <oPlatform/oStream.h>
#include <oGUI/window.h>
#include "resource.h"

#include <oCore/filesystem_monitor.h>

#include "../about_ouroboros.h"

using namespace ouro;

enum oWMENU
{
	oWMENU_FILE,
	oWMENU_EDIT,
	oWMENU_VIEW,
	oWMENU_VIEW_STYLE,
	oWMENU_VIEW_STATE,
	oWMENU_HELP,
	oWMENU_COUNT,
	oWMENU_TOPLEVEL,
};

struct oWMENU_HIER
{
	oWMENU Parent; // use oWMENU_TOPLEVEL for root parent menu
	oWMENU Menu;
	const char* Name;
};

static oWMENU_HIER sMenuHier[] = 
{
	{ oWMENU_TOPLEVEL, oWMENU_FILE, "&File" },
	{ oWMENU_TOPLEVEL, oWMENU_EDIT, "&Edit" },
	{ oWMENU_TOPLEVEL, oWMENU_VIEW, "&View" },
	{ oWMENU_VIEW, oWMENU_VIEW_STYLE, "Border Style" },
	{ oWMENU_VIEW, oWMENU_VIEW_STATE, "&Window State" },
	{ oWMENU_TOPLEVEL, oWMENU_HELP, "&Help" },
};
static_assert(oCOUNTOF(sMenuHier) == oWMENU_COUNT, "array mismatch");

enum oWMI // menuitems
{
	oWMI_FILE_EXIT,

	oWMI_VIEW_STYLE_FIRST,
	oWMI_VIEW_STYLE_LAST = oWMI_VIEW_STYLE_FIRST + ouro::window_style::count - 1,

	oWMI_VIEW_STATE_FIRST,
	oWMI_VIEW_STATE_LAST = oWMI_VIEW_STATE_FIRST + ouro::window_state::count - 1,

	oWMI_HELP_ABOUT,
};

enum oWHK // hotkeys
{
	oWHK_DEFAULT_STYLE,
	oWHK_TOGGLE_FULLSCREEN,
};

oGUI_HOTKEY_DESC_NO_CTOR HotKeys[] =
{
	// reset style
	{ ouro::input_key::f3, oWHK_DEFAULT_STYLE, false, false, false },
	{ ouro::input_key::f11, oWHK_TOGGLE_FULLSCREEN, false, false, false },
};

struct oWindowTestAppPulseContext
{
	oWindowTestAppPulseContext() : Count(0) {}
	unsigned int Count;
};

enum oWCTL // controls
{
	oWCTL_EASY_BUTTON,
};

class oWindowTestApp
{
public:
	oWindowTestApp();

	void Run();

private:
	std::shared_ptr<window> Window;
	std::shared_ptr<ouro::about> About;
	ouro::menu_handle Menus[oWMENU_COUNT];
	oWindowTestAppPulseContext PulseContext;
	oGUIMenuEnumRadioListHandler MERL; 
	ouro::window_state::value PreFullscreenState;
	bool Running;

	// This gets deleted by parent window automatically.
	HWND hButton;

	std::shared_ptr<filesystem::monitor> DirWatcher;

private:
	void ActionHook(const oGUI_ACTION_DESC& _Action);
	void EventHook(const oGUI_EVENT_DESC& _Event);
	bool CreateMenu(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	bool CreateControls(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	void CheckState(ouro::window_state::value _State);
	void CheckStyle(ouro::window_style::value _Style);
	void OnDirectoryEvent(filesystem::file_event::value _Event, const path& _Path);
};

oWindowTestApp::oWindowTestApp()
	: Running(true)
	, PreFullscreenState(ouro::window_state::hidden)
	, hButton(nullptr)
{
	{
		filesystem::monitor::info i;
		i.accessibility_poll_rate_ms = 2000;
		i.accessibility_timeout_ms = 5000;
		DirWatcher = filesystem::monitor::make(i, std::bind(&oWindowTestApp::OnDirectoryEvent, this, std::placeholders::_1, std::placeholders::_2));
	}

	{
		path watched = filesystem::desktop_path() / "test/";
		try { DirWatcher->watch(watched, oKB(64), true); }
		catch (std::exception& e) { oTRACEA("Cannot watch %s: %s", watched.c_str(), e.what()); }
	}

	{
		window::init i;
		i.title = "oWindowTestApp";
		i.icon = (ouro::icon_handle)oGDILoadIcon(IDI_APPICON);
		i.action_hook = std::bind(&oWindowTestApp::ActionHook, this, oBIND1);
		i.event_hook = std::bind(&oWindowTestApp::EventHook, this, oBIND1);
		i.shape.client_size = int2(320, 240);
		i.shape.state = ouro::window_state::hidden;
		i.shape.style = ouro::window_style::sizable_with_menu_and_statusbar;
		i.alt_f4_closes = true;
		i.cursor_state = ouro::cursor_state::hand;
		i.debug = true;

		Window = window::make(i);
	}

	Window->set_hotkeys(HotKeys);

	const int sSections[] = { 85, 120, 100 };
	Window->set_num_status_sections(sSections, oCOUNTOF(sSections));
	Window->set_status_text(0, "Timer: 0");
	Window->set_status_text(1, "F3 for default style");
	Window->set_status_text(2, "Easy: 0");

	Window->start_timer((uintptr_t)&PulseContext, 2000);

	Window->show();
}

bool oWindowTestApp::CreateMenu(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	oFOR(auto& m, Menus)
		m = oGUIMenuCreate();

	oFOR(const auto& h, sMenuHier)
	{
		oGUIMenuAppendSubmenu(
			h.Parent == oWMENU_TOPLEVEL ? _CreateEvent.hMenu : Menus[h.Parent]
		, Menus[h.Menu], h.Name);
	}

	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	oGUIMenuAppendEnumItems(ouro::window_style::count, Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, _CreateEvent.Shape.style);
	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle)
	{
		Window->style((ouro::window_style::value)_BorderStyle);
	});

	oGUIMenuAppendEnumItems(ouro::window_state::count, Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, _CreateEvent.Shape.state);
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State)
	{
		Window->show((ouro::window_state::value)_State);
	});

	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "&About...");

	return true;
}

bool oWindowTestApp::CreateControls(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	{
		oGUI_CONTROL_DESC d;
		d.hParent = _CreateEvent.hWindow;
		d.Type = ouro::control_type::button;
		d.Text = "&Easy";
		d.Position = int2(20, 20);
		d.Size = int2(80, 20);
		d.ID = oWCTL_EASY_BUTTON;
		d.StartsNewGroup = true;
		hButton = oWinControlCreate(d);
	}

	return true;
}

void oWindowTestApp::CheckState(ouro::window_state::value _State)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oWindowTestApp::CheckStyle(ouro::window_style::value _Style)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE]
	, oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oWMI_VIEW_STYLE_FIRST + _Style);
}

void oWindowTestApp::OnDirectoryEvent(filesystem::file_event::value _Event, const path& _Path)
{
	static int counter = 0;

	if (_Event == filesystem::file_event::added && !filesystem::is_directory(_Path))
	{
		int old = oStd::atomic_increment(&counter);
		oTRACE("%s: %s (%d)", as_string(_Event), _Path.c_str(), old + 1);
	}

	else
	{
		oTRACE("%s: %s", as_string(_Event), _Path.c_str());
	}
}

void oWindowTestApp::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case ouro::gui_event::timer:
			if (_Event.AsTimer().Context == (uintptr_t)&PulseContext)
			{
				PulseContext.Count++;
				Window->set_status_text(0, "Timer: %d", PulseContext.Count);
			}
			else
				oTRACE("ouro::gui_event::timer");
			break;
		case ouro::gui_event::activated:
			oTRACE("ouro::gui_event::activated");
			break;
		case ouro::gui_event::deactivated:
			oTRACE("ouro::gui_event::deactivated");
			break;
		case ouro::gui_event::creating:
		{
			oTRACE("ouro::gui_event::creating");
			if (!CreateMenu(_Event.AsCreate()))
				oThrowLastError();
			if (!CreateControls(_Event.AsCreate()))
				oThrowLastError();
			break;
		}
		case ouro::gui_event::paint:
			//oTRACE("ouro::gui_event::paint");
			break;
		case ouro::gui_event::display_changed:
			oTRACE("ouro::gui_event::display_changed");
			break;
		case ouro::gui_event::moving:
			oTRACE("ouro::gui_event::moving");
			break;
		case ouro::gui_event::moved:
			oTRACE("ouro::gui_event::moved %dx%d", _Event.AsShape().Shape.client_position.x, _Event.AsShape().Shape.client_position.y);
			break;
		case ouro::gui_event::sizing:
			oTRACE("ouro::gui_event::sizing %s %dx%d", as_string(_Event.AsShape().Shape.state), _Event.AsShape().Shape.client_size.x, _Event.AsShape().Shape.client_size.y);
			break;
		case ouro::gui_event::sized:
		{
			oTRACE("ouro::gui_event::sized %s %dx%d", as_string(_Event.AsShape().Shape.state), _Event.AsShape().Shape.client_size.x, _Event.AsShape().Shape.client_size.y);
			CheckState(_Event.AsShape().Shape.state);
			CheckStyle(_Event.AsShape().Shape.style);

			// center the button
			{
				RECT rParent, rButton;
				GetClientRect((HWND)_Event.hWindow, &rParent);
				GetClientRect(hButton, &rButton);
				RECT Centered = oWinRect(oGUIResolveRect(oRect(rParent), oRect(rButton), ouro::alignment::middle_center, false));
				SetWindowPos(hButton, nullptr, Centered.left, Centered.top, oWinRectW(Centered), oWinRectH(Centered), SWP_SHOWWINDOW);
			}
			break;
		}
		case ouro::gui_event::closing:
			oTRACE("ouro::gui_event::closing");
			Window->quit();
			break;
		case ouro::gui_event::closed:
			oTRACE("ouro::gui_event::closed");
			break;
		case ouro::gui_event::to_fullscreen:
			oTRACE("ouro::gui_event::to_fullscreen");
			break;
		case ouro::gui_event::from_fullscreen:
			oTRACE("ouro::gui_event::from_fullscreen");
			break;
		case ouro::gui_event::lost_capture:
			oTRACE("ouro::gui_event::lost_capture");
			break;
		case ouro::gui_event::drop_files:
			oTRACE("ouro::gui_event::drop_files (at %d,%d starting with %s)", _Event.AsDrop().ClientDropPosition.x, _Event.AsDrop().ClientDropPosition.y, _Event.AsDrop().pPaths[0]);
			break;
		case ouro::gui_event::input_device_changed:
			oTRACE("ouro::gui_event::input_device_changed %s %s %s", as_string(_Event.AsInputDevice().Type), as_string(_Event.AsInputDevice().Status), _Event.AsInputDevice().InstanceName);
			break;
		oNODEFAULT;
	}
}

void oWindowTestApp::ActionHook(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case ouro::gui_action::unknown:
			oTRACE("ouro::gui_action::unknown");
			break;
		case ouro::gui_action::menu:
			switch (_Action.DeviceID)
			{
				case oWMI_FILE_EXIT:
					Window->quit();
					break;
				case oWMI_HELP_ABOUT:
				{
					#if 1
						oDECLARE_ABOUT_INFO(i, oGDILoadIcon(IDI_APPICON));
						if (!About)
							About = ouro::about::make(i);
						About->show_modal(Window);
					#else
						ouro::msgbox(ouro::msg_type::info, _Action.hWindow, "About", "oWindowTestApp: a small program to show a basic window and its events and actions");
					#endif
					break;
				}
				default:
					MERL.OnAction(_Action);
			}
			break;

		case ouro::gui_action::control_activated:
			switch (_Action.DeviceID)
			{
				case oWCTL_EASY_BUTTON:
				{
					sstring text;
					Window->get_status_text(text, 2);
					int n = 0;
					from_string(&n, text.c_str() + 6);
					n++;
					Window->set_status_text(2, "Easy: %d", n);
					break;
				}
				default:
					oTRACE("ouro::gui_action::control_activated");
					break;
			}
			break;
		case ouro::gui_action::control_deactivated:
			oTRACE("ouro::gui_action::control_deactivated");
			break;
		case ouro::gui_action::control_selection_changing:
			oTRACE("ouro::gui_action::control_selection_changing");
			break;
		case ouro::gui_action::control_selection_changed:
			oTRACE("ouro::gui_action::control_selection_changed");
			break;
		case ouro::gui_action::hotkey:
			switch (_Action.DeviceID)
			{
				case oWHK_DEFAULT_STYLE:
				{
					ouro::window_shape s;
					s.state = Window->state();
					if (s.state == ouro::window_state::fullscreen)
						s.state = ouro::window_state::restored;
					s.style = ouro::window_style::sizable_with_menu_and_statusbar;
					Window->shape(s);
					break;
				}
				case oWHK_TOGGLE_FULLSCREEN:
					if (Window->state() != ouro::window_state::fullscreen)
					{
						PreFullscreenState = Window->state();
						Window->state(ouro::window_state::fullscreen);
					}
					else
					{
						Window->state(PreFullscreenState);
					}
					break;
				default:
					oTRACE("ouro::gui_action::hotkey");
					break;
			}
			
			break;
		case ouro::gui_action::key_down:
			oTRACE("ouro::gui_action::key_down %s", as_string(_Action.Key));
			break;
		case ouro::gui_action::key_up:
			oTRACE("ouro::gui_action::key_up %s", as_string(_Action.Key));
			break;
		case ouro::gui_action::pointer_move:
			//oTRACE("ouro::gui_action::pointer_move");
			break;
		case ouro::gui_action::skeleton:
			oTRACE("ouro::gui_action::skeleton");
			break;
		case ouro::gui_action::skeleton_acquired:
			oTRACE("ouro::gui_action::skeleton_acquired");
			break;
		case ouro::gui_action::skeleton_lost:
			oTRACE("ouro::gui_action::skeleton_lost");
			break;
		oNODEFAULT;
	}
}

void oWindowTestApp::Run()
{
	Window->flush_messages(true);
}

int main(int argc, const char* argv[])
{
	oWindowTestApp App;
	App.Run();
	return 0;
}
