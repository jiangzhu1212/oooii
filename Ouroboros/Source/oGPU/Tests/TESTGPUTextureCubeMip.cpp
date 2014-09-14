// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/texturecube.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kIsDevMode = false;

struct gpu_test_texturecubemip : public gpu_texture_test
{
	gpu_test_texturecubemip() : gpu_texture_test("GPU test: texturecube mip", kIsDevMode) {}

	pipeline get_pipeline() override { pipeline p; p.input = gpu::intrinsic::vertex_layout::pos_uvw; p.vs = gpu::intrinsic::vertex_shader::texturecube; p.ps = gpu::intrinsic::pixel_shader::texturecube; return p; } 
	resource* make_test_texture() override
	{
		auto _0 = surface_load(filesystem::data_path() / "Test/Textures/CubePosX.png", false);
		auto _1 = surface_load(filesystem::data_path() / "Test/Textures/CubeNegX.png", false);
		auto _2 = surface_load(filesystem::data_path() / "Test/Textures/CubePosY.png", false);
		auto _3 = surface_load(filesystem::data_path() / "Test/Textures/CubeNegY.png", false);
		auto _4 = surface_load(filesystem::data_path() / "Test/Textures/CubePosZ.png", false);
		auto _5 = surface_load(filesystem::data_path() / "Test/Textures/CubeNegZ.png", false);

		const surface::image* images[6] = { &_0, &_1, &_2, &_3, &_4, &_5 };
		surface::image image;
		image.initialize_array(images, true);
		image.set_semantic(surface::semantic::customcube);
		t.initialize("Test cube", Device, image, true);
		return &t;
	}

	texturecube t;
};

oGPU_COMMON_TEST(texturecubemip);

	}
}
