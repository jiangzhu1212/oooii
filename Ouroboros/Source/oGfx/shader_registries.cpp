// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGfx/shader_registries.h>
#include <oGfx/oGfxShaders.h>

namespace ouro { namespace gfx {

void vs_registry::initialize(gpu::device& dev)
{
	static unsigned int kCapacity = 30;

	base_type::initialize(kCapacity, dev
		, (void*)gpu::intrinsic::byte_code(gpu::intrinsic::vertex_shader::pass_through_pos)
		, (void*)gpu::intrinsic::byte_code(gpu::intrinsic::vertex_shader::pass_through_pos)
		, (void*)gpu::intrinsic::byte_code(gpu::intrinsic::vertex_shader::pass_through_pos));

	for (int i = 0; i < gpu::intrinsic::vertex_shader::count; i++)
	{
		gpu::intrinsic::vertex_shader::value vs = gpu::intrinsic::vertex_shader::value(i);
		scoped_allocation bytecode((void*)gpu::intrinsic::byte_code(vs), 1, noop_deallocate);
		r.make(i, as_string(vs), bytecode);
	}

	r.flush();
}

void vs_registry::set(gpu::command_list& cl, const gpu::intrinsic::vertex_shader::value& shader) const
{
	gpu::vertex_shader* s = (gpu::vertex_shader*)r.get(shader);
	s->set(cl);
}

void ps_registry::initialize(gpu::device& dev)
{
	static unsigned int kCapacity = 30;

	base_type::initialize(kCapacity, dev
		, (void*)gpu::intrinsic::byte_code(gpu::intrinsic::pixel_shader::yellow)
		, (void*)gpu::intrinsic::byte_code(gpu::intrinsic::pixel_shader::red)
		, (void*)gpu::intrinsic::byte_code(gpu::intrinsic::pixel_shader::white));

	for (int i = 0; i < gpu::intrinsic::pixel_shader::count; i++)
	{
		gpu::intrinsic::pixel_shader::value ps = gpu::intrinsic::pixel_shader::value(i);
		scoped_allocation bytecode((void*)gpu::intrinsic::byte_code(ps), 1, noop_deallocate);
		r.make(i, as_string(ps), bytecode);
	}

	r.flush();
}

void ps_registry::set(gpu::command_list& cl, const gpu::intrinsic::pixel_shader::value& shader) const
{
	gpu::pixel_shader* s = (gpu::pixel_shader*)r.get(shader);
	s->set(cl);
}

}}
