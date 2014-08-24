/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBase/backoff.h>

// oGUI
#include <oGUI/msgbox.h>
#include <oGUI/msgbox_reporting.h>
#include <oGUI/Windows/win_gdi_bitmap.h>
#include <oGUI/menu.h>
#include <oGUI/enum_radio_handler.h>
#include <oGUI/window.h>
#include "resource.h"
#include "../about_ouroboros.h"

// oGfx
#include <oGPU/all.h>
#include <oGfx/render_window.h>
#include "model_view.h"

using namespace ouro;
using namespace ouro::gui;
using namespace windows::gdi;

static const char* sAppName = "oGfxView";

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

enum oWHOTKEY
{
	oWHK_TOGGLE_UI_MODE,
	oWHK_DEFAULT_STYLE,
	oWHK_TOGGLE_FULLSCREEN,
};

enum oWMI // menuitems
{
	oWMI_FILE_EXIT,

	oWMI_VIEW_STYLE_FIRST,
	oWMI_VIEW_STYLE_LAST = oWMI_VIEW_STYLE_FIRST + window_style::count - 1,

	oWMI_VIEW_STATE_FIRST,
	oWMI_VIEW_STATE_LAST = oWMI_VIEW_STATE_FIRST + window_state::count - 1,

	oWMI_VIEW_EXCLUSIVE,

	oWMI_HELP_ABOUT,
};

basic_hotkey_info HotKeys[] =
{
	// reset style
	{ input::f2, oWHK_TOGGLE_UI_MODE, false, false, false },
	{ input::f3, oWHK_DEFAULT_STYLE, false, false, false },
	{ input::enter, oWHK_TOGGLE_FULLSCREEN, true, false, false },
};

class oGfxViewApp
{
public:
	oGfxViewApp();
	~oGfxViewApp();

	void Run();

private:
	std::shared_ptr<window> AppWindow;
	window* pGPUWindow;

	gfx::core gfxcore;
	gfx::render_window gpuwin;
	gfx::model_view first;

	menu_handle Menus[oWMENU_COUNT];
	menu::enum_radio_handler EnumRadioHandler;
	window_state::value PreFullscreenState;
	bool Running;
	bool UIMode;
	bool AllowUIModeChange;
private:
	void ActionHook(const input::action& _Action);
	void AppEventHook(const window::basic_event& _Event);
	bool CreateMenus(const window::create_event& _CreateEvent);
	void CheckState(window_state::value _State);
	void CheckStyle(window_style::value _Style);
	void EnableStatusBarStyles(bool _Enabled);
	void SetUIModeInternal(bool _UIMode, const window_shape& _CurrentAppShape, const window_shape& _CurrentGPUShape);
	void SetUIMode(bool _UIMode);
	void ToggleFullscreenCooperative(window* _pWindow);
	void Render();
};

oGfxViewApp::oGfxViewApp()
	: pGPUWindow(nullptr)
	, Running(true)
	, UIMode(true)
	, AllowUIModeChange(true)
	, PreFullscreenState(window_state::hidden)
{
	// Set up application window
	{
		window::init i;
		i.title = sAppName;
		i.icon = (icon_handle)load_icon(IDI_APPICON);
		i.on_action = std::bind(&oGfxViewApp::ActionHook, this, std::placeholders::_1);
		i.on_event = std::bind(&oGfxViewApp::AppEventHook, this, std::placeholders::_1);
		i.shape.client_size = int2(256, 256);
		i.shape.state = window_state::hidden;
		i.shape.style = window_style::sizable_with_menu_and_statusbar;
		i.alt_f4_closes = true;
		i.cursor_state = cursor_state::arrow;
		AppWindow = window::make(i);
		AppWindow->set_hotkeys(HotKeys);

		const int sSections[] = { 120, -1 };
		AppWindow->set_num_status_sections(sSections, oCOUNTOF(sSections));
		AppWindow->set_status_text(0, "F3 for default style");
		AppWindow->set_status_text(1, "Fullscreen cooperative");
	}

	// Initialize render resources
	gfxcore.initialize("gpu window thread", true);
	
	first.initialize(gfxcore);

	// Set up child window so the app can have Win32 GUI and GPU rendering
	{
		gfx::render_window_init i("render_window");
		i.icon = (icon_handle)load_icon(IDI_APPICON);
		i.hotkeys = HotKeys;
		i.num_hotkeys = oCOUNTOF(HotKeys);
		i.color_format = surface::format::r8g8b8a8_unorm;
		i.depth_format = surface::format::d24_unorm_s8_uint;
		i.on_action = std::bind(&oGfxViewApp::ActionHook, this, std::placeholders::_1);
		i.on_stop = [&] { Running = false; };
		i.render = std::bind(&gfx::model_view::render, &first);
		i.parent = AppWindow;

		pGPUWindow = gpuwin.start(gfxcore.device, i);
	}

	// attach render app to window rendering
	auto& ct = gpuwin.color_target();
	auto& dt = gpuwin.depth_target();
	first.set_draw_target(&ct, &dt);
	first.set_clear(almost_black);
	AppWindow->show();
}

oGfxViewApp::~oGfxViewApp()
{
	filesystem::join();
}

void oGfxViewApp::CheckState(window_state::value _State)
{
	menu::check_radio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oGfxViewApp::CheckStyle(window_style::value _Style)
{
	menu::check_radio(Menus[oWMENU_VIEW_STYLE]
	, oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oWMI_VIEW_STYLE_FIRST + _Style);
}

void oGfxViewApp::EnableStatusBarStyles(bool _Enabled)
{
	// Enable styles not allowed for render target windows
	menu::enable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::fixed_with_statusbar, _Enabled);
	menu::enable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::fixed_with_menu_and_statusbar, _Enabled);
	menu::enable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::sizable_with_statusbar, _Enabled);
	menu::enable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::sizable_with_menu_and_statusbar, _Enabled);
}

void oGfxViewApp::SetUIModeInternal(bool _UIMode, const window_shape& _CurrentAppShape, const window_shape& _CurrentGPUShape)
{
	window_shape CurAppShape(_CurrentAppShape), CurGPUShape(_CurrentGPUShape);

	if (AppWindow->is_window_thread())
		CurAppShape = AppWindow->shape();

	else if (pGPUWindow->is_window_thread())
		CurGPUShape = pGPUWindow->shape();

	if (_UIMode)
	{
		window_shape GPUShape;
		GPUShape.state = window_state::hidden;
		GPUShape.style = window_style::borderless;
		GPUShape.client_position = int2(0, 0);
		GPUShape.client_size = CurAppShape.client_size;
		pGPUWindow->shape(GPUShape);
		pGPUWindow->parent(AppWindow);
		pGPUWindow->show();
		AppWindow->show(CurGPUShape.state);
		AppWindow->focus(true);
	}

	else
	{
		AppWindow->hide();
		window_shape GPUShape(CurAppShape);
		if (has_statusbar(GPUShape.style))
			GPUShape.style = window_style::value(GPUShape.style - 2);
		pGPUWindow->parent(nullptr);
		pGPUWindow->shape(GPUShape);
	}

	UIMode = _UIMode;
}

void oGfxViewApp::SetUIMode(bool _UIMode)
{
	if (!AllowUIModeChange)
		return;

	window_shape AppShape, GPUShape;

	if (AppWindow->is_window_thread())
	{
		AppShape = AppWindow->shape();
	
		pGPUWindow->dispatch([=] { SetUIModeInternal(_UIMode, AppShape, GPUShape); });
	}

	else if (pGPUWindow->is_window_thread())
	{
		GPUShape = pGPUWindow->shape();

		AppWindow->dispatch([=] { SetUIModeInternal(_UIMode, AppShape, GPUShape); });
	}
}

void oGfxViewApp::ToggleFullscreenCooperative(window* _pWindow)
{
	if (_pWindow->state() != window_state::fullscreen)
	{
		PreFullscreenState = _pWindow->state();
		_pWindow->state(window_state::fullscreen);
		AllowUIModeChange = false;
	}
	else
	{
		_pWindow->state(PreFullscreenState);
		AllowUIModeChange = true;
	}
}

bool oGfxViewApp::CreateMenus(const window::create_event& _CreateEvent)
{
	for (auto& m : Menus)
		m = menu::make_menu();

	for (const auto& h : sMenuHier)
	{
		menu::append_submenu(
			h.Parent == oWMENU_TOPLEVEL ? _CreateEvent.menu : Menus[h.Parent]
		, Menus[h.Menu], h.Name);
	}

	// File menu
	menu::append_item(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	// Edit menu
	// (nothing yet)

	// View menu
	menu::append_enum_items(window_style::count, Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, _CreateEvent.shape.style);
	EnableStatusBarStyles(true);

	EnumRadioHandler.add(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle) { AppWindow->style((window_style::value)_BorderStyle); });
	menu::check_radio(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST
		, oWMI_VIEW_STYLE_FIRST + window_style::sizable_with_menu);

	menu::append_enum_items(window_state::count, Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, _CreateEvent.shape.state);
	EnumRadioHandler.add(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State) { AppWindow->show((window_state::value)_State); });

	menu::append_item(Menus[oWMENU_VIEW], oWMI_VIEW_EXCLUSIVE, "Fullscreen E&xclusive");

	// Help menu
	menu::append_item(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "About...");
	return true;
}

void oGfxViewApp::AppEventHook(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case event_type::timer:
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
			if (!CreateMenus(_Event.as_create()))
				oThrowLastError();
			break;
		}

		case event_type::sized:
		{
			oTRACE("event_type::sized %s %dx%d", as_string(_Event.as_shape().shape.state), _Event.as_shape().shape.client_size.x, _Event.as_shape().shape.client_size.y);
			if (pGPUWindow)
				pGPUWindow->client_size(_Event.as_shape().shape.client_size);
			CheckState(_Event.as_shape().shape.state);
			CheckStyle(_Event.as_shape().shape.style);
			break;
		}
		case event_type::closing:
			gpuwin.stop();
			break;

		default:
			break;
	}
}

void oGfxViewApp::ActionHook(const input::action& _Action)
{
	switch (_Action.action_type)
	{
		case input::menu:
		{
			switch (_Action.device_id)
			{
				case oWMI_FILE_EXIT:
					gpuwin.stop();
					break;
				case oWMI_VIEW_EXCLUSIVE:
				{
					const bool checked = menu::checked(Menus[oWMENU_VIEW], _Action.device_id);
					menu::check(Menus[oWMENU_VIEW], _Action.device_id, !checked);
					AppWindow->set_status_text(1, "Fullscreen %s", !checked ? "exclusive" : "cooperative");
					break;
				}
				case oWMI_HELP_ABOUT:
				{
					oDECLARE_ABOUT_INFO(i, load_icon(IDI_APPICON));
					std::shared_ptr<about> a = about::make(i);
					a->show_modal(AppWindow);
					break;
				}
				default:
					EnumRadioHandler.on_action(_Action);
					break;
			}
			break;
		}

		case input::hotkey:
		{
			switch (_Action.device_id)
			{
				case oWHK_TOGGLE_UI_MODE:
					SetUIMode(!UIMode);
					break;

				case oWHK_DEFAULT_STYLE:
				{
					if (UIMode)
					{
						if (AppWindow->state() == window_state::fullscreen)
							AppWindow->state(window_state::restored);
						AppWindow->style(window_style::sizable_with_menu_and_statusbar);
					}
					break;
				}
				case oWHK_TOGGLE_FULLSCREEN:
				{
					if (UIMode)
						ToggleFullscreenCooperative(AppWindow.get());
					else
					{
						const bool checked = menu::checked(Menus[oWMENU_VIEW], oWMI_VIEW_EXCLUSIVE);
						if (checked)
						{
							const bool GoFullscreen = !gpuwin.is_fullscreen_exclusive();
							try { gpuwin.set_fullscreen_exclusive(GoFullscreen); }
							catch (std::exception& e) { oTRACEA("set_fullscreen_exclusive(%s) failed: %s", GoFullscreen ? "true" : "false", e.what()); }
							AllowUIModeChange = !GoFullscreen;
						}
						else
							ToggleFullscreenCooperative(pGPUWindow);
					}
					break;
				}
				default:
					oTRACE("input::hotkey");
					break;
			}
			
			break;
		}

		default:
			break;
	}
}

void oGfxViewApp::Run()
{
	try
	{
		while (Running)
			AppWindow->flush_messages();
	}
	catch (std::exception& e)
	{
		msgbox(msg_type::info, nullptr, sAppName, "ERROR\n%s", e.what());
	}
}

int main(int argc, const char* argv[])
{
	reporting::set_prompter(prompt_msgbox);
	oGfxViewApp App;
	App.Run();
	return 0;
}
