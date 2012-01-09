// $(header)
#include <oBasis/oFixedString.h>
#include <oPlatform/oDXGI.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinWindowing.h>

#if oDXVER >= oDXVER_10

// {CF4B314B-E3BC-4DCF-BDE7-86040A0ED295}
static const GUID oWKPDID_PreFullscreenMode = { 0xcf4b314b, 0xe3bc, 0x4dcf, { 0xbd, 0xe7, 0x86, 0x4, 0xa, 0xe, 0xd2, 0x95 } };

static const char* oWinDXGI_exports[] = 
{
	"CreateDXGIFactory1",
};

struct oWinDXGI : oModuleSingleton<oWinDXGI>
{
	oWinDXGI() { hModule = oModuleLink("dxgi.dll", oWinDXGI_exports, (void**)&CreateDXGIFactory1, oCOUNTOF(oWinDXGI_exports)); oASSERT(hModule, ""); }
	~oWinDXGI() { oModuleUnlink(hModule); }
	HRESULT (__stdcall *CreateDXGIFactory1)(REFIID riid, void **ppFactory);
protected:
	oHMODULE hModule;
};

oSURFACE_FORMAT oDXGIToSurfaceFormat(DXGI_FORMAT _Format)
{
	// @oooii-tony: For now, oSURFACE_FORMAT and DXGI_FORMAT are the same thing.
	return static_cast<oSURFACE_FORMAT>(_Format <= DXGI_FORMAT_BC7_UNORM_SRGB ? _Format : DXGI_FORMAT_UNKNOWN);
}

DXGI_FORMAT oDXGIFromSurfaceFormat(oSURFACE_FORMAT _Format)
{
	// @oooii-tony: For now, oSURFACE_FORMAT and DXGI_FORMAT are the same thing.
	return static_cast<DXGI_FORMAT>(_Format <= DXGI_FORMAT_BC7_UNORM_SRGB ? _Format : DXGI_FORMAT_UNKNOWN);
}

bool oDXGICreateFactory(IDXGIFactory1** _ppFactory)
{
	HRESULT hr = oWinDXGI::Singleton()->CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)_ppFactory); 
	if (FAILED(hr))
	{
		oWinSetLastError(hr);
		return false;
	}

	return true;
}

bool oDXGICreateSwapChain(IUnknown* _pDevice, bool _Fullscreen, UINT _Width, UINT _Height, DXGI_FORMAT _Format, UINT RefreshRateN, UINT RefreshRateD, HWND _hWnd, IDXGISwapChain** _ppSwapChain)
{
	if (!_pDevice)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC d;
	d.BufferDesc.Width = _Width;
	d.BufferDesc.Height = _Height;
	d.BufferDesc.RefreshRate.Numerator = RefreshRateN;
	d.BufferDesc.RefreshRate.Denominator = RefreshRateD;
	d.BufferDesc.Format = _Format;
	d.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	d.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	d.SampleDesc.Count = 1;
	d.SampleDesc.Quality = 0;
	d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	d.BufferCount = 3;
	d.OutputWindow = _hWnd;
	d.Windowed = !_Fullscreen;
	d.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	// @oooii-tony: 8/31/2011, DX Jun2010 SDK: If DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE 
	// is specified, DXGISwapChain can cause a DXGI_ERROR_DEVICE_REMOVED when 
	// going from fullscreen to windowed, basically indicating a crash in the driver.
	//d.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	d.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	oRef<IDXGIDevice> D3DDevice;
	oVB_RETURN2(_pDevice->QueryInterface(&D3DDevice));

	oRef<IDXGIAdapter> Adapter;
	oVB_RETURN2(D3DDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&Adapter));

	oRef<IDXGIFactory> Factory;
	oVB_RETURN2(Adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory));
	oVB_RETURN2(Factory->CreateSwapChain(_pDevice, &d, _ppSwapChain));
	
	// Auto Alt-Enter is nice, but there are threading issues to be concerned 
	// about, so we need to control this explicitly in applications.
	oVB_RETURN2(Factory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER));

	return true;
}

static float oDXGIGetRefreshRate(const DXGI_MODE_DESC& _Mode)
{
	return _Mode.RefreshRate.Numerator / static_cast<float>(_Mode.RefreshRate.Denominator);
}

static DXGI_RATIONAL oDXGIGetRefreshRate(int _RefreshRate)
{
	DXGI_RATIONAL r;
	r.Numerator = _RefreshRate;
	r.Denominator = 1;
	return r;
}

bool oDXGISetFullscreenState(IDXGISwapChain* _pSwapChain, bool _Fullscreen, const int2& _FullscreenSize, int _FullscreenRefreshRate)
{
	if (!_pSwapChain)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC SCDesc;
	
	// Confirm we're on the same thread as the message pump
	// http://msdn.microsoft.com/en-us/library/ee417025(v=vs.85).aspx
	// "Multithreading and DXGI"
	_pSwapChain->GetDesc(&SCDesc);

	if (GetCurrentThreadId() != GetWindowThreadProcessId(SCDesc.OutputWindow, nullptr))
	{
		oErrorSetLast(oERROR_WRONG_THREAD, "oDXGISetFullscreenState called from thread %d for hwnd %x pumping messages on thread %d", GetCurrentThreadId(), SCDesc.OutputWindow, GetWindowThreadProcessId(SCDesc.OutputWindow, nullptr));
		return false;
	}

	oRef<IDXGIOutput> Output;
	HRESULT HR = _pSwapChain->GetContainingOutput(&Output);
	if (HR == DXGI_ERROR_UNSUPPORTED)
	{
		RECT r;
		oVB(GetWindowRect(SCDesc.OutputWindow, &r));
		oRef<IDXGIFactory1> Factory;
		oVERIFY(oDXGIGetFactory(_pSwapChain, &Factory));
		oRef<IDXGIOutput> Output;
		oVERIFY(oDXGIFindOutput(Factory, oWinRectPosition(r), &Output));
		oRef<IDXGIAdapter1> Adapter;
		oVERIFY(oDXGIGetAdapter(Output, &Adapter));
		DXGI_ADAPTER_DESC1 adesc;
		Adapter->GetDesc1(&adesc);

		if (FAILED(_pSwapChain->GetContainingOutput(&Output)))
		{
			oStringM WinTitle;
			oWinGetTitle(SCDesc.OutputWindow, WinTitle.c_str(), WinTitle.capacity());
			oErrorSetLast(oERROR_REFUSED, "SetFullscreenState failed on adapter \"%s\" because the IDXGISwapChain created with the associated window entitled \"%s\" was created with another adapter. Cross-adapter exclusive mode is not currently (DXGI 1.1) supported.", oStringS(adesc.Description), WinTitle);
		}
		else 
			oErrorSetLast(oERROR_INVALID_PARAMETER, "SetFullscreenState failed though the attempt was on the same adapters with which the swapchain was created");

		return false;
	}

	else oVB_RETURN2(HR);

	DXGI_OUTPUT_DESC ODesc;
	Output->GetDesc(&ODesc);

	if (_Fullscreen)
		oDXGISetPreFullscreenMode(_pSwapChain, oWinRectSize(ODesc.DesktopCoordinates), static_cast<int>(oDXGIGetRefreshRate(SCDesc.BufferDesc)));

	SCDesc.BufferDesc.Width = _FullscreenSize.x == oDEFAULT ? oWinRectW(ODesc.DesktopCoordinates) : _FullscreenSize.x;
	SCDesc.BufferDesc.Height = _FullscreenSize.y == oDEFAULT ? oWinRectH(ODesc.DesktopCoordinates) : _FullscreenSize.y;

	if (_FullscreenRefreshRate == oDEFAULT)
		SCDesc.BufferDesc.RefreshRate = oDXGIGetRefreshRate(0);
	else
		SCDesc.BufferDesc.RefreshRate = oDXGIGetRefreshRate(_FullscreenRefreshRate);

	DXGI_MODE_DESC closestMatch;
	oV(Output->FindClosestMatchingMode(&SCDesc.BufferDesc, &closestMatch, nullptr));

	if (closestMatch.Width != SCDesc.BufferDesc.Width || closestMatch.Height != SCDesc.BufferDesc.Height || oEqual(oDXGIGetRefreshRate(SCDesc.BufferDesc), oDXGIGetRefreshRate(closestMatch)))
		oTRACE("SetFullscreenState initiated by HWND 0x%x asked for %dx%d@%dHz and will instead set the closest match %dx%d@%.02fHz", SCDesc.OutputWindow, SCDesc.BufferDesc.Width, SCDesc.BufferDesc.Height, SCDesc.BufferDesc.RefreshRate.Numerator, closestMatch.Width, closestMatch.Height, oDXGIGetRefreshRate(closestMatch));

	// Move to 0,0 on the screen because resize targets will resize the window,
	// in which case the evaluation of which output the window is on will change.
	oWinSetPosition(SCDesc.OutputWindow, int2(ODesc.DesktopCoordinates.left, ODesc.DesktopCoordinates.top));

	if (_Fullscreen)
	{
		_pSwapChain->ResizeBuffers(SCDesc.BufferCount, closestMatch.Width, closestMatch.Height, SCDesc.BufferDesc.Format, SCDesc.Flags);

		oVB_RETURN2(_pSwapChain->ResizeTarget(&closestMatch));
	}

	oVB_RETURN2(_pSwapChain->SetFullscreenState(_Fullscreen, nullptr));
	// Ensure the refresh rate is matched against the fullscreen mode
	// http://msdn.microsoft.com/en-us/library/ee417025(v=vs.85).aspx
	// "Full-Screen Issues"
	if (_Fullscreen)
	{
		_pSwapChain->GetDesc(&SCDesc);
		SCDesc.BufferDesc.RefreshRate.Numerator = 0;
		SCDesc.BufferDesc.RefreshRate.Denominator = 0;
		oVB_RETURN2(_pSwapChain->ResizeTarget(&SCDesc.BufferDesc));
	}

	else
	{
		// Restore prior resolution (not the one necessarily saved in the registry)
		oDISPLAY_MODE mode;
  		oVERIFY(oDXGIGetPreFullscreenMode(_pSwapChain, &mode.Size, &mode.RefreshRate));

		#ifdef _DEBUG
			oStringS rate;
			if (mode.RefreshRate == oDEFAULT)
				sprintf_s(rate, "defaultHz");
			else
				sprintf_s(rate, "%dHz", mode.RefreshRate);
			oTRACE("Restoring prior display mode of %dx%d@%s", mode.Size.x, mode.Size.y, rate.c_str());
		#endif

		oVERIFY(oDisplaySetMode(oDXGIFindDisplayIndex(Output), mode));
	}

	// If we're calling a function called "SetFullscreenState", it'd be nice if 
	// the swapchain/window was actually IN that state when we came out of this 
	// call, so sit here and don't allow anyone else to be tricksy until we've
	// flushed the SetFullScreenState messages.
	oWinPumpMessages(SCDesc.OutputWindow, 60000);
	return true;
}

struct oSCREEN_MODE
{
	int2 Size;
	int RefreshRate;
};

bool oDXGISetPreFullscreenMode(IDXGISwapChain* _pSwapChain, const int2& _Size, int _RefreshRate)
{
	if (!_pSwapChain)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	oSCREEN_MODE mode;
	mode.Size = _Size;
	mode.RefreshRate = _RefreshRate;
	oVB_RETURN2(_pSwapChain->SetPrivateData(oWKPDID_PreFullscreenMode, sizeof(mode), &mode));
	return true;
}

bool oDXGIGetPreFullscreenMode(IDXGISwapChain* _pSwapChain, int2* _pSize, int* _pRefreshRate)
{
	if (!_pSwapChain || !_pSwapChain || !_pRefreshRate)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	UINT size;
	oSCREEN_MODE mode;
	oVB_RETURN2(_pSwapChain->GetPrivateData(oWKPDID_PreFullscreenMode, &size, &mode));
	if (size != sizeof(oSCREEN_MODE))
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Size retreived (%u) does not match size requested (%u)", size, sizeof(oSCREEN_MODE));
		return false;
	}

	*_pSize = mode.Size;
	*_pRefreshRate = mode.RefreshRate;
	return true;
}

float oDXGIGetD3DVersion(IDXGIAdapter* _pAdapter)
{
	#if D3D11_MAJOR_VERSION
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D11Device), 0)) return 11.0f;
	#endif
	#ifdef _D3D10_1_CONSTANTS
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device1), 0)) return 10.1f;
	#endif
	#ifdef _D3D10_CONSTANTS
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device), 0)) return 10.0f;
	#endif
	return 0.0f;
}

bool oDXGIEnumOutputs(IDXGIFactory* _pFactory, oFUNCTION<bool(unsigned int _AdapterIndex, IDXGIAdapter* _pAdapter, unsigned int _OutputIndex, IDXGIOutput* _pOutput)> _EnumFunction)
{
	if (!_pFactory || !_EnumFunction)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	oRef<IDXGIAdapter> Adapter;
	unsigned int a = 0;
	while (DXGI_ERROR_NOT_FOUND != _pFactory->EnumAdapters(a++, &Adapter))
	{
		oRef<IDXGIOutput> Output;
		unsigned int o = 0;
		while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(o++, &Output))
			if (!_EnumFunction(a, Adapter, o, Output))
				break;
	}

	return true;
}

static bool MatchHMONITOR(unsigned int _AdapterIndex, IDXGIAdapter* _pAdapter, unsigned int _OutputIndex, IDXGIOutput* _pOutput, HMONITOR _hMonitor, IDXGIOutput** _ppFoundOutput)
{
	DXGI_OUTPUT_DESC desc;
	_pOutput->GetDesc(&desc);
	if (desc.Monitor == _hMonitor)
	{
		_pOutput->AddRef();
		*_ppFoundOutput = _pOutput;
		return false;
	}

	return true;
}

static bool MatchPosition(unsigned int _AdapterIndex, IDXGIAdapter* _pAdapter, unsigned int _OutputIndex, IDXGIOutput* _pOutput, const int2& _VirtualDesktopPosition, IDXGIOutput** _ppFoundOutput)
{
	DXGI_OUTPUT_DESC desc;
	_pOutput->GetDesc(&desc);
	if (oWinRectContains(desc.DesktopCoordinates, _VirtualDesktopPosition))
	{
		_pOutput->AddRef();
		*_ppFoundOutput = _pOutput;
		return false;
	}

	return true;
}

bool oDXGIFindOutput(IDXGIFactory* _pFactory, HMONITOR _hMonitor, IDXGIOutput** _ppOutput)
{
	bool result = oDXGIEnumOutputs(_pFactory, oBIND(MatchHMONITOR, oBIND1, oBIND2, oBIND3, oBIND4, _hMonitor, _ppOutput));
	return result && !!*_ppOutput;
}

bool oDXGIFindOutput(IDXGIFactory* _pFactory, const int2& _VirtualDesktopPosition, IDXGIOutput** _ppOutput)
{
	bool result = oDXGIEnumOutputs(_pFactory, oBIND(MatchPosition, oBIND1, oBIND2, oBIND3, oBIND4, oBINDREF(_VirtualDesktopPosition), _ppOutput));
	return result && !!*_ppOutput;
}

bool oDXGIGetAdapter(IDXGIObject* _pObject, IDXGIAdapter1** _ppAdapter)
{
	if (_pObject)
	{
		oVB_RETURN2(_pObject->GetParent(__uuidof(IDXGIAdapter1), (void**)_ppAdapter));
		return true;
	}

	return false;
}

bool oDXGIGetFactory(IDXGIObject* _pObject, IDXGIFactory1** _ppFactory)
{
	if (_pObject)
	{
		oVB_RETURN2(_pObject->GetParent(__uuidof(IDXGIFactory1), (void**)_ppFactory));
		return true;
	}

	return false;
}

unsigned int oDXGIFindDisplayIndex(IDXGIOutput* _pOutput)
{
	DXGI_OUTPUT_DESC odesc;
	_pOutput->GetDesc(&odesc);
	
	oDISPLAY_DESC ddesc;

	unsigned int index = 0;
	while (oDisplayEnum(index, &ddesc))
	{
		if ((HMONITOR)ddesc.NativeHandle == odesc.Monitor)
			return index;
		index++;
	}

	oErrorSetLast(oERROR_NOT_FOUND);
	return oInvalid;
}

bool oDXGIIsDepthFormat(DXGI_FORMAT _Format)
{
	switch (_Format)
	{
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_TYPELESS:
			return true;
		default:
			break;
	}

	return false;
}

DXGI_FORMAT oDXGIGetDepthCompatibleFormat(DXGI_FORMAT _TypelessDepthFormat)
{
	switch (_TypelessDepthFormat)
	{
		case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case DXGI_FORMAT_R16_TYPELESS: return DXGI_FORMAT_D16_UNORM;
		default: return _TypelessDepthFormat;
	}
}

DXGI_FORMAT oDXGIGetColorCompatibleFormat(DXGI_FORMAT _DepthFormat)
{
	switch (_DepthFormat)
	{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM: return DXGI_FORMAT_R16_UNORM;
		default: return _DepthFormat;
	}
}

#endif // oDXVER >= oDXVER_10
