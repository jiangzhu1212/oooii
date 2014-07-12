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
#include <oGfx/oGfxShaders.h>

typedef unsigned char BYTE;

#include <VSPositionPassThrough.h>

#include <PSBlack.h>
#include <PSWhite.h>
#include <PSRed.h>
#include <PSGreen.h>
#include <PSBlue.h>
#include <PSYellow.h>
#include <PSMagenta.h>
#include <PSCyan.h>

namespace ouro {
	namespace gfx {

const void* byte_code(const vertex_shader::value& shader)
{
	static const void* sShaders[] = 
	{
		VSPositionPassThrough,
	};
	static_assert(oCOUNTOF(sShaders) == vertex_shader::count, "array mismatch");
	return sShaders[shader];
}

const void* byte_code(const hull_shader::value& shader)
{
	static const void* sShaders[] = 
	{
		nullptr,
	};
	static_assert(oCOUNTOF(sShaders) == hull_shader::count, "array mismatch");
	return sShaders[shader];
}

const void* byte_code(const domain_shader::value& shader)
{
	static const void* sShaders[] = 
	{
		nullptr,
	};
	static_assert(oCOUNTOF(sShaders) == domain_shader::count, "array mismatch");
	return sShaders[shader];
}

const void* byte_code(const geometry_shader::value& shader)
{
	static const void* sShaders[] = 
	{
		nullptr,
	};
	static_assert(oCOUNTOF(sShaders) == geometry_shader::count, "array mismatch");
	return sShaders[shader];
}

const void* byte_code(const pixel_shader::value& shader)
{
	static const void* sShaders[] = 
	{
		PSBlack,
		PSWhite,
		PSRed,
		PSGreen,
		PSBlue,
		PSYellow,
		PSMagenta,
		PSCyan,
	};
	static_assert(oCOUNTOF(sShaders) == pixel_shader::count, "array mismatch");
	return sShaders[shader];
}

const void* byte_code(const compute_shader::value& shader)
{
	static const void* sShaders[] = 
	{
		nullptr,
	};
	static_assert(oCOUNTOF(sShaders) == compute_shader::count, "array mismatch");
	return sShaders[shader];
}

	} // namespace gfx

const char* as_string(const gfx::vertex_shader::value& shader)
{
	const char* sNames[] = 
	{
		"PositionPassThrough",
	};
	static_assert(oCOUNTOF(sNames) == gfx::vertex_shader::count, "array mismatch");
	return sNames[shader];
}

const char* as_string(const gfx::pixel_shader::value& shader)
{
	const char* sNames[] = 
	{
		"black",
		"white",
		"red",
		"green",
		"blue",
		"yellow",
		"magenta",
		"cyan",
	};
	static_assert(oCOUNTOF(sNames) == gfx::pixel_shader::count, "array mismatch");
	return sNames[shader];
}

} // namespace ouro
