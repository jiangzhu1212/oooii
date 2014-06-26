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
#include "oGPUTestPipelines.h"
#include <PSTestColor.h>
#include <VSTestPassThroughColor.h>
#include <VSTestPassThrough.h>
#include <VSTestBuffer.h>
#include <PSTestBuffer.h>
#include <PSTestTexture1D.h>
#include <VSTestTexture1D.h>
#include <PSTestTexture2D.h>
#include <VSTestTexture2D.h>
#include <PSTestTexture3D.h>
#include <VSTestTexture3D.h>
#include <PSTestTextureCube.h>
#include <VSTestTextureCube.h>
#include <VSTestWhiteInstanced.h>
#include <VSTestWhite.h>
#include <PSTestWhite.h>

enum class common_elements : char
{
	pos,
	pos_color,
	pos_uv,
	pos_uvw,

	count,
};

ouro::mesh::element_array oGPUTestGetElements(const common_elements& c)
{
	ouro::mesh::element_array e;
	switch (c)
	{
		default:
		case common_elements::pos:
			e[0] = ouro::mesh::element(ouro::mesh::semantic::position, 0, ouro::mesh::format::xyz32_float, 0);
			break;
		case common_elements::pos_color:
			e[0] = ouro::mesh::element(ouro::mesh::semantic::position, 0, ouro::mesh::format::xyz32_float, 0);
			e[1] = ouro::mesh::element(ouro::mesh::semantic::color, 0, ouro::mesh::format::xyzw8_unorm, 0);
			break;
		case common_elements::pos_uv:
			e[0] = ouro::mesh::element(ouro::mesh::semantic::position, 0, ouro::mesh::format::xyz32_float, 0);
			e[1] = ouro::mesh::element(ouro::mesh::semantic::texcoord, 0, ouro::mesh::format::xy32_float, 0);
			break;
		case common_elements::pos_uvw:
			e[0] = ouro::mesh::element(ouro::mesh::semantic::position, 0, ouro::mesh::format::xyz32_float, 0);
			e[1] = ouro::mesh::element(ouro::mesh::semantic::texcoord, 0, ouro::mesh::format::xyz32_float, 0);
			break;
	}

	return e;
}

ouro::gpu::pipeline1_info oGPUTestGetPipeline(oGPU_TEST_PIPELINE _Pipeline)
{
	ouro::gpu::pipeline1_info i;

	switch (_Pipeline)
	{
		case oGPU_TEST_PASS_THROUGH:
			i.debug_name = "PassThrough";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos);
			i.primitive_type = ouro::mesh::primitive_type::triangles;
			i.vs = VSTestPassThrough;
			i.ps = PSTestWhite;
			break;

		case oGPU_TEST_PASS_THROUGH_COLOR:
			i.debug_name = "PassThroughColor";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos_color);
			i.primitive_type = ouro::mesh::primitive_type::lines;
			i.vs = VSTestPassThroughColor;
			i.ps = PSTestColor;
			break;

		case oGPU_TEST_TRANSFORMED_WHITE:
			i.debug_name = "TransformedWhite";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos);
			i.primitive_type = ouro::mesh::primitive_type::triangles;
			i.vs = VSTestWhiteInstanced;
			i.ps = PSTestWhite;
			break;

		case oGPU_TEST_BUFFER:
			i.debug_name = "Buffer";
			i.primitive_type = ouro::mesh::primitive_type::points;
			i.vs = VSTestBuffer;
			i.ps = PSTestBuffer;
			break;

		case oGPU_TEST_TEXTURE_1D:
			i.debug_name = "Texture1D";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos_uv);
			i.primitive_type = ouro::mesh::primitive_type::triangles;
			i.vs = VSTestTexture1D;
			i.ps = PSTestTexture1D;
			break;

		case oGPU_TEST_TEXTURE_2D:
			i.debug_name = "Texture2D";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos_uv);
			i.primitive_type = ouro::mesh::primitive_type::triangles;
			i.vs = VSTestTexture2D;
			i.ps = PSTestTexture2D;
			break;

		case oGPU_TEST_TEXTURE_3D:
			i.debug_name = "Texture3D";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos_uvw);
			i.primitive_type = ouro::mesh::primitive_type::triangles;
			i.vs = VSTestTexture3D;
			i.ps = PSTestTexture3D;
			break;

		case oGPU_TEST_TEXTURE_CUBE:
			i.debug_name = "TextureCube";
			i.vertex_layout = oGPUTestGetElements(common_elements::pos_uvw);
			i.primitive_type = ouro::mesh::primitive_type::triangles;
			i.vs = VSTestTextureCube;
			i.ps = PSTestTextureCube;
			break;

		default:
			break;
	}

	return i;
}
