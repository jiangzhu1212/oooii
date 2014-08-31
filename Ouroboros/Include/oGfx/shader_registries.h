// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oGfxShaderRegistry_h
#define oGfxShaderRegistry_h

#include <oGfx/shader_registry.h>
#include <oGfx/oGfxShaders.h>
#include <oGPU/shader.h>
#include <oGPU/shaders.h>

namespace ouro { namespace gfx {

class vs_registry : public shader_registry<gpu::vertex_shader>
{
	typedef shader_registry<gpu::vertex_shader> base_type;

public:
	typedef base_type::shader_type shader_type;
	static const gpu::stage::value stage = base_type::stage;
	
	~vs_registry() { deinitialize(); }

	void initialize(gpu::device& dev);

	void set(gpu::command_list& cl, const gpu::intrinsic::vertex_shader::value& shader) const;
};

class ps_registry : public shader_registry<gpu::pixel_shader>
{
	typedef shader_registry<gpu::pixel_shader> base_type;

public:
	typedef base_type::shader_type shader_type;
	static const gpu::stage::value stage = base_type::stage;
	
	~ps_registry() { deinitialize(); }

	void initialize(gpu::device& dev);

	void set(gpu::command_list& cl, const gpu::intrinsic::pixel_shader::value& shader) const;
};

}}

#endif
