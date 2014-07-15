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
#include <oGUI/oGUIMenu.h>
#include <oGUI/window.h>
#include "resource.h"

#include "../about_ouroboros.h"

// Whereas oGPU is an abstraction of modern hardware-accelerated graphics APIs,
// oGfx is an abstraction of a particular rendering policy. It so happens to 
// include some simple shaders, so rather than reproduce such shaders, just use
// them from the oGfx library.
#include <oGfx/oGfxShaders.h>
#include <oGfx/oGfxShaderRegistry.h>

#include <oGPU/all.h>
#include <oGPU/oGPUUtilMesh.h>

using namespace ouro;
using namespace windows::gdi;

// This function treats shader_source as hlsl and it then iterates on all entry points which are defined to be all functions
// that begin with two letter describing how it should be compiled: VS for vertex shader, PS for pixel shader (DS HS GS CS etc.)
// The specified enumerator is called on each one.
void enumerate_shader_entries(const char* shader_source, const std::function<void(const char* entry_point, gpu::stage::value stage)>& enumerator)
{
	// Make a copy so we can delete all the irrelevant parts for easier parsing
	std::string ForParsing(shader_source);
	char* p = (char*)ForParsing.c_str();

	// Simplify code of anything that might not be a top-level function.
	zero_non_code(p, nullptr);
	zero_block_comments(p, "{", "}"); // ignore function/struct bodies
	zero_block_comments(p, "[", "]"); // ignore hlsl compiler hints
	zero_line_comments(p, "#"); // ignore preprocessing #defines/#includes
	zero_line_comments(p, ":"); // ignore semantics

	// assume every non-space-containing identifier in front of '(' is a 
	// function name.
	sstring Entry;
	while (*p)
	{
		p = strstr(p, "(");
		if (!p)
			break;

		size_t n = rstrcspn(ForParsing.c_str(), p, " \t");

		if (n)
		{
			Entry.assign(p-n, p);

			if (!Entry.empty() && (Entry[1] == 's' || Entry[1] == 'S'))
			{
				gpu::stage::value Stage;
				static const char* sProfiles = "vhdgpc";
				const char* Profile = strchr(sProfiles, tolower(Entry[0]));
				if (Profile)
				{
					Stage = gpu::stage::value(Profile - sProfiles);
					enumerator(Entry, Stage);
				}
			}
		}
		p++;
	}
}

void shader_on_loaded(gfx::vs_registry& vs, gfx::ps_registry& ps, const path& p, scoped_allocation& b, const std::system_error* err)
{
	if (err)
	{
		oTRACEA("Load '%s' failed: %s", p.c_str(), err->what());
		return;
	}

	lstring IncludePaths, Defines;
	IncludePaths += filesystem::dev_path() / "Ouroboros/Include;";

	enumerate_shader_entries((char*)b, [&](const char* entry_point, gpu::stage::value stage)
	{
		if (stage == vs.stage) vs.compile(IncludePaths, Defines, p, (const char*)b, entry_point);
		if (stage == ps.stage) ps.compile(IncludePaths, Defines, p, (const char*)b, entry_point);
	});
}

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

struct oGPUWindowClearToggle
{
	oGPUWindowClearToggle(color _Color1, color _Color2)
	{
		Color[0] = _Color1;
		Color[1] = _Color2;
	}

	color Color[2];
};

basic_hotkey_info HotKeys[] =
{
	// reset style
	{ input::f2, oWHK_TOGGLE_UI_MODE, false, false, false },
	{ input::f3, oWHK_DEFAULT_STYLE, false, false, false },
	{ input::enter, oWHK_TOGGLE_FULLSCREEN, true, false, false },
};

class oGPUWindowThread
{
public:
	oGPUWindowThread();
	~oGPUWindowThread();

	// Returns a handle to the window. This should not be ref-counted. Application
	// logic should ensure the lifetime of this window comes and goes before the
	// owning/parent application windows since it is pretty much as child thread 
	// and thus is created after the main thread (app window) and must be joined 
	// before the app/thread exits.
	window* Start(const std::shared_ptr<window>& _Parent, const input::action_hook& _OnAction, const std::function<void()>& _OnThreadExit);
	void Stop();

	gpu::device& GetDevice() { return Device; }
	void SetClearColor(const color& c) { ClearColor = c; }
	color GetClearColor() const { return ClearColor; }

	bool IsFullscreenExclusive() const { return WindowColorTarget.is_fullscreen_exclusive(); }
	void SetFullscreenExclusive(bool fullscreen) { return WindowColorTarget.set_fullscreen_exclusive(fullscreen); }

private:
	void OnEvent(const window::basic_event& _Event);
	void Run();
	void Render();

private:
	std::shared_ptr<window> Parent;
	gpu::device Device;
	gpu::command_list cl;
	gpu::blend_state BlendState;
	gpu::depth_stencil_state DepthStencilState;
	gpu::rasterizer_state RasterizerState;
	gpu::sampler_state SamplerState;
	gpu::primary_target WindowColorTarget;
	gpu::depth_target WindowDepthTarget;
	gpu::util_mesh Mesh;

	// stubs until registries are fully debugged
	gpu::vertex_shader VertexShader;
	gpu::pixel_shader PixelShader;

	gfx::layout_state LayoutState;
	gfx::vs_registry VertexShaders;
	gfx::ps_registry PixelShaders;

	window* pGPUWindow;
	std::thread Thread;
	color ClearColor;
	bool Running;

	std::function<void()> OnThreadExit;
	input::action_hook OnAction;
};

oGPUWindowThread::oGPUWindowThread()
	: pGPUWindow(nullptr)
	, Running(true)
	, ClearColor(black)
{
	gpu::device_init di;
	di.enable_driver_reporting = true;
	try { Device.initialize(di); }
	catch (std::exception&)
	{
		Running = false;
		return;
	}

	LayoutState.initialize(Device);
	VertexShaders.initialize(Device);
	PixelShaders.initialize(Device);

	VertexShader.initialize("VS", Device, gfx::byte_code(gfx::vertex_shader::pass_through_pos));
	PixelShader.initialize("PS", Device, gfx::byte_code(gfx::pixel_shader::white));

	// jist: load the library file knowing all registries where content will go.
	// there is a registry per shader type. Shaders are registered by entry point 
	// name.
	filesystem::load_async(filesystem::dev_path() / "Ouroboros/Source/oGfx/oGfxShaders.hlsl"
		, std::bind(shader_on_loaded, std::ref(VertexShaders), std::ref(PixelShaders), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		, filesystem::load_option::text_read);

	BlendState.initialize(Device);
	DepthStencilState.initialize(Device);
	RasterizerState.initialize(Device);
	SamplerState.initialize(Device);

	Mesh.initialize_first_triangle(Device);

	//filesystem::join();
}

oGPUWindowThread::~oGPUWindowThread()
{
	Stop();
	OnAction = nullptr;
	Parent = nullptr;
	Thread.join();
	filesystem::join(); // ensure all loading is done
	PixelShaders.deinitialize();
	VertexShaders.deinitialize();
	LayoutState.deinitialize();
}

window* oGPUWindowThread::Start(const std::shared_ptr<window>& _Parent, const input::action_hook& _OnAction, const std::function<void()>& _OnThreadExit)
{
	Parent = _Parent;
	OnAction = _OnAction;
	OnThreadExit = _OnThreadExit;
	Thread = std::thread(&oGPUWindowThread::Run, this);

	backoff bo;
	while (!pGPUWindow)
		bo.pause();
	return pGPUWindow;
}

void oGPUWindowThread::Stop()
{
	Running = false;
}

void oGPUWindowThread::OnEvent(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case event_type::sized:
		{
			if (WindowColorTarget)
			{
				WindowColorTarget.resize(_Event.as_shape().shape.client_size);
				WindowDepthTarget.resize(_Event.as_shape().shape.client_size);
			}
			break;
		}

		case event_type::closing:
			Running = false;
			break;

	default:
		break;
	}
}

void oGPUWindowThread::Run()
{
	core_thread_traits::begin_thread("Window Render Target Thread");
	{
		std::shared_ptr<window> GPUWindow;

		// Set up child window as a render target (this allows other client-area 
		// controls to be drawn by parent since the primary render target consumes 
		// the entire client area).
		{
			window::init i;
			i.title = "Render Target Window";
			i.icon = (icon_handle)load_icon(IDI_APPICON);
			i.on_action = OnAction;
			i.on_event = std::bind(&oGPUWindowThread::OnEvent, this, std::placeholders::_1);
			i.shape.client_position = int2(0, 0); // important to think client-relative for this
			i.shape.client_size = int2(256, 256); // @tony: Try making this 1,1 and see if a resize takes over
			i.shape.state = window_state::hidden; // don't show the window before it is child-ized
			i.shape.style = window_style::borderless;
			i.cursor_state = cursor_state::hand;
			i.alt_f4_closes = true;
			GPUWindow = window::make(i);
			GPUWindow->set_hotkeys(HotKeys);
			WindowColorTarget.initialize(GPUWindow.get(), Device, true);
			uint2 dimensions = WindowColorTarget.dimensions();
			WindowDepthTarget.initialize("primary depth", Device, surface::d24_unorm_s8_uint, dimensions.x, dimensions.y, 0, false, 0);
			GPUWindow->parent(Parent);
			GPUWindow->show(); // now that the window is a child, show it (it will only show when parent shows)
			pGPUWindow = GPUWindow.get();
		}

		try
		{
			while (Running)
			{
				GPUWindow->flush_messages();
				Render();
			}
		}

		catch (std::exception& e)
		{
			msgbox(msg_type::info, nullptr, "oGPUWindowTestApp", "ERROR\n%s", e.what());
		}

		WindowColorTarget.deinitialize();
		WindowDepthTarget.deinitialize();
		pGPUWindow = nullptr;
	}
	if (OnThreadExit)
		OnThreadExit();
	core_thread_traits::end_thread();
}

void oGPUWindowThread::Render()
{
	if (WindowColorTarget)
	{
		gpu::command_list& cl = Device.immediate();

		WindowColorTarget.clear(cl, ClearColor);
		WindowColorTarget.set_draw_target(cl, WindowDepthTarget);
		
		BlendState.set(cl, gpu::blend_state::opaque);
		DepthStencilState.set(cl, gpu::depth_stencil_state::none);
		RasterizerState.set(cl, gpu::rasterizer_state::front_face);
		SamplerState.set(cl, gpu::sampler_state::linear_wrap, gpu::sampler_state::linear_wrap);
		
		LayoutState.set(cl, gfx::vertex_input::pos, mesh::primitive_type::triangles);
		VertexShader.set(cl);
		PixelShader.set(cl);

		Mesh.draw(cl);
		cl;
		WindowColorTarget.present();
	}
}

class oGPUWindowTestApp
{
public:
	oGPUWindowTestApp();

	void Run();

private:
	std::shared_ptr<window> AppWindow;
	window* pGPUWindow;
	oGPUWindowThread GPUWindow;

	menu_handle Menus[oWMENU_COUNT];
	oGPUWindowClearToggle ClearToggle;
	oGUIMenuEnumRadioListHandler MERL;
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

oGPUWindowTestApp::oGPUWindowTestApp()
	: pGPUWindow(nullptr)
	, Running(true)
	, UIMode(true)
	, AllowUIModeChange(true)
	, PreFullscreenState(window_state::hidden)
	, ClearToggle(green, red)
{
	// Set up application window
	{
		window::init i;
		i.title = "oGPUWindowTestApp";
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
		AppWindow->start_timer((uintptr_t)&ClearToggle, 1000);

		const int sSections[] = { 120, -1 };
		AppWindow->set_num_status_sections(sSections, oCOUNTOF(sSections));
		AppWindow->set_status_text(0, "F3 for default style");
		AppWindow->set_status_text(1, "Fullscreen cooperative");
	}

	// Now set up separate child thread for rendering. This allows UI to be 
	// detached from potentially slow rendering.
	pGPUWindow = GPUWindow.Start(AppWindow, std::bind(&oGPUWindowTestApp::ActionHook, this, std::placeholders::_1), [&] { Running = false; });
	GPUWindow.SetClearColor(ClearToggle.Color[0]);
	AppWindow->show();
}

void oGPUWindowTestApp::CheckState(window_state::value _State)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oGPUWindowTestApp::CheckStyle(window_style::value _Style)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE]
	, oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oWMI_VIEW_STYLE_FIRST + _Style);
}

void oGPUWindowTestApp::EnableStatusBarStyles(bool _Enabled)
{
	// Enable styles not allowed for render target windows
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::fixed_with_statusbar, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::fixed_with_menu_and_statusbar, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::sizable_with_statusbar, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + window_style::sizable_with_menu_and_statusbar, _Enabled);
}

void oGPUWindowTestApp::SetUIModeInternal(bool _UIMode, const window_shape& _CurrentAppShape, const window_shape& _CurrentGPUShape)
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

void oGPUWindowTestApp::SetUIMode(bool _UIMode)
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

void oGPUWindowTestApp::ToggleFullscreenCooperative(window* _pWindow)
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
	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	// Edit menu
	// (nothing yet)

	// View menu
	oGUIMenuAppendEnumItems(window_style::count, Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, _CreateEvent.shape.style);
	EnableStatusBarStyles(true);

	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle) { AppWindow->style((window_style::value)_BorderStyle); });
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST
		, oWMI_VIEW_STYLE_FIRST + window_style::sizable_with_menu);

	oGUIMenuAppendEnumItems(window_state::count, Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, _CreateEvent.shape.state);
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State) { AppWindow->show((window_state::value)_State); });

	oGUIMenuAppendItem(Menus[oWMENU_VIEW], oWMI_VIEW_EXCLUSIVE, "Fullscreen E&xclusive");

	// Help menu
	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "About...");
	return true;
}

void oGPUWindowTestApp::AppEventHook(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case event_type::timer:
			if (_Event.as_timer().context == (uintptr_t)&ClearToggle)
			{
				if (GPUWindow.GetClearColor() == ClearToggle.Color[0]) GPUWindow.SetClearColor(ClearToggle.Color[1]);
				else GPUWindow.SetClearColor(ClearToggle.Color[0]);
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
			GPUWindow.Stop();
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
				case oWMI_FILE_EXIT:
					GPUWindow.Stop();
					break;
				case oWMI_VIEW_EXCLUSIVE:
				{
					const bool checked = oGUIMenuIsChecked(Menus[oWMENU_VIEW], _Action.device_id);
					oGUIMenuCheck(Menus[oWMENU_VIEW], _Action.device_id, !checked);
					AppWindow->set_status_text(1, "Fullscreen %s", !checked ? "exclusive" : "cooperative");
					break;
				}
				case oWMI_HELP_ABOUT:
				{
					#if 1
						oDECLARE_ABOUT_INFO(i, load_icon(IDI_APPICON));
						std::shared_ptr<about> a = about::make(i);
						a->show_modal(AppWindow);
					#else
						msgbox(msg_type::info, _Action.window, "About", "oGPUWindowTestApp: a small program to show a basic window and its events and actions");
					#endif
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
						const bool checked = oGUIMenuIsChecked(Menus[oWMENU_VIEW], oWMI_VIEW_EXCLUSIVE);
						if (checked)
						{
							const bool GoFullscreen = !GPUWindow.IsFullscreenExclusive();
							try { GPUWindow.SetFullscreenExclusive(GoFullscreen); }
							catch (std::exception& e) { oTRACEA("SetFullscreenExclusive(%s) failed: %s", GoFullscreen ? "true" : "false", e.what()); }
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

void oGPUWindowTestApp::Run()
{
	try
	{
		while (Running)
			AppWindow->flush_messages();
	}
	catch (std::exception& e)
	{
		msgbox(msg_type::info, nullptr, "oGPUWindowTestApp", "ERROR\n%s", e.what());
	}
}

int main(int argc, const char* argv[])
{
	reporting::set_prompter(prompt_msgbox);

	oGPUWindowTestApp App;
	App.Run();
	return 0;
}
