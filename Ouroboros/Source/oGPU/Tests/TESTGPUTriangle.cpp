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

using namespace ouro;

static const int sSnapshotFrames[] = { 0 };
static const bool kIsDevMode = false;

struct GPU_Triangle_App : public oGPUTestApp
{
	GPU_Triangle_App() : oGPUTestApp("GPU_Triangle", kIsDevMode, sSnapshotFrames) {}

	bool Initialize() override
	{
		PrimaryRenderTarget->SetClearColor(almost_black);

		oGPUPipeline::DESC pld = oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH);

		if (!Device->CreatePipeline(pld.debug_name, pld, &Pipeline))
			return false;

		Mesh = ouro::gpu::make_first_triangle(Device);

		return true;
	}

	bool Render() override
	{
		CommandList->Begin();
		CommandList->Clear(PrimaryRenderTarget, ouro::gpu::clear_type::color_depth_stencil);
		CommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		CommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::none);
		CommandList->SetSurfaceState(ouro::gpu::surface_state::front_face);
		CommandList->SetPipeline(Pipeline);
		CommandList->SetRenderTarget(PrimaryRenderTarget);
		Mesh->draw(CommandList);
		CommandList->End();
		return true;
	}

private:
	intrusive_ptr<oGPUPipeline> Pipeline;
	std::shared_ptr<ouro::gpu::util_mesh> Mesh;
};

oDEFINE_GPU_TEST(GPU_Triangle)
