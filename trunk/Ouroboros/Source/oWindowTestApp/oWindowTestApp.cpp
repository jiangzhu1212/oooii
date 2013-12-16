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

#include <oGUI/Windows/oGDI.h>
#include <oGUI/oGUIMenu.h>
#include <oGUI/oMsgBox.h>
#include <oPlatform/oStream.h>
#include <oGUI/window.h>
#include "resource.h"

#include <oCore/filesystem_monitor.h>

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
	oWMI_VIEW_STYLE_LAST = oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_STYLE_COUNT - 1,

	oWMI_VIEW_STATE_FIRST,
	oWMI_VIEW_STATE_LAST = oWMI_VIEW_STATE_FIRST + oGUI_WINDOW_STATE_COUNT - 1,

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
	{ oGUI_KEY_F3, oWHK_DEFAULT_STYLE, false, false, false },
	{ oGUI_KEY_F11, oWHK_TOGGLE_FULLSCREEN, false, false, false },
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
	oGUI_MENU Menus[oWMENU_COUNT];
	oWindowTestAppPulseContext PulseContext;
	oGUIMenuEnumRadioListHandler MERL; 
	oGUI_WINDOW_STATE PreFullscreenState;
	bool Running;

	// This gets deleted by parent window automatically.
	HWND hButton;

	std::shared_ptr<filesystem::monitor> DirWatcher;

private:
	void ActionHook(const oGUI_ACTION_DESC& _Action);
	void EventHook(const oGUI_EVENT_DESC& _Event);
	bool CreateMenu(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	bool CreateControls(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	void CheckState(oGUI_WINDOW_STATE _State);
	void CheckStyle(oGUI_WINDOW_STYLE _Style);
	void OnDirectoryEvent(filesystem::file_event::value _Event, const path& _Path);
};

oWindowTestApp::oWindowTestApp()
	: Running(true)
	, PreFullscreenState(oGUI_WINDOW_HIDDEN)
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
		i.icon = (oGUI_ICON)oGDILoadIcon(IDI_APPICON);
		i.action_hook = oBIND(&oWindowTestApp::ActionHook, this, oBIND1);
		i.event_hook = oBIND(&oWindowTestApp::EventHook, this, oBIND1);
		i.shape.ClientSize = int2(320, 240);
		i.shape.State = oGUI_WINDOW_HIDDEN;
		i.shape.Style = oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR;
		i.alt_f4_closes = true;
		i.cursor_state = oGUI_CURSOR_HAND;

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

	oGUIMenuAppendEnumItems(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oRTTI_OF(oGUI_WINDOW_STYLE), _CreateEvent.Shape.Style);
	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle)
	{
		Window->style((oGUI_WINDOW_STYLE)_BorderStyle);
	});

	oGUIMenuAppendEnumItems(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oRTTI_OF(oGUI_WINDOW_STATE), _CreateEvent.Shape.State);
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State)
	{
		Window->show((oGUI_WINDOW_STATE)_State);
	});

	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "&About...");

	return true;
}

bool oWindowTestApp::CreateControls(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	{
		oGUI_CONTROL_DESC d;
		d.hParent = _CreateEvent.hWindow;
		d.Type = oGUI_CONTROL_BUTTON;
		d.Text = "&Easy";
		d.Position = int2(20, 20);
		d.Size = int2(80, 20);
		d.ID = oWCTL_EASY_BUTTON;
		d.StartsNewGroup = true;
		hButton = oWinControlCreate(d);
	}

	return true;
}

void oWindowTestApp::CheckState(oGUI_WINDOW_STATE _State)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oWindowTestApp::CheckStyle(oGUI_WINDOW_STYLE _Style)
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
		case oGUI_TIMER:
			if (_Event.AsTimer().Context == (uintptr_t)&PulseContext)
			{
				PulseContext.Count++;
				Window->set_status_text(0, "Timer: %d", PulseContext.Count);
			}
			else
				oTRACE("oGUI_TIMER");
			break;
		case oGUI_ACTIVATED:
			oTRACE("oGUI_ACTIVATED");
			break;
		case oGUI_DEACTIVATED:
			oTRACE("oGUI_DEACTIVATED");
			break;
		case oGUI_CREATING:
		{
			oTRACE("oGUI_CREATING");
			if (!CreateMenu(_Event.AsCreate()))
				oThrowLastError();
			if (!CreateControls(_Event.AsCreate()))
				oThrowLastError();
			break;
		}
		case oGUI_PAINT:
			//oTRACE("oGUI_PAINT");
			break;
		case oGUI_DISPLAY_CHANGED:
			oTRACE("oGUI_DISPLAY_CHANGED");
			break;
		case oGUI_MOVING:
			oTRACE("oGUI_MOVING");
			break;
		case oGUI_MOVED:
			oTRACE("oGUI_MOVED %dx%d", _Event.AsShape().Shape.ClientPosition.x, _Event.AsShape().Shape.ClientPosition.y);
			break;
		case oGUI_SIZING:
			oTRACE("oGUI_SIZING %s %dx%d", as_string(_Event.AsShape().Shape.State), _Event.AsShape().Shape.ClientSize.x, _Event.AsShape().Shape.ClientSize.y);
			break;
		case oGUI_SIZED:
		{
			oTRACE("oGUI_SIZED %s %dx%d", as_string(_Event.AsShape().Shape.State), _Event.AsShape().Shape.ClientSize.x, _Event.AsShape().Shape.ClientSize.y);
			CheckState(_Event.AsShape().Shape.State);
			CheckStyle(_Event.AsShape().Shape.Style);

			// center the button
			{
				RECT rParent, rButton;
				GetClientRect((HWND)_Event.hWindow, &rParent);
				GetClientRect(hButton, &rButton);
				RECT Centered = oWinRect(oGUIResolveRect(oRect(rParent), oRect(rButton), oGUI_ALIGNMENT_MIDDLE_CENTER, false));
				SetWindowPos(hButton, nullptr, Centered.left, Centered.top, oWinRectW(Centered), oWinRectH(Centered), SWP_SHOWWINDOW);
			}
			break;
		}
		case oGUI_CLOSING:
			oTRACE("oGUI_CLOSING");
			Window->quit();
			break;
		case oGUI_CLOSED:
			oTRACE("oGUI_CLOSED");
			break;
		case oGUI_TO_FULLSCREEN:
			oTRACE("oGUI_TO_FULLSCREEN");
			break;
		case oGUI_FROM_FULLSCREEN:
			oTRACE("oGUI_FROM_FULLSCREEN");
			break;
		case oGUI_LOST_CAPTURE:
			oTRACE("oGUI_LOST_CAPTURE");
			break;
		case oGUI_DROP_FILES:
			oTRACE("oGUI_DROP_FILES (at %d,%d starting with %s)", _Event.AsDrop().ClientDropPosition.x, _Event.AsDrop().ClientDropPosition.y, _Event.AsDrop().pPaths[0]);
			break;
		case oGUI_INPUT_DEVICE_CHANGED:
			oTRACE("oGUI_INPUT_DEVICE_CHANGED %s %s %s", as_string(_Event.AsInputDevice().Type), as_string(_Event.AsInputDevice().Status), _Event.AsInputDevice().InstanceName);
			break;
		oNODEFAULT;
	}
}

void oWindowTestApp::ActionHook(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_UNKNOWN:
			oTRACE("oGUI_ACTION_UNKNOWN");
			break;
		case oGUI_ACTION_MENU:
			switch (_Action.DeviceID)
			{
				case oWMI_FILE_EXIT:
					Window->quit();
					break;
				case oWMI_HELP_ABOUT:
				{
					ouro::msgbox(ouro::msg_type::info, _Action.hWindow, "About", "oWindowTestApp: a small program to show a basic window and its events and actions");
					break;
				}
				default:
					MERL.OnAction(_Action);
			}
			break;

		case oGUI_ACTION_CONTROL_ACTIVATED:
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
					oTRACE("oGUI_ACTION_CONTROL_ACTIVATED");
					break;
			}
			break;
		case oGUI_ACTION_CONTROL_DEACTIVATED:
			oTRACE("oGUI_ACTION_CONTROL_DEACTIVATED");
			break;
		case oGUI_ACTION_CONTROL_SELECTION_CHANGING:
			oTRACE("oGUI_ACTION_CONTROL_SELECTION_CHANGING");
			break;
		case oGUI_ACTION_CONTROL_SELECTION_CHANGED:
			oTRACE("oGUI_ACTION_CONTROL_SELECTION_CHANGED");
			break;
		case oGUI_ACTION_HOTKEY:
			switch (_Action.DeviceID)
			{
				case oWHK_DEFAULT_STYLE:
				{
					oGUI_WINDOW_SHAPE_DESC s;
					s.State = Window->state();
					if (s.State == oGUI_WINDOW_FULLSCREEN)
						s.State = oGUI_WINDOW_RESTORED;
					s.Style = oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR;
					Window->shape(s);
					break;
				}
				case oWHK_TOGGLE_FULLSCREEN:
					if (Window->state() != oGUI_WINDOW_FULLSCREEN)
					{
						PreFullscreenState = Window->state();
						Window->state(oGUI_WINDOW_FULLSCREEN);
					}
					else
					{
						Window->state(PreFullscreenState);
					}
					break;
				default:
					oTRACE("oGUI_ACTION_HOTKEY");
					break;
			}
			
			break;
		case oGUI_ACTION_KEY_DOWN:
			oTRACE("oGUI_ACTION_KEY_DOWN %s", as_string(_Action.Key));
			break;
		case oGUI_ACTION_KEY_UP:
			oTRACE("oGUI_ACTION_KEY_UP %s", as_string(_Action.Key));
			break;
		case oGUI_ACTION_POINTER_MOVE:
			//oTRACE("oGUI_ACTION_POINTER_MOVE");
			break;
		case oGUI_ACTION_SKELETON:
			oTRACE("oGUI_ACTION_SKELETON");
			break;
		case oGUI_ACTION_SKELETON_ACQUIRED:
			oTRACE("oGUI_ACTION_SKELETON_ACQUIRED");
			break;
		case oGUI_ACTION_SKELETON_LOST:
			oTRACE("oGUI_ACTION_SKELETON_LOST");
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
