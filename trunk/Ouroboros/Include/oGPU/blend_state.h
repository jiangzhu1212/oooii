// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_blend_state_h
#define oGPU_blend_state_h

#include <array>

namespace ouro { namespace gpu {

class device;
class command_list;

class blend_state
{
public:
	enum value : uchar
	{
		// Blend mode math from http://en.wikipedia.org/wiki/Blend_modes

		opaque, // Output.rgba = Source.rgba
		alpha_test, // Same as opaque, test is done in user code
		accumulate, // Output.rgba = Source.rgba + Destination.rgba
		additive, // Output.rgb = Source.rgb * Source.a + Destination.rgb
		multiply, // Output.rgb = Source.rgb * Destination.rgb
		screen, // Output.rgb = Source.rgb * (1 - Destination.rgb) + Destination.rgb (as reduced from webpage's 255 - [((255 - Src)*(255 - Dst))/255])
		translucent, // Output.rgb = Source.rgb * Source.a + Destination.rgb * (1 - Source.a)
		minimum, // Output.rgba = min(Source.rgba, Destination.rgba)
		maximum, // Output.rgba = max(Source.rgba, Destination.rgba)

		count,
	};

	blend_state() { states.fill(nullptr); }
	~blend_state() { deinitialize(); }

	void initialize(const char* name, device& dev);
	void deinitialize();

	void set(command_list& cl, const value& state);
	void set(command_list& cl, const value& state, const float blend_factor[4], uint sample_mask = 0xffffffff);

private:
	std::array<void*, count> states;
};

}}

#endif
