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
#include "oGfx/oGfxPipelines.h"

#include <oGPU/oGPU.h>
#include <oGPU/oGPUUtil.h>
#include <oGPU/oGPUUtilMesh.h>

#if 0
// everything is a lib of multiple assets

// 1. request load
// 2. async load calls cont
// 3. cont calls forker -> forker analyzes file format for subassets
// 4. for each asset, call compile(type, file, subtypeptr)
// 5. each asset once compiled queues binary



class asset_factory_interface
{
public:
	virtual int get_type(const path& _Path) const = 0;
	virtual int default_asset(int _Type) const = 0;
	virtual int error_asset(int _Type) const = 0;

	virtual void* allocate(size_t _NumBytes, unsigned int _Options) = 0;
	virtual void deallocate(const void* _Pointer) = 0;

	virtual scoped_allocation compile(int _Type, scoped_allocation& _MemFile) = 0;
	virtual void* make_device_object(int _Type, scoped_allocation& _Compiled) = 0;
	virtual void update_device_object(int _Type, void* _pDeviceObject) = 0;
	virtual void unmake_device_object(int _Type, void* _pDeviceObject) = 0;
};


namespace asset_type
{	enum value {

	unknown = 0, // required value

	shader,
	texture,

	count,

};}

void* asset_allocate(size_t _NumBytes, unsigned int _Options)
{
	allocate_options o; o.options = _Options;
	memory_type::value MemType = o.type;
	int AssetType = o.extra;
	size_t alignment = o.get_alignment();

	switch (AssetType)
	{
		case asset_type::shader:
		case asset_type::texture:
			break;
	}

	return malloc(_NumBytes);
};

void asset_deallocate(const void* _Pointer)
{
	free(const_cast<void*>(_Pointer));
}

scoped_allocation asset_compile(int _Type, scoped_allocation& _MemFile)
{
	switch (_Type)
	{
		case asset_type::shader:
			// for each entry, compile entry... each needs to be its own scoped_allocations
			// though I could allocate an array of binaries
			break;
		case asset_type::texture:

			// pump allocator through surface code
			std::shared_ptr<buffer> compiled = decode(_MemFile, _MemFile.size());
			break;
	}
}


class asset_manager
{
public:
	int load(const path& _Path);

	std::function<void(const path& _Path, scoped_allocation& _Buffer, size_t _Size)> OnLoaded;
};

asset_manager::asset_manager()
{
	OnLoaded = std::bind(&asset_manager::on_loaded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
};

void asset_manager::on_loaded(const path& _Path, scoped_allocation& _Buffer, size_t _Size)
{
	oTRACEA("Load complete: %s %u bytes", _Path.c_str(), _Size);
}

int asset_manager::load(const path& _Path)
{
	bool IsNew = false;
	int& id = *Lookup.get_existing_or_new_ptr(_Path.hash(), &IsNew);
	if (IsNew) id = Pool.allocate();

	auto& runtime = RuntimeEntries[id];
	auto& lifetime = LifetimeEntries[id];
	auto modified = filesystem::last_write_time(_Path);
	
	if (modified != lifetime.modified)
	{
		runtime.type = get_type(_Path);
		runtime.state = asset_state::loading;
		lifetime.modified = modified;
		lifetime.path = _Path;
		filesystem::load_async(_Path, OnLoaded, is_text(runtime.type) ? filesystem::load_option::text_read : filesystem::load_option::binary_read, Allocator);
	}

	return id;
}

bool asset_manager::reload(const char& _Path)
{
	bool IsNew = false;
	int& id = *Lookup.get_existing_or_new_ptr(_Path.hash(), &IsNew);
	if (IsNew) return false;

	auto& runtime = RuntimeEntries[id];
	auto& lifetime = LifetimeEntries[id];
	auto modified = filesystem::last_write_time(_Path);
	
	if (modified != lifetime.modified)
	{
		runtime.type = get_type(_Path);
		runtime.state = asset_state::loading;
		lifetime.modified = modified;
		lifetime.path = _Path;
		filesystem::load_async(_Path, OnLoaded, is_text(runtime.type) ? filesystem::load_option::text_read : filesystem::load_option::binary_read, Allocator);
		return true;
	}

	return false;
}

void asset_manager::reload(int _ID)
{
	if (!Pool.valid(_ID))
		throw std::invalid_argument("invalid id");
	auto& runtime RuntimeEntries[_ID];
	auto& lifetime LifetimeEntries[_ID];

	auto modified = filesystem::last_write_time(lifetime.Path);
	
	if (modified != lifetime.modified)
	{
		runtime.state = asset_state::loading;
		lifetime.modified = modified;
		filesystem::load_async(lifetime.Path, OnLoaded, is_text(runtime.type) ? filesystem::load_option::text_read : filesystem::load_option::binary_read, Allocator);
	}
}

int asset_manager::make(const path& _Path, scoped_allocation& _Compiled)
{
	bool IsNew = false;
	int id = *Lookup.get_existing_or_new_ptr(_Path.hash(), &IsNew);
	if (!IsNew)
		return invalid;

	auto& runtime = RuntimeEntries[id];
	auto& lifetime = LifetimeEntries[id];

	lifetime.modified = time(nullptr);
	lifetime.path = _Path;
	runtime.state = asset_state::make_queued;
	runtime.type = get_type(_Path);
	runtime.entry = std::move(_Compiled);
	MakeQueue.push(id);
	return id;
}

bool asset_manager::update(int _ID, scoped_allocation& _Compiled)
{
	bool IsNew = false;
	int id = *Lookup.get_existing_or_new_ptr(_Path.hash(), &IsNew);
	if (IsNew) return false;

	auto& runtime = RuntimeEntries[id];
	auto& lifetime = LifetimeEntries[id];

	lifetime.modified = time(nullptr);
	runtime.entry = std::move(_Compiled);
	runtime.state = asset_state::update_queued;
	UpdateQueue.push(id);
	return id;
}

void asset_manager::remove(int _ID)
{
	if (!Pool.valid(_ID))
		throw std::invalid_argument("invalid id");
	auto& runtime = RuntimeEntries[_ID];
	runtime.state = asset_state::unmake_queued;
	UnmakeQueue.push(_ID);
}

void asset_manager::flush_lifetimes(unsigned int _MaxNumMakes, unsigned int _MaxNumUnmakes)
{
	int id = invalid;
	while (_MaxNumUnmakes && UnmakeQueue.pop(id))
	{
		auto& runtime = RuntimeEntries[id];
		auto& lifetime = LifetimeEntries[id];
		oASSERT(runtime.state == asset_state::unmake_queued, "");
		unmake(lifetime.type, runtime.entry);
		_MaxNumUnmakes--;
	}
	
	while (_MaxNumMakes > 0 && MakeQueue.pop(id))
	{
		auto& runtime = RuntimeEntries[id];
		auto& lifetime = LifetimeEntries[id];
		oASSERT(runtime.state == asset_state::update_queued, "");
		runtime.entry = std::move(make(lifetype.type, runtime.entry));
		_MaxNumMakes--;
	}
}


#endif

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
	oWMI_VIEW_STYLE_LAST = oWMI_VIEW_STYLE_FIRST + ouro::window_style::count - 1,

	oWMI_VIEW_STATE_FIRST,
	oWMI_VIEW_STATE_LAST = oWMI_VIEW_STATE_FIRST + ouro::window_state::count - 1,

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

ouro::basic_hotkey_info HotKeys[] =
{
	// reset style
	{ ouro::input::f2, oWHK_TOGGLE_UI_MODE, false, false, false },
	{ ouro::input::f3, oWHK_DEFAULT_STYLE, false, false, false },
	{ ouro::input::enter, oWHK_TOGGLE_FULLSCREEN, true, false, false },
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

	ouro::gpu::device* GetDevice() { return Device.get(); }
	ouro::gpu::render_target* GetRenderTarget() { return WindowRenderTarget.get(); }

private:
	void OnEvent(const window::basic_event& _Event);
	void Run();
	void Render();

private:
	std::shared_ptr<window> Parent;
	std::shared_ptr<ouro::gpu::device> Device;
	std::shared_ptr<ouro::gpu::command_list> CommandList;
	std::shared_ptr<ouro::gpu::render_target> WindowRenderTarget;
	std::shared_ptr<ouro::gpu::pipeline1> Pipeline;
	std::shared_ptr<ouro::gpu::util_mesh> Mesh;

	window* pGPUWindow;
	std::thread Thread;
	bool Running;

	std::function<void()> OnThreadExit;
	input::action_hook OnAction;
};

oGPUWindowThread::oGPUWindowThread()
	: pGPUWindow(nullptr)
	, Running(true)
{
	ouro::gpu::device_init di;
	di.driver_debug_level = gpu::debug_level::normal;
	try { Device = ouro::gpu::device::make(di); }
	catch (std::exception&)
	{
		Running = false;
		return;
	}

	Pipeline = Device->make_pipeline1(oGfxGetPipeline(oGFX_PIPELINE_PASS_THROUGH));
	Mesh = gpu::make_first_triangle(Device);
	CommandList = Device->get_immediate_command_list();
}

oGPUWindowThread::~oGPUWindowThread()
{
	Stop();
	OnAction = nullptr;
	Parent = nullptr;
	Thread.join();
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
		case ouro::event_type::sized:
		{
			if (WindowRenderTarget)
				WindowRenderTarget->resize(int3(_Event.as_shape().shape.client_size, 1));
			break;
		}

		case ouro::event_type::closing:
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
			i.icon = (ouro::icon_handle)load_icon(IDI_APPICON);
			i.on_action = OnAction;
			i.on_event = std::bind(&oGPUWindowThread::OnEvent, this, std::placeholders::_1);
			i.shape.client_position = int2(0, 0); // important to think client-relative for this
			i.shape.client_size = int2(256, 256); // @tony: Try making this 1,1 and see if a resize takes over
			i.shape.state = ouro::window_state::hidden; // don't show the window before it is child-ized
			i.shape.style = ouro::window_style::borderless;
			i.cursor_state = ouro::cursor_state::hand;
			i.alt_f4_closes = true;
			GPUWindow = window::make(i);
			GPUWindow->set_hotkeys(HotKeys);
			WindowRenderTarget = Device->make_primary_render_target(GPUWindow, ouro::surface::d24_unorm_s8_uint, true);
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
			ouro::msgbox(ouro::msg_type::info, nullptr, "oGPUWindowTestApp", "ERROR\n%s", e.what());
		}

		WindowRenderTarget = nullptr;
		pGPUWindow = nullptr;
	}
	if (OnThreadExit)
		OnThreadExit();
	core_thread_traits::end_thread();
}

void oGPUWindowThread::Render()
{
	if (WindowRenderTarget && Device->begin_frame())
	{
		CommandList->begin();
		CommandList->set_render_target(WindowRenderTarget);
		CommandList->clear(WindowRenderTarget, ouro::gpu::clear_type::color_depth_stencil);
		CommandList->set_blend_state(ouro::gpu::blend_state::opaque);
		CommandList->set_depth_stencil_state(ouro::gpu::depth_stencil_state::none);
		CommandList->set_surface_state(ouro::gpu::surface_state::front_face);
		CommandList->set_pipeline(Pipeline);

		Mesh->draw(CommandList);
		CommandList->end();
		Device->end_frame();
		Device->present(1);
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

	ouro::menu_handle Menus[oWMENU_COUNT];
	oGPUWindowClearToggle ClearToggle;
	oGUIMenuEnumRadioListHandler MERL;
	ouro::window_state::value PreFullscreenState;
	bool Running;
	bool UIMode;
	bool AllowUIModeChange;
private:
	void ActionHook(const ouro::input::action& _Action);
	void AppEventHook(const window::basic_event& _Event);
	bool CreateMenus(const window::create_event& _CreateEvent);
	void CheckState(ouro::window_state::value _State);
	void CheckStyle(ouro::window_style::value _Style);
	void EnableStatusBarStyles(bool _Enabled);
	void SetUIModeInternal(bool _UIMode, const ouro::window_shape& _CurrentAppShape, const ouro::window_shape& _CurrentGPUShape);
	void SetUIMode(bool _UIMode);
	void ToggleFullscreenCooperative(window* _pWindow);
	void Render();
};

oGPUWindowTestApp::oGPUWindowTestApp()
	: pGPUWindow(nullptr)
	, Running(true)
	, UIMode(true)
	, AllowUIModeChange(true)
	, PreFullscreenState(ouro::window_state::hidden)
	, ClearToggle(green, red)
{
	// Set up application window
	{
		window::init i;
		i.title = "oGPUWindowTestApp";
		i.icon = (ouro::icon_handle)load_icon(IDI_APPICON);
		i.on_action = std::bind(&oGPUWindowTestApp::ActionHook, this, std::placeholders::_1);
		i.on_event = std::bind(&oGPUWindowTestApp::AppEventHook, this, std::placeholders::_1);
		i.shape.client_size = int2(256, 256);
		i.shape.state = ouro::window_state::hidden;
		i.shape.style = ouro::window_style::sizable_with_menu_and_statusbar;
		i.alt_f4_closes = true;
		i.cursor_state = ouro::cursor_state::arrow;
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
	GPUWindow.GetRenderTarget()->set_clear_color(ClearToggle.Color[0]);
	AppWindow->show();
}

void oGPUWindowTestApp::CheckState(ouro::window_state::value _State)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oGPUWindowTestApp::CheckStyle(ouro::window_style::value _Style)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE]
	, oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oWMI_VIEW_STYLE_FIRST + _Style);
}

void oGPUWindowTestApp::EnableStatusBarStyles(bool _Enabled)
{
	// Enable styles not allowed for render target windows
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + ouro::window_style::fixed_with_statusbar, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + ouro::window_style::fixed_with_menu_and_statusbar, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + ouro::window_style::sizable_with_statusbar, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + ouro::window_style::sizable_with_menu_and_statusbar, _Enabled);
}

void oGPUWindowTestApp::SetUIModeInternal(bool _UIMode, const ouro::window_shape& _CurrentAppShape, const ouro::window_shape& _CurrentGPUShape)
{
	ouro::window_shape CurAppShape(_CurrentAppShape), CurGPUShape(_CurrentGPUShape);

	if (AppWindow->is_window_thread())
		CurAppShape = AppWindow->shape();

	else if (pGPUWindow->is_window_thread())
		CurGPUShape = pGPUWindow->shape();

	if (_UIMode)
	{
		ouro::window_shape GPUShape;
		GPUShape.state = ouro::window_state::hidden;
		GPUShape.style = ouro::window_style::borderless;
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
		ouro::window_shape GPUShape(CurAppShape);
		if (ouro::has_statusbar(GPUShape.style))
			GPUShape.style = ouro::window_style::value(GPUShape.style - 2);
		pGPUWindow->parent(nullptr);
		pGPUWindow->shape(GPUShape);
	}

	UIMode = _UIMode;
}

void oGPUWindowTestApp::SetUIMode(bool _UIMode)
{
	if (!AllowUIModeChange)
		return;

	ouro::window_shape AppShape, GPUShape;

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
	if (_pWindow->state() != ouro::window_state::fullscreen)
	{
		PreFullscreenState = _pWindow->state();
		_pWindow->state(ouro::window_state::fullscreen);
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
	oGUIMenuAppendEnumItems(ouro::window_style::count, Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, _CreateEvent.shape.style);
	EnableStatusBarStyles(true);

	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle) { AppWindow->style((ouro::window_style::value)_BorderStyle); });
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST
		, oWMI_VIEW_STYLE_FIRST + ouro::window_style::sizable_with_menu);

	oGUIMenuAppendEnumItems(ouro::window_state::count, Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, _CreateEvent.shape.state);
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State) { AppWindow->show((ouro::window_state::value)_State); });

	oGUIMenuAppendItem(Menus[oWMENU_VIEW], oWMI_VIEW_EXCLUSIVE, "Fullscreen E&xclusive");

	// Help menu
	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "About...");
	return true;
}

void oGPUWindowTestApp::AppEventHook(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case ouro::event_type::timer:
			if (_Event.as_timer().context == (uintptr_t)&ClearToggle)
			{
				ouro::gpu::render_target* pRT = GPUWindow.GetRenderTarget();
				if (pRT)
				{
					ouro::gpu::render_target_info RTI = pRT->get_info();
					if (RTI.clear.clear_color[0] == ClearToggle.Color[0]) RTI.clear.clear_color[0] = ClearToggle.Color[1];
					else RTI.clear.clear_color[0] = ClearToggle.Color[0];
					pRT->set_clear_info(RTI.clear);
				}
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
			if (!CreateMenus(_Event.as_create()))
				oThrowLastError();
			break;
		}

		case ouro::event_type::sized:
		{
			oTRACE("ouro::event_type::sized %s %dx%d", ouro::as_string(_Event.as_shape().shape.state), _Event.as_shape().shape.client_size.x, _Event.as_shape().shape.client_size.y);
			if (pGPUWindow)
				pGPUWindow->client_size(_Event.as_shape().shape.client_size);
			CheckState(_Event.as_shape().shape.state);
			CheckStyle(_Event.as_shape().shape.style);
			break;
		}
		case ouro::event_type::closing:
			GPUWindow.Stop();
			break;

		default:
			break;
	}
}

void oGPUWindowTestApp::ActionHook(const ouro::input::action& _Action)
{
	switch (_Action.action_type)
	{
		case ouro::input::menu:
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
						std::shared_ptr<ouro::about> a = ouro::about::make(i);
						a->show_modal(AppWindow);
					#else
						ouro::msgbox(msg_type::info, _Action.window, "About", "oGPUWindowTestApp: a small program to show a basic window and its events and actions");
					#endif
					break;
				}
				default:
					MERL.OnAction(_Action);
					break;
			}
			break;
		}

		case ouro::input::hotkey:
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
						if (AppWindow->state() == ouro::window_state::fullscreen)
							AppWindow->state(ouro::window_state::restored);
						AppWindow->style(ouro::window_style::sizable_with_menu_and_statusbar);
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
							const bool GoFullscreen = !GPUWindow.GetDevice()->is_fullscreen_exclusive();
							try { GPUWindow.GetDevice()->set_fullscreen_exclusive(GoFullscreen); }
							catch (std::exception& e) { oTRACEA("SetFullscreenExclusive(%s) failed: %s", GoFullscreen ? "true" : "false", e.what()); }
							AllowUIModeChange = !GoFullscreen;
						}
						else
							ToggleFullscreenCooperative(pGPUWindow);
					}
					break;
				}
				default:
					oTRACE("ouro::input::hotkey");
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
		ouro::msgbox(ouro::msg_type::info, nullptr, "oGPUWindowTestApp", "ERROR\n%s", e.what());
	}
}

void enumerate_shader_entries(const char* _ShaderSourceString, const std::function<void(const char* _EntryPoint, gpu::pipeline_stage::value _Stage)>& _Enumerator)
{
	// Make a copy so we can delete all the irrelevant parts for easier parsing
	std::string ForParsing(_ShaderSourceString);
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
				gpu::pipeline_stage::value Stage;
				static const char* sProfiles = "vhdgpc";
				const char* Profile = strchr(sProfiles, tolower(Entry[0]));
				if (Profile)
				{
					Stage = gpu::pipeline_stage::value(Profile - sProfiles);
					_Enumerator(Entry, Stage);
			
				}
			}
		}
		p++;
	}
}

void ShaderOnLoaded(const path& _Path, scoped_allocation& _Buffer, size_t _Size)
{
	enumerate_shader_entries((char*)_Buffer, [&](const char* _EntryPoint, gpu::pipeline_stage::value _Stage)
	{
		try
		{
			lstring IncludePaths, Defines;
			IncludePaths += "C:/dev/oooii/Ouroboros/Include;";

			scoped_allocation bytecode = gpu::compile_shader(IncludePaths, Defines, _Path, _Stage, _EntryPoint, (const char*)_Buffer);
			oTRACEA("Insert \"%s\" from %s", _EntryPoint, _Path.c_str());
		}
			
		catch (std::exception&)
		{
			//oTRACEA("%s Error: %s", _EntryPoint, e.what());
		}

	});
}


int main(int argc, const char* argv[])
{
	reporting::set_prompter(prompt_msgbox);

	// jist: load the library file knowing all registries where content will go.
	// there is a registry per shader type. Shaders are registered by entry point 
	// name.
	filesystem::load_async("C:/dev/oooii/Ouroboros/Source/oGfx/oGfxShaders.hlsl"
		, std::bind(ShaderOnLoaded, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		, filesystem::load_option::text_read);

	oGPUWindowTestApp App;
	App.Run();
	return 0;
}
