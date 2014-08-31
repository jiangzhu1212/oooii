// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// base class for buffers writable in a compute shader
#pragma once
#ifndef oGPU_compute_target_h
#define oGPU_compute_target_h

#include <oGPU/resource.h>
#include <oBase/types.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class compute_target : public resource
{
public:
	static const uint preserve_prior_value = ~0u;

	compute_target() : rw(nullptr) {}
	~compute_target() { deinitialize(); }
	void deinitialize();
	void* get_target() const { return rw; }

	void cleari(command_list& cl, const uint4& clear_value);
	void clearf(command_list& cl, const float4& clear_value);

	// NOTE: with the current api RTs and UAVs cannot be bound at the same time - one will kick the other out
	void set_draw_target(command_list& cl, uint slot, uint initial_count = preserve_prior_value) const;
	void set_dispatch_target(command_list& cl, uint slot, uint initial_count = preserve_prior_value) const;

protected:
	void* rw;
};
	
}}

#endif
