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
#ifndef oGPU_color_target_h
#define oGPU_color_target_h

#include <oGPU/resource.h>
#include <vector>

namespace ouro { namespace gpu {

static const uint max_num_mrts = 8;

class device;
class command_list;
class depth_target;
class window;

class color_target : public resource
{
public:
	~color_target() { deinitialize(); }

	void initialize(const char* name, device* dev, surface::format format, uint width, uint height, uint array_size, bool mips);
	void deinitialize();

	uint2 dimensions() const;
	uint array_size() const;
	void* get_target(uint index = 0) const { return rws[index]; }
	
	// todo: add viewport

	static void set_draw_target(command_list* cl, uint num_colors, color_target* const* colors, depth_target* depth, const viewport& vp = viewport());
	static void set_draw_target(command_list* cl, uint num_colors, color_target* const* colors, const uint* color_indices, depth_target* depth, uint depth_index, const viewport& vp = viewport());

	void set_draw_target(command_list* cl, uint index = 0, depth_target* depth = nullptr, uint depth_index = 0, const viewport& vp = viewport());

	void clear(command_list* cl, const color& c, uint index = 0);
	void generate_mips(command_list* cl);

private:
	std::vector<void*> rws;
};
	
}}

#endif
