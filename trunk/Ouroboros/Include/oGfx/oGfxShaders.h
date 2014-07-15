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
// Encapsulate compiled shader code into a C++-accessible form.

#ifndef oGfxShaders_h
#define oGfxShaders_h

#include <oMesh/mesh.h>

namespace ouro {
	namespace gfx {

namespace vertex_input
{	enum value : uchar {
	
	pos,
	pos_color,
	pos_uv,
	pos_uvw,

	count,

};}

namespace vertex_shader
{	enum value : uchar {

	pass_through_pos, // passes POSITION unmodified
	pass_through_pos_color,
	pass_through_pos_uv,
	pass_through_pos_uvw,
	
	texture1d,
	texture1darray,
	texture2d,
	texture2darray,
	texture3d,
	texturecube,
	texturecubearray,

	test_buffer,
	test_transform,
	test_instanced,

	count,

};}

namespace hull_shader
{	enum value : uchar {

	null,
	count,

};}

namespace domain_shader
{	enum value : uchar {

	null,
	count,

};}

namespace geometry_shader
{	enum value : uchar {

	vertex_normals,
	vertex_tangents,

	count,

};}

namespace pixel_shader
{	enum value : uchar {

	black,
	white,
	red,
	green,
	blue,
	yellow,
	magenta,
	cyan,

	texture1d,
	texture1darray,
	texture2d,
	texture2darray,
	texture3d,
	texturecube,
	texturecubearray,
	
	vertex_color,

	test_buffer,

	count,

};}

namespace compute_shader
{	enum value : uchar {

	null,
	count,

};}

// returns the elements of input
mesh::element_array elements(const vertex_input::value& input);

// returns the vertex shader byte code with the same input
// signature as input.
const void* vs_byte_code(const vertex_input::value& input);

// returns the buffer of bytecode compiled during executable compilation time
// (not runtime compilation)
const void* byte_code(const vertex_shader::value& shader);
const void* byte_code(const hull_shader::value& shader);
const void* byte_code(const domain_shader::value& shader);
const void* byte_code(const geometry_shader::value& shader);
const void* byte_code(const pixel_shader::value& shader);
const void* byte_code(const compute_shader::value& shader);

	} // namespace gfx
} // namespace ouro

#endif
