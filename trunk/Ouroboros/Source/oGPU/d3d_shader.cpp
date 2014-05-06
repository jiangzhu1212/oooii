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
#include "d3d_shader.h"
#include "d3d_debug.h"
#include <oBase/macros.h>
#include <oCore/windows/win_util.h>
#include <d3d11.h>

namespace ouro {
	namespace gpu {
		namespace d3d {

#define MAKE_SH(type, fn) type* fn(Device* dev, const void* bytecode, const char* name) { type* sh = nullptr; if (!bytecode) return nullptr; if (FAILED(dev->Create##type(bytecode, bytecode_size(bytecode), nullptr, &sh))) return nullptr; if (name) debug_name(sh, name); return sh; }

MAKE_SH(VertexShader, make_vertex_shader)
MAKE_SH(HullShader, make_hull_shader)
MAKE_SH(DomainShader, make_domain_shader)
MAKE_SH(GeometryShader, make_geometry_shader)
MAKE_SH(PixelShader, make_pixel_shader)
MAKE_SH(ComputeShader, make_compute_shader)

// Return the size stored inside D3D11-era bytecode. This can be used anywhere bytecode size is expected.
unsigned int bytecode_size(const void* bytecode)
{
	// Discovered empirically
	return bytecode ? ((const unsigned int*)bytecode)[6] : 0;
}

		} // namespace d3d
	} // namespace gpu
} // namespace ouro
