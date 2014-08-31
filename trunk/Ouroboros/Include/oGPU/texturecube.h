// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_texturecube_h
#define oGPU_texturecube_h

#include <oGPU/resource.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class texturecube : public resource
{
public:
	~texturecube() { deinitialize(); }
	void initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size_in_num_cubes, bool mips);
	void initialize(const char* name, device& dev, const surface::image& src, bool mips);
	void deinitialize();
	uint2 dimensions() const;
	uint array_size() const;
};
	
}}

#endif
