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
#include <oGUI/Windows/win_gdi_bitmap.h>
#include <oGUI/oGUIMenu.h>
#include <oGUI/msgbox.h>
#include <oGUI/msgbox_reporting.h>
#include <oPlatform/oStream.h>
#include <oGUI/window.h>
#include "resource.h"
#include <atomic>

#include <oCore/filesystem_monitor.h>

#include "../about_ouroboros.h"

using namespace ouro;
using namespace ouro::windows::gdi;

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
	oWMI_VIEW_STYLE_LAST = oWMI_VIEW_STYLE_FIRST + window_style::count - 1,

	oWMI_VIEW_STATE_FIRST,
	oWMI_VIEW_STATE_LAST = oWMI_VIEW_STATE_FIRST + window_state::count - 1,

	oWMI_HELP_ABOUT,
};

enum oWHK // hotkeys
{
	oWHK_DEFAULT_STYLE,
	oWHK_TOGGLE_FULLSCREEN,
};

basic_hotkey_info HotKeys[] =
{
	// reset style
	{ input::f3, oWHK_DEFAULT_STYLE, false, false, false },
	{ input::f11, oWHK_TOGGLE_FULLSCREEN, false, false, false },
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
	std::shared_ptr<about> About;
	menu_handle Menus[oWMENU_COUNT];
	oWindowTestAppPulseContext PulseContext;
	oGUIMenuEnumRadioListHandler MERL; 
	window_state::value PreFullscreenState;
	bool Running;

	// This gets deleted by parent window automatically.
	HWND hButton;

	std::shared_ptr<filesystem::monitor> DirWatcher;

private:
	void ActionHook(const input::action& _Action);
	void EventHook(const window::basic_event& _Event);
	void CreateMenu(const window::create_event& _CreateEvent);
	void CreateControls(const window::create_event& _CreateEvent);
	void CheckState(window_state::value _State);
	void CheckStyle(window_style::value _Style);
	void OnDirectoryEvent(filesystem::file_event::value _Event, const path& _Path);
};

oWindowTestApp::oWindowTestApp()
	: Running(true)
	, PreFullscreenState(window_state::hidden)
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
		i.icon = (icon_handle)load_icon(IDI_APPICON);
		i.on_action = std::bind(&oWindowTestApp::ActionHook, this, std::placeholders::_1);
		i.on_event = std::bind(&oWindowTestApp::EventHook, this, std::placeholders::_1);
		i.shape.client_size = int2(320, 240);
		i.shape.state = window_state::hidden;
		i.shape.style = window_style::sizable_with_menu_and_statusbar;
		i.alt_f4_closes = true;
		i.cursor_state = cursor_state::hand;
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

void oWindowTestApp::CreateMenu(const window::create_event& _CreateEvent)
{
	for (auto& m : Menus)
		m = oGUIMenuCreate();

	for (const auto& h : sMenuHier)
	{
		oGUIMenuAppendSubmenu(
			h.Parent == oWMENU_TOPLEVEL ? _CreateEvent.menu : Menus[h.Parent]
		, Menus[h.Menu], h.Name);
	}

	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	oGUIMenuAppendEnumItems(window_style::count, Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, _CreateEvent.shape.style);
	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle)
	{
		Window->style((window_style::value)_BorderStyle);
	});

	oGUIMenuAppendEnumItems(window_state::count, Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, _CreateEvent.shape.state);
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State)
	{
		Window->show((window_state::value)_State);
	});

	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "&About...");
}

void oWindowTestApp::CreateControls(const window::create_event& _CreateEvent)
{
	control_info d;
	d.parent = _CreateEvent.window;
	d.type = control_type::button;
	d.text = "&Easy";
	d.position = int2(20, 20);
	d.size = int2(80, 20);
	d.id = oWCTL_EASY_BUTTON;
	d.starts_new_group = true;
	hButton = oWinControlCreate(d);
}

void oWindowTestApp::CheckState(window_state::value _State)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oWindowTestApp::CheckStyle(window_style::value _Style)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE]
	, oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oWMI_VIEW_STYLE_FIRST + _Style);
}

void oWindowTestApp::OnDirectoryEvent(filesystem::file_event::value _Event, const path& _Path)
{
	static std::atomic<int> counter = 0;

	if (_Event == filesystem::file_event::added && !filesystem::is_directory(_Path))
	{
		int old = counter++;
		oTRACE("%s: %s (%d)", as_string(_Event), _Path.c_str(), old + 1);
	}

	else
	{
		oTRACE("%s: %s", as_string(_Event), _Path.c_str());
	}
}

void oWindowTestApp::EventHook(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case event_type::timer:
			if (_Event.as_timer().context == (uintptr_t)&PulseContext)
			{
				PulseContext.Count++;
				Window->set_status_text(0, "Timer: %d", PulseContext.Count);
			}
			else
				oTRACE("event_type::timer");
			break;
		case event_type::activated:
			oTRACE("event_type::activated");
			break;
		case event_type::deactivated:
			oTRACE("event_type::deactivated");
			break;
		case event_type::creating:
		{
			oTRACE("event_type::creating");
			CreateMenu(_Event.as_create());
			CreateControls(_Event.as_create());
			break;
		}
		case event_type::paint:
			//oTRACE("event_type::paint");
			break;
		case event_type::display_changed:
			oTRACE("event_type::display_changed");
			break;
		case event_type::moving:
			oTRACE("event_type::moving");
			break;
		case event_type::moved:
			oTRACE("event_type::moved %dx%d", _Event.as_shape().shape.client_position.x, _Event.as_shape().shape.client_position.y);
			break;
		case event_type::sizing:
			oTRACE("event_type::sizing %s %dx%d", as_string(_Event.as_shape().shape.state), _Event.as_shape().shape.client_size.x, _Event.as_shape().shape.client_size.y);
			break;
		case event_type::sized:
		{
			oTRACE("event_type::sized %s %dx%d", as_string(_Event.as_shape().shape.state), _Event.as_shape().shape.client_size.x, _Event.as_shape().shape.client_size.y);
			CheckState(_Event.as_shape().shape.state);
			CheckStyle(_Event.as_shape().shape.style);

			// center the button
			{
				RECT rParent, rButton;
				GetClientRect((HWND)_Event.window, &rParent);
				GetClientRect(hButton, &rButton);
				RECT Centered = oWinRect(resolve_rect(oRect(rParent), oRect(rButton), alignment::middle_center, false));
				SetWindowPos(hButton, nullptr, Centered.left, Centered.top, oWinRectW(Centered), oWinRectH(Centered), SWP_SHOWWINDOW);
			}
			break;
		}
		case event_type::closing:
			oTRACE("event_type::closing");
			Window->quit();
			break;
		case event_type::closed:
			oTRACE("event_type::closed");
			break;
		case event_type::to_fullscreen:
			oTRACE("event_type::to_fullscreen");
			break;
		case event_type::from_fullscreen:
			oTRACE("event_type::from_fullscreen");
			break;
		case event_type::lost_capture:
			oTRACE("event_type::lost_capture");
			break;
		case event_type::drop_files:
			oTRACE("event_type::drop_files (at %d,%d starting with %s)", _Event.as_drop().client_drop_position.x, _Event.as_drop().client_drop_position.y, _Event.as_drop().paths[0]);
			break;
		case event_type::input_device_changed:
			oTRACE("event_type::input_device_changed %s %s %s", as_string(_Event.as_input_device().type), as_string(_Event.as_input_device().status), _Event.as_input_device().instance_name);
			break;
		oNODEFAULT;
	}
}

void oWindowTestApp::ActionHook(const input::action& _Action)
{
	switch (_Action.action_type)
	{
		case input::unknown:
			oTRACE("input::unknown");
			break;
		case input::menu:
			switch (_Action.device_id)
			{
				case oWMI_FILE_EXIT:
					Window->quit();
					break;
				case oWMI_HELP_ABOUT:
				{
					#if 1
						oDECLARE_ABOUT_INFO(i, load_icon(IDI_APPICON));
						if (!About)
							About = about::make(i);
						About->show_modal(Window);
					#else
						msgbox(msg_type::info, _Action.window, "About", "oWindowTestApp: a small program to show a basic window and its events and actions");
					#endif
					break;
				}
				default:
					MERL.OnAction(_Action);
			}
			break;

		case input::control_activated:
			switch (_Action.device_id)
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
					oTRACE("input::control_activated");
					break;
			}
			break;
		case input::control_deactivated:
			oTRACE("input::control_deactivated");
			break;
		case input::control_selection_changing:
			oTRACE("input::control_selection_changing");
			break;
		case input::control_selection_changed:
			oTRACE("input::control_selection_changed");
			break;
		case input::hotkey:
			switch (_Action.device_id)
			{
				case oWHK_DEFAULT_STYLE:
				{
					if (Window->state() == window_state::fullscreen)
						Window->state(window_state::restored);
					Window->style(window_style::sizable_with_menu_and_statusbar);
					break;
				}
				case oWHK_TOGGLE_FULLSCREEN:
					if (Window->state() != window_state::fullscreen)
					{
						PreFullscreenState = Window->state();
						Window->state(window_state::fullscreen);
					}
					else
					{
						Window->state(PreFullscreenState);
					}
					break;
				default:
					oTRACE("input::hotkey");
					break;
			}
			
			break;
		case input::key_down:
			oTRACE("input::key_down %s", as_string(_Action.key));
			break;
		case input::key_up:
			oTRACE("input::key_up %s", as_string(_Action.key));
			break;
		case input::pointer_move:
			//oTRACE("input::pointer_move");
			break;
		case input::skeleton_update:
			oTRACE("input::skeleton");
			break;
		case input::skeleton_acquired:
			oTRACE("input::skeleton_acquired");
			break;
		case input::skeleton_lost:
			oTRACE("input::skeleton_lost");
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
	reporting::set_prompter(prompt_msgbox);
	oWindowTestApp App;
	App.Run();
	return 0;
}
