/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oGPU/oGPUWindow.h>
#include <oConcurrency/event.h>
#include <oConcurrency/backoff.h>
#include <oStd/oStdFuture.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/Windows/oD3D11.h>
#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinWindowing.h>

#include "oD3D11RenderTarget.h"

// {43217250-62DC-4F08-ADE3-8D45637E451A}
oDEFINE_GUID_S(oD3D11Window, 0x43217250, 0x62dc, 0x4f08, 0xad, 0xe3, 0x8d, 0x45, 0x63, 0x7e, 0x45, 0x1a);
struct oD3D11Window : oGPUWindow
{
	// @oooii-tony: Note to others: do not extend this class to support other D3D
	// interfaces. Create a new class and leave this solely a D3D11 window. Modify
	// the oGPUWindowCreate API to specify which instance gets created, but don't
	// merge the code because it causes more headaches than its worth.
	oD3D11Window(const oGPU_WINDOW_INIT& _Init, oGPUDevice* _pDevice, bool* _pSuccess);
	~oD3D11Window();

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	oGUI_WINDOW GetNativeHandle() const threadsafe { return Window->GetNativeHandle(); }
	bool IsOpen() const threadsafe override { return Window->IsOpen(); }
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override { return Window->WaitUntilClosed(_TimeoutMS); }
	bool WaitUntilOpaque(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override { return Window->WaitUntilOpaque(_TimeoutMS); }
	bool Close() threadsafe override { return Window->Close(); }
	bool IsWindowThread() const threadsafe override { return Window->IsWindowThread(); }
	bool HasFocus() const threadsafe override { return Window->HasFocus(); }
	void SetFocus() threadsafe override { return Window->SetFocus(); }
	void GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe override { Window->GetDesc(_pDesc); }
	bool Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe override { return Window->Map(_ppDesc); }
	void Unmap() threadsafe override { Window->Unmap(); }
	void SetDesc(const oGUI_WINDOW_CURSOR_DESC& _pDesc) threadsafe override { Window->SetDesc(_pDesc); }
	void GetDesc(oGUI_WINDOW_CURSOR_DESC* _pDesc) const threadsafe override { Window->GetDesc(_pDesc); }
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe override { return Window->GetTitle(_StrDestination, _SizeofStrDestination); }
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override { Window->SetTitleV(_Format, _Args); }
	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe override { return Window->GetStatusText(_StrDestination, _SizeofStrDestination, _StatusSectionIndex); }
	void SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override { Window->SetStatusText(_StatusSectionIndex, _Format, _Args); }
	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override { Window->SetHotKeys(_pHotKeys, _NumHotKeys); }
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe override { return Window->GetHotKeys(_pHotKeys, _MaxNumHotKeys); }
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override { Window->Trigger(_Action); }
	void Dispatch(const oTASK& _Task) threadsafe override { Window->Dispatch(_Task); }
	void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe override { Window->SetTimer(_Context, _RelativeTimeMS); }
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override { return Window->HookActions(_Hook); }
	void UnhookActions(int _ActionHookID) threadsafe override { Window->UnhookActions(_ActionHookID); }
	int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe override { return Window->HookEvents(_Hook); }
	void UnhookEvents(int _EventHookID) threadsafe override { Window->UnhookEvents(_EventHookID); }
	oStd::future<oRef<oImage>> CreateSnapshot(int _Frame = oInvalid, bool _IncludeBorder = false) const threadsafe override;

	void GetDevice(oGPUDevice** _ppDevice) const threadsafe
	{
		*_ppDevice = thread_cast<oD3D11Window*>(this)->Device; // safe because Device never changes, and Reference() is threadsafe
		(*_ppDevice)->Reference();
	}

	void Step(bool _UseStepping = true) threadsafe
	{
		UseStepping = _UseStepping;
		ShouldStep = true;
		if (!IsWindowThread()) // StepEvent is on the window thread, so don't wait on it
			StepEvent.wait();
		StepEvent.reset();
	}

	int GetFrameCount() const threadsafe { return thread_cast<oInt&>(FrameCount); }

private:
	oRef<threadsafe oWindow> Window;
	oRef<oGPUDevice> Device;
	oRef<oGPURenderTarget> RenderTarget;
	oRef<IDXGISwapChain> SwapChain;
	oRefCount RefCount;
	oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)> RenderFunction;
	oFUNCTION<void(oGUI_DRAW_CONTEXT _hDC, const int2& _ClientSize)> OSRenderFunction;
	int SyncInterval;
	oInt FrameCount;
	bool UseStepping;
	bool ShouldStep;
	oConcurrency::event StepEvent;

	struct oPROMISED_SNAPSHOT
	{
		oPROMISED_SNAPSHOT()
			: pPromisedImage(nullptr)
			, Frame(oInvalid)
			, IncludeBorder(false)
		{}

		oPROMISED_SNAPSHOT(oPROMISED_SNAPSHOT&& _That) { operator=(std::move(_That)); }

		const oPROMISED_SNAPSHOT& operator=(oPROMISED_SNAPSHOT&& _That)
		{
			if (this != &_That)
			{
				pPromisedImage = _That.pPromisedImage;
				_That.pPromisedImage = nullptr;
				Frame = _That.Frame;
				IncludeBorder = _That.IncludeBorder;
			}
			return *this;
		}

		~oPROMISED_SNAPSHOT()
		{
			if (pPromisedImage)
			{
				oErrorSetLast(std::errc::protocol_error, "Frame %d never reached", Frame);
				pPromisedImage->set_exception(std::make_exception_ptr(std::system_error(std::make_error_code((std::errc::errc)oErrorGetLast()), oErrorGetLastString())));
				delete pPromisedImage;
			}
		}

		oStd::promise<oRef<oImage>>* pPromisedImage;
		int Frame;
		bool IncludeBorder;

	private:
		oPROMISED_SNAPSHOT(const oPROMISED_SNAPSHOT&);
		const oPROMISED_SNAPSHOT& operator=(const oPROMISED_SNAPSHOT&);
	};

	oConcurrency::mutex ScheduleSnapshotsMutex;
	std::vector<oPROMISED_SNAPSHOT> ScheduledSnapshots;
	// This needs to be a private queue because rendering does not occur at a
	// one to one rate with the windows message pump (there may be several frames
	// of where rendering isn't finished and the message loop is actively idling).  
	// If it was one to one, we could use a waitable task and block when rendering falls behind.
	oRef<threadsafe oDispatchQueuePrivate> RenderQueue;
	bool Rendering;

	// It is important that the GPU handler be the first registered, but client 
	// code could still register something in the ctor to handle the oGUI_CREATING
	// message. To facilitate that, register the GPU handler, and have that tail-
	// chain call the user-specified one, so store it here for future calling.
	oGUI_EVENT_HOOK CtorUserHook;
	bool EventHook(const oGUI_EVENT_DESC& _Event);
	bool OnSetFullscreen(const oGUI_EVENT_DESC& _Event);
	void WTHandleSnapshots();
	void EnqueueRender();
};

oD3D11Window::oD3D11Window(const oGPU_WINDOW_INIT& _Init, oGPUDevice* _pDevice, bool* _pSuccess)
	: Device(_pDevice)
	, RenderFunction(_Init.RenderFunction)
	, OSRenderFunction(_Init.OSRenderFunction)
	, CtorUserHook(_Init.EventHook)
	, SyncInterval(_Init.VSynced ? 1 : 0)
	, UseStepping(true) // initialize in step mode... this way the client code Render callback isn't called during bootstrapping of this window
	, ShouldStep(false)
	, FrameCount(oInvalid)
	, Rendering(false)
{
	*_pSuccess = false;

	if( !oDispatchQueueCreatePrivate("RenderQueue", 1, &RenderQueue) )
		return;

	if (_Init.WinDesc.ShowStatusBar)
	{
		oErrorSetLast(std::errc::invalid_argument, "ShowStatusBar=true is incompatible with oGPUWindow because DXGI overrides all client-area drawing and technically a status bar lives in the client area. To have a window with a status bar, create a borderless GPUWindow and parent it to the client area of a separate oWindow instance.");
		return; // pass through error
	}

	oWINDOW_INIT init = (oWINDOW_INIT)_Init;
	init.EventHook = oBIND(&oD3D11Window::EventHook, this, oBIND1); // CtorUserHook is called by this, so replacing this is ok.

	if (!init.WinDesc.EnableMainLoopEvent && init.WinDesc.Debug)
		oTRACE("oGPUWindow created with EnableMainLoopEvent=false. Rendering will not occur until this is enabled.");

	if (!oWindowCreate(init, &Window))
		return; // pass through error

	oStd::sstring RTName;
	oPrintf(RTName, "%s.WindowRT", _pDevice->GetName());
	if (!oD3D11CreateRenderTarget(Device, RTName, this, _Init.DepthStencilFormat, &RenderTarget))
		return; // pass through error

	// Now that everything is ready, flag that we can allow the client code render go through
	UseStepping = _Init.StartStepping;

	*_pSuccess = true;
}

oD3D11Window::~oD3D11Window()
{
	UseStepping = true;
	// Ensure this is destroyed before CtorUserHook
	Window = nullptr;
	RenderQueue->Join();
}

bool oGPUWindowCreate(const oGPU_WINDOW_INIT& _Init, oGPUDevice* _pDevice, threadsafe oGPUWindow** _ppGPUWindow)
{
	bool success = false;
	oCONSTRUCT(_ppGPUWindow, oD3D11Window(_Init, _pDevice, &success));
	return success;
}

bool oD3D11Window::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;
	if (_InterfaceID == oGUID_oInterface || _InterfaceID == oGUID_oGPUWindow || _InterfaceID == oGUID_oD3D11Window)
	{
		Reference();
		*_ppInterface = this;
	}

	else if (_InterfaceID == oGUID_oGPUDevice)
	{
		Device->Reference();
		*_ppInterface = Device;
	}

	else if (_InterfaceID == (const oGUID&)__uuidof(IDXGISwapChain))
	{
		SwapChain->AddRef();
		*_ppInterface = SwapChain;
	}

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11Texture2D))
	{
		if (!IsWindowThread())
			return oErrorSetLast(std::errc::operation_not_permitted, "Retrieving a swapchain-dependent resource from anywhere but the windows thread is not threadsafe");
		if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)_ppInterface)))
			return oWinSetLastError();
	}

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11RenderTargetView))
	{
		if (!IsWindowThread())
			return oErrorSetLast(std::errc::operation_not_permitted, "Retrieving a swapchain-dependent resource from anywhere but the windows thread is not threadsafe");

		oRef<ID3D11Texture2D> BackBuffer;
		if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer)))
			return oWinSetLastError();

		if (!oD3D11CreateRenderTargetView("oGPUWindow.RTV", BackBuffer, (ID3D11RenderTargetView**)_ppInterface))
			return false; // pass through error
	}

	else
		return Window->QueryInterface(_InterfaceID, _ppInterface);

	return !!*_ppInterface ? true : oErrorSetLast(std::errc::function_not_supported);
}

bool oD3D11Window::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Event)
	{
		case oGUI_MAINLOOP:
		{
			if (!UseStepping || ShouldStep)
			{
				if (UseStepping)
				{
					if (!Rendering)
						EnqueueRender();

					oConcurrency::backoff bo;
					while (Rendering)
					{
						bo.pause();
					}

					WTHandleSnapshots();
					oV(SwapChain->Present(SyncInterval, 0));
				}

				else
				{
					if (!Rendering)
					{
						WTHandleSnapshots();
						oV(SwapChain->Present(SyncInterval, 0));
						EnqueueRender();
					}
				}

				ShouldStep = false;

				// Set stepped event 
				StepEvent.set();
			}
			break;
		}

		case oGUI_CREATING:
		{
			oRef<ID3D11Device> D3D11Device;
			oVERIFY(Device->QueryInterface((const oGUID&)__uuidof(ID3D11Device), &D3D11Device));
			oVB_RETURN2(oDXGICreateSwapChain(D3D11Device
				, false
				, __max(1, _Event.ClientSize.x)
				, __max(1, _Event.ClientSize.y)
				, DXGI_FORMAT_B8G8R8A8_UNORM
				, 0
				, 0
				, (HWND)_Event.hWindow
				, !!OSRenderFunction
				, &SwapChain));
			break;
		}

		case oGUI_CLOSED:
		{
			oGUI_WINDOW_DESC d;
			Window->GetDesc(&d);
			if (d.State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE)
			{
				oDXGI_FULLSCREEN_STATE FSState;
				FSState.Fullscreen = false;
				oVERIFY(oDXGISetFullscreenState(SwapChain, FSState));
			}

			RenderFunction = nullptr;
			OSRenderFunction = nullptr;
			break;
		}

		// Because the EventHook will be registered before any client event hook,
		// this event will execute before any client oGUI_SIZED, which is where we 
		// want it, after all oGUI_SIZING so client code can free swapchain-
		// dependent resources and before client code allocates any new dependents.
		case oGUI_SIZED:
			if (RenderTarget)
			{
				// Need to wait for the RenderQueue to finish before resizing
				RenderQueue->Flush();

				static_cast<oD3D11RenderTarget*>(RenderTarget.c_ptr())->ResizeLock();

				if (_Event.State != oGUI_WINDOW_MINIMIZED)
					oVERIFY(oDXGISwapChainResizeBuffers(SwapChain, _Event.ClientSize));

				static_cast<oD3D11RenderTarget*>(RenderTarget.c_ptr())->ResizeUnlock();
			}
			break;

		case oGUI_TO_FULLSCREEN: case oGUI_FROM_FULLSCREEN:
			oVERIFY(OnSetFullscreen(_Event));
			break;

		default:
			break;
	}

	if (!!CtorUserHook)
		return CtorUserHook(_Event);
	return true;
}

bool oD3D11Window::OnSetFullscreen(const oGUI_EVENT_DESC& _Event)
{
	oGUI_WINDOW_DESC wd;
	Window->GetDesc(&wd);
	const bool GoingToExclusiveFullscreen = (_Event.State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE);

	if (GoingToExclusiveFullscreen || wd.State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE)
	{
		// use resolution of current display for fullscreen
		oGUI_WINDOW hWnd = nullptr;
		Window->QueryInterface(oGetGUID<oGUI_WINDOW>(), &hWnd);
		oDISPLAY_DESC DDesc;
		oDisplayEnum(oWinGetDisplayIndex((HWND)hWnd), &DDesc);

		oDXGI_FULLSCREEN_STATE FSState;
		FSState.Fullscreen = GoingToExclusiveFullscreen;
		FSState.Size = DDesc.Mode.Size;
		FSState.RefreshRate = DDesc.Mode.RefreshRate;

		if (!oDXGISetFullscreenState(SwapChain, FSState))
		{
			if (std::errc::permission_denied == oErrorGetLast())
			{
				oStd::lstring title;
				oWinGetText(title, (HWND)_Event.hWindow);
				oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, title, _Event.hWindow), oErrorGetLastString());
			}

			else
				oVERIFY(false);
			return false;
		}
	}

	if (!GoingToExclusiveFullscreen)
		oWinSetState((HWND)_Event.hWindow, _Event.State);

	return true;
}

oStd::future<oRef<oImage>> oD3D11Window::CreateSnapshot(int _Frame, bool _IncludeBorder) const threadsafe
{
	oPROMISED_SNAPSHOT promisedSnapshot;
	promisedSnapshot.pPromisedImage = new oStd::promise<oRef<oImage>>();
	promisedSnapshot.Frame = _Frame;
	promisedSnapshot.IncludeBorder = _IncludeBorder;
	oStd::future<oRef<oImage>> Image = promisedSnapshot.pPromisedImage->get_future();

	{
		oConcurrency::lock_guard<oConcurrency::mutex> lock(thread_cast<oD3D11Window*>(this)->ScheduleSnapshotsMutex); // cast here to remove const (using "const" in the theoretical/interface way, but practically this does modify this class)
		thread_cast<oD3D11Window*>(this)->ScheduledSnapshots.push_back(std::move(promisedSnapshot)); // safe because vector is protected with a mutex above
	}

	if (IsWindowThread())
		thread_cast<oD3D11Window*>(this)->WTHandleSnapshots();

	return Image;
}

void oD3D11Window::WTHandleSnapshots()
{
	if (!ScheduledSnapshots.empty())
	{
		oConcurrency::lock_guard<oConcurrency::mutex> lock(ScheduleSnapshotsMutex);
		for (auto it = ScheduledSnapshots.begin(); it != ScheduledSnapshots.end(); )
		{
			bool IncrementIt = true;
			if (it->Frame == FrameCount || it->Frame == oInvalid)
			{
				oRef<ID3D11Texture2D> RT;
				oV(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));

				oRef<oImage> Image;
				if (oD3D11CreateSnapshot(RT, &Image))
					it->pPromisedImage->set_value(Image);
				else
					it->pPromisedImage->set_exception(std::make_exception_ptr(std::system_error(std::make_error_code((std::errc::errc)oErrorGetLast()), oErrorGetLastString())));

				delete it->pPromisedImage;
				it->pPromisedImage = nullptr;
				it = ScheduledSnapshots.erase(it);
				IncrementIt = false;
			}

			else if (it->Frame < FrameCount)
			{
				oErrorSetLast(std::errc::invalid_argument, "Frame %d could not be captured because the request was processed on frame %d, which is past the capture frame.", it->Frame, FrameCount);
				it->pPromisedImage->set_exception(std::make_exception_ptr(std::system_error(std::make_error_code((std::errc::errc)oErrorGetLast()), oErrorGetLastString())));
			}

			if (IncrementIt)
				++it;
		}
	}
}

void oD3D11Window::EnqueueRender()
{
	if (RenderFunction)	
	{
		Rendering = true;
		RenderQueue->Dispatch([&]
		{
			if (Device->BeginFrame())
			{
				RenderFunction(RenderTarget);
				Device->EndFrame();

				if (OSRenderFunction)
				{
					oRef<IDXGISwapChain> SwapChain;
					if (RenderTarget->QueryInterface(&SwapChain))
					{
						HDC hDeviceDC = nullptr;
						if (oDXGIGetDC(SwapChain, &hDeviceDC))
						{
							oRef<ID3D11Texture2D> SCTexture;
							oV(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SCTexture));
							D3D11_TEXTURE2D_DESC TDesc;
							SCTexture->GetDesc(&TDesc);

							const int2 ClientSize(TDesc.Width, TDesc.Height);
							OSRenderFunction((oGUI_DRAW_CONTEXT)hDeviceDC, ClientSize);

							oVERIFY(oDXGIReleaseDC(SwapChain, nullptr));
						}
					}
				}

				FrameCount++;
				if (FrameCount == oInvalid)
					FrameCount = 0;
			}

			Rendering = false;
		});
	}
	else
		Rendering = false;
}
