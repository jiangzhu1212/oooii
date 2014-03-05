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
// A very simple mesh container that mainly provides very simple creation
// and drawing, mostly used for unit testing and samples.
#pragma once
#ifndef oGPU_util_mesh_h
#define oGPU_util_mesh_h

#include <oGPU/oGPU.h>

#include <oBasis/oGeometry.h>
#include <oMesh/obj.h>

namespace ouro {
	namespace gpu {

class util_mesh
{
public:
	static std::shared_ptr<util_mesh> make(device* _pDevice, const char* _Name, const mesh::info& _Info);

	static std::shared_ptr<util_mesh> make(device* _pDevice, const char* _Name
		, const mesh::layout_array& _VertexLayouts, const oGeometry* _pGeometry);

	virtual mesh::info get_info() const = 0;
	virtual const buffer* index_buffer() const = 0;
	virtual buffer* index_buffer() = 0;
	virtual const buffer* vertex_buffer(uint _Index) const = 0;
	virtual buffer* vertex_buffer(uint _Index) = 0;
	inline const buffer* vertex_buffer(const mesh::usage::value& _Usage) const { return vertex_buffer((uint)_Usage); }
	inline buffer* vertex_buffer(const mesh::usage::value& _Usage) { return vertex_buffer((uint)_Usage); }
	virtual void vertex_buffers(const buffer* _Buffers[mesh::usage::count]) = 0;
	virtual void draw(command_list* _pCommandList) = 0;
	inline void draw(std::shared_ptr<command_list>& _pCommandList) { draw(_pCommandList.get()); }
};

// Creates a very simple front-facing triangle that can be rendered with all-
// identify world, view, projection matrices. This is useful for very simple 
// tests and first bring-up.
std::shared_ptr<util_mesh> make_first_triangle(device* _pDevice);
inline std::shared_ptr<util_mesh> make_first_triangle(std::shared_ptr<device>& _pDevice) { return make_first_triangle(_pDevice.get()); }

// Creates a very simple unit cube. This is useful for bringing up world, view,
// projection transforms quickly.
std::shared_ptr<util_mesh> make_first_cube(device* _pDevice);
inline std::shared_ptr<util_mesh> make_first_cube(std::shared_ptr<device>& _pDevice) { return make_first_cube(_pDevice.get()); }

	} // namespace gpu
} // namespace ouro

#endif
