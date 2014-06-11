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
#include <oMesh/mesh.h>

namespace ouro {
	namespace gpu {

static const uint max_vertex_elements = 16;
static const uint3 max_dispatches = uint3(65535, 65535, 65535);
static const uint3 max_num_group_threads = uint3(1024, 1024, 64);

class device;
class vertex_layout;
class shader {};
class vertex_shader : shader {};
class hull_shader : shader {};
class domain_shader : shader {};
class geometry_shader : shader {};
class pixel_shader : shader {};
class compute_shader : shader {};

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

vertex_layout* make_vertex_layout(device* dev, mesh::layout_array& layout, const void* vs_bytecode, const char* debug_name = "");
vertex_shader* make_vertex_shader(device* dev, const void* bytecode, const char* debug_name = "");
hull_shader* make_hull_shader(device* dev, const void* bytecode, const char* debug_name = "");
domain_shader* make_domain_shader(device* dev, const void* bytecode, const char* debug_name = "");
geometry_shader* make_geometry_shader(device* dev, const void* bytecode, const char* debug_name = "");
pixel_shader* make_pixel_shader(device* dev, const void* bytecode, const char* debug_name = "");
compute_shader* make_compute_shader(device* dev, const void* bytecode, const char* debug_name = "");

void unmake_vertex_layout(vertex_layout* layout);
void unmake_shader(shader* s);

template<typename shaderT> struct delete_shader { void operator()(shaderT* s) const { unmake_shader((shader*)s); } };
typedef std::unique_ptr<vertex_shader, delete_shader<vertex_shader>> unique_vertex_shader_ptr;
typedef std::unique_ptr<hull_shader, delete_shader<hull_shader>> unique_hull_shader_ptr;
typedef std::unique_ptr<domain_shader, delete_shader<domain_shader>> unique_domain_shader_ptr;
typedef std::unique_ptr<geometry_shader, delete_shader<geometry_shader>> unique_geometry_shader_ptr;
typedef std::unique_ptr<pixel_shader, delete_shader<pixel_shader>> unique_pixel_shader_ptr;
typedef std::unique_ptr<compute_shader, delete_shader<compute_shader>> unique_compute_shader_ptr;

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
