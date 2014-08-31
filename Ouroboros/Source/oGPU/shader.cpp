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
#include <oGPU/shader.h>
#include <oGPU/raw_buffer.h>
#include <oCore/windows/win_util.h>
#include "d3d_compile.h"
#include "d3d_debug.h"
#include "d3d_util.h"
#include <string>

using namespace ouro::gpu::d3d;

namespace ouro { 
	
const char* as_string(const gpu::stage::value& _Value)
{
	switch (_Value)
	{
		case gpu::stage::vertex: return "vertex";
		case gpu::stage::hull: return "hull";
		case gpu::stage::domain: return "domain";
		case gpu::stage::geometry: return "geometry";
		case gpu::stage::pixel: return "pixel";
		default: break;
	}
	return "?";
}

oDEFINE_ENUM_TO_STRING(gpu::stage::value)
oDEFINE_ENUM_FROM_STRING(gpu::stage::value)
	
namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void shader::deinitialize()
{
	oSAFE_RELEASEV(sh);
}

char* shader::name(char* dst, size_t dst_size) const
{
	return debug_name(dst, dst_size, (DeviceChild*)sh);
}

#define INIT_SH(type, d3d_type, d3d_short_type) \
	void type::initialize(const char* name, device& dev, const void* bytecode) \
	{	deinitialize(); \
		if (!bytecode) return; \
		oV(get_device(dev)->Create##d3d_type(bytecode, bytecode_size(bytecode), nullptr, (d3d_type**)&sh)); \
		debug_name((DeviceChild*)sh, name); \
	}

#define SET_SH(type, d3d_type, d3d_short_type) \
	void type::set(command_list& cl) const { get_dc(cl)->d3d_short_type##SetShader((d3d_type*)sh, nullptr, 0); }
#define CLEAR_SH(type, d3d_type, d3d_short_type) \
	void type::clear(command_list& cl) const { get_dc(cl)->d3d_short_type##SetShader(nullptr, nullptr, 0); }

#define DEFINE_SH(type, d3d_type, d3d_short_type) INIT_SH(type, d3d_type, d3d_short_type) SET_SH(type, d3d_type, d3d_short_type) CLEAR_SH(type, d3d_type, d3d_short_type)
	
DEFINE_SH(vertex_shader, VertexShader, VS)
DEFINE_SH(hull_shader, HullShader, HS)
DEFINE_SH(domain_shader, DomainShader, DS)
DEFINE_SH(geometry_shader, GeometryShader, GS)
DEFINE_SH(pixel_shader, PixelShader, PS)

INIT_SH(compute_shader, ComputeShader, CS) CLEAR_SH(compute_shader, ComputeShader, CS)

void compute_shader::dispatch(command_list& cl, const uint3& dispatch_thread_count) const
{
	DeviceContext* dc = get_dc(cl);
	dc->CSSetShader((ComputeShader*)sh, nullptr, 0);
	dc->Dispatch(dispatch_thread_count.x, dispatch_thread_count.y, dispatch_thread_count.z);
}

void compute_shader::dispatch(command_list& cl, raw_buffer* dispatch_thread_counts, uint offset_in_uints) const
{
	intrusive_ptr<Buffer> b;
	((View*)dispatch_thread_counts->get_target())->GetResource((Resource**)&b);
	DeviceContext* dc = get_dc(cl);
	dc->CSSetShader((ComputeShader*)sh, nullptr, 0);
	dc->DispatchIndirect(b, offset_in_uints);
}

scoped_allocation compile_shader(const char* include_paths
	, const char* defines
	, const char* shader_source_path
	, const stage::value& type
	, const char* entry_point
	, const char* shader_source
	, const allocator& alloc)
{
	std::string cmdline;
	cmdline.reserve(2048);
	cmdline = "/O3 /T ";
	cmdline += shader_profile(D3D_FEATURE_LEVEL_11_0, type);
	cmdline += " /E ";
	cmdline += entry_point;

	// Add defines
	cmdline += " /D oHLSL ";
	if (defines)
	{
		char* ctx = nullptr;
		std::string defs(defines);
		char* def = strtok_r((char*)defs.c_str(), ";", &ctx);
		while (def)
		{
			cmdline += " /D ";
			cmdline += def;
			def = strtok_r(nullptr, ";", &ctx);
		}
	}
	
	// Add includes
	if (include_paths)
	{
		char* ctx = nullptr;
		std::string incs(include_paths);
		char* inc = strtok_r((char*)incs.c_str(), ";", &ctx);
		while (inc)
		{
			cmdline += " /I ";
			cmdline += inc;
			inc = strtok_r(nullptr, ";", &ctx);
		}
	}

	return d3d::compile_shader(cmdline.c_str(), shader_source_path, shader_source, alloc);
}

}}
