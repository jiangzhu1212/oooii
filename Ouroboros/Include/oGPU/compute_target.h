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
// base class for buffers writable in a compute shader
#pragma once
#ifndef oGPU_compute_target_h
#define oGPU_compute_target_h

#include <oGPU/resource.h>
#include <oBase/types.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class compute_target : public resource
{
public:
	static const uint preserve_prior_value = ~0u;

	compute_target() : rw(nullptr) {}
	~compute_target() { deinitialize(); }
	void deinitialize();
	void* get_target() const { return rw; }

	void cleari(command_list& cl, const uint4& clear_value);
	void clearf(command_list& cl, const float4& clear_value);

	// NOTE: with the current api RTs and UAVs cannot be bound at the same time - one will kick the other out
	void set_draw_target(command_list& cl, uint slot, uint initial_count = preserve_prior_value) const;
	void set_dispatch_target(command_list& cl, uint slot, uint initial_count = preserve_prior_value) const;

protected:
	void* rw;
};
	
}}

#endif
