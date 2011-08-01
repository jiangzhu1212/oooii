// $(header)
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <oooii/oSurface.h>

bool oD3D11Device::CreateRenderTarget2(const char* _Name, threadsafe oWindow* _pWindow, oSurface::FORMAT _DepthStencilFormat, oGfxRenderTarget2** _ppRenderTarget) threadsafe
{
	oGFXCREATE_CHECK_NAME();
	if (!_pWindow)
	{
		oSetLastError(EINVAL, "A window to associate with this new render target must be specified");
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppRenderTarget, oD3D11RenderTarget2(this, _pWindow, _DepthStencilFormat, _Name, &success)); \
	return success;
}

oDEFINE_GFXDEVICE_CREATE(oD3D11, RenderTarget2);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, RenderTarget2)
{
	Desc = _Desc;
	// nullify width/height to force allocation in this call to resize
	Desc.Width = 0;
	Desc.Height = 0;
	Resize(_Desc.Width, _Desc.Height);
	*_pSuccess = true;
}

oD3D11RenderTarget2::oD3D11RenderTarget2(threadsafe oGfxDevice* _pDevice, threadsafe oWindow* _pWindow, oSurface::FORMAT _DSFormat, const char* _Name, bool* _pSuccess)
	: oGfxDeviceChildMixin(_pDevice, _Name)
	, Window(_pWindow)
{
	*_pSuccess = false;
	if (!Window->QueryInterface((const oGUID&)__uuidof(IDXGISwapChain), &DXGISwapChain))
	{
		oSetLastError(ENOSYS, "Could not find an iDXGISwapChain in the specified oWindow");
		return;
	}

	HRESULT hr = DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Texture[0]);
	if (FAILED(hr))
	{
		hr = DXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3D10Texture);
		if (FAILED(hr))
		{
			oWinSetLastError(hr);
			return;
		}
	}

	Desc.MRTCount = 1;
	Desc.ArraySize = 1;
	Desc.GenerateMips = false;
	Desc.ClearDesc = CLEAR_DESC(); // still settable by client code
	Desc.DepthStencilFormat = _DSFormat;

	DXGI_SWAP_CHAIN_DESC SCDesc;
	DXGISwapChain->GetDesc(&SCDesc);
	RecreateDepthBuffer(uint2(SCDesc.BufferDesc.Width, SCDesc.BufferDesc.Height));

	// will be populated on-demand from swapchain
	Desc.Width = 0;
	Desc.Height = 0;
	for (size_t i = 0; i < MAX_MRT_COUNT; i++)
		Desc.Format[i] = oSurface::UNKNOWN;
	*_pSuccess = true;
}

void oD3D11RenderTarget2::GetDesc(DESC* _pDesc) const threadsafe
{
	oRWMutex::ScopedLockRead lock(DescMutex);
	*_pDesc = thread_cast<DESC&>(Desc); // safe because of lock above

	if (DXGISwapChain)
	{
		DXGI_SWAP_CHAIN_DESC d;
		oV(const_cast<IDXGISwapChain*>(DXGISwapChain.c_ptr())->GetDesc(&d));
		_pDesc->Width = d.BufferDesc.Width;
		_pDesc->Height = d.BufferDesc.Height;
		_pDesc->Format[0] = static_cast<oSurface::FORMAT>(d.BufferDesc.Format);
		_pDesc->DepthStencilFormat = oSurface::UNKNOWN;
	}
}

void oD3D11RenderTarget2::SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe
{
	oRWMutex::ScopedLock lock(DescMutex);
	thread_cast<DESC&>(Desc).ClearDesc = _ClearDesc; // safe because of lock above
}

void oD3D11RenderTarget2::RecreateDepthBuffer(const uint2& _Dimensions)
{
	char name[1024];
	if (Desc.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		sprintf_s(name, "%sDS", GetName());
		oD3D11DEVICE();
		oVERIFY(oD3D11CreateRenderTarget(D3DDevice, name, _Dimensions.x, _Dimensions.y, 1, oSurface::GetPlatformFormat<DXGI_FORMAT>(Desc.DepthStencilFormat), &Depth, &DSV, &SRVDepth));
	}
}

void oD3D11RenderTarget2::Resize(uint _Width, uint _Height)
{
	if (D3D10Texture)
	{
		DXGI_SWAP_CHAIN_DESC d;
		DXGISwapChain->GetDesc(&d);
		if (d.BufferDesc.Width != _Width || d.BufferDesc.Height != _Height)
		{
			oTRACE("%s %s Resizing %ux%u to %ux%u", typeid(*this), GetName(), d.BufferDesc.Width, d.BufferDesc.Height, _Width, _Height);
			D3D10Texture = nullptr;
			oV(DXGISwapChain->ResizeBuffers(d.BufferCount, _Width, _Height, d.BufferDesc.Format, d.Flags));
			oV(DXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3D10Texture));
			DXGI_SWAP_CHAIN_DESC SCDesc;
			DXGISwapChain->GetDesc(&SCDesc);
			RecreateDepthBuffer(uint2(SCDesc.BufferDesc.Width, SCDesc.BufferDesc.Height));
		}
	}

	else
	{
		if (_Width != Desc.Width || _Height != Desc.Height)
		{
			oTRACE("%s %s Resize %ux%u -> %ux%u", typeid(*this), GetName(), Desc.Width, Desc.Height, _Width, _Height);

			if (!Desc.Width || !Desc.Height)
			{
				for (unsigned int i = 0; i < oCOUNTOF(Texture); i++)
				{
					Texture[i] = 0;
					RTVs[i] = 0;
					SRVs[i] = 0;
				}

				Depth = 0;
				DSV = 0;
				SRVDepth = 0;
			}

			else
			{
				//oD3D11DEVICE();
				oRef<ID3D11Device> D3DDevice;

				char name[1024];

				for (unsigned int i = 0; i < Desc.ArraySize; i++)
				{
					sprintf_s(name, "%s%02u", GetName(), i);
					oVERIFY(oD3D11CreateRenderTarget(D3DDevice, name, Desc.Width, Desc.Height, Desc.ArraySize, oSurface::GetPlatformFormat<DXGI_FORMAT>(Desc.Format[i]), &Texture[i], &RTVs[i], &SRVs[i]));
				}

				RecreateDepthBuffer(uint2(Desc.Width, Desc.Height));
			}

			oRWMutex::ScopedLock lock(DescMutex);
			Desc.Width = _Width;
			Desc.Height = _Height;
		}
	}
}
