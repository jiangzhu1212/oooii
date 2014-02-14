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

static const bool kIsDevMode = false;

struct GPU_Texture2DMip_App : public oGPUTextureTestApp
{
	GPU_Texture2DMip_App() : oGPUTextureTestApp("GPU_Texture2DMip", kIsDevMode) {}

	oGPU_TEST_PIPELINE GetPipeline() override { return oGPU_TEST_TEXTURE_2D; }
	bool CreateTexture() override
	{
		auto image = surface_load(filesystem::data_path() / "Test/Textures/lena_1.png", surface::alpha_option::force_alpha);
		auto* p = image.get();
		Texture = ouro::gpu::make_texture(Device, "Test 2D mipped", &p, 1, ouro::gpu::texture_type::mipped_2d);
		return true;
	}
};

oDEFINE_GPU_TEST(GPU_Texture2DMip)
