/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oGfx/surface_view.h>
#include <oGPU/vertex_buffer.h>

namespace ouro { namespace gfx {

void surface_view::initialize(gfx::core& _core)
{
	core = &_core;
	active = nullptr;
	vs.initialize("fullscreen_tri", core->device, gpu::intrinsic::byte_code(gpu::intrinsic::vertex_shader::fullscreen_tri));
	ps.initialize("texture", core->device, gpu::intrinsic::byte_code(gpu::intrinsic::pixel_shader::texture2d));
}

void surface_view::deinitialize()
{
	t1d.deinitialize();
	t2d.deinitialize();
	t3d.deinitialize();
	tcube.deinitialize();
	ps.deinitialize();
	vs.deinitialize();
	active = nullptr;
	core = nullptr;
}

void surface_view::set_texels(const char* name, const surface::image& img)
{
	t1d.deinitialize();
	t2d.deinitialize();
	t3d.deinitialize();
	tcube.deinitialize();

	inf = img.get_info();

	if (inf.is_1d())
	{
		t1d.initialize(name, core->device, img, inf.mips());
		active = &t1d;
		oTHROW(operation_not_supported, "1d viewing not yet enabled");
	}

	else if (inf.is_2d())
	{
		t2d.initialize(name, core->device, img, inf.mips());
		active = &t2d;
	}

	else if (inf.is_3d())
	{
		t3d.initialize(name, core->device, img, inf.mips());
		active = &t3d;
		oTHROW(operation_not_supported, "3d slice viewing not yet enabled");
	}

	else if (inf.is_cube())
	{
		tcube.initialize(name, core->device, img, inf.mips());
		active = &tcube;
		oTHROW(operation_not_supported, "cube slice viewing not yet enabled");
	}

	needs_update = 2;
}

void surface_view::render()
{
	if (!ctarget || !*ctarget || !needs_update)
		return;

	auto& c = *core;
	auto& ct = *ctarget;
	auto& cl = c.device.immediate();

	ct.clear(cl, black);
	ct.set_draw_target(cl);

	if (active)
	{
		c.bs.set(cl, gpu::blend_state::opaque);
		c.dss.set(cl, gpu::depth_stencil_state::none);
		c.rs.set(cl, gpu::rasterizer_state::front_face);
		c.ss.set(cl, gpu::sampler_state::linear_wrap, gpu::sampler_state::linear_wrap);
		c.ls.set(cl, gpu::intrinsic::vertex_layout::none, mesh::primitive_type::triangles);
		vs.set(cl);
		ps.set(cl);
		active->set(cl, 0);
		gpu::vertex_buffer::draw_unindexed(cl, 3);
	}

	if (present)
		((gpu::primary_target&)ct).present();

	needs_update--;
}

}}
