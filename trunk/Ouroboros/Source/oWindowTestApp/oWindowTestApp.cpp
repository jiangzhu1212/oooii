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

ouro::basic_hotkey_info HotKeys[] =
{
	// reset style
	{ ouro::input::f3, oWHK_DEFAULT_STYLE, false, false, false },
	{ ouro::input::f11, oWHK_TOGGLE_FULLSCREEN, false, false, false },
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
	void ActionHook(const ouro::input::action& _Action);
	void EventHook(const window::basic_event& _Event);
	bool CreateMenu(const window::create_event& _CreateEvent);
	bool CreateControls(const window::create_event& _CreateEvent);
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
		i.on_action = std::bind(&oWindowTestApp::ActionHook, this, oBIND1);
		i.on_event = std::bind(&oWindowTestApp::EventHook, this, oBIND1);
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

bool oWindowTestApp::CreateMenu(const window::create_event& _CreateEvent)
{
	oFOR(auto& m, Menus)
		m = oGUIMenuCreate();

	oFOR(const auto& h, sMenuHier)
	{
		oGUIMenuAppendSubmenu(
			h.Parent == oWMENU_TOPLEVEL ? _CreateEvent.menu : Menus[h.Parent]
		, Menus[h.Menu], h.Name);
	}

	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	oGUIMenuAppendEnumItems(ouro::window_style::count, Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, _CreateEvent.shape.style);
	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle)
	{
		Window->style((ouro::window_style::value)_BorderStyle);
	});

	oGUIMenuAppendEnumItems(ouro::window_state::count, Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, _CreateEvent.shape.state);
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State)
	{
		Window->show((ouro::window_state::value)_State);
	});

	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "&About...");

	return true;
}

bool oWindowTestApp::CreateControls(const window::create_event& _CreateEvent)
{
	{
		ouro::control_info d;
		d.parent = _CreateEvent.window;
		d.type = ouro::control_type::button;
		d.text = "&Easy";
		d.position = int2(20, 20);
		d.size = int2(80, 20);
		d.id = oWCTL_EASY_BUTTON;
		d.starts_new_group = true;
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

void oWindowTestApp::EventHook(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case ouro::event_type::timer:
			if (_Event.as_timer().context == (uintptr_t)&PulseContext)
			{
				PulseContext.Count++;
				Window->set_status_text(0, "Timer: %d", PulseContext.Count);
			}
			else
				oTRACE("ouro::event_type::timer");
			break;
		case ouro::event_type::activated:
			oTRACE("ouro::event_type::activated");
			break;
		case ouro::event_type::deactivated:
			oTRACE("ouro::event_type::deactivated");
			break;
		case ouro::event_type::creating:
		{
			oTRACE("ouro::event_type::creating");
			if (!CreateMenu(_Event.as_create()))
				oThrowLastError();
			if (!CreateControls(_Event.as_create()))
				oThrowLastError();
			break;
		}
		case ouro::event_type::paint:
			//oTRACE("ouro::event_type::paint");
			break;
		case ouro::event_type::display_changed:
			oTRACE("ouro::event_type::display_changed");
			break;
		case ouro::event_type::moving:
			oTRACE("ouro::event_type::moving");
			break;
		case ouro::event_type::moved:
			oTRACE("ouro::event_type::moved %dx%d", _Event.as_shape().shape.client_position.x, _Event.as_shape().shape.client_position.y);
			break;
		case ouro::event_type::sizing:
			oTRACE("ouro::event_type::sizing %s %dx%d", as_string(_Event.as_shape().shape.state), _Event.as_shape().shape.client_size.x, _Event.as_shape().shape.client_size.y);
			break;
		case ouro::event_type::sized:
		{
			oTRACE("ouro::event_type::sized %s %dx%d", as_string(_Event.as_shape().shape.state), _Event.as_shape().shape.client_size.x, _Event.as_shape().shape.client_size.y);
			CheckState(_Event.as_shape().shape.state);
			CheckStyle(_Event.as_shape().shape.style);

			// center the button
			{
				RECT rParent, rButton;
				GetClientRect((HWND)_Event.window, &rParent);
				GetClientRect(hButton, &rButton);
				RECT Centered = oWinRect(oGUIResolveRect(oRect(rParent), oRect(rButton), ouro::alignment::middle_center, false));
				SetWindowPos(hButton, nullptr, Centered.left, Centered.top, oWinRectW(Centered), oWinRectH(Centered), SWP_SHOWWINDOW);
			}
			break;
		}
		case ouro::event_type::closing:
			oTRACE("ouro::event_type::closing");
			Window->quit();
			break;
		case ouro::event_type::closed:
			oTRACE("ouro::event_type::closed");
			break;
		case ouro::event_type::to_fullscreen:
			oTRACE("ouro::event_type::to_fullscreen");
			break;
		case ouro::event_type::from_fullscreen:
			oTRACE("ouro::event_type::from_fullscreen");
			break;
		case ouro::event_type::lost_capture:
			oTRACE("ouro::event_type::lost_capture");
			break;
		case ouro::event_type::drop_files:
			oTRACE("ouro::event_type::drop_files (at %d,%d starting with %s)", _Event.as_drop().client_drop_position.x, _Event.as_drop().client_drop_position.y, _Event.as_drop().paths[0]);
			break;
		case ouro::event_type::input_device_changed:
			oTRACE("ouro::event_type::input_device_changed %s %s %s", as_string(_Event.as_input_device().type), as_string(_Event.as_input_device().status), _Event.as_input_device().instance_name);
			break;
		oNODEFAULT;
	}
}

void oWindowTestApp::ActionHook(const ouro::input::action& _Action)
{
	switch (_Action.action_type)
	{
		case ouro::input::unknown:
			oTRACE("ouro::input::unknown");
			break;
		case ouro::input::menu:
			switch (_Action.device_id)
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
						ouro::msgbox(ouro::msg_type::info, _Action.window, "About", "oWindowTestApp: a small program to show a basic window and its events and actions");
					#endif
					break;
				}
				default:
					MERL.OnAction(_Action);
			}
			break;

		case ouro::input::control_activated:
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
					oTRACE("ouro::input::control_activated");
					break;
			}
			break;
		case ouro::input::control_deactivated:
			oTRACE("ouro::input::control_deactivated");
			break;
		case ouro::input::control_selection_changing:
			oTRACE("ouro::input::control_selection_changing");
			break;
		case ouro::input::control_selection_changed:
			oTRACE("ouro::input::control_selection_changed");
			break;
		case ouro::input::hotkey:
			switch (_Action.device_id)
			{
				case oWHK_DEFAULT_STYLE:
				{
					if (Window->state() == ouro::window_state::fullscreen)
						Window->state(ouro::window_state::restored);
					Window->style(ouro::window_style::sizable_with_menu_and_statusbar);
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
					oTRACE("ouro::input::hotkey");
					break;
			}
			
			break;
		case ouro::input::key_down:
			oTRACE("ouro::input::key_down %s", as_string(_Action.key));
			break;
		case ouro::input::key_up:
			oTRACE("ouro::input::key_up %s", as_string(_Action.key));
			break;
		case ouro::input::pointer_move:
			//oTRACE("ouro::input::pointer_move");
			break;
		case ouro::input::skeleton_update:
			oTRACE("ouro::input::skeleton");
			break;
		case ouro::input::skeleton_acquired:
			oTRACE("ouro::input::skeleton_acquired");
			break;
		case ouro::input::skeleton_lost:
			oTRACE("ouro::input::skeleton_lost");
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
