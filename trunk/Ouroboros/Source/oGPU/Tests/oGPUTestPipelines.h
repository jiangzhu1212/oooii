/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#pragma once
#ifndef oGPUTestPipelines_h
#define oGPUTestPipelines_h

#include <oGPU/oGPU.h>

enum oGPU_TEST_PIPELINE
{
	// Vertex: float3 Positions
	// VS: ScreenSpacePos = float4(LocalSpacePos, 1)
	// PS: Color = white
	oGPU_TEST_PASS_THROUGH, 

	// Vertex: float3 Positions
	// VS: ScreenSpacePos = float4(LocalSpacePos, 1)
	// PS: Color = Vertex Color
	oGPU_TEST_PASS_THROUGH_COLOR,

	// Vertex: float3 Positions
	// VS: ScreenSpacePos = full WVP transformation
	// PS: Color = white
	oGPU_TEST_TRANSFORMED_WHITE,

	// Vertex: float3 Positions, instanced float3 translation, instanced quatf rotation
	// VS: ScreenSpacePos = full WVP transformation from instance data
	// PS: Color = white
	oGPU_TEST_TRANSFORMED_WHITE_INSTANCED,

	// Vertex: NONE
	// VS: ScreenSpacePos = full WVP transformation
	// PS: AppendBuffer
	oGPU_TEST_BUFFER,

	// Vertex: float3 Positions, float Texcoords
	// VS: ScreenSpacePos = full WVP transformation
	// PS: Color = texture0
	oGPU_TEST_TEXTURE_1D,

	// Vertex: float3 Positions, float2 Texcoords
	// VS: ScreenSpacePos = full WVP transformation
	// PS: Color = texture0
	oGPU_TEST_TEXTURE_2D,

	// Vertex: float3 Positions, float3 Texcoords
	// VS: ScreenSpacePos = full WVP transformation
	// PS: Color = texture0
	oGPU_TEST_TEXTURE_3D,

	// Vertex: float3 Positions, float3 Texcoords
	// VS: ScreenSpacePos = full WVP transformation
	// PS: Color = texture0
	oGPU_TEST_TEXTURE_CUBE,

	// Vertex: float3 Positions, uint BGRA Color
	// VS: ScreenSpacePos = full WVP transformation
	// PS: Color = Vertex Color
	//oGPU_TEST_COLOR,
	//oGPU_TEST_PHONG,

	oGPU_TEST_NUM_PIPELINES,
};

struct oGPU_TEST_INSTANCE
{
	float3 Translation;
	quatf Rotation;
};

oAPI bool oGPUTestGetPipeline(oGPU_TEST_PIPELINE _Pipeline, oGPUPipeline::DESC* _pDesc);

#endif
