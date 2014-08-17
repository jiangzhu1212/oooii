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
#include "model_view.h"

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
