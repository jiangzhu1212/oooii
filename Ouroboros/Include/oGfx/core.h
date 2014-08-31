// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Common boilerplate init for fixed-function state
#ifndef oGfx_core_h
#define oGfx_core_h

#include <oGPU/device.h>
#include <oGPU/blend_state.h>
#include <oGPU/depth_stencil_state.h>
#include <oGPU/rasterizer_state.h>
#include <oGPU/sampler_state.h>
#include <oGfx/layout_state.h>
#include <oGfx/shader_registries.h>

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
