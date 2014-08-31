// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_depth_stencil_state_h
#define oGPU_depth_stencil_state_h

#include <oBase/macros.h>
#include <array>

namespace ouro { namespace gpu {

class device;
class command_list;

class depth_stencil_state
{
public:
	enum value : uchar
	{
		// No depth or stencil operation.
		none,

		// normal z-buffering mode where if occluded or same value (<= less_equal 
		// comparison), exit else render and write new Z value. No stencil operation.
		test_and_write,
	
		// test depth only using same method as test-and-write but do not write. No 
		// stencil operation.
		test,
	
		count,
	};

	depth_stencil_state() { states.fill(nullptr); }
	~depth_stencil_state() { deinitialize(); }

	void initialize(const char* name, device& dev);
	void deinitialize();

	void set(command_list& cl, const value& state, uint stencil_ref = 0);

private:
	std::array<void*, count> states;
};

}}

#endif
