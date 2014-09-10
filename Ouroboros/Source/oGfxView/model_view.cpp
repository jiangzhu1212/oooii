// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "model_view.h"
#include <oBase/assert.h>

namespace ouro { namespace gfx {

void model_view::initialize(gfx::core& _core)
{
	core = &_core;

	vs.initialize("VS", core->device, gpu::intrinsic::byte_code(gpu::intrinsic::vertex_shader::pass_through_pos));
	ps.initialize("PS", core->device, gpu::intrinsic::byte_code(gpu::intrinsic::pixel_shader::white));
	tri.initialize_first_triangle(core->device);
}

void model_view::deinitialize()
{
	ctarget = nullptr;
	dtarget = nullptr;
	tri.deinitialize();
	ps.deinitialize();
	vs.deinitialize();
}

void model_view::render()
{
	if (!ctarget || !*ctarget)
	{
		oTRACE("No color target");
		return;
	}

	auto& c = *core;
	auto& ct = *ctarget;
	auto& cl = c.device.immediate();

	ct.clear(cl, clear);
	ct.set_draw_target(cl, dtarget);
		
	c.bs.set(cl, gpu::blend_state::opaque);
	c.dss.set(cl, gpu::depth_stencil_state::none);
	c.rs.set(cl, gpu::rasterizer_state::front_face);
	c.ss.set(cl, gpu::sampler_state::linear_wrap, gpu::sampler_state::linear_wrap);
	c.ls.set(cl, gpu::intrinsic::vertex_layout::pos, mesh::primitive_type::triangles);
	vs.set(cl);
	ps.set(cl);

	tri.draw(cl);
	ct.present();
}

}}
