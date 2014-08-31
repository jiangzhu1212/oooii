// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_texture1d_h
#define oGPU_texture1d_h

#include <oGPU/resource.h>
#include <oSurface/surface.h>
#include <oSurface/image.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class texture1d : public resource
{
public:
	texture1d() {}
	~texture1d() { deinitialize(); }
	void initialize(const char* name, device& dev, surface::format format, uint width, uint array_size, bool mips);
	void initialize(const char* name, device& dev, const surface::image& src, bool mips);
	void deinitialize();
	uint width() const;
	uint array_size() const;
};
	
}}

#endif
