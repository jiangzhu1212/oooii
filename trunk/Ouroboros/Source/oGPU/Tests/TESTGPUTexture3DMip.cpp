// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/texture3d.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kIsDevMode = false;

struct gpu_test_texture3dmip : public gpu_texture_test
{
	gpu_test_texture3dmip() : gpu_texture_test("GPU test: texture3D mip", kIsDevMode) {}

	pipeline get_pipeline() override { pipeline p; p.input = gpu::intrinsic::vertex_layout::pos_uvw; p.vs = gpu::intrinsic::vertex_shader::texture3d; p.ps = gpu::intrinsic::pixel_shader::texture3d; return p; } 
	resource* make_test_texture() override
	{
		surface::info si;
		si.semantic = surface::semantic::custom3d;
		si.mip_layout = surface::mip_layout::tight;
		si.format = surface::format::b8g8r8a8_unorm;
		si.dimensions = int3(64,64,64);
		surface::image image(si);
		{
			surface::lock_guard lock(image);
			surface::fill_color_cube((color*)lock.mapped.data, lock.mapped.row_pitch, lock.mapped.depth_pitch, si.dimensions);
		}
		image.generate_mips();

		t.initialize("Test 3D", Device, image, true);
		return &t;
	}

	texture3d t;
};

oGPU_COMMON_TEST(texture3dmip);

	} // namespace tests
} // namespace ouro
