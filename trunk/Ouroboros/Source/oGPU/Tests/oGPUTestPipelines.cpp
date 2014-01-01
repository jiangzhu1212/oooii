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

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_ONLY_VERTEX[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_COLOR_VERTEX[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'CLR0', ouro::surface::b8g8r8a8_unorm, 0, false },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_ONLY_INSTANCED_VERTEX[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TX  ', ouro::surface::r32g32b32a32_float, 1, true },
	{ 'ROT ', ouro::surface::r32g32b32a32_float, 1, true },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_TEXCOORD1D_VERTEX[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TEX0', ouro::surface::r32_float, 0, false },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_TEXCOORD2D_VERTEX[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TEX0', ouro::surface::r32g32_float, 0, false },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_TEXCOORD3D_VERTEX[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TEX0', ouro::surface::r32g32b32a32_float, 0, false },
};

bool oGPUTestGetPipeline(oGPU_TEST_PIPELINE _Pipeline, oGPUPipeline::DESC* _pDesc)
{
	// @tony: Can some of these be converted to oGPU_UTIL_DEFAULT_PIPELINE?

	switch (_Pipeline)
	{
		case oGPU_TEST_PASS_THROUGH:
			_pDesc->DebugName = "PassThrough";
			_pDesc->pElements = oGPU_TEST_POSITION_ONLY_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestPassThrough;
			_pDesc->pPixelShader = PSTestWhite;
			return true;

		case oGPU_TEST_PASS_THROUGH_COLOR:
			_pDesc->DebugName = "PassThroughColor";
			_pDesc->pElements = oGPU_TEST_POSITION_COLOR_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_COLOR_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::lines;
			_pDesc->pVertexShader = VSTestPassThroughColor;
			_pDesc->pPixelShader = PSTestColor;
			return true;

		case oGPU_TEST_TRANSFORMED_WHITE:
			_pDesc->DebugName = "TransformedWhite";
			_pDesc->pElements = oGPU_TEST_POSITION_ONLY_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestWhite;
			_pDesc->pPixelShader = PSTestWhite;
			return true;

		case oGPU_TEST_TRANSFORMED_WHITE_INSTANCED:
			_pDesc->DebugName = "TransformedWhiteInstanced";
			_pDesc->pElements = oGPU_TEST_POSITION_ONLY_INSTANCED_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_INSTANCED_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestWhiteInstanced;
			_pDesc->pPixelShader = PSTestWhite;
			return true;

		case oGPU_TEST_BUFFER:
			_pDesc->DebugName = "Buffer";
			_pDesc->pElements = nullptr;
			_pDesc->NumElements = 0;
			_pDesc->InputType = ouro::gpu::primitive_type::points;
			_pDesc->pVertexShader = VSTestBuffer;
			_pDesc->pPixelShader = PSTestBuffer;
			return true;

		case oGPU_TEST_TEXTURE_1D:
			_pDesc->DebugName = "Texture1D";
			_pDesc->pElements = oGPU_TEST_POSITION_TEXCOORD1D_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_TEXCOORD1D_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestTexture1D;
			_pDesc->pPixelShader = PSTestTexture1D;
			return true;

		case oGPU_TEST_TEXTURE_2D:
			_pDesc->DebugName = "Texture2D";
			_pDesc->pElements = oGPU_TEST_POSITION_TEXCOORD2D_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_TEXCOORD2D_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestTexture2D;
			_pDesc->pPixelShader = PSTestTexture2D;
			return true;

		case oGPU_TEST_TEXTURE_3D:
			_pDesc->DebugName = "Texture3D";
			_pDesc->pElements = oGPU_TEST_POSITION_TEXCOORD3D_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_TEXCOORD3D_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestTexture3D;
			_pDesc->pPixelShader = PSTestTexture3D;
			return true;

		case oGPU_TEST_TEXTURE_CUBE:
			_pDesc->DebugName = "TextureCube";
			_pDesc->pElements = oGPU_TEST_POSITION_TEXCOORD3D_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_TEXCOORD3D_VERTEX);
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			_pDesc->pVertexShader = VSTestTextureCube;
			_pDesc->pPixelShader = PSTestTextureCube;
			return true;

		//case oGPU_TEST_COLOR:
		//	_pDesc->DebugName = "Color";
		//	_pDesc->pElements = oGPU_TEST_POSITION_ONLY_VERTEX;
		//	_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_VERTEX);
		//	_pDesc->InputType = ouro::gpu::primitive_type::triangles;
		//	_pDesc->pVertexShader = VSTestWhite;
		//	_pDesc->pPixelShader = PSTestColor;
		//	return true;

		//case oGPU_TEST_PHONG:
		//	break;
		default:
			break;
	}

	return false;
}
