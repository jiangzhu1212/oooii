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
#ifndef oGPU_d3d_shader_h
#define oGPU_d3d_shader_h

#include "d3d_types.h"
#include <d3dcommon.h>

namespace ouro {
	namespace gpu {
		namespace d3d {

VertexShader* make_vertex_shader(Device* dev, const void* bytecode, const char* debug_name);
HullShader* make_hull_shader(Device* dev, const void* bytecode, const char* debug_name);
DomainShader* make_domain_shader(Device* dev, const void* bytecode, const char* debug_name);
GeometryShader* make_geometry_shader(Device* dev, const void* bytecode, const char* debug_name);
PixelShader* make_pixel_shader(Device* dev, const void* bytecode, const char* debug_name);
ComputeShader* make_compute_shader(Device* dev, const void* bytecode, const char* debug_name);

// return the size of the specified bytecode
unsigned int bytecode_size(const void* bytecode);

		} // namespace d3d
	} // namespace gpu
} // namespace ouro

#endif
