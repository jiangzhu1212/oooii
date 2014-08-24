/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#ifndef oGPU_vertex_layout_h
#define oGPU_vertex_layout_h

#include <oMesh/mesh.h>

namespace ouro {
	namespace gpu {

static const uint max_vertex_elements = 16;

class device;
class command_list;

class vertex_layout
{
public:
	vertex_layout() : layout(nullptr) {}
	vertex_layout(const char* name, device& dev, const mesh::element_array& elements, const void* vs_bytecode) : layout(nullptr) { initialize(name, dev, elements, vs_bytecode); }
	~vertex_layout() { deinitialize(); }
	void initialize(const char* name, device& dev, const mesh::element_array& elements, const void* vs_bytecode);
	void deinitialize();
	void set(command_list& cl, const mesh::primitive_type::value& prim_type) const;
	void* get() const { return layout; }
private:
	void* layout;
};

}}

#endif
