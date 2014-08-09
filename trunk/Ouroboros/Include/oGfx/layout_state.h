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
// Manages vertex_layouts so they can be accessed by enum.
#ifndef oGfx_layout_state_h
#define oGfx_layout_state_h

#include <oGPU/shaders.h>
#include <oGPU/vertex_layout.h>

namespace ouro { namespace gfx {

class layout_state
{
public:
	layout_state() {}
	~layout_state() { deinitialize(); }

	void initialize(const char* name, gpu::device& dev);
	void deinitialize();

	inline void set(gpu::command_list& cl, const gpu::intrinsic::vertex_layout::value& input, const mesh::primitive_type::value& prim_type) const { layouts[input].set(cl, prim_type); }

private:
	std::array<gpu::vertex_layout, gpu::intrinsic::vertex_layout::count> layouts;
};

}}

#endif
