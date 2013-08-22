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

#include <oConcurrency/backoff.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/oGUIMenu.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oStream.h>
#include <oPlatform/oWindow.h>
#include "resource.h"

// Whereas oGPU is an abstraction of modern hardware-accelerated graphics APIs,
// oGfx is an abstraction of a particular rendering policy. It so happens to 
// include some simple shaders, so rather than reproduce such shaders, just use
// them from the oGfx library.
#include "oGfx/oGfxPipelines.h"

#include <oGPU/oGPU.h>
#include <oGPU/oGPUUtil.h>

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
	oWMI_VIEW_STYLE_LAST = oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_STYLE_COUNT - 1,

	oWMI_VIEW_STATE_FIRST,
	oWMI_VIEW_STATE_LAST = oWMI_VIEW_STATE_FIRST + oGUI_WINDOW_STATE_COUNT - 1,

	oWMI_VIEW_EXCLUSIVE,

	oWMI_HELP_ABOUT,
};

struct oGPUWindowClearToggle
{
	oGPUWindowClearToggle(oStd::color _Color1, oStd::color _Color2)
	{
		Color[0] = _Color1;
		Color[1] = _Color2;
	}

	oStd::color Color[2];
};

oGUI_HOTKEY_DESC_NO_CTOR HotKeys[] =
{
	// reset style
	{ oGUI_KEY_F2, oWHK_TOGGLE_UI_MODE, false, false, false },
	{ oGUI_KEY_F3, oWHK_DEFAULT_STYLE, false, false, false },
	{ oGUI_KEY_ENTER, oWHK_TOGGLE_FULLSCREEN, true, false, false },
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
	threadsafe oWindow* Start(oWindow* _pParent, const oGUI_ACTION_HOOK& _OnAction, const oTASK& _OnThreadExit);
	void Stop();

	oGPUDevice* GetDevice() { return Device; }
	oGPURenderTarget* GetRenderTarget() { return WindowRenderTarget; }

private:
	void OnEvent(const oGUI_EVENT_DESC& _Event);
	void Run();
	void Render();

private:
	oRef<oWindow> Parent;
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CommandList;
	oRef<oGPURenderTarget> WindowRenderTarget;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUUtilMesh> Mesh;

	oWindow* pGPUWindow;
	oStd::thread Thread;
	bool Running;

	oTASK OnThreadExit;
	oGUI_ACTION_HOOK OnAction;
};

oGPUWindowThread::oGPUWindowThread()
	: pGPUWindow(nullptr)
	, Running(true)
{
	if (!oGPUDeviceCreate(oGPU_DEVICE_INIT(), &Device))
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

threadsafe oWindow* oGPUWindowThread::Start(oWindow* _pParent, const oGUI_ACTION_HOOK& _OnAction, const oTASK& _OnThreadExit)
{
	Parent = _pParent;
	OnAction = _OnAction;
	OnThreadExit = _OnThreadExit;
	Thread = std::move(oStd::thread(&oGPUWindowThread::Run, this));

	oConcurrency::backoff bo;
	while (!pGPUWindow)
		bo.pause();
	return pGPUWindow;
}

void oGPUWindowThread::Stop()
{
	Running = false;
}

void oGPUWindowThread::OnEvent(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_SIZED:
		{
			if (WindowRenderTarget)
				WindowRenderTarget->Resize(int3(_Event.AsShape().Shape.ClientSize, 1));
			break;
		}

		case oGUI_CLOSING:
			Running = false;
			break;

	default:
		break;
	}
}

void oGPUWindowThread::Run()
{
	oConcurrency::begin_thread("Window Render Target Thread");
	{
		oRef<oWindow> GPUWindow;

		// Set up child window as a render target (this allows other client-area 
		// controls to be drawn by parent since the primary render target consumes 
		// the entire client area).
		{
			oWINDOW_INIT Init;
			Init.Title = "Render Target Window";
			Init.hIcon = (oGUI_ICON)oGDILoadIcon(IDI_APPICON);
			Init.ActionHook = OnAction;
			Init.EventHook = oBIND(&oGPUWindowThread::OnEvent, this, oBIND1);
			Init.Shape.ClientPosition = int2(0, 0); // important to think client-relative for this
			Init.Shape.ClientSize = int2(256, 256); // @oooii-tony: Try making this 1,1 and see if a resize takes over
			Init.Shape.State = oGUI_WINDOW_HIDDEN; // don't show the window before it is child-ized
			Init.Shape.Style = oGUI_WINDOW_BORDERLESS;
			Init.ClientCursorState = oGUI_CURSOR_HAND;
			Init.AltF4Closes = true;
			oVERIFY(oWindowCreate(Init, &GPUWindow));
			GPUWindow->SetHotKeys(HotKeys);
			oVERIFY(Device->CreatePrimaryRenderTarget(GPUWindow, oSURFACE_D24_UNORM_S8_UINT, true, &WindowRenderTarget));
			GPUWindow->SetParent(Parent);
			GPUWindow->Show(); // now that the window is a child, show it (it will only show when parent shows)
			pGPUWindow = GPUWindow;
		}

		try
		{
			while (Running)
			{
				GPUWindow->FlushMessages();
				Render();
			}
		}

		catch (std::exception& e)
		{
			oMSGBOX_DESC m;
			m.hParent = nullptr;
			m.Title = "oGPUWindowTestApp";
			m.Type = oMSGBOX_INFO;
			oMsgBox(m, "ERROR\n%s", e.what());
		}

		pGPUWindow = nullptr;
	}
	if (OnThreadExit)
		OnThreadExit();
	oConcurrency::end_thread();
}

void oGPUWindowThread::Render()
{
	if (WindowRenderTarget && Device->BeginFrame())
	{
		CommandList->Begin();
		CommandList->SetRenderTarget(WindowRenderTarget);
		CommandList->Clear(WindowRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CommandList->SetBlendState(oGPU_OPAQUE);
		CommandList->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
		CommandList->SetSurfaceState(oGPU_FRONT_FACE);
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
	oRef<oWindow> AppWindow;
	threadsafe oWindow* pGPUWindow;
	oGPUWindowThread GPUWindow;

	oGUI_MENU Menus[oWMENU_COUNT];
	oGPUWindowClearToggle ClearToggle;
	oGUIMenuEnumRadioListHandler MERL;
	oGUI_WINDOW_STATE PreFullscreenState;
	bool Running;
	bool UIMode;
	bool AllowUIModeChange;
private:
	void ActionHook(const oGUI_ACTION_DESC& _Action);
	void AppEventHook(const oGUI_EVENT_DESC& _Event);
	bool CreateMenus(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	void CheckState(oGUI_WINDOW_STATE _State);
	void CheckStyle(oGUI_WINDOW_STYLE _Style);
	void EnableStatusBarStyles(bool _Enabled);
	void SetUIModeInternal(bool _UIMode, const oGUI_WINDOW_SHAPE_DESC& _CurrentAppShape, const oGUI_WINDOW_SHAPE_DESC& _CurrentGPUShape);
	void SetUIMode(bool _UIMode);
	void ToggleFullscreenCooperative(oWindow* _pWindow);
	void Render();
};

oGPUWindowTestApp::oGPUWindowTestApp()
	: pGPUWindow(nullptr)
	, Running(true)
	, UIMode(true)
	, AllowUIModeChange(true)
	, PreFullscreenState(oGUI_WINDOW_HIDDEN)
	, ClearToggle(oStd::Green, oStd::Red)
{
	// Set up application window
	{
		oWINDOW_INIT Init;
		Init.Title = "oGPUWindowTestApp";
		Init.hIcon = (oGUI_ICON)oGDILoadIcon(IDI_APPICON);
		Init.ActionHook = oBIND(&oGPUWindowTestApp::ActionHook, this, oBIND1);
		Init.EventHook = oBIND(&oGPUWindowTestApp::AppEventHook, this, oBIND1);
		Init.Shape.ClientSize = int2(256, 256);
		Init.Shape.State = oGUI_WINDOW_HIDDEN;
		Init.Shape.Style = oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR;
		Init.AltF4Closes = true;
		Init.ClientCursorState = oGUI_CURSOR_ARROW;
		oVERIFY(oWindowCreate(Init, &AppWindow));
		AppWindow->SetHotKeys(HotKeys);
		AppWindow->SetTimer((uintptr_t)&ClearToggle, 1000);

		const int sSections[] = { 120, -1 };
		AppWindow->SetNumStatusSections(sSections, oCOUNTOF(sSections));
		AppWindow->SetStatusText(0, "F3 for default style");
		AppWindow->SetStatusText(1, "Fullscreen cooperative");
	}

	// Now set up separate child thread for rendering. This allows UI to be 
	// detached from potentially slow rendering.
	pGPUWindow = GPUWindow.Start(AppWindow, oBIND(&oGPUWindowTestApp::ActionHook, this, oBIND1), [&] { Running = false; });
	GPUWindow.GetRenderTarget()->SetClearColor(ClearToggle.Color[0]);
	AppWindow->Show();
}

void oGPUWindowTestApp::CheckState(oGUI_WINDOW_STATE _State)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STATE]
	, oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oWMI_VIEW_STATE_FIRST + _State);
}

void oGPUWindowTestApp::CheckStyle(oGUI_WINDOW_STYLE _Style)
{
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE]
	, oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oWMI_VIEW_STYLE_FIRST + _Style);
}

void oGPUWindowTestApp::EnableStatusBarStyles(bool _Enabled)
{
	// Enable styles not allowed for render target windows
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_FIXED_WITH_STATUSBAR, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_FIXED_WITH_MENU_AND_STATUSBAR, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_SIZABLE_WITH_STATUSBAR, _Enabled);
	oGUIMenuEnable(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR, _Enabled);
}

void oGPUWindowTestApp::SetUIModeInternal(bool _UIMode, const oGUI_WINDOW_SHAPE_DESC& _CurrentAppShape, const oGUI_WINDOW_SHAPE_DESC& _CurrentGPUShape)
{
	oGUI_WINDOW_SHAPE_DESC CurAppShape(_CurrentAppShape), CurGPUShape(_CurrentGPUShape);

	if (AppWindow->IsWindowThread())
		CurAppShape = AppWindow->GetShape();

	else if (pGPUWindow->IsWindowThread())
		CurGPUShape = thread_cast<oWindow*>(pGPUWindow)->GetShape();

	if (_UIMode)
	{
		oGUI_WINDOW_SHAPE_DESC GPUShape;
		GPUShape.State = oGUI_WINDOW_HIDDEN;
		GPUShape.Style = oGUI_WINDOW_BORDERLESS;
		GPUShape.ClientPosition = int2(0, 0);
		GPUShape.ClientSize = CurAppShape.ClientSize;
		pGPUWindow->SetShape(GPUShape);
		pGPUWindow->SetParent(AppWindow);
		pGPUWindow->Show();
		AppWindow->Show(CurGPUShape.State);
		AppWindow->SetFocus();
	}

	else
	{
		AppWindow->Hide();
		oGUI_WINDOW_SHAPE_DESC GPUShape(CurAppShape);
		if (oGUIStyleHasStatusBar(GPUShape.Style))
			GPUShape.Style = oGUI_WINDOW_STYLE(GPUShape.Style - 2);
		pGPUWindow->SetParent(nullptr);
		pGPUWindow->SetShape(GPUShape);
	}

	UIMode = _UIMode;
}

void oGPUWindowTestApp::SetUIMode(bool _UIMode)
{
	if (!AllowUIModeChange)
		return;

	oGUI_WINDOW_SHAPE_DESC AppShape, GPUShape;

	if (AppWindow->IsWindowThread())
	{
		AppShape = AppWindow->GetShape();
	
		pGPUWindow->Dispatch([=] { SetUIModeInternal(_UIMode, AppShape, GPUShape); });
	}

	else if (pGPUWindow->IsWindowThread())
	{
		GPUShape = thread_cast<oWindow*>(pGPUWindow)->GetShape();

		AppWindow->Dispatch([=] { SetUIModeInternal(_UIMode, AppShape, GPUShape); });
	}
}

void oGPUWindowTestApp::ToggleFullscreenCooperative(oWindow* _pWindow)
{
	if (_pWindow->GetState() != oGUI_WINDOW_FULLSCREEN)
	{
		PreFullscreenState = _pWindow->GetState();
		_pWindow->SetState(oGUI_WINDOW_FULLSCREEN);
		AllowUIModeChange = false;
	}
	else
	{
		_pWindow->SetState(PreFullscreenState);
		AllowUIModeChange = true;
	}
}

bool oGPUWindowTestApp::CreateMenus(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	oFOR(auto& m, Menus)
		m = oGUIMenuCreate();

	oFOR(const auto& h, sMenuHier)
	{
		oGUIMenuAppendSubmenu(
			h.Parent == oWMENU_TOPLEVEL ? _CreateEvent.hMenu : Menus[h.Parent]
		, Menus[h.Menu], h.Name);
	}

	// File menu
	oGUIMenuAppendItem(Menus[oWMENU_FILE], oWMI_FILE_EXIT, "E&xit\tAlt+F4");

	// Edit menu
	// (nothing yet)

	// View menu
	oVERIFY_R(oGUIMenuAppendEnumItems(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, oRTTI_OF(oGUI_WINDOW_STYLE), _CreateEvent.Shape.Style));
	EnableStatusBarStyles(true);

	MERL.Register(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST, [=](int _BorderStyle) { AppWindow->SetStyle((oGUI_WINDOW_STYLE)_BorderStyle); });
	oGUIMenuCheckRadio(Menus[oWMENU_VIEW_STYLE], oWMI_VIEW_STYLE_FIRST, oWMI_VIEW_STYLE_LAST
		, oWMI_VIEW_STYLE_FIRST + oGUI_WINDOW_SIZABLE_WITH_MENU);

	oVERIFY_R(oGUIMenuAppendEnumItems(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, oRTTI_OF(oGUI_WINDOW_STATE), _CreateEvent.Shape.State));
	MERL.Register(Menus[oWMENU_VIEW_STATE], oWMI_VIEW_STATE_FIRST, oWMI_VIEW_STATE_LAST, [=](int _State) { AppWindow->Show((oGUI_WINDOW_STATE)_State); });

	oGUIMenuAppendItem(Menus[oWMENU_VIEW], oWMI_VIEW_EXCLUSIVE, "Fullscreen E&xclusive");

	// Help menu
	oGUIMenuAppendItem(Menus[oWMENU_HELP], oWMI_HELP_ABOUT, "About...");
	return true;
}

void oGPUWindowTestApp::AppEventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_TIMER:
			if (_Event.AsTimer().Context == (uintptr_t)&ClearToggle)
			{
				oGPURenderTarget* pRT = GPUWindow.GetRenderTarget();
				if (pRT)
				{
					oGPU_RENDER_TARGET_DESC RTD;
					pRT->GetDesc(&RTD);
					if (RTD.ClearDesc.ClearColor[0] == ClearToggle.Color[0]) RTD.ClearDesc.ClearColor[0] = ClearToggle.Color[1];
					else RTD.ClearDesc.ClearColor[0] = ClearToggle.Color[0];
					pRT->SetClearDesc(RTD.ClearDesc);
				}
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
			if (!CreateMenus(_Event.AsCreate()))
				oThrowLastError();
			break;
		}

		case oGUI_SIZED:
		{
			oTRACE("oGUI_SIZED %s %dx%d", oStd::as_string(_Event.AsShape().Shape.State), _Event.AsShape().Shape.ClientSize.x, _Event.AsShape().Shape.ClientSize.y);
			if (pGPUWindow)
				pGPUWindow->SetClientSize(_Event.AsShape().Shape.ClientSize);
			CheckState(_Event.AsShape().Shape.State);
			CheckStyle(_Event.AsShape().Shape.Style);
			break;
		}
		case oGUI_CLOSING:
			GPUWindow.Stop();
			break;

		default:
			break;
	}
}

void oGPUWindowTestApp::ActionHook(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_MENU:
		{
			switch (_Action.DeviceID)
			{
				case oWMI_FILE_EXIT:
					GPUWindow.Stop();
					break;
				case oWMI_VIEW_EXCLUSIVE:
				{
					const bool checked = oGUIMenuIsChecked(Menus[oWMENU_VIEW], _Action.DeviceID);
					oGUIMenuCheck(Menus[oWMENU_VIEW], _Action.DeviceID, !checked);
					AppWindow->SetStatusText(1, "Fullscreen %s", !checked ? "exclusive" : "cooperative");
					break;
				}
				case oWMI_HELP_ABOUT:
				{
					oMSGBOX_DESC m;
					m.hParent = _Action.hWindow;
					m.Title = "About";
					m.Type = oMSGBOX_INFO;
					oMsgBox(m, "oGPUWindowTestApp: a small program to show a basic window and its events and actions");
					break;
				}
				default:
					MERL.OnAction(_Action);
					break;
			}
			break;
		}

		case oGUI_ACTION_HOTKEY:
		{
			switch (_Action.DeviceID)
			{
				case oWHK_TOGGLE_UI_MODE:
					SetUIMode(!UIMode);
					break;

				case oWHK_DEFAULT_STYLE:
				{
					if (UIMode)
					{
						oGUI_WINDOW_SHAPE_DESC s;
						s.State = AppWindow->GetState();
						if (s.State == oGUI_WINDOW_FULLSCREEN)
							s.State = oGUI_WINDOW_RESTORED;
						s.Style = oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR;
						AppWindow->SetShape(s);
					}
					break;
				}
				case oWHK_TOGGLE_FULLSCREEN:
				{
					if (UIMode)
						ToggleFullscreenCooperative(AppWindow);
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
							ToggleFullscreenCooperative(thread_cast<oWindow*>(pGPUWindow));
					}
					break;
				}
				default:
					oTRACE("oGUI_ACTION_HOTKEY");
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
			AppWindow->FlushMessages();
	}
	catch (std::exception& e)
	{
		oMSGBOX_DESC m;
		m.hParent = nullptr;
		m.Title = "oGPUWindowTestApp";
		m.Type = oMSGBOX_INFO;
		oMsgBox(m, "ERROR\n%s", e.what());
	}
}

oMAINA()
{
	oGPUWindowTestApp App;
	App.Run();
	return 0;
}
