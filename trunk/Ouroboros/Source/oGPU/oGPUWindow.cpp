/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/oDisplay.h>
#include <oPlatform/oInterprocessEvent.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/Windows/oD3D11.h>
#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinWindowing.h>

#include "oD3D11RenderTarget.h"

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

	bool IsOpen() const threadsafe override { return Window->IsOpen(); }
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override { return Window->WaitUntilClosed(_TimeoutMS); }
	bool WaitUntilOpaque(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override { return Window->WaitUntilOpaque(_TimeoutMS); }
	bool Close(bool _AskFirst = true) threadsafe override { return Window->Close(_AskFirst); }
	bool IsWindowThread() const threadsafe override { return Window->IsWindowThread(); }
	bool HasFocus() const threadsafe override { return Window->HasFocus(); }
	void SetFocus() threadsafe override { return Window->SetFocus(); }
	void GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe override { Window->GetDesc(_pDesc); }
	bool Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe override { return Window->Map(_ppDesc); }
	void Unmap() threadsafe override { Window->Unmap(); }
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe override { return Window->GetTitle(_StrDestination, _SizeofStrDestination); }
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override { Window->SetTitleV(_Format, _Args); }
	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe override { return Window->GetStatusText(_StrDestination, _SizeofStrDestination, _StatusSectionIndex); }
	void SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override { Window->SetStatusText(_StatusSectionIndex, _Format, _Args); }
	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override { Window->SetHotKeys(_pHotKeys, _NumHotKeys); }
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe override { return Window->GetHotKeys(_pHotKeys, _MaxNumHotKeys); }
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override { Window->Trigger(_Action); }
	void Dispatch(const oTASK& _Task) threadsafe override { Window->Dispatch(_Task); }
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
		oASSERT( !IsWindowThread(), "Can not step from the window thread");
		UseStepping = _UseStepping;
		ShouldStep = true;
		StepEvent.Wait();
		StepEvent.Reset();
	}

	int GetFrameCount() const threadsafe { return thread_cast<oInt&>(FrameCount); }

private:
	oRef<threadsafe oWindow> Window;
	oRef<oGPUDevice> Device;
	oRef<oGPURenderTarget> RenderTarget;
	oRef<IDXGISwapChain> SwapChain;
	oRefCount RefCount;
	oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)> RenderFunction;
	int SyncInterval;
	oInt FrameCount;
	bool UseStepping;
	bool ShouldStep;
	oEvent StepEvent;
	bool CreateSwapChainGDICompatible;

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
				pPromisedImage->set_error(oERROR_NOT_FOUND, "Frame %d never reached", Frame);
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

	oMutex ScheduleSnapshotsMutex;
	std::vector<oPROMISED_SNAPSHOT> ScheduledSnapshots;

	// It is important that the GPU handler be the first registered, but client 
	// code could still register something in the ctor to handle the oGUI_CREATING
	// message. To facilitate that, register the GPU handler, and have that tail-
	// chain call the user-specified one, so store it here for future calling.
	oGUI_EVENT_HOOK CtorUserHook;
	bool EventHook(const oGUI_EVENT_DESC& _Event);
	bool OnSetFullscreen(const oGUI_EVENT_DESC& _Event);
	void WTHandleSnapshots();
};

oD3D11Window::oD3D11Window(const oGPU_WINDOW_INIT& _Init, oGPUDevice* _pDevice, bool* _pSuccess)
	: Device(_pDevice)
	, RenderFunction(_Init.RenderFunction)
	, CtorUserHook(_Init.EventHook)
	, SyncInterval(_Init.VSynced ? 1 : 0)
	, UseStepping(true) // initialize in step mode... this way the client code Render callback isn't called during bootstrapping of this window
	, ShouldStep(false)
	, FrameCount(oInvalid)
	, CreateSwapChainGDICompatible(_Init.UIRenderingCompatible)
{
	*_pSuccess = false;

	if (_Init.WinDesc.ShowStatusBar)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "ShowStatusBar=true is incompatible with oGPUWindow because DXGI overrides all client-area drawing and technically a status bar lives in the client area. To have a window with a status bar, create a borderless GPUWindow and parent it to the client area of a separate oWindow instance.");
		return; // pass through error
	}

	oWINDOW_INIT init = (oWINDOW_INIT)_Init;
	init.EventHook = oBIND(&oD3D11Window::EventHook, this, oBIND1); // CtorUserHook is called by this, so replacing this is ok.

	if (!init.WinDesc.EnableIdleEvent && init.WinDesc.Debug)
		oTRACE("oGPUWindow created with EnableIdleEvent=false. Rendering will not occur until this is enabled.");

	if (!oWindowCreate(init, &Window))
		return; // pass through error

	oStringS RTName;
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
}

bool oGPUWindowCreate(const oGPU_WINDOW_INIT& _Init, oGPUDevice* _pDevice, threadsafe oGPUWindow** _ppGPUWindow)
{
	bool success = false;
	oCONSTRUCT(_ppGPUWindow, oD3D11Window(_Init, _pDevice, &success));
	return success;
}

const oGUID& oGetGUID(threadsafe const oGPUWindow* threadsafe const*)
{
	// {A79296BF-A665-4DC6-9F33-1EF080732371}
	static const oGUID GUID_oGPUWindow = { 0xa79296bf, 0xa665, 0x4dc6, { 0x9f, 0x33, 0x1e, 0xf0, 0x80, 0x73, 0x23, 0x71 } };
	return GUID_oGPUWindow;
}

const oGUID& oGetGUID(threadsafe const oD3D11Window* threadsafe const*)
{
	// {43217250-62DC-4F08-ADE3-8D45637E451A}
	static const oGUID GUID_oD3D11Window = { 0x43217250, 0x62dc, 0x4f08, { 0xad, 0xe3, 0x8d, 0x45, 0x63, 0x7e, 0x45, 0x1a } };
	return GUID_oD3D11Window;
}

bool oD3D11Window::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;
	if (_InterfaceID == oGetGUID<oInterface>() || _InterfaceID == oGetGUID<oGPUWindow>() || _InterfaceID == oGetGUID<oD3D11Window>())
	{
		Reference();
		*_ppInterface = this;
	}

	else if (_InterfaceID == oGetGUID<oGPUDevice>())
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
			return oErrorSetLast(oERROR_WRONG_THREAD, "Retrieving a swapchain-dependent resource from anywhere but the windows thread is not threadsafe");
		if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)_ppInterface)))
			return oWinSetLastError();
	}

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11RenderTargetView))
	{
		if (!IsWindowThread())
			return oErrorSetLast(oERROR_WRONG_THREAD, "Retrieving a swapchain-dependent resource from anywhere but the windows thread is not threadsafe");

		oRef<ID3D11Texture2D> BackBuffer;
		if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer)))
			return oWinSetLastError();

		if (!oD3D11CreateRenderTargetView("oGPUWindow.RTV", BackBuffer, (ID3D11RenderTargetView**)_ppInterface))
			return false; // pass through error
	}

	else
		return Window->QueryInterface(_InterfaceID, _ppInterface);

	return !!*_ppInterface ? true : oErrorSetLast(oERROR_NOT_FOUND);
}

bool oD3D11Window::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Event)
	{
		case oGUI_IDLE:
		{
			if (!UseStepping || ShouldStep)
			{
				if (RenderFunction)
				{
					RenderFunction(RenderTarget);

					if (FrameCount == oInvalid)
						FrameCount = 0;
					else
						FrameCount++;
				}

				WTHandleSnapshots();

				oV(SwapChain->Present(SyncInterval, 0));
				ShouldStep = false;

				// Set stepped event 
				StepEvent.Set();
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
				, (HWND)_Event.hSource
				, CreateSwapChainGDICompatible
				, &SwapChain));
			break;
		}

		case oGUI_CLOSED:
			RenderFunction = nullptr;
			break;

			// Because the EventHook will be registered before any client event hook,
			// this event will execute before any client oGUI_SIZED, which is where we 
			// want it, after all oGUI_SIZING so client code can free swapchain-
			// dependent resources and before client code allocates any new dependents.
		case oGUI_SIZED:
			if (RenderTarget)
			{
				static_cast<oD3D11RenderTarget*>(RenderTarget.c_ptr())->ResizeLock();

				if (_Event.State != oGUI_WINDOW_MINIMIZED)
					oVERIFY(oDXGISwapChainResizeBuffers(SwapChain, _Event.ClientSize));

				static_cast<oD3D11RenderTarget*>(RenderTarget.c_ptr())->ResizeUnlock();
			}
			break;

		case oGUI_TO_FULLSCREEN:
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
	if (_Event.State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE)
	{
		oDXGI_FULLSCREEN_STATE FSState;
		FSState.Fullscreen = _Event.Event == oGUI_TO_FULLSCREEN;

		// Should we cache an explicit value here? If we do, how would an app change
		// it? Probably one day this should check for a registry setting and if not 
		// found just default to the current display.

		// For now just default to current display settings

		oGUI_WINDOW hWnd = nullptr;
		Window->QueryInterface(oGetGUID<oGUI_WINDOW>(), &hWnd);

		oDISPLAY_DESC DDesc;
		oDisplayEnum(oWinGetDisplayIndex((HWND)hWnd), &DDesc);
		FSState.Size = DDesc.Mode.Size;
		FSState.RefreshRate = DDesc.Mode.RefreshRate;

		if (oGUIIsFullscreen(_Event.State))
		{
			if (!oDXGISetFullscreenState(SwapChain, FSState))
			{
				if (oERROR_REFUSED == oErrorGetLast())
				{
					oStringL title;
					oWinGetText(title, (HWND)_Event.hSource);

					oMSGBOX_DESC mb;
					mb.Type = oMSGBOX_ERR;
					mb.Title = title;
					mb.ParentNativeHandle = (HWND)_Event.hSource;
					oMsgBox(mb, oErrorGetLastString());
				}

				else
					oVERIFY(false);
				return false;
			}
		}
	}

	else
	{
		oWinSetState((HWND)_Event.hSource, _Event.State);
	}

	return true;
}

oStd::future<oRef<oImage>> oD3D11Window::CreateSnapshot(int _Frame, bool _IncludeBorder) const threadsafe
{
	oPROMISED_SNAPSHOT promisedSnapshot;
	promisedSnapshot.pPromisedImage = new oStd::promise<oRef<oImage>>();
	promisedSnapshot.Frame = _Frame;
	promisedSnapshot.IncludeBorder = _IncludeBorder;
	oStd::future<oRef<oImage>> Image = promisedSnapshot.pPromisedImage->get_future();

	oLockGuard<oMutex> lock(thread_cast<oD3D11Window*>(this)->ScheduleSnapshotsMutex); // cast here to remove const (using "const" in the theoretical/interface way, but practically this does modify this class)
	thread_cast<oD3D11Window*>(this)->ScheduledSnapshots.push_back(std::move(promisedSnapshot)); // safe because vector is protected with a mutex above

	return Image;
}

void oD3D11Window::WTHandleSnapshots()
{
	if (!ScheduledSnapshots.empty())
	{
		oLockGuard<oMutex> lock(ScheduleSnapshotsMutex);
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
					it->pPromisedImage->take_last_error();

				delete it->pPromisedImage;
				it->pPromisedImage = nullptr;
				it = ScheduledSnapshots.erase(it);
				IncrementIt = false;
			}

			else if (it->Frame < FrameCount)
				it->pPromisedImage->set_error(oERROR_INVALID_PARAMETER, "Frame %d could not be captured because the request was processed on frame %d, which is past the capture frame.", it->Frame, FrameCount);

			if (IncrementIt)
				++it;
		}
	}

}