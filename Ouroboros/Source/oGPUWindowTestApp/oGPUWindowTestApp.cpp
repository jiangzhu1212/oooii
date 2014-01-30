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
#include <oGUI/Windows/oGDI.h>
#include <oGUI/oGUIMenu.h>
#include <oPlatform/oStream.h>
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

	oGPUDevice* GetDevice() { return Device; }
	oGPURenderTarget* GetRenderTarget() { return WindowRenderTarget; }

private:
	void OnEvent(const window::basic_event& _Event);
	void Run();
	void Render();

private:
	std::shared_ptr<window> Parent;
	intrusive_ptr<oGPUDevice> Device;
	intrusive_ptr<oGPUCommandList> CommandList;
	intrusive_ptr<oGPURenderTarget> WindowRenderTarget;
	intrusive_ptr<oGPUPipeline> Pipeline;
	intrusive_ptr<oGPUUtilMesh> Mesh;

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
	if (!oGPUDeviceCreate(ouro::gpu::device_init(), &Device))
	{
		oASSERT(false, "Could not create device:\n%s", oErrorGetLastString());
		Running = false;
		return;
	}

	oGPUPipeline::DESC pld;
	oVERIFY(oGfxGetPipeline(oGFX_PIPELINE_PASS_THROUGH, &pld));
	oVERIFY(Device->CreatePipeline(pld.DebugName, pld, &Pipeline));
	oVERIFY(oGPUUtilCreateFirstTriangle(Device, pld.pElements, pld.NumElements, &Mesh));

	Device->GetImmediateCommandList(&CommandList);
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
				WindowRenderTarget->Resize(int3(_Event.as_shape().shape.client_size, 1));
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
			i.icon = (ouro::icon_handle)oGDILoadIcon(IDI_APPICON);
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
			oVERIFY(Device->CreatePrimaryRenderTarget(GPUWindow.get(), ouro::surface::d24_unorm_s8_uint, true, &WindowRenderTarget));
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

		pGPUWindow = nullptr;
	}
	if (OnThreadExit)
		OnThreadExit();
	core_thread_traits::end_thread();
}

void oGPUWindowThread::Render()
{
	if (WindowRenderTarget && Device->BeginFrame())
	{
		CommandList->Begin();
		CommandList->SetRenderTarget(WindowRenderTarget);
		CommandList->Clear(WindowRenderTarget, ouro::gpu::clear_type::color_depth_stencil);
		CommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		CommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::none);
		CommandList->SetSurfaceState(ouro::gpu::surface_state::front_face);
		CommandList->SetPipeline(Pipeline);

		oGPUUtilMeshDraw(CommandList, Mesh);
		CommandList->End();
		Device->EndFrame();
		Device->Present(1);
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
	, ClearToggle(Green, Red)
{
	// Set up application window
	{
		window::init i;
		i.title = "oGPUWindowTestApp";
		i.icon = (ouro::icon_handle)oGDILoadIcon(IDI_APPICON);
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
	GPUWindow.GetRenderTarget()->SetClearColor(ClearToggle.Color[0]);
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
				oGPURenderTarget* pRT = GPUWindow.GetRenderTarget();
				if (pRT)
				{
					oGPURenderTarget::DESC RTD;
					pRT->GetDesc(&RTD);
					if (RTD.clear.clear_color[0] == ClearToggle.Color[0]) RTD.clear.clear_color[0] = ClearToggle.Color[1];
					else RTD.clear.clear_color[0] = ClearToggle.Color[0];
					pRT->SetClearDesc(RTD.clear);
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
						oDECLARE_ABOUT_INFO(i, oGDILoadIcon(IDI_APPICON));
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
							const bool GoFullscreen = !GPUWindow.GetDevice()->IsFullscreenExclusive();
							if (!GPUWindow.GetDevice()->SetFullscreenExclusive(GoFullscreen))
								oTRACE("SetFullscreenExclusive(%s) failed: %s", GoFullscreen ? "true" : "false", oErrorGetLastString());
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

int main(int argc, const char* argv[])
{
	reporting::set_prompter(prompt_msgbox);

	oGPUWindowTestApp App;
	App.Run();
	return 0;
}
