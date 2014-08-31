// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGfx_model_view_h
#define oGfx_model_view_h

#include <oGfx/core.h>
#include <oGPU/depth_target.h>
#include <oGPU/primary_target.h>

#include <oGPU/oGPUUtilMesh.h>

namespace ouro { namespace gfx {

class model_view
{
public:
	model_view() : core(nullptr), clear(black), ctarget(nullptr), dtarget(nullptr) {}
	~model_view() { deinitialize(); }

	void initialize(core& core);
	void deinitialize();
	
	inline void set_draw_target(gpu::primary_target* t, gpu::depth_target* d = nullptr) { ctarget = t; dtarget = d; }

	inline void set_clear(const color& c) { clear = c; }
	inline color get_clear() const { return clear; }

	void render();

private:
	core* core;
	gpu::primary_target* ctarget;
	gpu::depth_target* dtarget;
	gpu::util_mesh tri;
	gpu::vertex_shader vs;
	gpu::pixel_shader ps;

	color clear;
};

}}

#endif
