// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_raw_buffer_h
#define oGPU_raw_buffer_h

#include <oGPU/compute_target.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class raw_buffer : public compute_target
{
public:
	~raw_buffer() { deinitialize(); }
	void initialize(const char* name, device& dev, uint num_uints, const uint* src = nullptr);
	uint num_uints() const;
	void update(command_list& cl, uint offset_in_uints, uint num_uints, const void* src);
};
	
}}

#endif
