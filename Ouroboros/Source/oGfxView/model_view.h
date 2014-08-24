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
