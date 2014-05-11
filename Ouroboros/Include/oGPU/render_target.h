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
#ifndef oGPU_render_target2_h
#define oGPU_render_target2_h

#include <oBase/colors.h>
#include <oSurface/surface.h>
#include <array>

namespace ouro {
	namespace gpu {
	
static const uint max_num_mrts = 8;
	
struct render_target_info
{
	render_target_info()
		: dimensions(0, 0, 0)
		, num_mrts(1)
		, array_size(1)
		, depth_stencil_format(surface::unknown)
		, has_mips(false)
		, depth_clear_value(1.0f)
		, stencil_clear_value(0)
	{ 
		format.fill(surface::unknown);
		clear_color.fill(black);
	}

	ushort3 dimensions;
	ushort num_mrts;
	ushort array_size;
	bool has_mips;
	surface::format depth_stencil_format;
	std::array<surface::format, max_num_mrts> format;
	std::array<color, max_num_mrts> clear_color;
	float depth_clear_value;
	uchar stencil_clear_value;
};
#if 0

class device;
class texture;
class render_target2
{
public:
	render_target2();
	~render_target2();

	render_target_info2 get_info() const;

	// modifies the values for color clearing without modifying other topology
	void set_clear_colors(uint num_clear_colors, const color* clear_colors);

	// resizes all buffers without changing formats or other topology
	void resize(const ushort3& new_dimensions);

	// Accesses a readable texture for the specified render target in an MRT.
	inline texture* get_texture(int mrt_index) { return textures[mrt_index]; }

	// Accesses a readable texture for the depth-stencil buffer. This will throw if 
	// there is no depth-stencil buffer or if the buffer is a non-readable format.
	inline texture* get_depth_texture() { return depth; }

	// Creates a buffer of the contents of the render target. This should be 
	// called at times when it is known the render target has been fully resolved,
	// mostly outside of begin_frame/end_frame. If this is called on the primary
	// render target the back-buffer is captured.
	std::shared_ptr<surface::buffer> snapshot(int mrt_index);

protected:
	friend device;
	render_target(device* dev, const render_target_info& info, const char* name);
	render_target(device* dev, void* swap_chain, const surface::format& depth_stencil_format, const char* name);

private:
	std::array<texture*, max_num_mrts> textures;
	std::array<texture*, max_num_mrts> targets;
	texture* depth;
	void* swap_chain;
	device* dev;
	render_target_info info;

	void name_resources(const char* name);
	void clear_resources();
};
#endif

	} // namespace gpu
} // namespace ouro

#endif
