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
#include <oBasis/oGPUConcepts.h>

using namespace ouro;

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_API)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_API)
		oRTTI_VALUE_CUSTOM(oGPU_API_UNKNOWN, "Unknown")
		oRTTI_VALUE_CUSTOM(oGPU_API_D3D, "Direct3D")
		oRTTI_VALUE_CUSTOM(oGPU_API_OGL, "OpenGL")
		oRTTI_VALUE_CUSTOM(oGPU_API_OGLES, "OpenGL ES")
		oRTTI_VALUE_CUSTOM(oGPU_API_WEBGL, "WebGL")
		oRTTI_VALUE_CUSTOM(oGPU_API_CUSTOM, "Custom")
	oRTTI_ENUM_END_VALUES(oGPU_API)
oRTTI_ENUM_END_DESCRIPTION(oGPU_API)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_CUBE_FACE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_CUBE_FACE)
		oRTTI_VALUE(oGPU_CUBE_POS_X)
		oRTTI_VALUE(oGPU_CUBE_NEG_X)
		oRTTI_VALUE(oGPU_CUBE_POS_Y)
		oRTTI_VALUE(oGPU_CUBE_NEG_Y)
		oRTTI_VALUE(oGPU_CUBE_POS_Z)
		oRTTI_VALUE(oGPU_CUBE_NEG_Z)
	oRTTI_ENUM_END_VALUES(oGPU_CUBE_FACE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_CUBE_FACE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_DEBUG_LEVEL)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_DEBUG_LEVEL)
		oRTTI_VALUE_CUSTOM(oGPU_DEBUG_NONE, "None")
		oRTTI_VALUE_CUSTOM(oGPU_DEBUG_NORMAL, "Normal")
		oRTTI_VALUE_CUSTOM(oGPU_DEBUG_UNFILTERED, "Unfiltered")
	oRTTI_ENUM_END_VALUES(oGPU_DEBUG_LEVEL)
oRTTI_ENUM_END_DESCRIPTION(oGPU_DEBUG_LEVEL)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_PIPELINE_STAGE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_PIPELINE_STAGE)
		oRTTI_VALUE(oGPU_VERTEX_SHADER)
		oRTTI_VALUE(oGPU_HULL_SHADER)
		oRTTI_VALUE(oGPU_DOMAIN_SHADER)
		oRTTI_VALUE(oGPU_GEOMETRY_SHADER)
		oRTTI_VALUE(oGPU_PIXEL_SHADER)
	oRTTI_ENUM_END_VALUES(oGPU_PIPELINE_STAGE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_PIPELINE_STAGE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_PRIMITIVE_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_PRIMITIVE_TYPE)
		oRTTI_VALUE(oGPU_UNKNOWN_PRIMITIVE_TYPE)
		oRTTI_VALUE(oGPU_POINTS)
		oRTTI_VALUE(oGPU_LINES)
		oRTTI_VALUE(oGPU_LINE_STRIPS)
		oRTTI_VALUE(oGPU_TRIANGLES)
		oRTTI_VALUE(oGPU_TRIANGLE_STRIPS)
		oRTTI_VALUE(oGPU_LINES_ADJACENCY)
		oRTTI_VALUE(oGPU_LINE_STRIPS_ADJACENCY)
		oRTTI_VALUE(oGPU_TRIANGLES_ADJACENCY)
		oRTTI_VALUE(oGPU_TRIANGLE_STRIPS_ADJACENCY)
		oRTTI_VALUE(oGPU_PATCHES1)
		oRTTI_VALUE(oGPU_PATCHES2)
		oRTTI_VALUE(oGPU_PATCHES3)
		oRTTI_VALUE(oGPU_PATCHES4)
		oRTTI_VALUE(oGPU_PATCHES5)
		oRTTI_VALUE(oGPU_PATCHES6)
		oRTTI_VALUE(oGPU_PATCHES7)
		oRTTI_VALUE(oGPU_PATCHES8)
		oRTTI_VALUE(oGPU_PATCHES9)
		oRTTI_VALUE(oGPU_PATCHES10)
		oRTTI_VALUE(oGPU_PATCHES11)
		oRTTI_VALUE(oGPU_PATCHES12)
		oRTTI_VALUE(oGPU_PATCHES13)
		oRTTI_VALUE(oGPU_PATCHES14)
		oRTTI_VALUE(oGPU_PATCHES15)
		oRTTI_VALUE(oGPU_PATCHES16)
		oRTTI_VALUE(oGPU_PATCHES17)
		oRTTI_VALUE(oGPU_PATCHES18)
		oRTTI_VALUE(oGPU_PATCHES19)
		oRTTI_VALUE(oGPU_PATCHES20)
		oRTTI_VALUE(oGPU_PATCHES21)
		oRTTI_VALUE(oGPU_PATCHES22)
		oRTTI_VALUE(oGPU_PATCHES23)
		oRTTI_VALUE(oGPU_PATCHES24)
		oRTTI_VALUE(oGPU_PATCHES25)
		oRTTI_VALUE(oGPU_PATCHES26)
		oRTTI_VALUE(oGPU_PATCHES27)
		oRTTI_VALUE(oGPU_PATCHES28)
		oRTTI_VALUE(oGPU_PATCHES29)
		oRTTI_VALUE(oGPU_PATCHES30)
		oRTTI_VALUE(oGPU_PATCHES31)
		oRTTI_VALUE(oGPU_PATCHES32)
	oRTTI_ENUM_END_VALUES(oGPU_PRIMITIVE_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_PRIMITIVE_TYPE)


oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_RESOURCE_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_RESOURCE_TYPE)
		oRTTI_VALUE(oGPU_BUFFER)
		oRTTI_VALUE(oGPU_MESH)
		oRTTI_VALUE(oGPU_TEXTURE)
	oRTTI_ENUM_END_VALUES(oGPU_RESOURCE_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_RESOURCE_TYPE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_BUFFER_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_BUFFER_TYPE)
		oRTTI_VALUE(oGPU_BUFFER_DEFAULT)
		oRTTI_VALUE(oGPU_BUFFER_READBACK)
		oRTTI_VALUE(oGPU_BUFFER_INDEX)
		oRTTI_VALUE(oGPU_BUFFER_INDEX_READBACK)
		oRTTI_VALUE(oGPU_BUFFER_VERTEX)
		oRTTI_VALUE(oGPU_BUFFER_VERTEX_READBACK)
		oRTTI_VALUE(oGPU_BUFFER_UNORDERED_RAW)
		oRTTI_VALUE(oGPU_BUFFER_UNORDERED_UNSTRUCTURED)
		oRTTI_VALUE(oGPU_BUFFER_UNORDERED_STRUCTURED)
		oRTTI_VALUE(oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND)
		oRTTI_VALUE(oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER)
	oRTTI_ENUM_END_VALUES(oGPU_BUFFER_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_BUFFER_TYPE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_TEXTURE_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_TEXTURE_TYPE)
		oRTTI_VALUE(oGPU_TEXTURE_1D_MAP)
		oRTTI_VALUE(oGPU_TEXTURE_1D_MAP_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_1D_RENDER_TARGET)
		oRTTI_VALUE(oGPU_TEXTURE_1D_RENDER_TARGET_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_1D_READBACK)
		oRTTI_VALUE(oGPU_TEXTURE_1D_READBACK_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_2D_MAP)
		oRTTI_VALUE(oGPU_TEXTURE_2D_MAP_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_2D_RENDER_TARGET)
		oRTTI_VALUE(oGPU_TEXTURE_2D_RENDER_TARGET_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_2D_READBACK)
		oRTTI_VALUE(oGPU_TEXTURE_2D_READBACK_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_2D_MAP_UNORDERED)
		oRTTI_VALUE(oGPU_TEXTURE_CUBE_MAP)
		oRTTI_VALUE(oGPU_TEXTURE_CUBE_MAP_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_CUBE_RENDER_TARGET)
		oRTTI_VALUE(oGPU_TEXTURE_CUBE_RENDER_TARGET_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_CUBE_READBACK)
		oRTTI_VALUE(oGPU_TEXTURE_CUBE_READBACK_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_3D_MAP)
		oRTTI_VALUE(oGPU_TEXTURE_3D_MAP_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_3D_RENDER_TARGET)
		oRTTI_VALUE(oGPU_TEXTURE_3D_RENDER_TARGET_MIPS)
		oRTTI_VALUE(oGPU_TEXTURE_3D_READBACK)
		oRTTI_VALUE(oGPU_TEXTURE_3D_READBACK_MIPS)
	oRTTI_ENUM_END_VALUES(oGPU_TEXTURE_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_TEXTURE_TYPE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_SURFACE_STATE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_SURFACE_STATE)
		oRTTI_VALUE(oGPU_FRONT_FACE)
		oRTTI_VALUE(oGPU_BACK_FACE)
		oRTTI_VALUE(oGPU_TWO_SIDED)
		oRTTI_VALUE(oGPU_FRONT_WIREFRAME)
		oRTTI_VALUE(oGPU_BACK_WIREFRAME)
		oRTTI_VALUE(oGPU_TWO_SIDED_WIREFRAME)
	oRTTI_ENUM_END_VALUES(oGPU_SURFACE_STATE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_SURFACE_STATE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_DEPTH_STENCIL_STATE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_DEPTH_STENCIL_STATE)
		oRTTI_VALUE(oGPU_DEPTH_STENCIL_NONE)
		oRTTI_VALUE(oGPU_DEPTH_TEST_AND_WRITE)
		oRTTI_VALUE(oGPU_DEPTH_TEST)
	oRTTI_ENUM_END_VALUES(oGPU_DEPTH_STENCIL_STATE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_DEPTH_STENCIL_STATE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_BLEND_STATE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_BLEND_STATE)
		oRTTI_VALUE(oGPU_OPAQUE)
		oRTTI_VALUE(oGPU_ALPHA_TEST)
		oRTTI_VALUE(oGPU_ACCUMULATE)
		oRTTI_VALUE(oGPU_ADDITIVE)
		oRTTI_VALUE(oGPU_MULTIPLY)
		oRTTI_VALUE(oGPU_SCREEN)
		oRTTI_VALUE(oGPU_TRANSLUCENT)
		oRTTI_VALUE(oGPU_MIN)
		oRTTI_VALUE(oGPU_MAX)
	oRTTI_ENUM_END_VALUES(oGPU_BLEND_STATE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_BLEND_STATE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_SAMPLER_STATE)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_SAMPLER_STATE)
		oRTTI_VALUE(oGPU_POINT_CLAMP)
		oRTTI_VALUE(oGPU_POINT_WRAP)
		oRTTI_VALUE(oGPU_LINEAR_CLAMP)
		oRTTI_VALUE(oGPU_LINEAR_WRAP)
		oRTTI_VALUE(oGPU_ANISO_CLAMP)
		oRTTI_VALUE(oGPU_ANISO_WRAP)

		oRTTI_VALUE(oGPU_POINT_CLAMP_BIAS_UP1)
		oRTTI_VALUE(oGPU_POINT_WRAP_BIAS_UP1)
		oRTTI_VALUE(oGPU_LINEAR_CLAMP_BIAS_UP1)
		oRTTI_VALUE(oGPU_LINEAR_WRAP_BIAS_UP1)
		oRTTI_VALUE(oGPU_ANISO_CLAMP_BIAS_UP1)
		oRTTI_VALUE(oGPU_ANISO_WRAP_BIAS_UP1)

		oRTTI_VALUE(oGPU_POINT_CLAMP_BIAS_DOWN1)
		oRTTI_VALUE(oGPU_POINT_WRAP_BIAS_DOWN1)
		oRTTI_VALUE(oGPU_LINEAR_CLAMP_BIAS_DOWN1)
		oRTTI_VALUE(oGPU_LINEAR_WRAP_BIAS_DOWN1)
		oRTTI_VALUE(oGPU_ANISO_CLAMP_BIAS_DOWN1)
		oRTTI_VALUE(oGPU_ANISO_WRAP_BIAS_DOWN1)

		oRTTI_VALUE(oGPU_POINT_CLAMP_BIAS_UP2)
		oRTTI_VALUE(oGPU_POINT_WRAP_BIAS_UP2)
		oRTTI_VALUE(oGPU_LINEAR_CLAMP_BIAS_UP2)
		oRTTI_VALUE(oGPU_LINEAR_WRAP_BIAS_UP2)
		oRTTI_VALUE(oGPU_ANISO_CLAMP_BIAS_UP2)
		oRTTI_VALUE(oGPU_ANISO_WRAP_BIAS_UP2)

		oRTTI_VALUE(oGPU_POINT_CLAMP_BIAS_DOWN2)
		oRTTI_VALUE(oGPU_POINT_WRAP_BIAS_DOWN2)
		oRTTI_VALUE(oGPU_LINEAR_CLAMP_BIAS_DOWN2)
		oRTTI_VALUE(oGPU_LINEAR_WRAP_BIAS_DOWN2)
		oRTTI_VALUE(oGPU_ANISO_CLAMP_BIAS_DOWN2)
		oRTTI_VALUE(oGPU_ANISO_WRAP_BIAS_DOWN2)
	oRTTI_ENUM_END_VALUES(oGPU_SAMPLER_STATE)
oRTTI_ENUM_END_DESCRIPTION(oGPU_SAMPLER_STATE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_CLEAR)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_CLEAR)
		oRTTI_VALUE(oGPU_CLEAR_DEPTH)
		oRTTI_VALUE(oGPU_CLEAR_STENCIL)
		oRTTI_VALUE(oGPU_CLEAR_DEPTH_STENCIL)
		oRTTI_VALUE(oGPU_CLEAR_COLOR)
		oRTTI_VALUE(oGPU_CLEAR_COLOR_DEPTH)
		oRTTI_VALUE(oGPU_CLEAR_COLOR_STENCIL)
		oRTTI_VALUE(oGPU_CLEAR_COLOR_DEPTH_STENCIL)
	oRTTI_ENUM_END_VALUES(oGPU_CLEAR)
oRTTI_ENUM_END_DESCRIPTION(oGPU_CLEAR)

static_assert(sizeof(oGPU_RANGE) == 16, "unexpected struct packing for oGPU_RANGE");
static_assert(sizeof(fourcc) == 4, "unexpected struct packing for fourcc");
static_assert(sizeof(ouro::resized_type<ouro::surface::format, short>) == 2, "unexpected struct packing for oResizedType<ouro::surface::format, short>");
static_assert(sizeof(oGPU_VERTEX_ELEMENT) == 8, "unexpected struct packing for oGPU_VERTEX_ELEMENT");

bool oGPUParseSemantic(const fourcc& _FourCC, char _Name[5], uint* _pIndex)
{
	*_pIndex = 0;
	*_Name = 0;

	char fcc[5];
	to_string(fcc, _FourCC);
	char* i = &fcc[3];
	while (isspace(*i)) i--;
	if (i == fcc)
		return false;

	while (isdigit(*i)) i--;
	if (i == fcc)
		return false;

	i++;
	*_pIndex = atoi(i);

	// there can be no numbers or space before the semantic index
	char* n = _Name;
	char* f = fcc;
	while (f < i)
	{
		if (isspace(*f) || isdigit(*f))
			return false;
		*n++ = *f++;
	}

	*n = 0;
	return true;
}

uint oGPUCalcVertexSize(const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, uint _InputSlot)
{
	bool IsFirstRun = true;
	bool IsInstanceList = false;

	uint size = 0;
	for (uint i = 0; i < _NumElements; i++)
	{
		if (_InputSlot == _pElements[i].InputSlot)
		{
			if (IsFirstRun)
			{
				IsInstanceList = _pElements[i].Instanced;
				IsFirstRun = false;
			}

			else
				oASSERT(IsInstanceList == _pElements[i].Instanced, "Elements in the same slot must either be all instanced or all not instanced.");

			size += ouro::surface::element_size(_pElements[i].Format);
		}
	}

	return size;
}

uint oGPUCalcNumInputSlots(const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements)
{
	uchar nSlots = 0;

	#ifdef _DEBUG
		uint lastSlot = oInvalid;
	#endif

	for (uint i = 0; i < _NumElements; i++)
	{
		nSlots = __max(nSlots, _pElements[i].InputSlot+1);

		#ifdef _DEBUG
			oASSERT(lastSlot == oInvalid 
				|| lastSlot == _pElements[i].InputSlot 
				|| lastSlot == oUInt(_pElements[i].InputSlot - 1), "Non-packed elements");
			lastSlot = _pElements[i].InputSlot;
		#endif
	}

	return nSlots;
}
