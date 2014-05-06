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
#pragma once
#ifndef oGPU_d3d_state_h
#define oGPU_d3d_state_h

#include <oGPU/state.h>
#include <d3d11.h>
#include "d3d_types.h"

namespace ouro {
	namespace gpu {
		namespace d3d {

class sampler_state_registry
{
public:
	sampler_state_registry() { states.fill(nullptr); }
	~sampler_state_registry() { deinitialize(); }
	void initialize(Device* dev);
	void deinitialize() { if (states[0]) { for (auto& s : states) { s->Release(); s = nullptr; } } }
	SamplerState* operator[](const sampler_state::value& state) const { return states[state]; }
private:
	std::array<SamplerState*, sampler_state::count> states;
};

class rasterizer_state_registry
{
public:
	rasterizer_state_registry() { states.fill(nullptr); }
	~rasterizer_state_registry() { deinitialize(); }
	void initialize(Device* dev);
	void deinitialize() { if (states[0]) { for (auto& s : states) { s->Release(); s = nullptr; } } }
	RasterizerState* operator[](const rasterizer_state::value& state) const { return states[state]; }
private:
	std::array<RasterizerState*, rasterizer_state::count> states;
};

class blend_state_registry
{
public:
	blend_state_registry() { states.fill(nullptr); }
	~blend_state_registry() { deinitialize(); }
	void initialize(Device* dev);
	void deinitialize() { if (states[0]) { for (auto& s : states) { s->Release(); s = nullptr; } } }
	BlendState* operator[](const blend_state::value& state) const { return states[state]; }
private:
	std::array<BlendState*, blend_state::count> states;
};

class depth_stencil_state_registry
{
public:
	depth_stencil_state_registry() { states.fill(nullptr); }
	~depth_stencil_state_registry() { deinitialize(); }
	void initialize(Device* dev);
	void deinitialize() { if (states[0]) { for (auto& s : states) { s->Release(); s = nullptr; } } }
	DepthStencilState* operator[](const depth_stencil_state::value& state) const { return states[state]; }
private:
	std::array<DepthStencilState*, depth_stencil_state::count> states;
};

		} // namespace d3d
	} // namespace gpu
} // namespace ouro

#endif
