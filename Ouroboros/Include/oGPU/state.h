/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
 // defines enumerated graphics state
#pragma once
#ifndef oGPU_state_h
#define oGPU_state_h

#include <oBase/macros.h>

namespace ouro {
	namespace gpu {

static const uint max_num_samplers = 16;

namespace sampler_state
{ oDECLARE_SMALL_ENUM(value, uchar) {

  point_clamp,
  point_wrap,
  linear_clamp,
  linear_wrap,
  aniso_clamp,
  aniso_wrap,

	count,

};}

namespace rasterizer_state
{ oDECLARE_SMALL_ENUM(value, uchar) {

	// Front-facing is clockwise winding order. Back-facing is counter-clockwise.

	front_face, // Draws all faces whose normal points towards the viewer
	back_face,  // Draws all faces whose normal points away from the viewer
	two_sided, // Draws all faces
	front_wireframe, // Draws the borders of all faces whose normal points towards the viewer
	back_wireframe,  // Draws the borders of all faces whose normal points away from the viewer
	two_sided_wireframe, // Draws the borders of all faces

	count,

};}

namespace depth_stencil_state
{ oDECLARE_SMALL_ENUM(value, uchar) {

	// No depth or stencil operation.
	none,

	// normal z-buffering mode where if occluded or same value (<= less_equal 
	// comparison), exit else render and write new Z value. No stencil operation.
	test_and_write,
	
	// test depth only using same method as test-and-write but do not write. No 
	// stencil operation.
	test,
	
	count,

};}

namespace blend_state
{ oDECLARE_SMALL_ENUM(value, uchar) {

	// Blend mode math from http://en.wikipedia.org/wiki/Blend_modes

	opaque, // Output.rgba = Source.rgba
	alpha_test, // Same as opaque, test is done in user code
	accumulate, // Output.rgba = Source.rgba + Destination.rgba
	additive, // Output.rgb = Source.rgb * Source.a + Destination.rgb
	multiply, // Output.rgb = Source.rgb * Destination.rgb
	screen, // Output.rgb = Source.rgb * (1 - Destination.rgb) + Destination.rgb (as reduced from webpage's 255 - [((255 - Src)*(255 - Dst))/255])
	translucent, // Output.rgb = Source.rgb * Source.a + Destination.rgb * (1 - Source.a)
	min_, // Output.rgba = min(Source.rgba, Destination.rgba)
	max_, // Output.rgba = max(Source.rgba, Destination.rgba)

	count,

};}

	} // namespace gpu
} // namespace ouro

#endif
