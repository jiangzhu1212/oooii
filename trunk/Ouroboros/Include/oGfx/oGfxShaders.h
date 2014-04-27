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

#include <oGPU/oGPU.h>

namespace ouro {
	namespace gfx {

namespace vertex_shader
{	enum value {

	pos_pass_through, // passes POSITION unmodified

	count,

};}

namespace hull_shader
{	enum value {

	null,
	count,

};}

namespace domain_shader
{	enum value {

	null,
	count,

};}

namespace geometry_shader
{	enum value {

	null,
	count,

};}

namespace pixel_shader
{	enum value {

	black,
	white,
	red,
	green,
	blue,
	yellow,
	magenta,
	cyan,

	count,

};}

namespace compute_shader
{	enum value {

	null,
	count,

};}

// returns the buffer of bytecode compiled during executable compilation time
// (not runtime compilation)
const void* byte_code(const vertex_shader::value& _Shader);
const void* byte_code(const hull_shader::value& _Shader);
const void* byte_code(const domain_shader::value& _Shader);
const void* byte_code(const geometry_shader::value& _Shader);
const void* byte_code(const pixel_shader::value& _Shader);
const void* byte_code(const compute_shader::value& _Shader);

	} // namespace gfx
} // namespace ouro

#endif
