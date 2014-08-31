// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_texture2d_h
#define oGPU_texture2d_h

#include <oGPU/resource.h>

namespace ouro { namespace gpu {

class device;

class texture2d : public resource
{
public:
	~texture2d() { deinitialize(); }
	void initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips);
	void initialize(const char* name, device& dev, const surface::image& src, bool mips);
	void deinitialize();
	uint2 dimensions() const;
	uint array_size() const;
};
	
}}

#endif
