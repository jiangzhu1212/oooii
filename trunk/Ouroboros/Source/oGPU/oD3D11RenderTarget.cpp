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
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include "oD3D11Texture.h"
#include <oBasis/oLockThis.h>
#include <oBasis/oSurface.h>
#include <oPlatform/Windows/oDXGI.h>

bool oD3D11CreateRenderTarget(oGPUDevice* _pDevice, const char* _Name, threadsafe oGPUWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, oGPURenderTarget** _ppRenderTarget)
{
	oGPU_CREATE_CHECK_NAME();
	if (!_pWindow)
		return oErrorSetLast(std::errc::invalid_argument, "A window to associate with this new render target must be specified");

	bool success = false;
	oCONSTRUCT(_ppRenderTarget, oD3D11RenderTarget(_pDevice, _pWindow, _DepthStencilFormat, _Name, &success)); \
	return success;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, RenderTarget);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, RenderTarget)
	, Window(nullptr)
	, Desc(_Desc)
{
	*_pSuccess = false;
	
	for (uint i = 0; i < oCOUNTOF(Desc.Format); i++)
	{
		if (oSurfaceFormatIsYUV(Desc.Format[i]))
		{
			oErrorSetLast(std::errc::invalid_argument, "YUV render targets are not supported (format %s specified)", oStd::as_string(Desc.Format[i]));
			return;
		}
	}

	// invalidate width/height to force allocation in this call to resize
	Desc.Dimensions = int3(oInvalid,oInvalid,oInvalid);
	Resize(_Desc.Dimensions);
	*_pSuccess = true;
}

oD3D11RenderTarget::oD3D11RenderTarget(oGPUDevice* _pDevice, threadsafe oGPUWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, const char* _Name, bool* _pSuccess)
	: oGPUDeviceChildMixin(_pDevice, _Name)
	, Window(_pWindow)
{
	Desc.DepthStencilFormat = _DepthStencilFormat;
	ResizeLock();
	*_pSuccess = ResizeUnlock();
}

bool oD3D11RenderTarget::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (MIXINQueryInterface(_InterfaceID, _ppInterface))
		return true;

	else if (_InterfaceID == (const oGUID&)__uuidof(IDXGISwapChain))
	{
		if (Window && Window->QueryInterface((IDXGISwapChain**)_ppInterface))
			return true;
	}

	return !!*_ppInterface;
}

void oD3D11RenderTarget::GetDesc(DESC* _pDesc) const threadsafe
{
	auto pThis = oLockSharedThis(DescMutex);
	*_pDesc = pThis->Desc;

	if (Window)
	{
		oRef<IDXGISwapChain> DXGISwapChain;
		oVERIFY(pThis->Window->QueryInterface((const oGUID&)__uuidof(IDXGISwapChain), &DXGISwapChain));

		DXGI_SWAP_CHAIN_DESC d;
		oV(DXGISwapChain->GetDesc(&d));
		_pDesc->Dimensions = int3(oInt(d.BufferDesc.Width), oInt(d.BufferDesc.Height), 1);
		_pDesc->Format[0] = oDXGIToSurfaceFormat(d.BufferDesc.Format);

		if (pThis->DepthStencilTexture)
		{
			oGPUTexture::DESC d;
			pThis->DepthStencilTexture->GetDesc(&d);
			_pDesc->DepthStencilFormat = d.Format;
		}

		else
			_pDesc->DepthStencilFormat = oSURFACE_UNKNOWN;
	}
}

void oD3D11RenderTarget::SetClearDesc(const oGPU_CLEAR_DESC& _ClearDesc) threadsafe
{
	oLockThis(DescMutex)->Desc.ClearDesc = _ClearDesc;
}

void oD3D11RenderTarget::ResizeLock()
{
	DescMutex.lock();
	RTVs.fill(nullptr);
	Textures.fill(nullptr);
}

bool oD3D11RenderTarget::ResizeUnlock()
{
	oStd::finally OSEUnlock([&] { DescMutex.unlock(); });

	oRef<IDXGISwapChain> SwapChain;
	if (!Window->QueryInterface(&SwapChain))
		return oErrorSetLast(std::errc::function_not_supported, "Could not find an IDXGISwapChain in the specified oGPUWindow");

	oRef<ID3D11Texture2D> SwapChainTexture;
	oV(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SwapChainTexture));
	bool textureSuccess = false;
	Textures[0] = oRef<oGPUTexture>(new oD3D11Texture(Device, oGPUTexture::DESC(), GetName(), &textureSuccess, SwapChainTexture), false);
	if (!textureSuccess)
		return false; // pass through error

	if (!oD3D11CreateRenderTargetView(GetName(), SwapChainTexture, (ID3D11View**)&RTVs[0]))
		return false; // pass through error

	Desc.ArraySize = 1;
	Desc.MRTCount = 1;
	Desc.Type = oGPU_TEXTURE_2D_RENDER_TARGET;
	Desc.ClearDesc = oGPU_CLEAR_DESC(); // still settable by client code
	// Desc.DepthStencilFormat; // set in ctor and recycled through recreates

	DXGI_SWAP_CHAIN_DESC SCDesc;
	SwapChain->GetDesc(&SCDesc);
	int2 Size = int2(oInt(SCDesc.BufferDesc.Width), oInt(SCDesc.BufferDesc.Height));
	RecreateDepthBuffer(Size);

	Desc.Dimensions = int3(Size, 1);
	Desc.Format.fill(oSURFACE_UNKNOWN);
	Desc.Format[0] = oDXGIToSurfaceFormat(SCDesc.BufferDesc.Format);

	return true;
}

void oD3D11RenderTarget::RecreateDepthBuffer(const int2& _Dimensions)
{
	if (Desc.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		oStd::lstring name;
		oPrintf(name, "%s.DS", GetName());
		oD3D11DEVICE();
		DepthStencilTexture = nullptr;
		DSV = nullptr;

		oGPU_TEXTURE_DESC d;
		d.Dimensions = int3(_Dimensions, 1);
		d.ArraySize = 1;
		d.Format = Desc.DepthStencilFormat;
		d.Type = oGPU_TEXTURE_2D_RENDER_TARGET;

		oRef<ID3D11Texture2D> Depth;
		oVERIFY(oD3D11CreateTexture(D3DDevice, name, d, nullptr, &Depth, nullptr, &DSV));
		bool textureSuccess = false;
		DepthStencilTexture = oRef<oGPUTexture>(new oD3D11Texture(Device, oGPUTexture::DESC(), GetName(), &textureSuccess, Depth), false);
		oASSERT(textureSuccess, "Creation of oD3D11Texture failed from ID3D11Texture2D: %s", oErrorGetLastString());
	}
}

void oD3D11RenderTarget::Resize(const int3& _NewDimensions)
{
	if (Window)
	{
		oASSERT(false, "What should happen here? If we use the window to resize this object will be destroyed. What if this is currently fullscreen");
	}

	else
	{
		if (Desc.Dimensions != _NewDimensions)
		{
			oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(DescMutex);

			oTRACE("%s %s Resize %dx%dx%d -> %dx%dx%d", typeid(*this).name(), GetName(), Desc.Dimensions.x, Desc.Dimensions.y, Desc.Dimensions.z, _NewDimensions.x, _NewDimensions.y, _NewDimensions.z);
			
			oFORI(i, Textures)
			{
				Textures[i] = nullptr;
				RTVs[i] = nullptr;
			}

			DepthStencilTexture = nullptr;
			DSV = nullptr;
			
			if (_NewDimensions.x && _NewDimensions.y && _NewDimensions.z)
			{
				oD3D11DEVICE();
				for (int i = 0; i < Desc.MRTCount; i++)
				{
					oStd::lstring name;
					oPrintf(name, "%s%02d", GetName(), i);

					oGPUTexture::DESC d;
					d.Dimensions = _NewDimensions;
					d.Format = Desc.Format[i];
					d.ArraySize = Desc.ArraySize;
					d.Type = oGPUTextureTypeGetRenderTargetType(Desc.Type);

					oVERIFY(Device->CreateTexture(name, d, &Textures[i]));
					oVERIFY(oD3D11CreateRenderTargetView(name, static_cast<oD3D11Texture*>(Textures[i].c_ptr())->Texture, &RTVs[i]));
				}

				RecreateDepthBuffer(_NewDimensions.xy());
			}

			Desc.Dimensions = _NewDimensions;	
		}
	}
}

void oD3D11RenderTarget::GetTexture(int _MRTIndex, oGPUTexture** _ppTexture)
{
	oASSERT(_MRTIndex < Desc.MRTCount, "Invalid MRT index");
	Textures[_MRTIndex]->Reference();
	*_ppTexture = Textures[_MRTIndex];
}

void oD3D11RenderTarget::GetDepthTexture(oGPUTexture** _ppTexture)
{
	DepthStencilTexture->Reference();
	*_ppTexture = DepthStencilTexture;
}
