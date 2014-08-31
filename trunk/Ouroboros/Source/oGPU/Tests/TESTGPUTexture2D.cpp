// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/texture2d.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kIsDevMode = false;

struct gpu_test_texture2d : public gpu_texture_test
{
	gpu_test_texture2d() : gpu_texture_test("GPU test: texture2d", kIsDevMode) {}

	pipeline get_pipeline() override { pipeline p; p.input = gpu::intrinsic::vertex_layout::pos_uv; p.vs = gpu::intrinsic::vertex_shader::texture2d; p.ps = gpu::intrinsic::pixel_shader::texture2d; return p; } 
	resource* make_test_texture() override
	{
		auto image = surface_load(filesystem::data_path() / "Test/Textures/lena_1.png", false);
		t.initialize("Test 2D", Device, image, false);
		return &t;
	}

	texture2d t;
};

oGPU_COMMON_TEST(texture2d);

	} // namespace tests
} // namespace ouro
