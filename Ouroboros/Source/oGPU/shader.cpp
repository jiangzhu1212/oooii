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

#include <oGPU/shader.h>
#include <oCore/windows/win_util.h>
#include "d3d_compile.h"
#include "d3d_debug.h"
#include "d3d_shader.h"
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

oDEFINE_TO_STRING(gpu::stage::value)
oDEFINE_FROM_STRING(gpu::stage::value, gpu::stage::count)

	namespace gpu {

Device* get_device(device* dev);

vertex_shader* make_vertex_shader(device* dev, const void* bytecode, const char* debug_name) { return (vertex_shader*)d3d::make_vertex_shader(get_device(dev), bytecode, debug_name); }
hull_shader* make_hull_shader(device* dev, const void* bytecode, const char* debug_name) { return (hull_shader*)d3d::make_hull_shader(get_device(dev), bytecode, debug_name); }
domain_shader* make_domain_shader(device* dev, const void* bytecode, const char* debug_name) { return (domain_shader*)d3d::make_domain_shader(get_device(dev), bytecode, debug_name); }
geometry_shader* make_geometry_shader(device* dev, const void* bytecode, const char* debug_name) { return (geometry_shader*)d3d::make_geometry_shader(get_device(dev), bytecode, debug_name); }
pixel_shader* make_pixel_shader(device* dev, const void* bytecode, const char* debug_name) { return (pixel_shader*)d3d::make_pixel_shader(get_device(dev), bytecode, debug_name); }
compute_shader* make_compute_shader(device* dev, const void* bytecode, const char* debug_name) { return (compute_shader*)d3d::make_compute_shader(get_device(dev), bytecode, debug_name); }

void unmake_shader(shader* s)
{
	if (s)
		((DeviceChild*)s)->Release();
}

scoped_allocation compile_shader(const char* _IncludePaths
	, const char* _Defines
	, const char* _ShaderSourcePath
	, const stage::value& _Stage
	, const char* _EntryPoint
	, const char* _ShaderSource
	, const allocator& _Allocator)
{
	std::string cmdline;
	cmdline.reserve(2048);
	cmdline = "/O3 /T ";
	cmdline += shader_profile(D3D_FEATURE_LEVEL_11_0, _Stage);
	cmdline += " /E ";
	cmdline += _EntryPoint;

	// Add defines
	cmdline += " /D oHLSL ";
	if (_Defines)
	{
		char* ctx = nullptr;
		std::string defs(_Defines);
		char* def = strtok_r((char*)defs.c_str(), ";", &ctx);
		while (def)
		{
			cmdline += " /D ";
			cmdline += def;
			def = strtok_r(nullptr, ";", &ctx);
		}
	}
	
	// Add includes
	if (_IncludePaths)
	{
		char* ctx = nullptr;
		std::string incs(_IncludePaths);
		char* inc = strtok_r((char*)incs.c_str(), ";", &ctx);
		while (inc)
		{
			cmdline += " /I ";
			cmdline += inc;
			inc = strtok_r(nullptr, ";", &ctx);
		}
	}

	return d3d::compile_shader(cmdline.c_str(), _ShaderSourcePath, _ShaderSource, _Allocator);
}

	} // namespace gpu
} // namespace ouro
