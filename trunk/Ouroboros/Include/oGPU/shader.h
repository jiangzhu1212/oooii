// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_shader_h
#define oGPU_shader_h

#include <oMemory/allocate.h>
#include <oBase/gpu_api.h>
#include <oGPU/vertex_layout.h>

namespace ouro { namespace gpu {

static const uint3 max_dispatches = uint3(65535, 65535, 65535);
static const uint3 max_num_group_threads = uint3(1024, 1024, 64);

class device;
class command_list;
class raw_buffer;

namespace stage
{ enum value : uchar {
	
	vertex,
	hull,
	domain,
	geometry,
	pixel,
	compute,

	count,

};}

namespace stage_flag
{ enum value : uchar {
	
	vertex = 1<<0,
	hull = 1<<1,
	domain = 1<<2,
	geometry = 1<<3,
	pixel = 1<<4,
	compute = 1<<5,

	graphics = vertex|hull|domain|geometry|pixel,

	count,

};}

class shader
{
public:
	shader() : sh(nullptr) {}
	~shader() { deinitialize(); }

	void deinitialize();
	char* name(char* dst, size_t dst_size) const;
	inline void* get_shader() const { return sh; }
protected:
	void* sh;
};

class vertex_shader : public shader
{
public:
	static const stage::value stage = stage::vertex;
	vertex_shader() {}
	vertex_shader(const char* name, device& dev, const void* bytecode) { initialize(name, dev, bytecode); }
	~vertex_shader() { deinitialize(); }
	void initialize(const char* name, device& dev, const void* bytecode);
	void set(command_list& cl) const;
	void clear(command_list& cl) const;
};

class hull_shader : public shader
{
public:
	static const stage::value stage = stage::hull;
	hull_shader() {}
	hull_shader(const char* name, device& dev, const void* bytecode) { initialize(name, dev, bytecode); }
	~hull_shader() { deinitialize(); }
	void initialize(const char* name, device& dev, const void* bytecode);
	void set(command_list& cl) const;
	void clear(command_list& cl) const;
};

class domain_shader : public shader
{
public:
	static const stage::value stage = stage::domain;
	domain_shader() {}
	domain_shader(const char* name, device& dev, const void* bytecode) { initialize(name, dev, bytecode); }
	~domain_shader() { deinitialize(); }
	void initialize(const char* name, device& dev, const void* bytecode);
	void set(command_list& cl) const;
	void clear(command_list& cl) const;
};

class geometry_shader : public shader
{
public:
	static const stage::value stage = stage::geometry;
	geometry_shader() {}
	geometry_shader(const char* name, device& dev, const void* bytecode) { initialize(name, dev, bytecode); }
	~geometry_shader() { deinitialize(); }
	void initialize(const char* name, device& dev, const void* bytecode);
	void set(command_list& cl) const;
	void clear(command_list& cl) const;
};

class pixel_shader : public shader
{
public:
	static const stage::value stage = stage::pixel;
	pixel_shader() {}
	pixel_shader(const char* name, device& dev, const void* bytecode) { initialize(name, dev, bytecode); }
	~pixel_shader() { deinitialize(); }
	void initialize(const char* name, device& dev, const void* bytecode);
	void set(command_list& cl) const;
	void clear(command_list& cl) const;
};

class compute_shader : public shader
{
public:
	static const stage::value stage = stage::compute;
	compute_shader() {}
	compute_shader(const char* name, device& dev, const void* bytecode) { initialize(name, dev, bytecode); }
	~compute_shader() { deinitialize(); }
	void initialize(const char* name, device& dev, const void* bytecode);
	void dispatch(command_list& cl, const uint3& dispatch_thread_count) const;
	void dispatch(command_list& cl, raw_buffer* dispatch_thread_counts, uint offset_in_uints) const;
	void clear(command_list& cl) const;
};

// Compiles a shader and returns the binary byte code.
//  include_paths: a semi-colon delimited list of include paths to search.
//  defines: a semi-colon delimited list of defines. 
//  shader_source_path: the full path to the shader source for resolving local includes
//  entry_point: the top-level shader function to compile.
scoped_allocation compile_shader(const char* include_paths
	, const char* defines
	, const char* shader_source_path
	, const stage::value& type
	, const char* entry_point
	, const char* shader_source
	, const allocator& alloc = default_allocator);

}}

#endif
