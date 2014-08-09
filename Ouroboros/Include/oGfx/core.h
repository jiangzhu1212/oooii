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
// Common boilerplate init for fixed-function state

#ifndef oGfx_core_h
#define oGfx_core_h

#include <oGPU/device.h>
#include <oGPU/blend_state.h>
#include <oGPU/depth_stencil_state.h>
#include <oGPU/rasterizer_state.h>
#include <oGPU/sampler_state.h>
#include <oGfx/layout_state.h>
#include <oGfx/oGfxShaderRegistry.h>

namespace ouro { namespace gfx {

class core
{
public:
	core() {}
	core(const char* name, bool enable_driver_reporting) { initialize(name, enable_driver_reporting); }
	~core() { deinitialize(); }

	void initialize(const char* name, bool enable_driver_reporting);
	void deinitialize();

	// fixed-function state managers
	gpu::device device;
	gpu::blend_state bs;
	gpu::depth_stencil_state dss;
	gpu::rasterizer_state rs;
	gpu::sampler_state ss;

	// shader managers
	gfx::layout_state ls;
	gfx::vs_registry vs;
	gfx::ps_registry ps;
};

}}

#endif
