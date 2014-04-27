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
#pragma once
#ifndef oGPU_shader_h
#define oGPU_shader_h

#include <oBase/allocate.h>
#include <oBase/macros.h>
#include <oBase/types.h>

namespace ouro {
	namespace gpu {

static const uint max_num_thread_groups_per_dimension = 65535;
static const uint max_num_thread_groups_per_dimension_mask = 0xffff;
static const uint max_num_thread_groups_per_dimension_shift = 16;

oDECLARE_HANDLE(shader);
oDECLARE_DERIVED_HANDLE(shader, vertex_shader);
oDECLARE_DERIVED_HANDLE(shader, hull_shader);
oDECLARE_DERIVED_HANDLE(shader, domain_shader);
oDECLARE_DERIVED_HANDLE(shader, geometry_shader);
oDECLARE_DERIVED_HANDLE(shader, pixel_shader);
oDECLARE_DERIVED_HANDLE(shader, compute_shader);

class device;
class device_context;

namespace shader_type
{ oDECLARE_SMALL_ENUM(value, uchar) {
	
	vertex,
	hull,
	domain,
	geometry,
	pixel,
	compute,

	count,

};}

struct pipeline
{
	pipeline() : vs(nullptr), hs(nullptr), ds(nullptr), gs(nullptr), ps(nullptr) {}

	vertex_shader vs;
	hull_shader hs;
	domain_shader ds;
	geometry_shader gs;
	pixel_shader ps;
};

// Binds the shader for use during subsequent draw calls
void bind_shader(device_context* _pContext, vertex_shader _Shader);
void bind_shader(device_context* _pContext, hull_shader _Shader);
void bind_shader(device_context* _pContext, domain_shader _Shader);
void bind_shader(device_context* _pContext, geometry_shader _Shader);
void bind_shader(device_context* _pContext, pixel_shader _Shader);
void bind_shader(device_context* _pContext, compute_shader _Shader);

inline void bind_shader(device_context* _pContext, const shader_type::value& _Type, shader _Shader)
{	switch (_Type)
	{	case shader_type::vertex: bind_shader(_pContext, (vertex_shader)_Shader); break;
		case shader_type::hull: bind_shader(_pContext, (hull_shader)_Shader); break;
		case shader_type::domain: bind_shader(_pContext, (domain_shader)_Shader); break;
		case shader_type::geometry: bind_shader(_pContext, (geometry_shader)_Shader); break;
		case shader_type::pixel: bind_shader(_pContext, (pixel_shader)_Shader); break;
		case shader_type::compute: bind_shader(_pContext, (compute_shader)_Shader); break;
	};
}

// binds all rasterization shaders at once
inline void bind_shaders(device_context* _pContext, const pipeline& _Pipeline)
{
	const shader* s = (shader*)&_Pipeline;
	bind_shader(_pContext, _Pipeline.vs);
	bind_shader(_pContext, _Pipeline.hs);
	bind_shader(_pContext, _Pipeline.ds);
	bind_shader(_pContext, _Pipeline.gs);
	bind_shader(_pContext, _Pipeline.ps);
}

// Creates a runtime object for the shader described by the specified bytecode.
shader make_shader(device* _pDevice, const shader_type::value& _Type, const void* _pByteCode, const char* _DebugName = nullptr);

// Destroys a shader created with create_shader
void unmake_shader(shader _Shader);

// Compiles a shader and returns the binary byte code. _IncludePaths is a semi-colon 
// delimited list of include paths to search. _Defines is a semi-colon delimited list
// of defines. Also required is the full path to the shader source for resolving local
// includes, the entry point to compile and the source of the shader itself. This will
// use the file system to load dependent headers. The final buffer will be allocated
// using the specified allocator. _EntryPoint must begin with two letters that describe
scoped_allocation compile_shader(const char* _IncludePaths
	, const char* _Defines
	, const char* _ShaderSourcePath
	, const shader_type::value& _Type
	, const char* _EntryPoint
	, const char* _ShaderSource
	, const allocator& _Allocator = default_allocator);

	} // namespace gpu
} // namespace ouro

#endif