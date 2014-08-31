// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_rasterizer_state_h
#define oGPU_rasterizer_state_h

#include <oBase/macros.h>
#include <array>

namespace ouro { namespace gpu {

class device;
class command_list;

class rasterizer_state
{
public:
	enum value : uchar
	{
		// Front-facing is clockwise winding order. Back-facing is counter-clockwise.

		front_face, // Draws all faces whose normal points towards the viewer
		back_face,  // Draws all faces whose normal points away from the viewer
		two_sided, // Draws all faces
		front_wireframe, // Draws the borders of all faces whose normal points towards the viewer
		back_wireframe,  // Draws the borders of all faces whose normal points away from the viewer
		two_sided_wireframe, // Draws the borders of all faces

		count,
	};

	rasterizer_state() { states.fill(nullptr); }
	~rasterizer_state() { deinitialize(); }

	void initialize(const char* name, device& dev);
	void deinitialize();

	void set(command_list& cl, const value& state);

private:
	std::array<void*, count> states;
};

}}

#endif
