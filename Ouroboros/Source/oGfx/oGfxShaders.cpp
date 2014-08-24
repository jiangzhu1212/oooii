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
#include <oGfx/oGfxShaders.h>

typedef unsigned char BYTE;

struct SHADER
{
	const char* name;
	const void* byte_code;
};

#define oSH(x) { #x, x },
#define oBYTE_CODE(type) const void* byte_code(const type::value& shader) { static_assert(oCOUNTOF(s_##type) == type::count, "array mismatch"); return s_##type[shader].byte_code; }
#define oAS_STRING(type) const char* as_string(const gfx::type::value& shader) { return gfx::s_##type[shader].name; }

using namespace ouro::mesh;

namespace ouro {
	namespace gfx {

mesh::element_array elements(const gfx_vl::value& input)
{
	element_array e;
	switch (input)
	{
		case gfx_vl::null:
		default:
			break;
	}

	return e;
}

const void* vs_byte_code(const gfx_vl::value& input)
{
	static const gfx_vs::value sVS[] =
	{
		gfx_vs::null,
	};
	static_assert(oCOUNTOF(sVS) == gfx::gfx_vl::count, "array mismatch");
	return byte_code(sVS[input]);
}

static const SHADER s_gfx_vs[] = 
{
	oSH(nullptr)
};

static const SHADER s_gfx_hs[] = 
{
	oSH(nullptr)
};

static const SHADER s_gfx_ds[] = 
{
	oSH(nullptr)
};

static const SHADER s_gfx_gs[] = 
{
	oSH(nullptr)
};

static const SHADER s_gfx_ps[] = 
{
	oSH(nullptr)
};

static const SHADER s_gfx_cs[] = 
{
	oSH(nullptr)
};

oBYTE_CODE(gfx_vs)
oBYTE_CODE(gfx_hs)
oBYTE_CODE(gfx_ds)
oBYTE_CODE(gfx_gs)
oBYTE_CODE(gfx_ps)
oBYTE_CODE(gfx_cs)

	} // namespace gfx

const char* as_string(const gfx::gfx_vl::value& input)
{
	static const char* sNames[] = 
	{
		"null",
	};
	static_assert(oCOUNTOF(sNames) == gfx::gfx_vl::count, "array mismatch");

	return sNames[input];
}

oAS_STRING(gfx_vs)
oAS_STRING(gfx_hs)
oAS_STRING(gfx_ds)
oAS_STRING(gfx_gs)
oAS_STRING(gfx_ps)
oAS_STRING(gfx_cs)

} // namespace ouro
