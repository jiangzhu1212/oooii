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
#ifndef oGPU_depth_stencil_state_h
#define oGPU_depth_stencil_state_h

#include <oBase/macros.h>
#include <array>

namespace ouro { namespace gpu {

class device;
class command_list;

class depth_stencil_state
{
public:
	enum value : uchar
	{
		// No depth or stencil operation.
		none,

		// normal z-buffering mode where if occluded or same value (<= less_equal 
		// comparison), exit else render and write new Z value. No stencil operation.
		test_and_write,
	
		// test depth only using same method as test-and-write but do not write. No 
		// stencil operation.
		test,
	
		count,
	};

	depth_stencil_state() { states.fill(nullptr); }
	~depth_stencil_state() { deinitialize(); }

	void initialize(const char* name, device& dev);
	void deinitialize();

	void set(command_list& cl, const value& state, uint stencil_ref = 0);

private:
	std::array<void*, count> states;
};

}}

#endif
