// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
	surface_view(gfx::core& core) { initialize(core); }
	~surface_view() { deinitialize(); }

	void initialize(gfx::core& core);
	void deinitialize();

	void set_texels(const char* name, const surface::image& img);

	inline void set_draw_target(gpu::primary_target* t) { ctarget = t; present = true; }
	inline void set_draw_target(gpu::primary_target& t) { ctarget = &t; present = true; }
	inline void set_draw_target(gpu::color_target* t) { ctarget = t; present = false; }
	inline void set_draw_target(gpu::color_target& t) { ctarget = &t; present = false; }

	inline gpu::basic_color_target* get_draw_target() const { return ctarget; }

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
