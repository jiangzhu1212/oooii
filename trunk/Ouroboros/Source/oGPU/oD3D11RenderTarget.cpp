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
#include <oSurface/surface.h>
#include "dxgi_util.h"
#include <oGUI/Windows/oWinWindowing.h>

using namespace ouro;
using namespace d3d11;

bool oD3D11CreateRenderTarget(oGPUDevice* _pDevice, const char* _Name, IDXGISwapChain* _pSwapChain, surface::format _DepthStencilFormat, oGPURenderTarget** _ppRenderTarget)
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
	
	for (uint i = 0; i < oCOUNTOF(Desc.format); i++)
	{
		if (surface::is_yuv(Desc.format[i]))
		{
			oErrorSetLast(std::errc::invalid_argument, "YUV render targets are not supported (format %s specified)", as_string(Desc.format[i]));
			return;
		}
	}

	// invalidate width/height to force allocation in this call to resize
	Desc.dimensions = ushort3(0,0,0);
	Resize(_Desc.dimensions);
	*_pSuccess = true;
}

oD3D11RenderTarget::oD3D11RenderTarget(oGPUDevice* _pDevice, IDXGISwapChain* _pSwapChain, surface::format _DepthStencilFormat, const char* _Name, bool* _pSuccess)
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
	Desc.depth_stencil_format = _DepthStencilFormat;
	Desc.format[0] = dxgi::to_surface_format(SCD.BufferDesc.Format);
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
	shared_lock<shared_mutex> lock(thread_cast<shared_mutex&>(DescMutex));
	oD3D11RenderTarget* pThis = thread_cast<oD3D11RenderTarget*>(this);

	*_pDesc = pThis->Desc;

	if (SwapChain)
	{
		DXGI_SWAP_CHAIN_DESC d;
		oV(pThis->SwapChain->GetDesc(&d));
		_pDesc->dimensions = int3(as_int(d.BufferDesc.Width), as_int(d.BufferDesc.Height), 1);
		_pDesc->format[0] = dxgi::to_surface_format(d.BufferDesc.Format);

		if (pThis->DepthStencilTexture)
		{
			oGPUTexture::DESC d;
			pThis->DepthStencilTexture->GetDesc(&d);
			_pDesc->depth_stencil_format = d.format;
		}

		else
			_pDesc->depth_stencil_format = surface::unknown;
	}
}

void oD3D11RenderTarget::SetClearDesc(const gpu::clear_info& _ClearInfo) threadsafe
{
	ouro::lock_guard<shared_mutex> lock(thread_cast<shared_mutex&>(DescMutex));
	oD3D11RenderTarget* pThis = thread_cast<oD3D11RenderTarget*>(this);

	pThis->Desc.clear = _ClearInfo;
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
	if (Desc.depth_stencil_format != surface::unknown)
	{
		lstring name;
		snprintf(name, "%s.DS", GetName());
		oD3D11DEVICE();
		DepthStencilTexture = nullptr;
		DSV = nullptr;

		gpu::texture_info d;
		d.dimensions = ushort3(_Dimensions, 1);
		d.array_size = 0;
		d.format = Desc.depth_stencil_format;
		d.type = gpu::texture_type::render_target_2d;
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
	ouro::lock_guard<shared_mutex> lock(DescMutex);

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

	if (any(Desc.dimensions != New))
	{
		oTRACE("%s %s Resize %dx%dx%d -> %dx%dx%d", type_name(typeid(*this).name()), GetName(), Desc.dimensions.x, Desc.dimensions.y, Desc.dimensions.z, _NewDimensions.x, _NewDimensions.y, _NewDimensions.z);
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
				Desc.array_size = 0;
				Desc.mrt_count = 1;
				Desc.type = gpu::texture_type::render_target_2d;
			}

			else
			{
				for (int i = 0; i < Desc.mrt_count; i++)
				{
					lstring name;
					snprintf(name, "%s%02d", GetName(), i);
					oGPUTexture::DESC d;
					d.dimensions = New;
					d.format = Desc.format[i];
					d.array_size = Desc.array_size;
					d.type = gpu::make_render_target(Desc.type);
					oVERIFY(Device->CreateTexture(name, d, &Textures[i]));
					make_rtv(GetName(), static_cast<oD3D11Texture*>(Textures[i].c_ptr())->Texture, RTVs[0]);
				}
			}

			RecreateDepthBuffer(New.xy());
		}
		
		Desc.dimensions = New;
	}
}

void oD3D11RenderTarget::GetTexture(int _MRTIndex, oGPUTexture** _ppTexture)
{
	oASSERT(_MRTIndex < Desc.mrt_count, "Invalid MRT index");
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

std::shared_ptr<surface::buffer> oD3D11RenderTarget::CreateSnapshot(int _MRTIndex)
{
	if (!Textures[_MRTIndex])		
		oTHROW(resource_unavailable_try_again, "The render target is minimized or not available for snapshot.");
	return make_snapshot(static_cast<oD3D11Texture*>(Textures[_MRTIndex].c_ptr())->Texture);
}
