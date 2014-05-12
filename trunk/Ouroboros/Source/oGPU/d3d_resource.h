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
#pragma once
#ifndef oGPU_d3d_resource_h
#define oGPU_d3d_resource_h

#include "d3d_types.h"
#include <d3d11.h>

#include <oGPU/oGPU.h> // texture_type, buffer_info, texture_info

namespace ouro {
	namespace gpu {
		namespace d3d {

// Allow Buffers to be a bit more self-describing - mainly for index buffers.
void set_info(Resource* _pBuffer, const buffer_info& _Desc);
buffer_info get_info(const Resource* _pBuffer);

// Create common interfaces. NOTE: It is often benchmarked as faster due to 
// driver PCIE usage to use D3D11_USAGE_DEFAULT and UpdateSubresource rather 
// than D3D11_USAGE_DYNAMIC and Map/Unmap. Benchmark to be sure, but it is 
// generally a cleaner design and easier to code if all resources are DEFAULT. 
// Pointers to initial data can be null, but number/size values are used to 
// allocate the buffer and thus must always be specified.

intrusive_ptr<ID3D11Buffer> make_buffer(ID3D11Device* _pDevice
	, const char* _DebugName
	, const gpu::buffer_info& _Info
	, const void* _pInitBuffer
	, ID3D11UnorderedAccessView** _ppUAV = nullptr
	, ID3D11ShaderResourceView** _ppSRV = nullptr);

// Copies the contents of the specified texture to _pBuffer, which is assumed to
// be properly allocated to receive the contents. If _FlipVertical is true, then
// the bitmap data will be copied such that the destination will be upside-down 
// compared to the source.
void copy(ID3D11Resource* _pTexture
	, unsigned int _Subresource
	, surface::mapped_subresource* _pDstSubresource
	, bool _FlipVertically = false);

// returns a unified info for any type of resource
texture_info get_texture_info(Resource* _pResource, bool _AsArray = false, D3D11_USAGE* _pUsage = nullptr);
inline texture_info get_texture_info(View* _pView, bool _AsArray = false, D3D11_USAGE* _pUsage = nullptr) { intrusive_ptr<Resource> r; _pView->GetResource(&r); return get_texture_info(r, _AsArray, _pUsage); }

// From the specified texture, create the correct shader resource view
intrusive_ptr<ShaderResourceView> make_srv(const char* _DebugName, Resource* _pTexture, bool _AsArray = false);

intrusive_ptr<View> make_rtv(const char* _DebugName, Resource* _pTexture);
template<typename ViewT> void make_rtv(const char* _DebugName, Resource* _pTexture, intrusive_ptr<ViewT>& _View) 
	{ _View = static_cast<ViewT*>(make_rtv(_DebugName, _pTexture).c_ptr()); }

// Creates a UAV that matches the meta-data of the specified texture along with
// the specified mip and array topology for the specified texture.
intrusive_ptr<UnorderedAccessView> make_uav(const char* _DebugName
	, Resource* _pTexture, unsigned int _MipSlice, unsigned int _ArraySlice);

// Creates a copy of the specified UAV that clears any raw/append/counter flags
intrusive_ptr<UnorderedAccessView> make_unflagged_copy(UnorderedAccessView* _pSourceUAV);

struct new_texture
{
	new_texture()
		: pResource(nullptr)
		, pSRV(nullptr)
		, pView(nullptr)
	{}

	new_texture(const new_texture& _That) { operator=(_That); }
	const new_texture& operator=(const new_texture& _That)
	{
		clear();
		pResource = _That.pResource; if (pResource) pResource->AddRef();
		pSRV = _That.pSRV; if (pSRV) pSRV->AddRef();
		pView = _That.pView; if (pView) pView->AddRef();
		return *this;
	}

	new_texture(new_texture&& _That) { eviscerate(std::move(_That)); }
	new_texture& operator=(new_texture&& _That)
	{
		if (this != &_That)
		{
			clear();
			eviscerate(std::move(_That));
		}
		return *this;
	}

	~new_texture() { clear(); }

	void eviscerate(new_texture&& _That)
	{
		pResource = _That.pResource; _That.pResource = nullptr;
		pSRV = _That.pSRV; _That.pSRV = nullptr;
		pView = _That.pView; _That.pView = nullptr;
	}

	void clear()
	{
		if (pResource) pResource->Release();
		if (pSRV) pSRV->Release();
		if (pView) pView->Release();
	}

	union
	{
		Resource* pResource;
		Texture1D* pTexture1D;
		Texture2D* pTexture2D;
		Texture3D* pTexture3D;
	};
	ShaderResourceView* pSRV;
	union
	{
		View* pView;
		RenderTargetView* pRTV;
		DepthStencilView* pDSV;
	};
};

// Creates a texture according to the specified desc. If the DESC describes the 
// texture as a render target, it will be created properly and the view will be 
// filled in. If a depth format is specified then the view will be an 
// DepthStencilView instead of an RenderTargetView.
new_texture make_texture(Device* _pDevice
	, const char* _DebugName
	, const gpu::texture_info& _Info
	, surface::const_mapped_subresource* _pInitData = nullptr);

// Creates a CPU-readable copy of the specified texture/render target. Only 
// textures are currently supported.
intrusive_ptr<Resource> make_cpu_copy(Resource* _pResource);

// Copies the specified render target to the specified image/path
std::shared_ptr<surface::buffer> make_snapshot(Texture2D* _pRenderTarget);

// Uses oTRACE to display the fields of the specified desc.
void trace_texture2d_desc(const D3D11_TEXTURE2D_DESC& _Desc, const char* _Prefix = "\t");

// This converts back from a texture_info to typical fields in texture-related
// structs (including D3DX11_IMAGE_LOAD_INFO, specify the info's format as 
// unknown to use DXGI_FORMAT_FROM_FILE)
void init_values(const gpu::texture_info& _Info
	, DXGI_FORMAT* _pFormat
	, D3D11_USAGE* _pUsage
	, unsigned int* _pCPUAccessFlags
	, unsigned int* _pBindFlags
	, unsigned int* _pMipLevels
	, unsigned int* _pMiscFlags = nullptr);

		} // namespace d3d
	} // namespace gpu
} // namespace ouro

#endif
