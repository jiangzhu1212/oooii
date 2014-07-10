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
#ifndef oGPU_sampler_state_h
#define oGPU_sampler_state_h

#include <oBase/macros.h>
#include <array>

namespace ouro { namespace gpu {

static const uint max_num_samplers = 16;

class device;
class command_list;

class sampler_state
{
public:
	enum value : uchar
	{
		point_clamp,
		point_wrap,
		linear_clamp,
		linear_wrap,
		aniso_clamp,
		aniso_wrap,

		count,
	};

	sampler_state() { states.fill(nullptr); }
	~sampler_state() { deinitialize(); }

	void initialize(device* dev);
	void deinitialize();

	void set(command_list* cl, uint slot, uint num_samplers, const value* samplers);
	inline void set(command_list* cl, uint slot, const value& sampler) { set(cl, slot, 1, &sampler); }

private:
	std::array<void*, count> states;
};

}}

#endif
