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
#ifndef oGPU_depth_target_h
#define oGPU_depth_target_h

#include <oGPU/resource.h>
#include <oSurface/surface.h>
#include <vector>

namespace ouro { namespace gpu {

class device;
class command_list;

class depth_target : public resource
{
public:
	depth_target() {}
	~depth_target() { deinitialize(); }

	operator bool() const { return !!ro; }

	void initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling);
	void deinitialize();

	surface::format format() const;
	uint2 dimensions() const;
	uint array_size() const;
	void* get_target(uint index = 0) const { return rws[index]; }

	void resize(const uint2& dimensions);

	void set(command_list& cl, uint index = 0, const viewport& vp = viewport());

	void clear(command_list& cl, uint index = 0, float depth = 1.0f);
	void clear_stencil(command_list& cl, uint index = 0, uchar stencil = 0);
	void clear_depth_stencil(command_list& cl, uint index = 0, float depth = 1.0f, uchar stencil = 0);
	void generate_mips(command_list& cl);

private:
	std::vector<void*> rws;
	
	void internal_initialize(const char* name, void* dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling);
};
	
}}

#endif
