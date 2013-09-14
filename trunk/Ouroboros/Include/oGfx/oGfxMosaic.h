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
// Utility object for defining several screen-space quads and drawing them with
// a specified pixel shader. The intent is enable composite/multi-screen 
// rendering with bezel support.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxMosaic_h
#define oGfxMosaic_h

#include <oBasis/oGeometry.h>
#include <oGPU/oGPU.h>

interface oGfxMosaic : oInterface
{
	// Sets up an oGeometry::MOSAIC mesh and renders it in a simple clip-space,
	// non-z-tested, opaque state. This is useful in setting up image and video
	// players. This is more than a full-screen quad because of the more complex
	// requirements of video wall presentation where the logical screen might be 
	// made up of several physical screen.

	virtual bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc, int _NumAdditionalTextureSets, const oRECT* _AdditionalSourceImageSpaces, const oRECT* const* _pAdditionalSourceRectArrays) = 0;
	inline bool Rebuild(const oGeometryFactory::MOSAIC_DESC& _Desc) { return Rebuild(_Desc, 0, nullptr, nullptr); }
	virtual void SetBlendState(oGPU_BLEND_STATE _BlendState) = 0; // will be oGPU_OPAQUE by default

	virtual void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oGPUTexture* const* _ppTextures) = 0;
	inline void Draw(oGPUCommandList* _pCommandList, oGPURenderTarget* _pRenderTarget, uint _TextureStartSlot, uint _NumTextures, const oStd::intrusive_ptr<oGPUTexture>* _ppTextures) { Draw(_pCommandList, _pRenderTarget, _TextureStartSlot, _NumTextures, (const oGPUTexture* const*)_ppTextures); }
};

oAPI bool oGfxMosaicCreate(oGPUDevice* _pDevice, const oGPU_PIPELINE_DESC& _PipelineDesc, oGfxMosaic** _ppMosaic);

#endif
