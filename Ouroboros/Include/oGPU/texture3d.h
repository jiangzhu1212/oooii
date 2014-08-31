// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_texture3d_h
#define oGPU_texture3d_h

#include <oGPU/resource.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class texture3d : public resource
{
public:
	~texture3d() { deinitialize(); }
	void initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint depth, bool mips);
	void initialize(const char* name, device& dev, const surface::image& src, bool mips);
	void deinitialize();
	uint3 dimensions() const;
};
	
}}

#endif
