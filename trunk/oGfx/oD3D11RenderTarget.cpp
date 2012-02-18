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
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <oBasis/oSurface.h>
#include <oPlatform/oDXGI.h>

bool oD3D11Device::CreateRenderTarget(const char* _Name, threadsafe oWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, threadsafe oGfxRenderTarget** _ppRenderTarget) threadsafe
{
	oGFXCREATE_CHECK_NAME();
	if (!_pWindow)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "A window to associate with this new render target must be specified");

	bool success = false;
	oCONSTRUCT(_ppRenderTarget, oD3D11RenderTarget(this, _pWindow, _DepthStencilFormat, _Name, &success)); \
	return success;
}

oDEFINE_GFXDEVICE_CREATE(oD3D11, RenderTarget);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, RenderTarget)
{
	Desc = _Desc;
	// nullify width/height to force allocation in this call to resize
	Desc.Dimensions = int2(0,0);
	Resize(_Desc.Dimensions);
	*_pSuccess = true;
}

oD3D11RenderTarget::oD3D11RenderTarget(threadsafe oGfxDevice* _pDevice, threadsafe oWindow* _pWindow, oSURFACE_FORMAT _DSFormat, const char* _Name, bool* _pSuccess)
	: oGfxDeviceChildMixin(_pDevice, _Name)
	, Window(_pWindow)
{
	*_pSuccess = false;

	oRef<IDXGISwapChain> DXGISwapChain;
	if (!Window->QueryInterface((const oGUID&)__uuidof(IDXGISwapChain), &DXGISwapChain))
	{
		oErrorSetLast(oERROR_NOT_FOUND, "Could not find an IDXGISwapChain in the specified oWindow");
		return;
	}

	HRESULT hr = DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Texture[0]);
	if (FAILED(hr))
	{
		oWinSetLastError(hr);
		return;
	}

	Desc.MRTCount = 1;
	Desc.ArraySize = 1;
	Desc.GenerateMips = false;
	Desc.ClearDesc = CLEAR_DESC(); // still settable by client code
	Desc.DepthStencilFormat = _DSFormat;

	DXGI_SWAP_CHAIN_DESC SCDesc;
	DXGISwapChain->GetDesc(&SCDesc);
	RecreateDepthBuffer(int2(static_cast<int>(SCDesc.BufferDesc.Width), static_cast<int>(SCDesc.BufferDesc.Height)));

	// will be populated on-demand since this render target can resize often due
	// to user interaction.
	Desc.Dimensions = int2(0,0);
	oINIT_ARRAY(Desc.Format, oSURFACE_UNKNOWN);
	*_pSuccess = true;
}

void oD3D11RenderTarget::GetDesc(DESC* _pDesc) const threadsafe
{
	oSharedLock<oSharedMutex> lock(DescMutex);
	oD3D11RenderTarget* pThis = thread_cast<oD3D11RenderTarget*>(this); // safe because of lock above
	*_pDesc = pThis->Desc;

	if (Window)
	{
		oRef<IDXGISwapChain> DXGISwapChain;
		oVERIFY(pThis->Window->QueryInterface((const oGUID&)__uuidof(IDXGISwapChain), &DXGISwapChain));

		DXGI_SWAP_CHAIN_DESC d;
		oV(DXGISwapChain->GetDesc(&d));
		_pDesc->Dimensions = int2(static_cast<int>(d.BufferDesc.Width), static_cast<int>(d.BufferDesc.Height));
		_pDesc->Format[0] = static_cast<oSURFACE_FORMAT>(d.BufferDesc.Format);
		_pDesc->DepthStencilFormat = oSURFACE_UNKNOWN;
	}
}

void oD3D11RenderTarget::SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe
{
	oLockGuard<oSharedMutex> lock(DescMutex);
	thread_cast<DESC&>(Desc).ClearDesc = _ClearDesc; // safe because of lock above
}

void oD3D11RenderTarget::RecreateDepthBuffer(const int2& _Dimensions)
{
	if (Desc.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		oStringL name;
		sprintf_s(name, "%sDS", GetName());
		oD3D11DEVICE();
		Depth = nullptr;
		DSV = nullptr;
		SRVDepth = nullptr;
		oVERIFY(oD3D11CreateRenderTarget(D3DDevice, name, _Dimensions.x, _Dimensions.y, 1, oDXGIFromSurfaceFormat(Desc.DepthStencilFormat), &Depth, &DSV, &SRVDepth));
	}
}

void oD3D11RenderTarget::Resize(const int2& _NewDimensions)
{
	if (Window)
	{
		oASSERT(false, "How should a resize from the RT be handled? Should it be allowed in this mode? Or should we force client code to use oWindow API?");

		//oRef<IDXGISwapChain> DXGISwapChain;
		//oVERIFY(pThis->Window->QueryInterface((const oGUID&)__uuidof(IDXGISwapChain), &DXGISwapChain));

		//DXGI_SWAP_CHAIN_DESC d;
		//DXGISwapChain->GetDesc(&d);
		//if (d.BufferDesc.Width != _NewDimensions.x || d.BufferDesc.Height != _NewDimensions.y)
		//{
		//	oTRACE("%s %s Resizing %ux%u to %dx%d", typeid(*this), GetName(), d.BufferDesc.Width, d.BufferDesc.Height, _NewDimensions.x, _NewDimensions.y);
		//	
		//	// @oooii-tony: Is this allowable without going through oWindow? Should this call oWindow API instead of IDXGISwapChain?
		//	oV(DXGISwapChain->ResizeBuffers(d.BufferCount, _Width, _Height, d.BufferDesc.Format, d.Flags));
		//	
		//	oV(DXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3D10Texture));
		//	
		//	DXGI_SWAP_CHAIN_DESC SCDesc;
		//	DXGISwapChain->GetDesc(&SCDesc);
		//	RecreateDepthBuffer(uint2(SCDesc.BufferDesc.Width, SCDesc.BufferDesc.Height));
		//}
	}

	else
	{
		if (Desc.Dimensions != _NewDimensions)
		{
			oLockGuard<oSharedMutex> lock(DescMutex);

			oTRACE("%s %s Resize %dx%d -> %dx%d", typeid(*this), GetName(), Desc.Dimensions.x, Desc.Dimensions.y, _NewDimensions.x, _NewDimensions.y);

			for (int i = 0; i < oCOUNTOF(Texture); i++)
			{
				Texture[i] = nullptr;
				RTVs[i] = nullptr;
				SRVs[i] = nullptr;
			}

			Depth = nullptr;
			DSV = nullptr;
			SRVDepth = nullptr;
			
			if (Desc.Dimensions.x && Desc.Dimensions.y)
			{
				//oD3D11DEVICE();
				oRef<ID3D11Device> D3DDevice;

				oStringL name;
				for (int i = 0; i < Desc.ArraySize; i++)
				{
					sprintf_s(name, "%s%02d", GetName(), i);
					oVERIFY(oD3D11CreateRenderTarget(D3DDevice, name, Desc.Dimensions.x, Desc.Dimensions.y, Desc.ArraySize, oDXGIFromSurfaceFormat(Desc.Format[i]), &Texture[i], &RTVs[i], &SRVs[i]));
				}

				RecreateDepthBuffer(Desc.Dimensions);
			}

			Desc.Dimensions = _NewDimensions;
		}
	}
}
