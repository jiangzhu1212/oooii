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
#include <oGfx/oGfxShaderRegistry.h>
#include <oGfx/oGfxShaders.h>

namespace ouro { namespace gfx {

void vs_registry::initialize(gpu::device* dev)
{
	static unsigned int kCapacity = 30;

	base_type::initialize(kCapacity, dev
		, (void*)gfx::byte_code(gfx::vertex_shader::pos_pass_through)
		, (void*)gfx::byte_code(gfx::vertex_shader::pos_pass_through)
		, (void*)gfx::byte_code(gfx::vertex_shader::pos_pass_through));

	for (int i = 0; i < gfx::vertex_shader::count; i++)
	{
		gfx::vertex_shader::value vs = gfx::vertex_shader::value(i);
		scoped_allocation bytecode((void*)gfx::byte_code(vs), 1, noop_deallocate);
		r.make(i, as_string(vs), bytecode);
	}

	r.flush();
}

void ps_registry::initialize(gpu::device* dev)
{
	static unsigned int kCapacity = 30;

	base_type::initialize(kCapacity, dev
		, (void*)gfx::byte_code(gfx::pixel_shader::yellow)
		, (void*)gfx::byte_code(gfx::pixel_shader::red)
		, (void*)gfx::byte_code(gfx::pixel_shader::white));

	for (int i = 0; i < gfx::pixel_shader::count; i++)
	{
		gfx::pixel_shader::value ps = gfx::pixel_shader::value(i);
		scoped_allocation bytecode((void*)gfx::byte_code(ps), 1, noop_deallocate);
		r.make(i, as_string(ps), bytecode);
	}

	r.flush();
}

}}
