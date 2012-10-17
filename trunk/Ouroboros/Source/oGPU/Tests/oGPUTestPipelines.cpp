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
#include "oGPUTestPipelines.h"
#include <oGPUTestColorPSByteCode.h>
#include <oGPUTestPassThroughColorVSByteCode.h>
#include <oGPUTestPassThroughVSByteCode.h>
#include <oGPUTestTexturePSByteCode.h>
#include <oGPUTestTextureVSByteCode.h>
#include <oGPUTestWhiteInstancedVSByteCode.h>
#include <oGPUTestWhitePSByteCode.h>
#include <oGPUTestWhiteVSByteCode.h>

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_ONLY_VERTEX[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_COLOR_VERTEX[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'CLR0', oSURFACE_B8G8R8A8_UNORM, 0, false },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_ONLY_INSTANCED_VERTEX[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TX  ', oSURFACE_R32G32B32_FLOAT, 1, true },
	{ 'ROT ', oSURFACE_R32G32B32A32_FLOAT, 1, true },
};

static const oGPU_VERTEX_ELEMENT oGPU_TEST_POSITION_TEXCOORD_VERTEX[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TEX0', oSURFACE_R32G32_FLOAT, 0, false },
};

bool oGPUTestGetPipeline(oGPU_TEST_PIPELINE _Pipeline, oGPUPipeline::DESC* _pDesc)
{
	switch (_Pipeline)
	{
		case oGPU_TEST_PASS_THROUGH:
			_pDesc->DebugName = "PassThrough";
			_pDesc->pElements = oGPU_TEST_POSITION_ONLY_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_VERTEX);
			_pDesc->pVertexShader = oGPUTestPassThroughVSByteCode;
			_pDesc->pPixelShader = oGPUTestWhitePSByteCode;
			return true;

		case oGPU_TEST_PASS_THROUGH_COLOR:
			_pDesc->DebugName = "PassThroughColor";
			_pDesc->pElements = oGPU_TEST_POSITION_COLOR_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_COLOR_VERTEX);
			_pDesc->pVertexShader = oGPUTestPassThroughColorVSByteCode;
			_pDesc->pPixelShader = oGPUTestColorPSByteCode;
			return true;

		case oGPU_TEST_TRANSFORMED_WHITE:
			_pDesc->DebugName = "TransformedWhite";
			_pDesc->pElements = oGPU_TEST_POSITION_ONLY_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_VERTEX);
			_pDesc->pVertexShader = oGPUTestWhiteVSByteCode;
			_pDesc->pPixelShader = oGPUTestWhitePSByteCode;
			return true;

		case oGPU_TEST_TRANSFORMED_WHITE_INSTANCED:
			_pDesc->DebugName = "TransformedWhiteInstanced";
			_pDesc->pElements = oGPU_TEST_POSITION_ONLY_INSTANCED_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_INSTANCED_VERTEX);
			_pDesc->pVertexShader = oGPUTestWhiteInstancedVSByteCode;
			_pDesc->pPixelShader = oGPUTestWhitePSByteCode;
			return true;

		case oGPU_TEST_TEXTURE:
			_pDesc->DebugName = "Texture";
			_pDesc->pElements = oGPU_TEST_POSITION_TEXCOORD_VERTEX;
			_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_TEXCOORD_VERTEX);
			_pDesc->pVertexShader = oGPUTestTextureVSByteCode;
			_pDesc->pPixelShader = oGPUTestTexturePSByteCode;
			return true;

		//case oGPU_TEST_COLOR:
		//	_pDesc->DebugName = "Color";
		//	_pDesc->pElements = oGPU_TEST_POSITION_ONLY_VERTEX;
		//	_pDesc->NumElements = oCOUNTOF(oGPU_TEST_POSITION_ONLY_VERTEX);
		//	_pDesc->pVertexShader = oGPUTestWhiteVSByteCode;
		//	_pDesc->pPixelShader = oGPUTestColorPSByteCode;
		//	return true;

		//case oGPU_TEST_PHONG:
		//	break;
		default:
			break;
	}

	return false;
}
