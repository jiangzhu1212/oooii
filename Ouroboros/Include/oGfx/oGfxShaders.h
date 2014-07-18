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

namespace gfx_vl
{	enum value : uchar {
	
	null,
	count,

};}

namespace gfx_vs
{	enum value : uchar {

	null,
	count,

};}

namespace gfx_hs
{	enum value : uchar {

	null,
	count,

};}

namespace gfx_ds
{	enum value : uchar {

	null,
	count,

};}

namespace gfx_gs
{	enum value : uchar {

	null,
	count,

};}

namespace gfx_ps
{	enum value : uchar {

	null,
	count,

};}

namespace gfx_cs
{	enum value : uchar {

	null,
	count,

};}

// returns the elements of input
mesh::element_array elements(const gfx_vl::value& input);

// returns the vertex shader byte code with the same input
// signature as input.
const void* vs_byte_code(const gfx_vl::value& input);

// returns the buffer of bytecode compiled during executable compilation time
// (not runtime compilation)
const void* byte_code(const gfx_vs::value& shader);
const void* byte_code(const gfx_hs::value& shader);
const void* byte_code(const gfx_ds::value& shader);
const void* byte_code(const gfx_gs::value& shader);
const void* byte_code(const gfx_ps::value& shader);
const void* byte_code(const gfx_cs::value& shader);

	} // namespace gfx
} // namespace ouro

#endif
