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
#include "pch.h"
#include <oooii/oD3D11.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oString.h>
#include <oooii/oSTL.h>

static const char* d3d11_dll_functions[] = 
{
	"D3D11CreateDevice",
};

oD3D11::oD3D11()
{
	hD3D11 = oLinkDLL("d3d11.dll", d3d11_dll_functions, (void**)&D3D11CreateDevice, oCOUNTOF(d3d11_dll_functions));
	oASSERT(hD3D11, "");
}

oD3D11::~oD3D11()
{
	oUnlinkDLL(hD3D11);
}

void oD3D11InitBufferDesc(D3D11_BUFFER_DESC* _pDesc, D3D11_BIND_FLAG _BindFlag, bool _CPUWritable, size_t _Size, size_t _Count)
{
	_pDesc->ByteWidth = static_cast<UINT>(_Size * _Count);
	_pDesc->Usage = _CPUWritable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	_pDesc->BindFlags = _BindFlag;
	_pDesc->CPUAccessFlags = _CPUWritable ? D3D11_CPU_ACCESS_WRITE : 0;
	_pDesc->MiscFlags = 0;
	_pDesc->StructureByteStride = static_cast<UINT>(_Size);
}

#define oCREATE_HANDLE_ERR(fnName) \
	if (FAILED(hr)) \
	{	char err[2 * 1024]; \
		oGetNativeErrorDesc(err, hr); \
		oSetLastError(EINVAL, #fnName " failed: %s", err); \
		return false; \
	}

bool oD3D11CreateConstantBuffer(ID3D11Device* _pDevice, bool _CPUWrite, const void* _pBufferStruct, size_t _SizeofBufferStruct, size_t _StructCount, ID3D11Buffer** _ppConstantBuffer)
{
	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(&desc, D3D11_BIND_CONSTANT_BUFFER, _CPUWrite, oByteAlign(_SizeofBufferStruct, 16), _StructCount);
	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pBufferStruct;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pBufferStruct ? &SRD : 0, _ppConstantBuffer);
	oCREATE_HANDLE_ERR(oD3D11CreateConstantBuffer);
	return true;
}

bool oD3D11CreateIndexBuffer(ID3D11Device* _pDevice, bool _CPUWrite, const void* _pIndices, size_t _NumIndices, bool _Use16BitIndices, ID3D11Buffer** _ppIndexBuffer)
{
	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(&desc, D3D11_BIND_INDEX_BUFFER, _CPUWrite, (_Use16BitIndices ? sizeof(unsigned short) : sizeof(unsigned int)) * _NumIndices, 1);
	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pIndices;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pIndices ? &SRD : 0, _ppIndexBuffer);
	oCREATE_HANDLE_ERR(DX11CreateIndexBuffer);
	return true;
}

bool oD3D11CreateVertexBuffer(ID3D11Device* _pDevice, bool _CPUWrite, const void* _pVertices, size_t _NumVertices, size_t _VertexStride, ID3D11Buffer** _ppVertexBuffer)
{
	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(&desc, D3D11_BIND_VERTEX_BUFFER, _CPUWrite, _VertexStride * _NumVertices, 1);
	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pVertices;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pVertices ? &SRD : 0, _ppVertexBuffer);
	oCREATE_HANDLE_ERR(DX11CreateVertexBuffer);
	return true;
}

#undef oCREATE_HANDLE_ERR

bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, unsigned int _Width, unsigned int _Height, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11View** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView)
{
	*_ppRenderTarget = 0;

	if (_ppRenderTargetView)
		*_ppRenderTargetView = 0;

	if (_ppShaderResourceView)
		*_ppShaderResourceView = 0;

	D3D11_TEXTURE2D_DESC d;
	d.Width = _Width;
	d.Height = _Height;
	d.MipLevels = 1;
	d.ArraySize = 1;
	d.Format = _Format;
	d.SampleDesc.Count = 1;
	d.SampleDesc.Quality = 0;
	d.Usage = D3D11_USAGE_DEFAULT;
	d.BindFlags = D3D11_BIND_SHADER_RESOURCE | (oDXGIIsDepthFormat(_Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET);
	d.CPUAccessFlags = 0;
	d.MiscFlags = 0;

	if (FAILED(_pDevice->CreateTexture2D(&d, 0, _ppRenderTarget)))
	{
		oSetLastError(EINVAL, "CreateTexture2D failed, check DX debug output");
		return false;
	}

	if (_ppRenderTargetView)
	{
		HRESULT hr = S_OK;
		if (d.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
			dsv.Format = oDXGIGetDepthCompatibleFormat(_Format);
			dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsv.Texture2D.MipSlice = 0;
			dsv.Flags = 0;
			hr = _pDevice->CreateDepthStencilView(*_ppRenderTarget, &dsv, (ID3D11DepthStencilView**)_ppRenderTargetView);
		}
		else
			hr = _pDevice->CreateRenderTargetView(*_ppRenderTarget, 0, (ID3D11RenderTargetView**)_ppRenderTargetView);

		if (FAILED(hr))
		{
			(*_ppRenderTarget)->Release();
			*_ppRenderTarget = 0;
			char err[1024];
			oGetNativeErrorDesc(err, hr);
			oSetLastError(EINVAL, "Creating view failed for %s: %s", (d.BindFlags & D3D11_BIND_DEPTH_STENCIL) ? "DepthStencil" : "RenderTarget", err);
			return false;
		}
	}

	if (_ppShaderResourceView)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dssrv;
		dssrv.Format = oDXGIGetColorCompatibleFormat(_Format);
		dssrv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dssrv.Texture2D.MostDetailedMip = 0;
		dssrv.Texture2D.MipLevels = 1;
		D3D11_SHADER_RESOURCE_VIEW_DESC* pSRV = (d.BindFlags & D3D11_BIND_DEPTH_STENCIL) ? &dssrv : 0;

		HRESULT hr = _pDevice->CreateShaderResourceView(*_ppRenderTarget, pSRV, _ppShaderResourceView);
		if (FAILED(hr))
		{
			if (_ppRenderTargetView)
			{
				(*_ppRenderTargetView)->Release();
				*_ppRenderTargetView = 0;
			}

			(*_ppRenderTarget)->Release();
			*_ppRenderTarget = 0;

			char err[1024];
			oGetNativeErrorDesc(err, hr);
			oSetLastError(EINVAL, "Creating shader resource view failed for %s: %s", (d.BindFlags & D3D11_BIND_DEPTH_STENCIL) ? "DepthStencil" : "RenderTarget", err);
			return false;
		}
	}

	return true;
}

template<> const char* oAsString(const oD3D11_PIPELINE_STAGE& _Stage)
{
	switch (_Stage)
	{
	case oD3D11_PIPELINE_STAGE_VERTEX: return "oD3D11_PIPELINE_STAGE_VERTEX";
	case oD3D11_PIPELINE_STAGE_HULL: return "oD3D11_PIPELINE_STAGE_HULL";
	case oD3D11_PIPELINE_STAGE_DOMAIN: return "oD3D11_PIPELINE_STAGE_DOMAIN";
	case oD3D11_PIPELINE_STAGE_GEOMETRY: return "oD3D11_PIPELINE_STAGE_GEOMETRY";
	case oD3D11_PIPELINE_STAGE_PIXEL: return "oD3D11_PIPELINE_STAGE_PIXEL";
	default: oASSUME(0);
	}
}

const char* oD3D11GetShaderProfile(ID3D11Device* _pDevice, oD3D11_PIPELINE_STAGE _Stage)
{
	static const char* sDX9Profiles[] = 
	{
		"vs_3_0",
		0,
		0,
		0,
		"ps_3_0",
		0,
	};

	static const char* sDX10Profiles[] = 
	{
		"vs_4_0",
		0,
		0,
		"gs_4_0",
		"ps_4_0",
		0,
	};

	static const char* sDX10_1Profiles[] = 
	{
		"vs_4_1",
		0,
		0,
		"gs_4_1",
		"ps_4_1",
		0,
	};

	static const char* sDX11Profiles[] = 
	{
		"vs_5_0",
		"hs_5_0",
		"ds_5_0",
		"gs_5_0",
		"ps_5_0",
		"cs_5_0",
	};

	const char** profiles = 0;
	switch (_pDevice->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_9_1:
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_3:
		profiles = sDX9Profiles;
		break;
	case D3D_FEATURE_LEVEL_10_0:
		profiles = sDX10Profiles;
		break;
	case D3D_FEATURE_LEVEL_10_1:
		profiles = sDX10_1Profiles;
		break;
	case D3D_FEATURE_LEVEL_11_0:
		profiles = sDX11Profiles;
		break;
	default:
		oASSUME(0);
	}

	const char* profile = profiles[_Stage];
	if (!profile)
		oSetLastError(ENOENT, "Shader profile does not exist for D3D%.2f's stage %s", oGetD3DVersion(_pDevice->GetFeatureLevel()), oAsString(_Stage));

	return profile;
}

bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages)
{
	if (!_OutErrorMessageString)
	{
		oSetLastError(EINVAL);
		return false;
	}

	if (_pErrorMessages)
	{
		errno_t err = oReplace(_OutErrorMessageString, _SizeofOutErrorMessageString, (const char*)_pErrorMessages->GetBufferPointer(), "%", "%%");
		if (err)
		{
			oSetLastError(err);
			return false;
		}
	}

	else
		*_OutErrorMessageString = 0;

	return true;
}

size_t oD3D11GetEncodedByteCodeSize(const BYTE* _pByteCode)
{
	// Discovered empirically
	return _pByteCode ? ((const unsigned int*)_pByteCode)[6] : 0;
}

void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, UINT _StartSlot, UINT _NumBuffers, ID3D11Buffer* const* _ppConstantBuffers)
{
	_pDeviceContext->VSSetConstantBuffers(_StartSlot, _NumBuffers, _ppConstantBuffers);
	_pDeviceContext->HSSetConstantBuffers(_StartSlot, _NumBuffers, _ppConstantBuffers);
	_pDeviceContext->DSSetConstantBuffers(_StartSlot, _NumBuffers, _ppConstantBuffers);
	_pDeviceContext->GSSetConstantBuffers(_StartSlot, _NumBuffers, _ppConstantBuffers);
	_pDeviceContext->PSSetConstantBuffers(_StartSlot, _NumBuffers, _ppConstantBuffers);
}

void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, UINT _StartSlot, UINT _NumSamplers, ID3D11SamplerState* const* _ppSamplers)
{
	_pDeviceContext->VSSetSamplers(_StartSlot, _NumSamplers, _ppSamplers);
	_pDeviceContext->HSSetSamplers(_StartSlot, _NumSamplers, _ppSamplers);
	_pDeviceContext->DSSetSamplers(_StartSlot, _NumSamplers, _ppSamplers);
	_pDeviceContext->GSSetSamplers(_StartSlot, _NumSamplers, _ppSamplers);
	_pDeviceContext->PSSetSamplers(_StartSlot, _NumSamplers, _ppSamplers);
}

void oD3D11GetTextureDesc(ID3D11Resource* _pResource, oD3D11_TEXTURE_DESC* _pDesc)
{
	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<ID3D11Texture1D*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.Width;
			_pDesc->Height = 1;
			_pDesc->ArraySize = desc.ArraySize;
			_pDesc->MipLevels = desc.MipLevels;
			_pDesc->Format = desc.Format;
			_pDesc->SampleDesc.Count = 0;
			_pDesc->SampleDesc.Quality = 0;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<ID3D11Texture2D*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.Width;
			_pDesc->Height = desc.Height;
			_pDesc->ArraySize = desc.ArraySize;
			_pDesc->MipLevels = desc.MipLevels;
			_pDesc->Format = desc.Format;
			_pDesc->SampleDesc = desc.SampleDesc;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<ID3D11Texture3D*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.Width;
			_pDesc->Height = desc.Height;
			_pDesc->Depth = desc.Depth;
			_pDesc->MipLevels = desc.MipLevels;
			_pDesc->Format = desc.Format;
			_pDesc->SampleDesc.Count = 0;
			_pDesc->SampleDesc.Quality = 0;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			D3D11_BUFFER_DESC desc;
			static_cast<ID3D11Buffer*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.ByteWidth;
			_pDesc->Height = 1;
			_pDesc->ArraySize = desc.ByteWidth / desc.StructureByteStride;
			_pDesc->Format = DXGI_FORMAT_UNKNOWN;
			_pDesc->MipLevels = 1;
			_pDesc->SampleDesc.Count = 0;
			_pDesc->SampleDesc.Quality = 0;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		};

		default: oASSUME(0);
	}
}

UINT oD3D11GetNumElements(D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology, UINT _NumPrimitives)
{
	switch (_PrimitiveTopology)
	{
	case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST: return _NumPrimitives;
	case D3D11_PRIMITIVE_TOPOLOGY_LINELIST: return _NumPrimitives * 2;
	case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP: return _NumPrimitives + 1;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return _NumPrimitives * 3;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return _NumPrimitives + 2;
	default: oASSUME(0);
	}
}

UINT oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
							 , D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology
							 , UINT _NumPrimitives
							 , UINT _NumVertexBuffers
							 , const ID3D11Buffer* const* _ppVertexBuffers
							 , const UINT* _VertexStrides
							 , UINT _IndexOfFirstVertexToDraw
							 , UINT _OffsetToAddToEachVertexIndex
							 , const ID3D11Buffer* _IndexBuffer
							 , bool _32BitIndexBuffer
							 , UINT _IndexOfFirstIndexToDraw
							 , UINT _NumInstances
							 , UINT _IndexOfFirstInstanceIndexToDraw)
{
	const UINT nElements = oD3D11GetNumElements(_PrimitiveTopology, _NumPrimitives);
	static UINT sOffsets[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// DirectX as an API has a funny definition for const, probably because of 
	// ref-counting, so consider it a platform-quirk and keep const correctness
	// above this API, but cast it away as DirectX requires here...

	_pDeviceContext->IASetVertexBuffers(0, _NumVertexBuffers, const_cast<ID3D11Buffer* const*>(_ppVertexBuffers), _VertexStrides, sOffsets);
	_pDeviceContext->IASetPrimitiveTopology(_PrimitiveTopology);

	if (_IndexBuffer)
	{
		_pDeviceContext->IASetIndexBuffer(const_cast<ID3D11Buffer*>(_IndexBuffer), _32BitIndexBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, _IndexOfFirstIndexToDraw);

		if (_NumInstances)
			_pDeviceContext->DrawIndexedInstanced(nElements, _NumInstances, _IndexOfFirstIndexToDraw, _OffsetToAddToEachVertexIndex, _IndexOfFirstInstanceIndexToDraw);
		else
			_pDeviceContext->DrawIndexed(nElements, _IndexOfFirstIndexToDraw, _OffsetToAddToEachVertexIndex);
	}

	else
	{
		if (_NumInstances)
			_pDeviceContext->DrawInstanced(nElements, _NumInstances, _IndexOfFirstVertexToDraw, _IndexOfFirstInstanceIndexToDraw);
		else
			_pDeviceContext->Draw(nElements, _IndexOfFirstVertexToDraw);
	}

	return _NumPrimitives * __min(1, _NumInstances);
}

void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11Texture2D* _pRenderTargetResource, float _MinDepth, float _MaxDepth)
{
	D3D11_TEXTURE2D_DESC desc;
	_pRenderTargetResource->GetDesc(&desc);

	D3D11_VIEWPORT v;
	v.TopLeftX = 0.0f;
	v.TopLeftY = 0.0f;
	v.Width = static_cast<float>(desc.Width);
	v.Height = static_cast<float>(desc.Height);
	v.MinDepth = 0.0f;
	v.MaxDepth = 1.0f;
	_pDeviceContext->RSSetViewports(1, &v);
}

void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11RenderTargetView* _pRenderTargetView, float _MinDepth, float _MaxDepth)
{
	oRef<ID3D11Texture2D> texture;
	_pRenderTargetView->GetResource((ID3D11Resource**)&texture);
#ifdef _DEBUG
	D3D11_RESOURCE_DIMENSION dimension;
	texture->GetType(&dimension);
	oASSERT(dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D, "Unexpected resource type");
#endif
	oD3D11SetFullTargetViewport(_pDeviceContext, texture, _MinDepth, _MaxDepth);
}

void oD3D11CopyStructs(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pBuffer, const void* _pStructs)
{
	D3D11_BUFFER_DESC desc;
	_pBuffer->GetDesc(&desc);

	switch (desc.Usage)
	{
		case D3D11_USAGE_DYNAMIC:
		{
			D3D11_MAPPED_SUBRESOURCE MSR;
			_pDeviceContext->Map(_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MSR);
			memcpy(MSR.pData, _pStructs, desc.ByteWidth);
			_pDeviceContext->Unmap(_pBuffer, 0);
			break;
		}

		case D3D11_USAGE_DEFAULT:
			_pDeviceContext->UpdateSubresource(_pBuffer, 0, 0, _pStructs, desc.ByteWidth, 0);
			break;

		default: oASSUME(0);
	}
}

oD3D11RasterizerState::oD3D11RasterizerState(ID3D11Device* _pDevice)
{
	static const D3D11_FILL_MODE sFills[] = 
	{
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_WIREFRAME,
	};
	oSTATICASSERT(oCOUNTOF(sFills) == NUM_STATES);

	static const D3D11_CULL_MODE sCulls[] = 
	{
		D3D11_CULL_BACK,
		D3D11_CULL_FRONT,
		D3D11_CULL_NONE,
		D3D11_CULL_NONE,
	};
	oSTATICASSERT(oCOUNTOF(sCulls) == NUM_STATES);

	D3D11_RASTERIZER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.DepthClipEnable = TRUE;

	for (size_t i = 0; i < oCOUNTOF(States); i++)
	{
		desc.FillMode = sFills[i];
		desc.CullMode = sCulls[i];
		oV(_pDevice->CreateRasterizerState(&desc, &States[i]));
	}
}

oD3D11RasterizerState::~oD3D11RasterizerState()
{
	for (size_t i = 0; i < oCOUNTOF(States); i++)
		States[i]->Release();
}

oD3D11BlendState::oD3D11BlendState(ID3D11Device* _pDevice)
{
	static const D3D11_RENDER_TARGET_BLEND_DESC sBlends[] =
	{
		{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
	};
	oSTATICASSERT(oCOUNTOF(sBlends) == NUM_STATES);

	D3D11_BLEND_DESC desc = {0};
	for (size_t i = 0; i < oCOUNTOF(States); i++)
	{
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0] = sBlends[i];
		oV(_pDevice->CreateBlendState(&desc, &States[i]));
	}
}

oD3D11BlendState::~oD3D11BlendState()
{
	for (size_t i = 0; i < oCOUNTOF(States); i++)
		States[i]->Release();
}

void oD3D11BlendState::SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_pDeviceContext->OMSetBlendState(States[_State], sBlendFactor, 0xffffffff);
}

void oD3D11BlendState::SetDefaultState(ID3D11DeviceContext* _pDeviceContext)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_pDeviceContext->OMSetBlendState(0, sBlendFactor, 0xffffffff);
}

oD3D11DepthStencilState::oD3D11DepthStencilState(ID3D11Device* _pDevice)
{
	D3D11_DEPTH_STENCIL_DESC desc = {0};

	oV(_pDevice->CreateDepthStencilState(&desc, &States[DEPTH_STENCIL_OFF]));

	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	oV(_pDevice->CreateDepthStencilState(&desc, &States[DEPTH_TEST_AND_WRITE]));

	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	oV(_pDevice->CreateDepthStencilState(&desc, &States[DEPTH_TEST]));
}

oD3D11DepthStencilState::~oD3D11DepthStencilState()
{
	for (size_t i = 0; i < oCOUNTOF(States); i++)
		States[i]->Release();
}

oD3D11SamplerState::oD3D11SamplerState(ID3D11Device* _pDevice)
{
	static const D3D11_FILTER sFilters[] = 
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_ANISOTROPIC,
		D3D11_FILTER_COMPARISON_ANISOTROPIC,
	};
	oSTATICASSERT(oCOUNTOF(sFilters) == NUM_SAMPLER_STATES);

	static const D3D11_TEXTURE_ADDRESS_MODE sAddresses[] = 
	{
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
	};
	oSTATICASSERT(oCOUNTOF(sAddresses) == NUM_SAMPLER_STATES);

	static const FLOAT sBiases[] =
	{
		0.0f,
		-1.0f,
		-2.0f,
		1.0f,
		2.0f,
	};
	oSTATICASSERT(oCOUNTOF(sBiases) == NUM_MIP_BIAS_LEVELS);

	D3D11_SAMPLER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	for (size_t state = 0; state < NUM_SAMPLER_STATES; state++)
	{
		desc.Filter = sFilters[state];
		desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[state];
		desc.MaxLOD = FLT_MAX; // documented default
		desc.MaxAnisotropy = 16; // documented default
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // documented default

		for (size_t bias = 0; bias < NUM_MIP_BIAS_LEVELS; bias++)
		{
			desc.MipLODBias = sBiases[bias];
			_pDevice->CreateSamplerState(&desc, &States[state][bias]);
		}
	}
}

oD3D11SamplerState::~oD3D11SamplerState()
{
	for (size_t state = 0; state < NUM_SAMPLER_STATES; state++)
		for (size_t bias = 0; bias < NUM_MIP_BIAS_LEVELS; bias++)
			States[state][bias]->Release();
}

void oD3D11SamplerState::SetState(ID3D11DeviceContext* _pDeviceContext, PIPELINE_STAGE _Stage, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oASSERT(oCOUNTOF(Samplers) > _NumSamplers, "Too many samplers specified");

	for (size_t i = 0; i < _NumSamplers; i++)
		Samplers[i] = States[_States[i]][_Levels[i]];

	switch (_Stage)
	{
		case VERTEX_SHADER: _pDeviceContext->VSSetSamplers(0, (UINT)_NumSamplers, Samplers); break;
		case HULL_SHADER: _pDeviceContext->HSSetSamplers(0, (UINT)_NumSamplers, Samplers); break;
		case DOMAIN_SHADER: _pDeviceContext->DSSetSamplers(0, (UINT)_NumSamplers, Samplers); break;
		case GEOMETRY_SHADER: _pDeviceContext->GSSetSamplers(0, (UINT)_NumSamplers, Samplers); break;
		case PIXEL_SHADER: _pDeviceContext->PSSetSamplers(0, (UINT)_NumSamplers, Samplers); break;
		default: oASSUME(0);
	}
}

void oD3D11SamplerState::SetState(ID3D11DeviceContext* _pDeviceContext, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oASSERT(oCOUNTOF(Samplers) > _NumSamplers, "Too many samplers specified");

	for (size_t i = 0; i < _NumSamplers; i++)
		Samplers[i] = States[_States[i]][_Levels[i]];

	oD3D11SetSamplers(_pDeviceContext, 0, static_cast<UINT>(_NumSamplers), Samplers);
}

void Release(oD3D11ShaderState::STATE& _State)
{
	IUnknown** ppUnknown = (IUnknown**)&_State.pInputLayout;
	const size_t nPointers = sizeof(_State) / sizeof(IUnknown*);
	for (size_t i = 0; i < nPointers; i++, ppUnknown++)
		if (*ppUnknown)
			(*ppUnknown)->Release();
}

void AddRef(oD3D11ShaderState::STATE& _State)
{
	IUnknown** ppUnknown = (IUnknown**)&_State.pInputLayout;
	const size_t nPointers = sizeof(_State) / sizeof(IUnknown*);
	for (size_t i = 0; i < nPointers; i++, ppUnknown++)
		if (*ppUnknown)
			(*ppUnknown)->AddRef();
}

oD3D11ShaderState::oD3D11ShaderState()
{
}

oD3D11ShaderState::~oD3D11ShaderState()
{
	for (std::vector<STATE>::iterator it = States.begin(); it != States.end(); ++it)
		Release(*it);
}

void oD3D11ShaderState::SetState(ID3D11DeviceContext* _pDeviceContext, unsigned int _Index)
{
	oASSERT(_Index < States.size(), "Invalid shader state %u specified.", _Index);
	STATE& s = States[_Index];
	_pDeviceContext->IASetInputLayout(s.pInputLayout);
	_pDeviceContext->VSSetShader(s.pVertexShader, 0, 0);
	_pDeviceContext->HSSetShader(s.pHullShader, 0, 0);
	_pDeviceContext->DSSetShader(s.pDomainShader, 0, 0);
	_pDeviceContext->GSSetShader(s.pGeometryShader, 0, 0);
	_pDeviceContext->PSSetShader(s.pPixelShader, 0, 0);
}

void oD3D11ShaderState::ClearState(ID3D11DeviceContext* _pDeviceContext)
{
	_pDeviceContext->IASetInputLayout(0);
	_pDeviceContext->VSSetShader(0, 0, 0);
	_pDeviceContext->HSSetShader(0, 0, 0);
	_pDeviceContext->DSSetShader(0, 0, 0);
	_pDeviceContext->GSSetShader(0, 0, 0);
	_pDeviceContext->PSSetShader(0, 0, 0);
}

void oD3D11ShaderState::RegisterState(unsigned int _Index, STATE& _State, bool _ReferenceObjects)
{
	if (States.size() > _Index)
		Release(States[_Index]);

	if (_ReferenceObjects)
		AddRef(_State);

	oSafeSet(States, _Index, _State);
}

void oD3D11ShaderState::CreateShaders(ID3D11Device* _pDevice
																		 , oD3D11ShaderState::STATE* _pState
																		 , const BYTE* _pByteCodeVS, size_t _SizeofByteCodeVS
																		 , const BYTE* _pByteCodeHS, size_t _SizeofByteCodeHS
																		 , const BYTE* _pByteCodeDS, size_t _SizeofByteCodeDS
																		 , const BYTE* _pByteCodeGS, size_t _SizeofByteCodeGS
																		 , const BYTE* _pByteCodePS, size_t _SizeofByteCodePS)
{
	size_t shaderPointersSize = oByteDiff(&_pState->pPixelShader, &_pState->pVertexShader) + sizeof(void*);
	memset(&_pState->pVertexShader, 0, shaderPointersSize);

	if (_pByteCodeVS)
		oV(_pDevice->CreateVertexShader(_pByteCodeVS, _SizeofByteCodeVS, 0, &_pState->pVertexShader));
	if (_pByteCodeHS)
		oV(_pDevice->CreateHullShader(_pByteCodeHS, _SizeofByteCodeHS, 0, &_pState->pHullShader));
	if (_pByteCodeDS)
		oV(_pDevice->CreateDomainShader(_pByteCodeDS, _SizeofByteCodeDS, 0, &_pState->pDomainShader));
	if (_pByteCodeGS)
		oV(_pDevice->CreateGeometryShader(_pByteCodeGS, _SizeofByteCodeGS, 0, &_pState->pGeometryShader));
	if (_pByteCodePS)
		oV(_pDevice->CreatePixelShader(_pByteCodePS, _SizeofByteCodePS, 0, &_pState->pPixelShader));
}

oD3D11FullScreenQuad::oD3D11FullScreenQuad(ID3D11Device* _pDevice, bool _CCWWinding)
{
	struct FSQ_VERTEX
	{
		float x, y, z;
		float u, v;
	};

	static const float DIM = 1.0f;
	static const float Z = 0.0f;
	FSQ_VERTEX FSQVertices[6] = 
	{
		{ -DIM, -DIM, Z, 0.0f, 0.0f },
		{ -DIM, DIM, Z, 0.0f, 1.0f },
		{ DIM, -DIM, Z, 1.0f, 0.0f },

		{ DIM, -DIM, Z, 1.0f, 0.0f },
		{ -DIM, DIM, Z, 0.0f, 1.0f },
		{ DIM, DIM, Z, 1.0f, 1.0f },
	};

	if (_CCWWinding)
	{
		std::swap(FSQVertices[1], FSQVertices[2]);
		std::swap(FSQVertices[4], FSQVertices[5]);
	}

	oVERIFY(oD3D11CreateVertexBuffer(_pDevice, false, FSQVertices, 6, sizeof(FSQ_VERTEX), &pVertices));
}

oD3D11FullScreenQuad::~oD3D11FullScreenQuad()
{
	if (pVertices)
		pVertices->Release();
}

void oD3D11FullScreenQuad::Draw(ID3D11DeviceContext* _pDeviceContext)
{
	D3D11_BUFFER_DESC d;
	pVertices->GetDesc(&d);
	unsigned int stride = d.ByteWidth / 6;
	oD3D11Draw(_pDeviceContext, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 2, 1, &pVertices, &stride, 0, 0, 0, false, 0);
}
