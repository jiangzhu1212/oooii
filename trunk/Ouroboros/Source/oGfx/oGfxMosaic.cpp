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
#include <oGfx/oGfxMosaic.h>
#include <oGPU/oGPUUtil.h>
#include <oGfxQuadPassThroughVS4ByteCode.h>

struct oGfxMosaicImpl : oGfxMosaic
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oGfxMosaicImpl(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, bool* _pSuccess);

	bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc) override;
	void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures) override;

	void SetBlendState(oGPU_BLEND_STATE _BlendState) override { BlendState = _BlendState; }

private:
	oRef<oGPUDevice> Device;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUBuffer> Indices;
	oRef<oGPUBuffer> Vertices;
	uint NumPrimitives;
	oGPU_BLEND_STATE BlendState;
	oRefCount RefCount;
};

static const oGPU_VERTEX_ELEMENT sMosaicElements[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false, },
	{ 'TEX0', oSURFACE_R32G32_FLOAT, 0, false, },
};

oGfxMosaicImpl::oGfxMosaicImpl(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, bool* _pSuccess)
	: Device(_pDevice)
	, NumPrimitives(0)
	, BlendState(oGPU_OPAQUE)
{
	*_pSuccess = false;

	oGPU_PIPELINE_DESC d;
	d.DebugName = "Mosaic.Pipeline";
	d.pElements = sMosaicElements;
	d.NumElements = oCOUNTOF(sMosaicElements);
	d.InputType = oGPU_TRIANGLES;
	d.pVertexShader = oGfxQuadPassThroughVS4ByteCode;
	d.pPixelShader = _pPixelShaderByteCode;

	if (!Device->CreatePipeline("MosaicPL", d, &Pipeline))
		return; // pass through error

	*_pSuccess = true;
}

bool oGfxMosaicCreate(oGPUDevice* _pDevice, const void* _pPixelShaderByteCode, oGfxMosaic** _ppMosaic)
{
	bool success = false;
	oCONSTRUCT(_ppMosaic, oGfxMosaicImpl(_pDevice, _pPixelShaderByteCode, &success));
	return success;
}

bool oGfxMosaicImpl::Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc)
{
	oRef<oGeometryFactory> GeoFactory;
	if (!oGeometryFactoryCreate(&GeoFactory))
		return false; // pass through error

	oGeometry::LAYOUT Layout;
	memset(&Layout, 0, sizeof(Layout));
	Layout.Positions = true;
	Layout.Texcoords = true;

	oRef<oGeometry> Geo;
	if (!GeoFactory->Create(_Desc, Layout, &Geo))
		return false; // pass through error

	oGeometry::DESC GeoDesc;
	Geo->GetDesc(&GeoDesc);

	oGeometry::CONST_MAPPED GeoMapped;
	if (!Geo->MapConst(&GeoMapped))
		return false; // pass through error

	oStd::finally OSEGeoUnmap([&] { Geo->UnmapConst(); });

	oSURFACE_CONST_MAPPED_SUBRESOURCE MSRGeo;
	MSRGeo.pData = GeoMapped.pIndices;
	MSRGeo.RowPitch = sizeof(uint);
	MSRGeo.DepthPitch = MSRGeo.RowPitch * GeoDesc.NumIndices;

	if (!oGPUCreateIndexBuffer(Device, "MosaicIB", GeoDesc.NumIndices, GeoDesc.NumVertices, MSRGeo, &Indices))
		return false; // pass through error

	if (!oGPUCreateVertexBuffer(Device
		, "MosaicVB"
		, GeoDesc.NumVertices
		, GeoDesc
		, GeoMapped
		, oCOUNTOF(sMosaicElements)
		, sMosaicElements
		, 0, &Vertices))
		return false; // pass through error

	NumPrimitives = GeoDesc.NumPrimitives;

	return true;
}

void oGfxMosaicImpl::Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures)
{
	std::array<oGPU_SAMPLER_STATE, oGPU_MAX_NUM_SAMPLERS> samplers;
	samplers.fill(oGPU_LINEAR_CLAMP);
	_pCommandList->SetRenderTarget(_pRenderTarget);
	_pCommandList->SetBlendState(BlendState);
	_pCommandList->SetSurfaceState(oGPU_TWO_SIDED);
	_pCommandList->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
	_pCommandList->SetSamplers(0, oUInt(samplers.size()), samplers.data());
	_pCommandList->SetShaderResources(_TextureStartSlot, _NumTextures, _ppTextures);
	_pCommandList->SetPipeline(Pipeline);
	_pCommandList->Draw(Indices, 0, 1, &Vertices, 0, NumPrimitives);
}
