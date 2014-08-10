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
#include <oBase/backoff.h>
#include <oGUI/msgbox.h>
#include <oGUI/msgbox_reporting.h>
#include <oGUI/Windows/win_gdi_bitmap.h>
#include <oGUI/Windows/win_common_dialog.h>
#include <oGUI/oGUIMenu.h>
#include <oGUI/window.h>
#include "resource.h"

#include "../about_ouroboros.h"

#include <oGfx/core.h>
#include <oGfx/render_window.h>
#include <oGPU/all.h>
#include <oSurface/codec.h>

using namespace ouro;
using namespace windows::gdi;

static const char* sAppName = "oTexView";

enum oWMENU
{
	oWMENU_FILE,
	oWMENU_EDIT,
	oWMENU_VIEW,
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
	{ oWMENU_TOPLEVEL, oWMENU_HELP, "&Help" },
};
static_assert(oCOUNTOF(sMenuHier) == oWMENU_COUNT, "array mismatch");

enum oWHOTKEY
{
	oWHK_FILE_OPEN,
	oWHK_TOGGLE_UI_MODE,
	oWHK_DEFAULT_STYLE,
};

enum oWMI // menuitems
{
	oWMI_FILE_OPEN,
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
	{ input::o, oWHK_FILE_OPEN, false, true, false },
};

class surface_view
{
public:
	surface_view() : core(nullptr), active(nullptr) {}
	~surface_view() { deinitialize(); }

	void initialize(gfx::core& core);
	void deinitialize();

	void set_texels(const char* name, const surface::texel_buffer& b);

	inline void set_draw_target(gpu::primary_target* t) { ctarget = t; }

	void render();

private:
	gfx::core* core;
	gpu::resource* active;
	gpu::texture1d t1d;
	gpu::texture2d t2d;
	gpu::texture3d t3d;
	gpu::texturecube tcube;
	surface::info inf;

	gpu::primary_target* ctarget;
	gpu::vertex_shader vs;
	gpu::pixel_shader ps;
};

void surface_view::initialize(gfx::core& _core)
{
	core = &_core;
	active = nullptr;
	vs.initialize("fullscreen_tri", core->device, gpu::intrinsic::byte_code(gpu::intrinsic::vertex_shader::fullscreen_tri));
	ps.initialize("texture", core->device, gpu::intrinsic::byte_code(gpu::intrinsic::pixel_shader::texture2d));
}

void surface_view::deinitialize()
{
	t1d.deinitialize();
	t2d.deinitialize();
	t3d.deinitialize();
	tcube.deinitialize();
	ps.deinitialize();
	vs.deinitialize();
	active = nullptr;
	core = nullptr;
}

void surface_view::set_texels(const char* name, const surface::texel_buffer& b)
{
	t1d.deinitialize();
	t2d.deinitialize();
	t3d.deinitialize();
	tcube.deinitialize();

	inf = b.get_info();

	if (inf.is_1d())
	{
		t1d.initialize(name, core->device, b, inf.mips());
		active = &t1d;
		oTHROW(operation_not_supported, "1d viewing not yet enabled");
	}

	else if (inf.is_2d())
	{
		t2d.initialize(name, core->device, b, inf.mips());
		active = &t2d;
	}

	else if (inf.is_3d())
	{
		t3d.initialize(name, core->device, b, inf.mips());
		active = &t3d;
		oTHROW(operation_not_supported, "3d slice viewing not yet enabled");
	}

	else if (inf.is_cube())
	{
		tcube.initialize(name, core->device, b, inf.mips());
		active = &tcube;
		oTHROW(operation_not_supported, "cube slice viewing not yet enabled");
	}
}

void surface_view::render()
{
	if (!ctarget || !*ctarget)
		return;

	auto& c = *core;
	auto& ct = *ctarget;
	auto& cl = c.device.immediate();

	ct.clear(cl, black);
	ct.set_draw_target(cl);

	if (active)
	{
		c.bs.set(cl, gpu::blend_state::opaque);
		c.dss.set(cl, gpu::depth_stencil_state::none);
		c.rs.set(cl, gpu::rasterizer_state::front_face);
		c.ss.set(cl, gpu::sampler_state::linear_wrap, gpu::sampler_state::linear_wrap);
		c.ls.set(cl, gpu::intrinsic::vertex_layout::none, mesh::primitive_type::triangles);
		vs.set(cl);
		ps.set(cl);
		active->set(cl, 0);
		gpu::vertex_buffer::draw_unindexed(cl, 3);
	}

	ct.present();
}

class oGPUWindowTestApp
{
public:
	oGPUWindowTestApp();
	~oGPUWindowTestApp();

	void Run();

private:
	std::shared_ptr<window> AppWindow;
	window* pGPUWindow;

	gfx::core gfxcore;
	gfx::render_window gpuwin;
	surface_view sv;

	// @tony: this is a bit lame. Everything is currently using the immediate context
	// which cannot be accessed from multiple threads even with a multi-threaded device.
	// so assign data to this loaded value and kick something to consume it.
	surface::texel_buffer loaded;

	menu_handle Menus[oWMENU_COUNT];
	oGUIMenuEnumRadioListHandler MERL;
	window_state::value PreFullscreenState;
	bool Running;
	bool UIMode;
	bool AllowUIModeChange;
private:
	void ActionHook(const input::action& _Action);
	void AppEventHook(const window::basic_event& _Event);
	bool CreateMenus(const window::create_event& _CreateEvent);
	void open_file();
};

oGPUWindowTestApp::oGPUWindowTestApp()
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
		i.on_action = std::bind(&oGPUWindowTestApp::ActionHook, this, std::placeholders::_1);
		i.on_event = std::bind(&oGPUWindowTestApp::AppEventHook, this, std::placeholders::_1);
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
	
	sv.initialize(gfxcore);

	// Set up child window so the app can have Win32 GUI and GPU rendering
	{
		gfx::render_window_init i(sAppName);
		i.icon = (icon_handle)load_icon(IDI_APPICON);
		i.hotkeys = HotKeys;
		i.num_hotkeys = oCOUNTOF(HotKeys);
		i.color_format = surface::format::r8g8b8a8_unorm;
		i.depth_format = surface::format::d24_unorm_s8_uint;
		i.on_action = std::bind(&oGPUWindowTestApp::ActionHook, this, std::placeholders::_1);
		i.on_stop = [&] { Running = false; };
		i.render = std::bind(&surface_view::render, &sv);
		i.parent = AppWindow;

		pGPUWindow = gpuwin.start(gfxcore.device, i);
	}

	// attach render app to window rendering
	auto& ct = gpuwin.color_target();
	sv.set_draw_target(&ct);
	AppWindow->show();
}

oGPUWindowTestApp::~oGPUWindowTestApp()
{
	filesystem::join();
}

bool oGPUWindowTestApp::CreateMenus(const window::create_event& _CreateEvent)
{
	for (auto& m : Menus)
		m = oGUIMenuCreate();

	for (const auto& h : sMenuHier)
	{
		oGUIMenuAppendSubmenu(
			h.Parent == oWMENU_TOPLEVEL ? _CreateEvent.menu : Menus[h.Parent]
		, Menus[h.Menu], h.Name);
	}

	// File menu
	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_OPEN, "&Open...\tCtrl+O");
	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	// Edit menu
	// (nothing yet)

	// View menu
	// (nothing yet)

	// Help menu
	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "About...");
	return true;
}

void oGPUWindowTestApp::AppEventHook(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
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
			break;
		}

		case event_type::closing:
			gpuwin.stop();
			break;

		default:
			break;
	}
}

void oGPUWindowTestApp::ActionHook(const input::action& _Action)
{
	switch (_Action.action_type)
	{
		case input::menu:
		{
			switch (_Action.device_id)
			{
				case oWMI_FILE_OPEN:
					open_file();
					break;

				case oWMI_FILE_EXIT:
					gpuwin.stop();
					break;

				case oWMI_HELP_ABOUT:
				{
					oDECLARE_ABOUT_INFO(i, load_icon(IDI_APPICON));
					std::shared_ptr<about> a = about::make(i);
					a->show_modal(AppWindow);
					break;
				}
				default:
					MERL.OnAction(_Action);
					break;
			}
			break;
		}

		case input::hotkey:
		{
			switch (_Action.device_id)
			{
				case oWHK_FILE_OPEN:
					open_file();
					break;

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

void oGPUWindowTestApp::open_file()
{
	const char* sSupportedFormats = 
		"Supported Image Types|*.bmp;*.dds;*.jpg;*.png;*.psd;*.tga" \
		"|All Files Types|*.*"\
		"|Bitmap Files|*.bmp" \
		"|DDS Files|*.dds" \
		"|JPG/JPEG Files|*.jpg" \
		"|PNG Files|*.png" \
		"|Photoshop Files|*.psd" \
		"|Targa Files|*.tga";

	path p(filesystem::data_path());
	if (!windows::common_dialog::open_path(p, "Open Texture", sSupportedFormats, (HWND)AppWindow->native_handle()))
		return;

	try
	{
		{
			auto decoded = surface::decode(filesystem::load(p));
			auto inf = decoded.get_info();

			if (is_texture(inf.format))
				loaded = std::move(decoded);
			else
			{
				auto converted = decoded.convert(surface::format::b8g8r8a8_unorm);
				loaded = std::move(converted);
			}
		}
		
		pGPUWindow->dispatch([&]
		{ 
			try
			{
				sv.set_texels("test", loaded);
				auto inf = loaded.get_info();
				AppWindow->set_title("%s - %s", sAppName, p.c_str());
				AppWindow->client_size(inf.dimensions.xy());
			}

			catch (std::exception& e)
			{
				msgbox(msg_type::info, AppWindow->native_handle(), sAppName, "Opening %s failed:\n%s", p.c_str(), e.what());
			}

			loaded.deinitialize();
		});
	}

	catch (std::exception& e)
	{
		msgbox(msg_type::info, AppWindow->native_handle(), sAppName, "Opening %s failed:\n%s", p.c_str(), e.what());
	}
}

void oGPUWindowTestApp::Run()
{
KeepGoing:
	try
	{
		while (Running)
			AppWindow->flush_messages();
	}
	catch (std::exception& e)
	{
		msgbox(msg_type::info, nullptr, "oGPUWindowTestApp", "ERROR\n%s", e.what());
		goto KeepGoing;
	}
}

int main(int argc, const char* argv[])
{
	reporting::set_prompter(prompt_msgbox);

	oGPUWindowTestApp App;
	App.Run();
	return 0;
}
