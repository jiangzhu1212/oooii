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
// This header is compiled both by HLSL and C++. It describes a oGfx-level 
// policy encapsulation of the available shaders, all of which conform to oGfx-
// level objects and assumption.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxPipelines_h
#define oGfxPipelines_h

#include <oGPU/oGPU.h>

enum oGFX_PIPELINE
{
	// POINTS

	// Render inputs as points and use the geometry shader to display lines 
	// representing per-vertex vectors.
	oGFX_PIPELINE_VERTEX_NORMALS,
	oGFX_PIPELINE_VERTEX_TANGENTS,

	
	// LINES
	
	// Render lines with a per-vertex color
	oGFX_PIPELINE_LINES,

	// Render line strips with a per-vertex color
	oGFX_PIPELINE_LINE_STRIPS,


	// TRIANGLES

	// Uses untransformed position only to render a white color.
	oGFX_PIPELINE_PASS_THROUGH,

	// Renders with a vertex shader that only transforms LS -> SS and uses a null
	// pixels shader.
	oGFX_PIPELINE_RIGID_ZPREPASS,

	// Renders normalized view-space depth directly to SV_Depth. This should not
	// be mixed with other shaders that write regular SV_Depth.
	oGFX_PIPELINE_RIGID_SHADOW,

	// Uses position only to render a white color
	oGFX_PIPELINE_RIGID_WHITE,

	// Use UVW from TEX0 as RGB
	oGFX_PIPELINE_RIGID_TEXCOORD,

	// Colorize world-space vertex normals XYZ from NML0 as RGB
	oGFX_PIPELINE_RIGID_WSVNORMAL,

	// Colorize world-space pixel normals XYZ from NML0 and TAN0 as RGB
	oGFX_PIPELINE_RIGID_WSPNORMAL,

	// Intensity of depth as a distanced in view-space from the eye. 0/black is on 
	// near plane, 1/white is on far plane
	oGFX_PIPELINE_RIGID_VSDEPTH,

	// Renders a procedural grid - good for a catch-all placeholder.
	oGFX_PIPELINE_RIGID_GRID,

	// Renders the object's per-draw ID to a UINT render target. If the target
	// is one channel of a bigger texture, a different pipeline will need to be
	// bound, which stands to reason because more information will need to be 
	// written. This pipeline is intended to use as a stand-alone path for simple
	// picking where only the IDs are relevant to return to CPU-land for 
	// selection as in a scene editor.
	oGFX_PIPELINE_RIGID_OBJECT_ID,

	// Render triangles sampling from a texture from vertex texcoords
	oGFX_PIPELINE_RIGID_TEXTURE_COLOR,

	// Render triangles with only basic shading from a one-light Phong model 
	oGFX_PIPELINE_RIGID_VERTEX_PHONG,

	// Render triangles lit with the oGfx material
	oGFX_PIPELINE_RIGID_MATERIAL,

	// Render triangles lit with the oGfx Hero material (replacing oGFX_PIPELINE_RIGID_MATERIAL)
	oGFX_PIPELINE_RIGID_HERO,
};

ouro::gpu::pipeline_info oGfxGetPipeline(oGFX_PIPELINE _Pipeline);

#endif
