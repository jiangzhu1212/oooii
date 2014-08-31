// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_sampler_state_h
#define oGPU_sampler_state_h

#include <oBase/macros.h>
#include <array>

namespace ouro { namespace gpu {

static const uint max_num_samplers = 16;

class device;
class command_list;

class sampler_state
{
public:
	enum value : uchar
	{
		point_clamp,
		point_wrap,
		linear_clamp,
		linear_wrap,
		aniso_clamp,
		aniso_wrap,

		count,
	};

	sampler_state() { states.fill(nullptr); }
	~sampler_state() { deinitialize(); }

	void initialize(const char* name, device& dev);
	void deinitialize();

	void set(command_list& cl, uint slot, uint num_samplers, const value* samplers);
	inline void set(command_list& cl, uint slot, const value& sampler) { set(cl, slot, 1, &sampler); }

private:
	std::array<void*, count> states;
};

}}

#endif
