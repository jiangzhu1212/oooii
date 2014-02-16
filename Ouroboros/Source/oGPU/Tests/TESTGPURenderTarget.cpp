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
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>

#include <oBasis/oMath.h>

using namespace ouro;

static const int sSnapshotFrames[] = { 0, 50 };
static const bool kIsDevMode = false;

struct GPU_RenderTarget_App : public oGPUTestApp
{
	GPU_RenderTarget_App() : oGPUTestApp("GPU_RenderTarget", kIsDevMode, sSnapshotFrames) {}

	bool Initialize() override
	{
		PrimaryRenderTarget->SetClearColor(almost_black);

		oGPUCommandList::DESC cld;
		cld.draw_order = 1;

		if (!Device->CreateCommandList("CLMainScene", cld, &CLMainScene))
			return false;

		cld.draw_order = 0;
		if (!Device->CreateCommandList("CLRenderTarget", cld, &CLRenderTarget))
			return false;

		oGPUBuffer::DESC DCDesc;
		DCDesc.struct_byte_size = sizeof(oGPUTestConstants);
		if (!Device->CreateBuffer("TestConstants", DCDesc, &TestConstants))
			return false;

		oGPUPipeline::DESC PassThroughDesc = oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH);

		if (!Device->CreatePipeline(PassThroughDesc.debug_name, PassThroughDesc, &PLPassThrough))
			return false;

		Triangle = ouro::gpu::make_first_triangle(Device);

		oGPUPipeline::DESC TextureDesc = oGPUTestGetPipeline(oGPU_TEST_TEXTURE_2D);
		if (!Device->CreatePipeline(TextureDesc.debug_name, TextureDesc, &PLTexture))
			return false;

		Cube = ouro::gpu::make_first_cube(Device);

		ouro::gpu::clear_info ci;
		ci.clear_color[0] = deep_sky_blue;

		oGPURenderTarget::DESC rtd;
		rtd.dimensions = ushort3(256, 256, 1);
		rtd.array_size = 1;
		rtd.mrt_count = 1;
		rtd.format[0] = ouro::surface::b8g8r8a8_unorm;
		rtd.depth_stencil_format = ouro::surface::d24_unorm_s8_uint;
		rtd.clear = ci;
		if (!Device->CreateRenderTarget("RenderTarget", rtd, &RenderTarget))
			return false;

		return true;
	}

	bool Render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		PrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTDesc.dimensions.x / oCastAsFloat(RTDesc.dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->GetFrameID() * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		// DrawOrder should be respected in out-of-order submits, so show that here
		// but executing on the main scene, THEN the render target, but because the
		// draw order of the command lists defines the render target before the 
		// main scene, this should come out as a cube with a triangle texture.

		intrusive_ptr<oGPUTexture> Texture;
		RenderTarget->GetTexture(0, &Texture);

		RenderMainScene(CLMainScene, Texture, PrimaryRenderTarget);
		RenderToTarget(CLRenderTarget, RenderTarget);
		return true;
	}

private:
	intrusive_ptr<oGPUCommandList> CLMainScene;
	intrusive_ptr<oGPUCommandList> CLRenderTarget;
	intrusive_ptr<oGPUPipeline> PLPassThrough;
	intrusive_ptr<oGPUPipeline> PLTexture;
	intrusive_ptr<oGPURenderTarget> RenderTarget;
	std::shared_ptr<ouro::gpu::util_mesh> Cube;
	std::shared_ptr<ouro::gpu::util_mesh> Triangle;
	intrusive_ptr<oGPUBuffer> TestConstants;

	void RenderToTarget(oGPUCommandList* _pCommandList, oGPURenderTarget* _pTarget)
	{
		_pCommandList->Begin();
		_pCommandList->Clear(_pTarget, ouro::gpu::clear_type::color_depth_stencil);
		_pCommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		_pCommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::none);
		_pCommandList->SetSurfaceState(ouro::gpu::surface_state::front_face);
		_pCommandList->SetPipeline(PLPassThrough);
		_pCommandList->SetRenderTarget(_pTarget);
		Triangle->draw(_pCommandList);
		_pCommandList->End();
	}

	void RenderMainScene(oGPUCommandList* _pCommandList, oGPUTexture* _pTexture, oGPURenderTarget* _pTarget)
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		_pTarget->GetDesc(&RTDesc);
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTDesc.dimensions.x / oCastAsFloat(RTDesc.dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->GetFrameID() * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		_pCommandList->Begin();

		ouro::gpu::commit_buffer(_pCommandList, TestConstants, oGPUTestConstants(W, V, P, white));

		_pCommandList->Clear(_pTarget, ouro::gpu::clear_type::color_depth_stencil);
		_pCommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		_pCommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::test_and_write);
		_pCommandList->SetSurfaceState(ouro::gpu::surface_state::front_face);
		_pCommandList->SetBuffers(0, 1, &TestConstants);
		ouro::gpu::sampler_type::value s = ouro::gpu::sampler_type::linear_wrap;
		_pCommandList->SetSamplers(0, 1, &s);
		_pCommandList->SetShaderResources(0, 1, &_pTexture);
		_pCommandList->SetPipeline(PLTexture);
		_pCommandList->SetRenderTarget(_pTarget);
		Cube->draw(_pCommandList);

		_pCommandList->End();
	}
};

oDEFINE_GPU_TEST(GPU_RenderTarget)
