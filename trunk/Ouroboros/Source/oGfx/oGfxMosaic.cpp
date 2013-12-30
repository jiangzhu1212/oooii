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
#include <oGfx/oGfxMosaic.h>
#include <oGPU/oGPUUtil.h>
#include <oBase/finally.h>

using namespace ouro;

struct oGfxMosaicImpl : oGfxMosaic
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oGfxMosaicImpl(oGPUDevice* _pDevice, const oGPU_PIPELINE_DESC& _pPipelineDesc, bool* _pSuccess);

	bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc, int _NumAdditionalTextureSets, const oRECT* _AdditionalSourceImageSpaces, const oRECT* const* _pAdditionalSourceRectArrays) override;
	void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures) override;

	void SetBlendState(ouro::gpu::blend_state::value _BlendState) override { BlendState = _BlendState; }

private:
	intrusive_ptr<oGPUDevice> Device;
	intrusive_ptr<oGPUPipeline> Pipeline;
	intrusive_ptr<oGPUBuffer> Indices;
	intrusive_ptr<oGPUBuffer> Vertices[2];
	uint NumPrimitives;
	ouro::gpu::blend_state::value BlendState;
	oRefCount RefCount;
};

oGfxMosaicImpl::oGfxMosaicImpl(oGPUDevice* _pDevice, const oGPU_PIPELINE_DESC& _PipelineDesc, bool* _pSuccess)
	: Device(_pDevice)
	, NumPrimitives(0)
	, BlendState(ouro::gpu::blend_state::opaque)
{
	*_pSuccess = false;

	if (!Device->CreatePipeline("MosaicExPL", _PipelineDesc, &Pipeline))
		return; // pass through error

	*_pSuccess = true;
}

bool oGfxMosaicCreate(oGPUDevice* _pDevice, const oGPU_PIPELINE_DESC& _PipelineDesc, oGfxMosaic** _ppMosaic)
{
	bool success = false;
	oCONSTRUCT(_ppMosaic, oGfxMosaicImpl(_pDevice, _PipelineDesc, &success));
	return success;
}

bool oGfxMosaicImpl::Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc, int _NumAdditionalTextureSets, const oRECT* _AdditionalSourceImageSpaces, const oRECT* const* _pAdditionalSourceRectArrays)
{
	intrusive_ptr<oGeometryFactory> GeoFactory;
	if (!oGeometryFactoryCreate(&GeoFactory))
		return false; // pass through error

	oGeometry::LAYOUT Layout;
	memset(&Layout, 0, sizeof(Layout));
	Layout.Positions = true;
	Layout.Texcoords = _NumAdditionalTextureSets ? !!_Desc.pSourceRects : true;

	intrusive_ptr<oGeometry> Geo;
	if (!GeoFactory->Create(_Desc, Layout, &Geo))
		return false; // pass through error

	oGeometry::DESC GeoDesc;
	Geo->GetDesc(&GeoDesc);

	oGeometry::CONST_MAPPED GeoMapped;
	if (!Geo->MapConst(&GeoMapped))
		return false; // pass through error

	finally OSEGeoUnmap([&] { Geo->UnmapConst(); });

	ouro::surface::const_mapped_subresource MSRGeo;
	MSRGeo.data = GeoMapped.pIndices;
	MSRGeo.row_pitch = sizeof(uint);
	MSRGeo.depth_pitch = MSRGeo.row_pitch * GeoDesc.NumIndices;

	if (!oGPUCreateIndexBuffer(Device, "MosaicIB", GeoDesc.NumIndices, GeoDesc.NumVertices, MSRGeo, &Indices))
		return false; // pass through error

	oGPU_PIPELINE_DESC pd;
	Pipeline->GetDesc(&pd);

	if (!oGPUCreateVertexBuffer(Device
		, "MosaicVB"
		, GeoDesc.NumVertices
		, GeoDesc
		, GeoMapped
		, pd.NumElements
		, pd.pElements
		, 0, &Vertices[0]))
		return false; // pass through error

	NumPrimitives = GeoDesc.NumPrimitives;

	if (_NumAdditionalTextureSets)
	{
		intrusive_ptr<oGeometryFactory> GeoFactory;
		if (!oGeometryFactoryCreate(&GeoFactory))
			return false; // pass through error

		std::vector<intrusive_ptr<oGeometry>> ExtraGeos;
		std::vector<oGeometry::CONST_MAPPED> ExtraGeoMapped;
		ExtraGeos.resize(_NumAdditionalTextureSets);
		ExtraGeoMapped.resize(_NumAdditionalTextureSets);
		finally OSEGeoUnmap([&] { oFOR(auto geo, ExtraGeos) geo->UnmapConst(); });

		for (int i = 0; i < _NumAdditionalTextureSets; i++)
		{
			// Rebuild extra UV sets
			oGeometryFactory::MOSAIC_DESC mosaicDesc;
			mosaicDesc = _Desc;
			mosaicDesc.SourceImageSpace = _AdditionalSourceImageSpaces[i];
			mosaicDesc.pSourceRects = _pAdditionalSourceRectArrays[i];

			oGeometry::LAYOUT Layout;
			memset(&Layout, 0, sizeof(Layout));
			Layout.Positions = true;
			Layout.Texcoords = !!mosaicDesc.pSourceRects;

			if (!GeoFactory->Create(mosaicDesc, Layout, &ExtraGeos[i]))
				return false; // pass through error

			if (!ExtraGeos[i]->MapConst(&ExtraGeoMapped[i]))
				return false; // pass through error
		}

		if (!oGPUCreateVertexBuffer(Device
			, "MosaicExVB"
			, GeoDesc.NumVertices
			, [&](const oGPU_VERTEX_ELEMENT& _Element, ouro::surface::const_mapped_subresource* _pElementData)
				{
					if (!_Element.Instanced)
					{
						int textureSet = ouro::invalid;
						switch((int)_Element.Semantic)
						{
							case 'TEX1': textureSet = 0; break;
							case 'TEX2': textureSet = 1; break;
							case 'TEX3': textureSet = 2; break;
							case 'TEX4': textureSet = 3; break;
							case 'TEX5': textureSet = 4; break;
							case 'TEX6': textureSet = 5; break;
							case 'TEX7': textureSet = 6; break;
							case 'TEX8': textureSet = 7; break;
							case 'TEX9': textureSet = 8; break;
							default: break;
						}
						if (textureSet != ouro::invalid)
						{
							_pElementData->data = ExtraGeoMapped[textureSet].pTexcoords;
							_pElementData->row_pitch = sizeof(*ExtraGeoMapped[textureSet].pTexcoords);
							_pElementData->depth_pitch = 0;
						}
						else
							_pElementData->data = nullptr;
					}
				}
			, pd.NumElements
			, pd.pElements
			, 1, &Vertices[1]))
			return false; // pass through error
	}
	return true;
}

void oGfxMosaicImpl::Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures)
{
	std::array<ouro::gpu::sampler_type::value, ouro::gpu::max_num_samplers> samplers;
	samplers.fill(ouro::gpu::sampler_type::linear_clamp);
	if (_pRenderTarget)
		_pCommandList->SetRenderTarget(_pRenderTarget);
	_pCommandList->SetBlendState(BlendState);
	_pCommandList->SetSurfaceState(ouro::gpu::surface_state::two_sided);
	_pCommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::none);
	_pCommandList->SetSamplers(0, as_uint(samplers.size()), samplers.data());
	_pCommandList->SetShaderResources(_TextureStartSlot, _NumTextures, _ppTextures);
	_pCommandList->SetPipeline(Pipeline);
	_pCommandList->Draw(Indices, 0, Vertices[1] ? 2 : 1, &Vertices[0], 0, NumPrimitives);
}
