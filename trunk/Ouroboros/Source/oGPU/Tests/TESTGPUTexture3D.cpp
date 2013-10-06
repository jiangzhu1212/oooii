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

struct GPU_Texture3D_App : public oGPUTextureTestApp
{
	GPU_Texture3D_App() : oGPUTextureTestApp("GPU_Texture3D", kIsDevMode) {}

	oGPU_TEST_PIPELINE GetPipeline() override { return oGPU_TEST_TEXTURE_3D; }
	bool CreateTexture() override
	{
		auto red = surface_load(filesystem::data_path() / "Test/Textures/Red.png", surface::alpha_option::force_alpha);
		auto green = surface_load(filesystem::data_path() / "Test/Textures/Green.png", surface::alpha_option::force_alpha);
		auto blue = surface_load(filesystem::data_path() / "Test/Textures/Blue.png", surface::alpha_option::force_alpha);

		const ouro::surface::buffer* images[3];
		images[0] = red.get();
		images[1] = green.get();
		images[2] = blue.get();

		if (!oGPUCreateTexture(Device, images, oCOUNTOF(images), oGPU_TEXTURE_3D_MAP, &Texture))
			return false;

		return true;
	}
};

oDEFINE_GPU_TEST(GPU_Texture3D)
