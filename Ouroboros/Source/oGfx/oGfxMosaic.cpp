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
using namespace ouro::gpu;

struct oGfxMosaicImpl : oGfxMosaic
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oGfxMosaicImpl(oGPUDevice* _pDevice, const pipeline_info& _pPipelineDesc, bool* _pSuccess);

	bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc, int _NumAdditionalTextureSets, const ouro::rect* _AdditionalSourceImageSpaces, const ouro::rect* const* _pAdditionalSourceRectArrays) override;
	void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures) override;

	void SetBlendState(blend_state::value _BlendState) override { BlendState = _BlendState; }

private:
	intrusive_ptr<oGPUDevice> Device;
	intrusive_ptr<oGPUPipeline> Pipeline;
	intrusive_ptr<oGPUBuffer> Indices;
	intrusive_ptr<oGPUBuffer> Vertices[2];
	uint NumPrimitives;
	blend_state::value BlendState;
	oRefCount RefCount;
};

oGfxMosaicImpl::oGfxMosaicImpl(oGPUDevice* _pDevice, const pipeline_info& _PipelineDesc, bool* _pSuccess)
	: Device(_pDevice)
	, NumPrimitives(0)
	, BlendState(blend_state::opaque)
{
	*_pSuccess = false;

	if (!Device->CreatePipeline("MosaicExPL", _PipelineDesc, &Pipeline))
		return; // pass through error

	*_pSuccess = true;
}

intrusive_ptr<oGfxMosaic> oGfxMosaicCreate(oGPUDevice* _pDevice, const pipeline_info& _Info)
{
	intrusive_ptr<oGfxMosaic> m;
	bool success = false;
	oCONSTRUCT(&m, oGfxMosaicImpl(_pDevice, _Info, &success));
	return success ? m : nullptr;
}

bool oGfxMosaicImpl::Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc, int _NumAdditionalTextureSets, const ouro::rect* _AdditionalSourceImageSpaces, const ouro::rect* const* _pAdditionalSourceRectArrays)
{
	intrusive_ptr<oGeometryFactory> GeoFactory;
	if (!oGeometryFactoryCreate(&GeoFactory))
		return false; // pass through error

	mesh::layout::value Layout = mesh::layout::pos;
	if (0 == _NumAdditionalTextureSets || !!_Desc.pSourceRects)
		Layout = mesh::layout::pos_uv0;

	intrusive_ptr<oGeometry> Geo;
	if (!GeoFactory->Create(_Desc, Layout, &Geo))
		return false; // pass through error

	ouro::mesh::info GeoInfo = Geo->get_info();
	ouro::mesh::source GeoSource = Geo->get_source();

	ouro::surface::const_mapped_subresource MSRGeo;
	MSRGeo.data = GeoSource.indicesi;
	MSRGeo.row_pitch = sizeof(uint);
	MSRGeo.depth_pitch = MSRGeo.row_pitch * GeoInfo.num_indices;
	Indices = make_index_buffer(Device, "MosaicIB", GeoInfo.num_indices, GeoInfo.num_vertices, MSRGeo);

	pipeline_info pi = Pipeline->get_info();

	Vertices[0] = make_vertex_buffer(Device, "MosaicVB", GeoInfo.vertex_layouts[0], GeoInfo.num_vertices, GeoSource);

	NumPrimitives = ouro::mesh::num_primitives(GeoInfo);

	if (_NumAdditionalTextureSets)
	{
		intrusive_ptr<oGeometryFactory> GeoFactory;
		if (!oGeometryFactoryCreate(&GeoFactory))
			return false; // pass through error

		std::vector<intrusive_ptr<oGeometry>> ExtraGeos;
		std::vector<ouro::mesh::source> ExtraGeoSource;
		ExtraGeos.resize(_NumAdditionalTextureSets);
		ExtraGeoSource.resize(_NumAdditionalTextureSets);

		for (int i = 0; i < _NumAdditionalTextureSets; i++)
		{
			// Rebuild extra UV sets
			oGeometryFactory::MOSAIC_DESC mosaicDesc;
			mosaicDesc = _Desc;
			mosaicDesc.SourceImageSpace = _AdditionalSourceImageSpaces[i];
			mosaicDesc.pSourceRects = _pAdditionalSourceRectArrays[i];

			mesh::layout::value Layout = mesh::layout::pos;
			if (!!mosaicDesc.pSourceRects)
				Layout = mesh::layout::pos_uv0;

			if (!GeoFactory->Create(mosaicDesc, Layout, &ExtraGeos[i]))
				return false; // pass through error

			ExtraGeoSource[i] = ExtraGeos[i]->get_source();
		}

		// @tony: what was this for? when uv1's are supported add this back?
#if 0
		Vertices[1] = make_vertex_buffer(Device, "MosaicExVB", 
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
#endif
	}
	return true;
}

void oGfxMosaicImpl::Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures)
{
	std::array<sampler_type::value, max_num_samplers> samplers;
	samplers.fill(sampler_type::linear_clamp);
	if (_pRenderTarget)
		_pCommandList->SetRenderTarget(_pRenderTarget);
	_pCommandList->SetBlendState(BlendState);
	_pCommandList->SetSurfaceState(surface_state::two_sided);
	_pCommandList->SetDepthStencilState(depth_stencil_state::none);
	_pCommandList->SetSamplers(0, as_uint(samplers.size()), samplers.data());
	_pCommandList->SetShaderResources(_TextureStartSlot, _NumTextures, _ppTextures);
	_pCommandList->SetPipeline(Pipeline);
	_pCommandList->Draw(Indices, 0, Vertices[1] ? 2 : 1, &Vertices[0], 0, NumPrimitives);
}
