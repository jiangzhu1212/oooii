// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_depth_target_h
#define oGPU_depth_target_h

#include <oGPU/resource.h>
#include <oSurface/surface.h>
#include <vector>

namespace ouro { namespace gpu {

class device;
class command_list;

class depth_target : public resource
{
public:
	depth_target() {}
	~depth_target() { deinitialize(); }

	operator bool() const { return !!ro; }

	void initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling);
	void deinitialize();

	surface::format format() const;
	uint2 dimensions() const;
	uint array_size() const;
	void* get_target(uint index = 0) const { return rws[index]; }

	void resize(const uint2& dimensions);

	void set(command_list& cl, uint index = 0, const viewport& vp = viewport());

	void clear(command_list& cl, uint index = 0, float depth = 1.0f);
	void clear_stencil(command_list& cl, uint index = 0, uchar stencil = 0);
	void clear_depth_stencil(command_list& cl, uint index = 0, float depth = 1.0f, uchar stencil = 0);
	void generate_mips(command_list& cl);

private:
	std::vector<void*> rws;
	
	void internal_initialize(const char* name, void* dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling);
};
	
}}

#endif
