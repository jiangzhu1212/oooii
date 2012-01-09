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
#include <oPlatform/oWindow.h>
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oMemory.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oEvent.h>
#include <oPlatform/oGDI.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinWindowing.h>
#include <oPlatform/oWinAsString.h>
#include <windowsx.h>

#if oDXVER >= oDXVER_10
	#include <oPlatform/oD2D.h>
	#include <oPlatform/oD3D10.h>
	#include <oPlatform/oDXGI.h>
	#if oDXVER >= oDXVER_11
		#include <oPlatform/oD3D11.h>
	#endif
#endif

const oGUID& oGetGUID(threadsafe const oHDC* threadsafe const*)
{
	// {DBF01A90-3C5A-46D2-A1E3-501ACF5D1BEE}
	static const oGUID oGUID_HDC = { 0xdbf01a90, 0x3c5a, 0x46d2, { 0xa1, 0xe3, 0x50, 0x1a, 0xcf, 0x5d, 0x1b, 0xee } };
	return oGUID_HDC;
}

const oGUID& oGetGUID(threadsafe const oHDCAA* threadsafe const*)
{
	// {D7693540-B6F0-4AB5-93DC-115E6B99C626}
	static const oGUID oGUID_HDCAA = { 0xd7693540, 0xb6f0, 0x4ab5, { 0x93, 0xdc, 0x11, 0x5e, 0x6b, 0x99, 0xc6, 0x26 } };
	return oGUID_HDCAA;
}

const char* oAsString(const oWindow::STATE& _State)
{
	switch (_State)
	{
		case oWindow::HIDDEN: return "hidden";
		case oWindow::RESTORED: return "restored";
		case oWindow::MINIMIZED: return "minimized";
		case oWindow::MAXIMIZED: return "maximized";
		case oWindow::FULLSCREEN: return "fullscreen";
		oNODEFAULT;
	}
}

const char* oAsString(const oWindow::STYLE& _Style)
{
	switch (_Style)
	{
		case oWindow::BORDERLESS: return "borderless";
		case oWindow::FIXED: return "fixed";
		case oWindow::DIALOG: return "dialog";
		case oWindow::SIZEABLE: return "sizeable";
		oNODEFAULT;
	}
}

const char* oAsString(const oWindow::DRAW_MODE& _DrawMode)
{
	switch (_DrawMode)
	{
		case oWindow::USE_DEFAULT: return "USE_DEFAULT";
		case oWindow::USE_GDI: return "USE_GDI";
		case oWindow::USE_D2D: return "USE_D2D";
		case oWindow::USE_DX11: return "USE_DX11";
		oNODEFAULT;
	}
}

bool oFromString(oWindow::STYLE* _pStyle, const char* _StrSource)
{
	if (!_stricmp(_StrSource,"borderless"))
		*_pStyle = oWindow::BORDERLESS;
	else if (!_stricmp(_StrSource,"fixed"))
		*_pStyle = oWindow::FIXED;
	else if (!_stricmp(_StrSource,"dialog"))
		*_pStyle = oWindow::DIALOG;
	else
		*_pStyle = oWindow::SIZEABLE;
	return true;
}

static oWINDOW_STATE oAsWinState(oWindow::STATE _State)
{
	static const oWINDOW_STATE sStates[] = 
	{
		oWINDOW_HIDDEN,
		oWINDOW_RESTORED,
		oWINDOW_MINIMIZED,
		oWINDOW_MAXIMIZED,
		oWINDOW_RESTORED,
	};
	return sStates[_State];
}

//static oWindow::STATE oAsState(oWINDOW_STATE _State) { return static_cast<oWindow::STATE>(_State-1); }
static oWINDOW_STYLE oAsWinStyle(oWindow::STYLE _Style) { return static_cast<oWINDOW_STYLE>(_Style); }
//static oWindow::STYLE oAsStyle(oWINDOW_STYLE _Style) { return static_cast<oWindow::STYLE>(_Style); }

static HCURSOR oWinGetCursor(oWindow::CURSOR_STATE _CursorState)
{
	LPSTR cursors[] =
	{
		0,
		IDC_ARROW,
		IDC_HAND,
		IDC_HELP,
		IDC_NO,
		IDC_WAIT,
		IDC_APPSTARTING,
		0,
	};

	return LoadCursor(nullptr, cursors[_CursorState]);
}

static void oWinSetCursorState(HWND _hWnd, oWindow::CURSOR_STATE _CursorState, HCURSOR _hUserCursor = nullptr)
{
	HCURSOR hCursor = _CursorState == oWindow::USER ? _hUserCursor : oWinGetCursor(_CursorState);
	oWinSetCursor(_hWnd, hCursor);
	oWinCursorSetVisible(_CursorState != oWindow::NONE);
}

static oWindow::DRAW_MODE CheckDefaultDrawMode(oWindow::DRAW_MODE _Mode)
{
	if (_Mode == oWindow::USE_DEFAULT)
	{
		oRef<ID2D1Factory> D2D1Factory;
		oD2DCreateFactory(&D2D1Factory);
		_Mode = D2D1Factory ? oWindow::USE_D2D : oWindow::USE_GDI;
	}

	return _Mode;
}

static RECT ResolveRect(HWND _hWnd, const int2& _Position, const int2& _Size, bool _FillDisplayByDefault)
{
	int2 p = _Position;
	int2 s = _Size;

	// Default size is the screen size the window is currently on
	// Default position is centered in the screen the window is currently on (after size is resolved)
	if (s.x == oDEFAULT || s.y == oDEFAULT || p.x == oDEFAULT || p.y == oDEFAULT)
	{
		oDISPLAY_DESC DDesc;
		oDisplayEnum(oWinGetDisplayIndex(_hWnd), &DDesc);

		if (_FillDisplayByDefault)
		{
			if (s.x == oDEFAULT) s.x = DDesc.Mode.Size.x;
			if (s.y == oDEFAULT) s.y = DDesc.Mode.Size.y;

			if (p.x == oDEFAULT) p.x = DDesc.Position.x;
			if (p.y == oDEFAULT) p.y = DDesc.Position.y;
		}

		else
		{
			if (s.x == oDEFAULT) s.x = 640;
			if (s.y == oDEFAULT) s.y = 480;

			if (p.x == oDEFAULT) p.x = ((DDesc.Mode.Size.x - s.x) / 2);
			if (p.y == oDEFAULT) p.y = ((DDesc.Mode.Size.y - s.y) / 2);
		}
	}

	return oWinRectWH(p, s);
}

struct oWinWindow : oWindow
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	oWinWindow(const DESC& _Desc, void* _pAssociatedNativeHandle, DRAW_MODE _DrawMode, bool* _pSuccess);
	~oWinWindow();

	bool IsOpen() const threadsafe override;
	void GetDesc(DESC* _pDesc) threadsafe override;
	DESC* Map() threadsafe override;
	void Unmap() threadsafe override;
	void Refresh(bool _Blocking = true, const oRECT* _pDirtyRect = nullptr) threadsafe override;
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe override;
	void SetTitle(const char* _Title) threadsafe override;
	bool HasFocus() const threadsafe override;
	void SetFocus() threadsafe override;
	DRAW_MODE GetDrawMode() const threadsafe;
	void* GetNativeHandle() threadsafe override;
	const void* GetProperty(const char* _Name) const override;
	bool SetProperty(const char* _Name, void* _Value) override;
	bool CreateSnapshot(oImage** _ppImage, bool _IncludeBorder = false) threadsafe override;
	unsigned int Hook(HookFn _Hook) threadsafe override;
	void Unhook(unsigned int _HookID) threadsafe override;

protected:
	inline oWinWindow* QThis() threadsafe { return thread_cast<oWinWindow*>(this); }
	inline const oWinWindow* QThis() const threadsafe { return thread_cast<oWinWindow*>(this); }

	bool CallHooks(EVENT _Event, void* _pDrawContext, DRAW_MODE _DrawMode, unsigned int _SuperSampleScale, const DESC& _Desc);

	void Initialize(oERROR* _pError, char* _pErrorString, size_t _szErrorString);
	void Deinitialize();
	void Run();

	bool CreateDevices(HWND _hWnd, IUnknown* _pUserSpecifiedDevice);
	bool CreateDeviceResources(HWND _hWnd);
	bool DestroyDeviceResources();
	bool D2DPaint(HWND _hWnd, bool _Force = false);
	bool GDIPaint(HWND _hWnd, HDC _hDC, bool _Force = false);
	
	// Some of these have params specified by-copy so oBIND keeps the param copy when enqueued.
	void SetTitle1(oStringM _Title);
	void SetIcon(HICON _hIcon);
	void SetCursor(HCURSOR _hCursor);
	void SetDesc(DESC _Desc);
	void CreateSnapshot1(oImage** _ppImage, bool _IncludeBorder, threadsafe oEvent* _pDone, bool* _pSuccess);
	void Hook1(HookFn _Hook, threadsafe oEvent* _pDone, size_t* _pIndex, oERROR* _pLastError, char* _StrLastError, size_t _SizeofStrLastError);
	void Unhook1(size_t _Index, threadsafe oEvent* _pDone);
	void ToggleFullscreen();
	void DestroyWindow1();

	struct REFRESH
	{
		bool UseRect;
		oRECT Rect;
		oEvent* pEvent;
	};
	void Refresh1(REFRESH _Refresh);

	oDECLARE_WNDPROC(, WndProc);
	oDECLARE_WNDPROC(static, StaticWndProc);

	// DESC/accessor/mutator support
	oSharedMutex DescMutex;
	DESC Desc; // current desc as returned by GetDesc
	DESC PendingDesc; // buffer returned by Map (sync'ed to Desc first so Map == GetDesc)
	DESC PreFullscreenDesc; // desc as it was before a fullscreen state was set.
	oTASK RunFunction; // cache the binding of this->Run();
	oRef<threadsafe oDispatchQueue> MessageQueue;

	std::vector<HookFn> Hooks;

	inline int SuperSampleScale() const { return Desc.UseAntialiasing ? 4 : 1; }

	// Nuts n bolts of oWindow/one-time init fields
	oRefCount RefCount;
	DRAW_MODE DrawMode;
	oGDIScopedIcon hIcon;
	oGDIScopedCursor hCursor;
	HWND hWnd;
	IUnknown* pUserSpecifiedDevice; // Non-refcounted, just to retain value through CreateDevices()
	oStd::atomic_bool CloseConfirmed;

	// Device Resources
	#if oDXVER >= oDXVER_10
		static const oSURFACE_FORMAT SWAP_CHAIN_FORMAT = oSURFACE_B8G8R8A8_UNORM;
		oRef<ID3D11Device> D3D11Device;
		oRef<ID3D10Device1> D3D10Device;
		oRef<ID2D1Factory> D2DFactory;
		oRef<IDXGISwapChain> DXGISwapChain;

		oRef<ID2D1RenderTarget> D2DRenderTarget;
		oRef<ID3D11RenderTargetView> D3D11RenderTargetView;
		oRef<ID3D10Texture2D> D3D10RenderTarget;
	#endif

	oGDIScopedObject<HBRUSH> hClearBrush;
	oGDIScopedDC hOffscreenAADC;
	oGDIScopedDC hOffscreenDC;
	oGDIScopedObject<HBITMAP> hOffscreenAABmp; // AA shapes
	oGDIScopedObject<HBITMAP> hOffscreenBmp; // AA is handled on text with cleartype, so let that pass through
};

oWinWindow::oWinWindow(const DESC& _Desc, void* _pAssociatedNativeHandle, DRAW_MODE _DrawMode, bool* _pSuccess)
	: Desc(_Desc)
	, PendingDesc(_Desc)
	, PreFullscreenDesc(_Desc)
	, DrawMode(CheckDefaultDrawMode(_DrawMode))
	, hWnd(nullptr)
	, pUserSpecifiedDevice(nullptr)
	, CloseConfirmed(false)
{
	*_pSuccess = false;
	RunFunction = oBIND(&oWinWindow::Run, this);

	if (!oDispatchQueueCreatePrivate("oWinWindow Message Thread", 2000, &MessageQueue))
		return;

	// init to something not fullscreen in case is started in fullscreen
	PreFullscreenDesc.State = RESTORED; 
	if (_Desc.State == FULLSCREEN)
	{
		Desc.State = RESTORED;
		PendingDesc.State = RESTORED;
	}
	
	oERROR InitError = oERROR_NONE;
	oFixedString<char, 256> ErrorStr;
	MessageQueue->Dispatch(oBIND(&oWinWindow::Initialize, this, &InitError, ErrorStr.c_str(), ErrorStr.capacity() ));
	MessageQueue->Flush();
	oASSERT(hWnd, "hWnd must be initialized by now (flush isn't blocking properly?)");
	if (oERROR_NONE != InitError)
	{
		oErrorSetLast(InitError, ErrorStr);
		return;
	}

	MessageQueue->Dispatch(RunFunction);
	Map(); Unmap();
	oTRACE("Created HWND %x using %s for drawing", hWnd, oAsString(DrawMode));

	if (_Desc.State == FULLSCREEN)
	{
		DESC* d = Map();
		*d = _Desc;
		Unmap();
	}

	*_pSuccess = true;
}

oWinWindow::~oWinWindow()
{
	MessageQueue->Dispatch(oBIND(&oWinWindow::Deinitialize, this));
	oWinWake(hWnd);
	MessageQueue->Join();
}

void oWinWindow::Initialize(oERROR* _pError, char* _pErrorString, size_t _szErrorString)
{
	oStd::thread::id id = oStd::this_thread::get_id();
	if (!oWinCreate(&hWnd, StaticWndProc, this, false, *(unsigned int*)&id))
	{
		*_pError = oErrorGetLast();
		sprintf_s(_pErrorString, _szErrorString, oErrorGetLastString() );
	}
	oTRACE("hWND valid");
}

void oWinWindow::Deinitialize()
{
	CallHooks(RESIZING, nullptr, DrawMode, SuperSampleScale(), Desc);
	if (Desc.State == FULLSCREEN)
	{
		DESC copy = Desc;
		copy.State = RESTORED;
		SetDesc(copy);
	}

	if (hWnd)
		MessageQueue->Dispatch(oBIND(&oWinWindow::DestroyWindow1, this));
}

void oWinWindow::DestroyWindow1()
{
	oVB(DestroyWindow(hWnd));
	hWnd = nullptr;
}

bool oWinWindow::CreateDevices(HWND _hWnd, IUnknown* _pUserSpecifiedDevice)
{
	#if oDXVER >= oDXVER_10

		switch (DrawMode)
		{
			case USE_DX11:
				if (!_pUserSpecifiedDevice || E_NOINTERFACE == _pUserSpecifiedDevice->QueryInterface(&D3D11Device))
					return oWinSetLastError(E_NOINTERFACE);
				break;

			case USE_GDI:
				// No devices associated with GDI
				break;

			case USE_D2D:
			{
				oD2DCreateFactory(&D2DFactory);
				if (!D2DFactory)
					return oWinSetLastError(E_NOINTERFACE);

				RECT r = ResolveRect(_hWnd, Desc.ClientPosition, Desc.ClientSize, false);
				if (!_pUserSpecifiedDevice || E_NOINTERFACE == _pUserSpecifiedDevice->QueryInterface(&D3D10Device))
					oVB_RETURN2(oD3D10CreateDevice(oWinRectCenter(r), &D3D10Device));
			
				break;
			}

			oNODEFAULT;
		}

		if (DrawMode == USE_D2D || DrawMode == USE_DX11)
		{
			RECT rClient;
			oVB_RETURN(GetClientRect(_hWnd, &rClient));
			oASSERT(D3D10Device || D3D11Device, "D3D Device should have been resolved by now");
			IUnknown* pD3DDevice = D3D11Device;
			if (!pD3DDevice)
				pD3DDevice = D3D10Device;
			oVB_RETURN2(oDXGICreateSwapChain(pD3DDevice, false, oWinRectW(rClient), oWinRectH(rClient), DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, _hWnd, &DXGISwapChain));
		}
	#endif

	return true;
}

bool oWinWindow::CreateDeviceResources(HWND _hWnd)
{
	RECT rClient;
	oVB(GetClientRect(_hWnd, &rClient));

	switch (DrawMode)
	{
		#if oDXVER >= oDXVER_10
			case USE_DX11:
			{
				oASSERT(D3D10Device, "D3D11Device should be resolved by now. Check ctor.");
				oASSERT(DXGISwapChain, "DXGISwapChain should be resolved by now. Check ctor.");
				oRef<ID3D11Texture2D> RT;
				oVB_RETURN2(DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
				oVB_RETURN2(D3D11Device->CreateRenderTargetView(RT, nullptr, &D3D11RenderTargetView));
				if (!oD2DCreateRenderTarget(D2DFactory, DXGISwapChain, SWAP_CHAIN_FORMAT, Desc.UseAntialiasing, &D2DRenderTarget))
					return false;
				CallHooks(RESIZED, D2DRenderTarget, DrawMode, 1, Desc);
				break;
			}

			case USE_D2D:
			{
				oASSERT(D3D10Device, "D3D10Device should be resolved by now. Check ctor.");
				oASSERT(DXGISwapChain, "DXGISwapChain should be resolved by now. Check ctor.");
				oVB_RETURN2(DXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3D10RenderTarget));
				if (!oD2DCreateRenderTarget(D2DFactory, DXGISwapChain, SWAP_CHAIN_FORMAT, Desc.UseAntialiasing, &D2DRenderTarget))
					return false;
				CallHooks(RESIZED, D2DRenderTarget, DrawMode, 1, Desc);
				break;
			}
		#endif

		case USE_GDI:
		{
			hClearBrush = oGDICreateBrush(Desc.ClearColor);
			HDC hDC = GetDC(_hWnd);
			hOffscreenAADC = CreateCompatibleDC(hDC);
			hOffscreenDC = CreateCompatibleDC(hOffscreenAADC);
			hOffscreenAABmp = CreateCompatibleBitmap(hDC, SuperSampleScale() * oWinRectW(rClient), SuperSampleScale() * oWinRectH(rClient));
			hOffscreenBmp = CreateCompatibleBitmap(hDC, oWinRectW(rClient), oWinRectH(rClient));
			SelectObject(hOffscreenAADC, hOffscreenAABmp);
			SelectObject(hOffscreenDC, hOffscreenBmp);
			ReleaseDC(_hWnd, hDC);
			CallHooks(RESIZED, hOffscreenDC, DrawMode, SuperSampleScale(), Desc);
			break;
		}

		oNODEFAULT;
	}

	return true;
}

bool oWinWindow::DestroyDeviceResources()
{
	CallHooks(RESIZING, nullptr, DrawMode, SuperSampleScale(), Desc);

	#if oDXVER >= oDXVER_10
		D2DRenderTarget = nullptr;
		D3D11RenderTargetView = nullptr;
		D3D10RenderTarget = nullptr;
	#endif

	hClearBrush = nullptr;
	hOffscreenAABmp = nullptr;
	hOffscreenBmp = nullptr;
	hOffscreenAADC = nullptr;
	hOffscreenDC = nullptr;

	return true;
}

bool oWinWindow::CallHooks(EVENT _Event, void* _pDrawContext, DRAW_MODE _DrawMode, unsigned int _SuperSampleScale, const DESC& _Desc)
{
	bool result = true;
	for (std::vector<HookFn>::iterator it = Hooks.begin(); it != Hooks.end(); ++it)
		if (*it)
			if (!(*it)(_Event, _SuperSampleScale, _Desc))
				result = false;
	return result;
}

const oGUID& oGetGUID(threadsafe const oWindow* threadsafe const*)
{
	// {963AD3B4-0F53-4e6f-8D6D-C0FBB3CB58A1}
	static const oGUID oIIDMWindow = { 0x963ad3b4, 0xf53, 0x4e6f, { 0x8d, 0x6d, 0xc0, 0xfb, 0xb3, 0xcb, 0x58, 0xa1 } };
	return oIIDMWindow;
}

bool oWinWindow::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	static const GUID GUID_IDXGISwapChainGUID = __uuidof(IDXGISwapChain);
	static const GUID GUID_ID3D10Texture2D = __uuidof(ID3D10Texture2D);

	if (oGetGUID<oWindow>() == _InterfaceID)
	{
		Reference();
		*_ppInterface = this;
		return true;
	}

	else if (__uuidof(ID2D1RenderTarget) == *(GUID*)&_InterfaceID && D2DRenderTarget)
	{
		D2DRenderTarget->AddRef();
		*_ppInterface = D2DRenderTarget;
		return true;
	}

	else if (__uuidof(IDXGISwapChain) == *(GUID*)&_InterfaceID && DXGISwapChain)
	{
		DXGISwapChain->AddRef();
		*_ppInterface = DXGISwapChain;
		return true;
	}

	else if (__uuidof(ID3D10Texture2D) == *(GUID*)&_InterfaceID && D3D10RenderTarget)
	{
		D3D10RenderTarget->AddRef();
		*_ppInterface = D3D10RenderTarget;
		return true;
	}
	
	else if (__uuidof(ID3D11RenderTargetView) == *(GUID*)&_InterfaceID && D3D11RenderTargetView)
	{
		D3D11RenderTargetView->AddRef();
		*_ppInterface = D3D11RenderTargetView;
		return true;
	}

	else if (__uuidof(ID3D10Device) == *(GUID*)&_InterfaceID && D3D10Device)
	{
		D3D10Device->AddRef();
		*_ppInterface = D3D10Device;
		return true;
	}

	else if (__uuidof(ID3D11Device) == *(GUID*)&_InterfaceID && D3D11Device)
	{
		D3D11Device->AddRef();
		*_ppInterface = D3D11Device;
		return true;
	}

	else if (oGetGUID<oHDCAA>() == _InterfaceID && hOffscreenAADC)
	{
		*_ppInterface = hOffscreenAADC;
		return true;
	}

	else if (oGetGUID<oHDC>() == _InterfaceID && hOffscreenDC)
	{
		*_ppInterface = hOffscreenDC;
		return true;
	}

	return oErrorSetLast(oERROR_NOT_FOUND);
}

bool oWindowCreate(const oWindow::DESC& _Desc, void* _pAssociatedNativeHandle, oWindow::DRAW_MODE _Mode, threadsafe oWindow** _ppWindow)
{
	if (!_ppWindow)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	bool success = false;
	oCONSTRUCT(_ppWindow, oWinWindow(_Desc, _pAssociatedNativeHandle, _Mode, &success));

	if (success)
	{
		oGDIScopedObject<HICON> hIcon = (HICON)oLoadStandardIcon();
		oVERIFY(thread_cast<oWindow*>(*_ppWindow)->SetProperty("Icon", &hIcon));
	}

	return success;
}

void oWinWindow::Run()
{
	if (hWnd)
	{
		if (Desc.BackgroundSleepMS && !oWinHasFocus(hWnd))
			oSleep(Desc.BackgroundSleepMS);

		MSG msg;
		if (GetMessage(&msg, hWnd, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		MessageQueue->Dispatch(RunFunction);
	}
}

bool oWinWindow::D2DPaint(HWND _hWnd, bool _Force)
{
	oASSERT(D2DRenderTarget, "D2DRenderTarget not properly created");
	// Clear the buffer. This is kinda lame. Maybe Clear() should just be pushed to the user?
	{
		D2DRenderTarget->BeginDraw();
		D2DRenderTarget->Clear(oD2DAsColorF(Desc.ClearColor));
		oV(D2DRenderTarget->EndDraw());
	}

	CallHooks(DRAW_BACKBUFFER, D2DRenderTarget, DrawMode, 1, Desc);

	D2DRenderTarget->BeginDraw();
	D2DRenderTarget->SetTransform(D2D1::IdentityMatrix());

	CallHooks(DRAW_UIAA, D2DRenderTarget, DrawMode, 1, Desc);
	CallHooks(DRAW_UI, D2DRenderTarget, DrawMode, 1, Desc);

	oV(D2DRenderTarget->EndDraw());
	oV(DXGISwapChain->Present(Desc.FullscreenVSync ? 1 : 0, 0));

	CallHooks(DRAW_FRONTBUFFER, D2DRenderTarget, DrawMode, 1, Desc);
	return true;
}

bool oWinWindow::GDIPaint(HWND _hWnd, HDC _hDC, bool _Force)
{
	RECT r;
	oVB(GetClientRect(_hWnd, &r));
	FillRect(hOffscreenAADC, &r, hClearBrush);
	CallHooks(DRAW_BACKBUFFER, hOffscreenAADC, DrawMode, SuperSampleScale(), Desc);
	CallHooks(DRAW_UIAA, hOffscreenAADC, DrawMode, SuperSampleScale(), Desc);

	int oldMode = GetStretchBltMode(hOffscreenDC);
	SetStretchBltMode(hOffscreenDC, HALFTONE);
	
	int2 Size = oWinRectSize(r);
	int2 SSSize = Size * SuperSampleScale();

	StretchBlt(hOffscreenDC, r.left, r.top, Size.x, Size.y, hOffscreenAADC, 0, 0, SSSize.x, SSSize.y, SRCCOPY);
	SetStretchBltMode(hOffscreenDC, oldMode);
	CallHooks(DRAW_UI, hOffscreenDC, DrawMode, 1, Desc);

	BitBlt(_hDC, r.left, r.top, oWinRectW(r), oWinRectH(r), hOffscreenDC, 0, 0, SRCCOPY);
	CallHooks(DRAW_FRONTBUFFER, _hDC, DrawMode, 1, Desc);

	return true;
}

//#define DEBUGGING_WINDOWS_MESSAGES
#ifdef DEBUGGING_WINDOWS_MESSAGES
	oDEFINE_WNDPROC_DEBUG(oWinWindow, StaticWndProc);
#else
	oDEFINE_WNDPROC(oWinWindow, StaticWndProc);
#endif

LRESULT oWinWindow::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		// handle WM_ERASEBKGND for flicker-free client area update
		// http://msdn.microsoft.com/en-us/library/ms969905.aspx
		case WM_ERASEBKGND:
			return 1;

		case WM_DISPLAYCHANGE:
			Refresh(false);
			break;

		case WM_PRINT:
		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			bool Handled = false;
			PAINTSTRUCT ps;
			oVB(BeginPaint(_hWnd, &ps));

			switch (DrawMode)
			{
				case USE_D2D:
					if (D2DPaint(_hWnd))
						return 0;
				case USE_GDI:
					if (GDIPaint(_hWnd, ps.hdc))
						return 0;
				default:
					break;
			}

			EndPaint(_hWnd, &ps);
			if (Handled)
				return 0;
			break;
		}

		case WM_SETCURSOR:
		{
			if (LOWORD(_lParam) == HTCLIENT)
			{
				oWinSetCursorState(_hWnd, Desc.CursorState, hCursor);
				break;
			}

			else
			{
				oWinCursorSetVisible(true);
				break;
			}
		}

		case WM_MOVE:
		{
			#ifdef _DEBUG
				// Each window gets its own thread, so this should be unique per window
				static thread_local void* sLastOutput = nullptr;
			#endif

			if (DXGISwapChain)
			{
				oRef<IDXGIOutput> Output;
				if (SUCCEEDED(DXGISwapChain->GetContainingOutput(&Output)))
				{
					DXGI_OUTPUT_DESC od;
					Output->GetDesc(&od);
					oStringS DeviceName = od.DeviceName;
					#ifdef _DEBUG
						if (Output != sLastOutput)
							oTRACE("HWND %x is on monitor %s", hWnd, DeviceName.c_str());
						sLastOutput = Output;
					#endif
				}
				else
				{
					#ifdef _DEBUG
						if (sLastOutput != nullptr)
							oTRACE("HWND %x is on monitor ???", hWnd);
						sLastOutput = nullptr;
					#endif
				}
			}

			oLockGuard<oSharedMutex> lock(DescMutex);
			PendingDesc.ClientPosition = Desc.ClientPosition = int2(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
			return 0;
		}

		case WM_SIZE:
		{
			oVERIFY(DestroyDeviceResources());
			#if oDXVER >= oDXVER_10
				if (DXGISwapChain && _wParam != SIZE_MINIMIZED)
				{
					DXGI_SWAP_CHAIN_DESC d;
					DXGISwapChain->GetDesc(&d);
					HRESULT HR = DXGISwapChain->ResizeBuffers(d.BufferCount, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam), d.BufferDesc.Format, d.Flags);
					if (HR == DXGI_ERROR_INVALID_CALL)
						oASSERTA(false, "DXGISwapChain->ResizeBuffers() cannot occur because there still are dependent resources in client code. Ensure all dependent resources are freed on the PRERESIZING event when implementing a SizeMove hook.");
					else
						oV(HR);
				}
			#endif
			{
				oLockGuard<oSharedMutex> lock(DescMutex);
				PendingDesc.ClientSize = Desc.ClientSize = int2(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
			}
			oVERIFY(CreateDeviceResources(_hWnd));
			return 0;
		}

		case WM_CREATE:
			if (!CreateDevices(_hWnd, pUserSpecifiedDevice))
				return -1;
			return 0;

		case WM_CLOSE:
			if (CallHooks(CLOSING, nullptr, DrawMode, 1, Desc))
			{
				if (CloseConfirmed.compare_exchange(false, true))
				{
					DESC* d = Map();
					d->State = HIDDEN;
					Unmap();
				}
			}

			return 0;

		case WM_DESTROY:
			CallHooks(CLOSED, nullptr, DrawMode, 1, Desc);
			return 0;

		// ALT-F4 turns into a WM_CLOSE in DefWindowProc() on key down... up is too late
		// Other than situations like this
		case WM_SYSKEYDOWN:
			if (_wParam == VK_F4 && !Desc.AllowUserKeyboardClose)
				return 0;
			break;

		case WM_SYSKEYUP:
			if (_wParam == VK_RETURN && Desc.AllowUserFullscreenToggle)
				MessageQueue->Dispatch(oBIND(&oWinWindow::ToggleFullscreen, QThis()));
			break;

		default:
			break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

void* oWinWindow::GetNativeHandle() threadsafe
{
	return hWnd;
}

oWindow::DRAW_MODE oWinWindow::GetDrawMode() const threadsafe
{
	return DrawMode;
}

bool oWinWindow::HasFocus() const threadsafe
{
	return oWinHasFocus(hWnd);
}

void oWinWindow::SetFocus() threadsafe
{
	oWinSetFocus(hWnd);
}

char* oWinWindow::GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe
{
	return oWinGetTitle(hWnd, _StrDestination, _SizeofStrDestination) ? _StrDestination : nullptr;
}

void oWinWindow::SetTitle(const char* _Title) threadsafe
{
	MessageQueue->Dispatch(oBIND(&oWinWindow::SetTitle1, QThis(), oStringM(oSAFESTR(_Title))));
	oWinWake(hWnd);
}

void oWinWindow::SetTitle1(oStringM _Title)
{
	oWinSetTitle(hWnd, _Title);
}

const void* oWinWindow::GetProperty(const char* _Name) const
{
	if (_Name)
	{
		if (!_stricmp("Icon", _Name)) return &hIcon;
		if (!_stricmp("Cursor", _Name)) return &hCursor;
	}

	return 0;
}

bool oWinWindow::SetProperty(const char* _Name, void* _pValue)
{
	bool result = false;
	if (hWnd && _Name)
	{
		if (!_stricmp("Icon", _Name))
		{
			MessageQueue->Dispatch(oBIND(&oWinWindow::SetIcon, QThis(), *(HICON*)_pValue));
			result = true;
		}
		
		else if (!_stricmp("Cursor", _Name))
		{
			MessageQueue->Dispatch(oBIND(&oWinWindow::SetCursor, QThis(), *(HCURSOR*)_pValue));
			result = true;
		}
	}

	else
		oTRACE("HWND INVALID!!");

	if (result)
		oWinWake(hWnd);

	return result;
}

void oWinWindow::SetIcon(HICON _hIcon)
{
	hIcon = _hIcon;
	oWinSetIcon(hWnd, hIcon, true);
}

void oWinWindow::SetCursor(HCURSOR _hCursor)
{
	hCursor = _hCursor;
	oWinSetCursor(hWnd, hCursor);
}

void oWinWindow::GetDesc(DESC* _pDesc) threadsafe
{
	oSharedLock<oSharedMutex> lock(DescMutex);
	*_pDesc = thread_cast<DESC&>(Desc);
}

oWindow::DESC* oWinWindow::Map() threadsafe
{
	DescMutex.lock();
	return thread_cast<DESC*>(&PendingDesc);
}

void oWinWindow::Unmap() threadsafe
{
	DescMutex.unlock();
	MessageQueue->Dispatch(oBIND(&oWinWindow::SetDesc, QThis(), thread_cast<DESC&>(PendingDesc)));
	oWinWake(hWnd);
}

void oWinWindow::SetDesc(DESC _Desc)
{
	oASSERT(Desc.ClearColor == _Desc.ClearColor, "Changing clear color is not yet supported.");

	oVERIFY(oWinSetAlwaysOnTop(hWnd, _Desc.AlwaysOnTop));
	EnableWindow(hWnd, _Desc.Enabled);
	
	// going to fullscreen completely discards state and style. Position is only
	// used to get a screen onto which fullscreen will be applied.
	if (_Desc.State == FULLSCREEN && Desc.State != FULLSCREEN)
	{
		// Move to the monitor we're going to go fullscreen on. Doing it here rather
		// than directly going fullscreen on an adapter allows us to call 
		// ResizeTarget in the MS-recommended way.
		// Don't record this move, let oWindow ignore position for any reason other
		// than placing the window on the right monitor.

		if (_Desc.ClientPosition.x == oDEFAULT)
			_Desc.ClientPosition.x = Desc.ClientPosition.x;
		if (_Desc.ClientPosition.y == oDEFAULT)
			_Desc.ClientPosition.y = Desc.ClientPosition.y;

		oWinSetPosition(hWnd, _Desc.ClientPosition);

		PreFullscreenDesc = Desc;
		if (PreFullscreenDesc.State == FULLSCREEN)
			PreFullscreenDesc.State = RESTORED;

		oTRACE("HWND %x Windowed->Fullscreen", hWnd);
		
		if (DXGISwapChain)
		{
			if (!oDXGISetFullscreenState(DXGISwapChain, true, _Desc.FullscreenSize, _Desc.FullscreenRefreshRate))
			{
				if (oERROR_REFUSED == oErrorGetLast())
				{
					oStringM title;
					oMSGBOX_DESC mb;
					mb.Type = oMSGBOX_ERR;
					mb.Title = GetTitle(title.c_str(), title.capacity());
					mb.ParentNativeHandle = hWnd;
					oMsgBox(mb, oErrorGetLastString());
					oWindow::DESC	copy = _Desc;
					copy.State = RESTORED;
					SetDesc(copy);
					return;
				}

				else
					oVERIFY(false);
			}

			// Because oDXGISetFullscreenState flushes the message queue, we could end
			// up eating an oWinWake() message from another thread, so send another just
			// in case...
			oWinWake(hWnd);
		}

		else
		{
			// Just make this a borderless fullscreen window
			oDISPLAY_DESC dd;
			oVERIFY(oDisplayEnum(oWinGetDisplayIndex(hWnd), &dd));
			RECT r = oWinRectWH(dd.WorkareaPosition, dd.Mode.Size);
			oVERIFY(oWinSetStyle(hWnd, oAsWinStyle(oWindow::BORDERLESS), &r));
			oVERIFY(oWinSetState(hWnd, oAsWinState(oWindow::RESTORED), true));
		}
	}

	else 
	{
		if (Desc.State == FULLSCREEN && _Desc.State != FULLSCREEN)
		{
			oTRACE("HWND %x Fullscreen->Windowed", hWnd);
			if (DXGISwapChain)
			{
				oVERIFY(oDXGISetFullscreenState(DXGISwapChain, false));
				oWinWake(hWnd);
			}

			else
			{
				Desc.State = PreFullscreenDesc.State; // prevents infinite recursion
				SetDesc(PreFullscreenDesc);
			}
		}

		RECT rClient = ResolveRect(hWnd, _Desc.ClientPosition, _Desc.ClientSize, _Desc.State == FULLSCREEN);
		_Desc.ClientPosition = oWinRectPosition(rClient);
		_Desc.ClientSize = oWinRectSize(rClient);
		oVERIFY(oWinSetStyle(hWnd, oAsWinStyle(_Desc.Style), &rClient));
		oVERIFY(oWinSetState(hWnd, oAsWinState(_Desc.State), _Desc.HasFocus));
	}

	oLockGuard<oSharedMutex> lock(DescMutex);
	Desc = _Desc;
}

bool oWinWindow::IsOpen() const threadsafe
{
	return !CloseConfirmed;
}

void oWinWindow::Refresh(bool _Blocking, const oRECT* _pDirtyRect) threadsafe
{
	REFRESH r;
	r.UseRect = !!_pDirtyRect;
	if (r.UseRect)
		r.Rect = *_pDirtyRect;

	if (_Blocking)
	{
		oEvent done;
		r.pEvent = &done;
		MessageQueue->Dispatch(oBIND(&oWinWindow::Refresh1, QThis(), r));
		oWinWake(hWnd);
		done.Wait();
	}
	
	else
	{
		r.pEvent = nullptr;
		MessageQueue->Dispatch(oBIND(&oWinWindow::Refresh1, QThis(), r));
		oWinWake(hWnd);
	}
}

void oWinWindow::Refresh1(REFRESH _Refresh)
{
	RECT r;
	RECT* pRect = nullptr;
	if (_Refresh.UseRect)
	{
		r = oWinRectWH(_Refresh.Rect.GetMin(), _Refresh.Rect.GetDimensions());
		pRect = &r;
	}
	
	if (_Refresh.pEvent)
	{
		oVB(RedrawWindow(hWnd, pRect, nullptr, RDW_UPDATENOW|RDW_INVALIDATE));
		_Refresh.pEvent->Set();
	}

	else
		InvalidateRect(hWnd, pRect, TRUE);
}

void oWinWindow::ToggleFullscreen()
{
	if (Desc.State == FULLSCREEN)
		SetDesc(PreFullscreenDesc);
	else
	{
		DESC newDesc = PreFullscreenDesc = Desc;
		newDesc.State = FULLSCREEN;
		SetDesc(newDesc);
	}
}

bool oWinWindow::CreateSnapshot(oImage** _ppImage, bool _IncludeBorder) threadsafe
{
	oEvent Finished;
	bool success = false;
	MessageQueue->Dispatch(oBIND(&oWinWindow::CreateSnapshot1, QThis(), _ppImage, _IncludeBorder, &Finished, &success));
	oWinWake(hWnd);
	Finished.Wait();
	return success;
}

void oWinWindow::CreateSnapshot1(oImage** _ppImage, bool _IncludeBorder, threadsafe oEvent* _pDone, bool* _pSuccess)
{
	*_pSuccess = false;
	oTRACE("HWND 0x%x CreateSnapshot1(%s)", hWnd, _IncludeBorder ? "true" : "false");

	if (Desc.State == FULLSCREEN && DXGISwapChain)
	{
		if (D3D10RenderTarget)
			oVERIFY(oD3D10CreateSnapshot(D3D10RenderTarget, _ppImage));
		else if (D3D11RenderTargetView)
		{
			oRef<ID3D11Texture2D> D3D11RenderTarget;
			D3D11RenderTargetView->GetResource((ID3D11Resource**)&D3D11RenderTarget);
			oVERIFY(oD3D11CreateSnapshot(D3D11RenderTarget, _ppImage));
		}

		else
		{
			oErrorSetLast(oERROR_NOT_FOUND, "Unsupported fullscreen render target - snapshot cannot be created");
			*_pSuccess = false;
			_pDone->Set();
			return;
		}
	}

	else 
	{
		void* buf = nullptr;
		size_t size = 0;
		SetFocus(); // Windows doesn't do well with hidden contents.
		if (oGDIScreenCaptureWindow(hWnd, _IncludeBorder, malloc, &buf, &size))
		{
			*_pSuccess = oImageCreate("Screen capture", buf, size, oImage::ForceAlpha, _ppImage);
			if (buf)
				free(buf);
		}
	}

	*_pSuccess = true;
	_pDone->Set();
}

unsigned int oWinWindow::Hook(HookFn _Hook) threadsafe
{
	oASSERT(_Hook, "A valid hook function must be specified");
	oERROR err = oERROR_NONE;
	oStringL ErrStr;
	oEvent IndexValid;
	size_t index = oInvalid;
	MessageQueue->Dispatch(oBIND(&oWinWindow::Hook1, QThis(), _Hook, &IndexValid, &index, &err, ErrStr.c_str(), ErrStr.capacity()));
	oWinWake(hWnd);
	IndexValid.Wait();

	if (err)
	{
		oErrorSetLast(err, ErrStr);
		return oInvalid;
	}

	return static_cast<unsigned int>(index);
}

void oWinWindow::Hook1(HookFn _Hook, threadsafe oEvent* _pDone, size_t* _pIndex, oERROR* _pLastError, char* _StrLastError, size_t _SizeofStrLastError)
{
	bool result = _Hook(RESIZED, SuperSampleScale(), Desc);
	if (result)
	{
		*_pIndex = oSparseSet(Hooks, _Hook);
		oTRACE("HWND 0x%x Hook1(%u)", hWnd, *_pIndex);
		InvalidateRect(hWnd, nullptr, FALSE);
	}

	else
	{
		oTRACE("HWND 0x%x Hook1() failed: %s: %s", hWnd, oAsString(oErrorGetLast()), oErrorGetLastString());
		*_pIndex = oInvalid;
		*_pLastError = oErrorGetLast();
		strcpy_s(_StrLastError, _SizeofStrLastError, oErrorGetLastString());
	}

	_pDone->Set();
}

void oWinWindow::Unhook(unsigned int _HookID) threadsafe
{
	if (_HookID != oInvalid)
	{
		oEvent HookRemoved;
		MessageQueue->Dispatch(oBIND(&oWinWindow::Unhook1, QThis(), _HookID, &HookRemoved));
		oWinWake(hWnd);
		HookRemoved.Wait();
	}
}

void oWinWindow::Unhook1(size_t _Index, threadsafe oEvent* _pDone)
{
	oTRACE("HWND 0x%x Unhook1(%u)", hWnd, _Index);
	Hooks[_Index](RESIZING, SuperSampleScale(), Desc);
	Hooks[_Index] = nullptr;
	InvalidateRect(hWnd, nullptr, FALSE);
	_pDone->Set();
}
