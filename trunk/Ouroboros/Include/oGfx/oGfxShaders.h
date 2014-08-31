// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oGfxShaders_h
#define oGfxShaders_h

// Encapsulate compiled shader code into a C++-accessible form.

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
