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
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include "oD3D11Texture.h"
#include <oBasis/oLockThis.h>
#include <oSurface/surface.h>
#include "dxgi_util.h"
#include <oGUI/Windows/oWinWindowing.h>

using namespace ouro;
using namespace ouro::d3d11;

bool oD3D11CreateRenderTarget(oGPUDevice* _pDevice, const char* _Name, IDXGISwapChain* _pSwapChain, ouro::surface::format _DepthStencilFormat, oGPURenderTarget** _ppRenderTarget)
{
	oGPU_CREATE_CHECK_NAME();
	if (!_pSwapChain)
		return oErrorSetLast(std::errc::invalid_argument, "A valid swap chain must be specified");

	bool success = false;
	oCONSTRUCT(_ppRenderTarget, oD3D11RenderTarget(_pDevice, _pSwapChain, _DepthStencilFormat, _Name, &success)); \
	return success;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, RenderTarget);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, RenderTarget)
	, Desc(_Desc)
{
	*_pSuccess = false;
	
	for (uint i = 0; i < oCOUNTOF(Desc.Format); i++)
	{
		if (ouro::surface::is_yuv(Desc.Format[i]))
		{
			oErrorSetLast(std::errc::invalid_argument, "YUV render targets are not supported (format %s specified)", ouro::as_string(Desc.Format[i]));
			return;
		}
	}

	// invalidate width/height to force allocation in this call to resize
	Desc.Dimensions = int3(oInvalid,oInvalid,oInvalid);
	Resize(_Desc.Dimensions);
	*_pSuccess = true;
}

oD3D11RenderTarget::oD3D11RenderTarget(oGPUDevice* _pDevice, IDXGISwapChain* _pSwapChain, ouro::surface::format _DepthStencilFormat, const char* _Name, bool* _pSuccess)
	: oGPUDeviceChildMixin(_pDevice, _Name)
	, SwapChain(_pSwapChain)
{
	*_pSuccess = false;

	DXGI_SWAP_CHAIN_DESC SCD;
	SwapChain->GetDesc(&SCD);

	if (oWinIsRenderTarget(SCD.OutputWindow))
	{
		oErrorSetLast(std::errc::invalid_argument, "The specified window is already associated with a render target and cannot be reassociated.");
		return;
	}
	
	oWinSetIsRenderTarget(SCD.OutputWindow);
	Desc.DepthStencilFormat = _DepthStencilFormat;
	Desc.Format[0] = dxgi::to_surface_format(SCD.BufferDesc.Format);
	Resize(int3(SCD.BufferDesc.Width, SCD.BufferDesc.Height, 1));
	*_pSuccess = true;
}

oD3D11RenderTarget::~oD3D11RenderTarget()
{
	if (SwapChain)
	{
		static_cast<oD3D11Device*>(Device.c_ptr())->RTReleaseSwapChain();
		DXGI_SWAP_CHAIN_DESC SCD;
		SwapChain->GetDesc(&SCD);
		oWinSetIsRenderTarget(SCD.OutputWindow, false);
	}
}

bool oD3D11RenderTarget::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (MIXINQueryInterface(_InterfaceID, _ppInterface))
		return true;

	else if (_InterfaceID == (const oGUID&)__uuidof(IDXGISwapChain) && SwapChain)
	{
		SwapChain->AddRef();
		*_ppInterface = SwapChain;
	}

	return !!*_ppInterface;
}

void oD3D11RenderTarget::GetDesc(DESC* _pDesc) const threadsafe
{
	auto pThis = oLockSharedThis(DescMutex);
	*_pDesc = pThis->Desc;

	if (SwapChain)
	{
		DXGI_SWAP_CHAIN_DESC d;
		oV(pThis->SwapChain->GetDesc(&d));
		_pDesc->Dimensions = int3(oInt(d.BufferDesc.Width), oInt(d.BufferDesc.Height), 1);
		_pDesc->Format[0] = dxgi::to_surface_format(d.BufferDesc.Format);

		if (pThis->DepthStencilTexture)
		{
			oGPUTexture::DESC d;
			pThis->DepthStencilTexture->GetDesc(&d);
			_pDesc->DepthStencilFormat = d.Format;
		}

		else
			_pDesc->DepthStencilFormat = ouro::surface::unknown;
	}
}

void oD3D11RenderTarget::SetClearDesc(const oGPU_CLEAR_DESC& _ClearDesc) threadsafe
{
	oLockThis(DescMutex)->Desc.ClearDesc = _ClearDesc;
}

void oD3D11RenderTarget::ClearResources()
{
	RTVs.fill(nullptr);
	Textures.fill(nullptr);
	DepthStencilTexture = nullptr;
	DSV = nullptr;
}

void oD3D11RenderTarget::RecreateDepthBuffer(const int2& _Dimensions)
{
	if (Desc.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		lstring name;
		snprintf(name, "%s.DS", GetName());
		oD3D11DEVICE();
		DepthStencilTexture = nullptr;
		DSV = nullptr;

		oGPU_TEXTURE_DESC d;
		d.Dimensions = int3(_Dimensions, 1);
		d.ArraySize = 1;
		d.Format = Desc.DepthStencilFormat;
		d.Type = oGPU_TEXTURE_2D_RENDER_TARGET;
		new_texture New = make_texture(D3DDevice, name, d, nullptr);
		intrusive_ptr<ID3D11Texture2D> Depth = New.pTexture2D;
		DSV = New.pDSV;
		bool textureSuccess = false;
		DepthStencilTexture = intrusive_ptr<oGPUTexture>(new oD3D11Texture(Device, oGPUTexture::DESC(), GetName(), &textureSuccess, Depth), false);
		oASSERT(textureSuccess, "Creation of oD3D11Texture failed from ID3D11Texture2D: %s", oErrorGetLastString());
	}
}

void oD3D11RenderTarget::Resize(const int3& _NewDimensions)
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(DescMutex);

	int3 New = _NewDimensions;
	if (SwapChain)
	{
		BOOL IsFullScreen = FALSE;
		intrusive_ptr<IDXGIOutput> Output;
		SwapChain->GetFullscreenState(&IsFullScreen, &Output);
		if (IsFullScreen)
		{
			DXGI_OUTPUT_DESC OD;
			Output->GetDesc(&OD);
			New = int3(OD.DesktopCoordinates.right - OD.DesktopCoordinates.left, OD.DesktopCoordinates.bottom - OD.DesktopCoordinates.top, 1);
		}
	}

	if (any(Desc.Dimensions != New))
	{
		oTRACE("%s %s Resize %dx%dx%d -> %dx%dx%d", type_name(typeid(*this).name()), GetName(), Desc.Dimensions.x, Desc.Dimensions.y, Desc.Dimensions.z, _NewDimensions.x, _NewDimensions.y, _NewDimensions.z);
		ClearResources();

		if (New.x && New.y && New.z)
		{
			if (SwapChain)
			{
				oASSERT(New.z == 1, "New.z must be set to 1 for primary render target");
				dxgi::resize_buffers(SwapChain, New.xy());
				intrusive_ptr<ID3D11Texture2D> SwapChainTexture;
				oV(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SwapChainTexture));
				bool textureSuccess = false;
				Textures[0] = intrusive_ptr<oGPUTexture>(new oD3D11Texture(Device, oGPUTexture::DESC(), GetName(), &textureSuccess, SwapChainTexture), false);
				oVERIFY(textureSuccess);
				make_rtv(GetName(), SwapChainTexture, RTVs[0]);
				Desc.ArraySize = 1;
				Desc.MRTCount = 1;
				Desc.Type = oGPU_TEXTURE_2D_RENDER_TARGET;
			}

			else
			{
				for (int i = 0; i < Desc.MRTCount; i++)
				{
					lstring name;
					snprintf(name, "%s%02d", GetName(), i);
					oGPUTexture::DESC d;
					d.Dimensions = New;
					d.Format = Desc.Format[i];
					d.ArraySize = Desc.ArraySize;
					d.Type = oGPUTextureTypeGetRenderTargetType(Desc.Type);
					oVERIFY(Device->CreateTexture(name, d, &Textures[i]));
					make_rtv(GetName(), static_cast<oD3D11Texture*>(Textures[i].c_ptr())->Texture, RTVs[0]);
				}
			}

			RecreateDepthBuffer(New.xy());
		}
		
		Desc.Dimensions = New;
	}
}

void oD3D11RenderTarget::GetTexture(int _MRTIndex, oGPUTexture** _ppTexture)
{
	oASSERT(_MRTIndex < Desc.MRTCount, "Invalid MRT index");
	if (Textures[_MRTIndex])
		Textures[_MRTIndex]->Reference();
	*_ppTexture = Textures[_MRTIndex];
}

void oD3D11RenderTarget::GetDepthTexture(oGPUTexture** _ppTexture)
{
	if (DepthStencilTexture)
		DepthStencilTexture->Reference();
	*_ppTexture = DepthStencilTexture;
}

std::shared_ptr<ouro::surface::buffer> oD3D11RenderTarget::CreateSnapshot(int _MRTIndex)
{
	if (!Textures[_MRTIndex])		
		oTHROW(resource_unavailable_try_again, "The render target is minimized or not available for snapshot.");
	return make_snapshot(static_cast<oD3D11Texture*>(Textures[_MRTIndex].c_ptr())->Texture);
}
