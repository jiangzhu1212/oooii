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
// Utility class for rendering a single surface fullscreen (image viewer,
// convert images using GPU hardware, etc.)
#ifndef oGfx_surface_view_h
#define oGfx_surface_view_h

#include <oGPU/color_target.h>
#include <oGPU/primary_target.h>
#include <oGPU/shader.h>
#include <oGPU/texture1d.h>
#include <oGPU/texture2d.h>
#include <oGPU/texture3d.h>
#include <oGPU/texturecube.h>
#include <oGfx/core.h>

namespace ouro { namespace gfx {

class surface_view
{
public:
	surface_view() : core(nullptr), active(nullptr), needs_update(0), present(false) {}
	~surface_view() { deinitialize(); }

	void initialize(gfx::core& core);
	void deinitialize();

	void set_texels(const char* name, const surface::texel_buffer& b);

	inline void set_draw_target(gpu::primary_target* t) { ctarget = t; present = true; }
	inline void set_draw_target(gpu::color_target* t) { ctarget = t; present = false; }

	void render();

private:
	gfx::core* core;
	gpu::resource* active;
	gpu::basic_color_target* ctarget;
	gpu::texture1d t1d;
	gpu::texture2d t2d;
	gpu::texture3d t3d;
	gpu::texturecube tcube;
	gpu::vertex_shader vs;
	gpu::pixel_shader ps;
	surface::info inf;
	int needs_update;
	bool present;
};

}}

#endif
