// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/texture1d.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kIsDevMode = false;

struct gpu_test_texture1dmip : public gpu_texture_test
{
	gpu_test_texture1dmip() : gpu_texture_test("GPU test: texture 1D mip", kIsDevMode) {}

	pipeline get_pipeline() override { pipeline p; p.input = gpu::intrinsic::vertex_layout::pos_uv; p.vs = gpu::intrinsic::vertex_shader::texture1d; p.ps = gpu::intrinsic::pixel_shader::texture1d; return p; } 
	resource* make_test_texture() override
	{
		auto image = make_1D(512, true);
		t.initialize("Test 1D", Device, image, true);
		return &t;
	}

	texture1d t;
};

oGPU_COMMON_TEST(texture1dmip);

	} // namespace tests
} // namespace ouro
