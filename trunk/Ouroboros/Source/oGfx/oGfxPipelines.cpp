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
#include <oGfx/oGfxPipelines.h>
#include <oGfx/oGfxVertexElements.h>

typedef unsigned char BYTE;

#include <PSColor.h>
#include <PSGrid.h>
#include <PSHero.h>
#include <VSLines.h>
#include <PSObjectID.h>
#include <PSMaterial.h>
#include <VSPassThrough.h>
#include <VSPositionPassThrough.h>
#include <VSPosition.h>
#include <VSRigid.h>
#include <PSShadow.h>
#include <VSShadow.h>
#include <PSTexcoord.h>
#include <PSTextureColor.h>
#include <GSVertexNormals.h>
#include <PSVertexPhong.h>
#include <GSVertexTangents.h>
#include <PSVSDepth.h>
#include <PSWhite.h>
#include <PSWSPixelNormal.h>
#include <PSWSVertexNormal.h>

bool oGfxGetPipeline(oGFX_PIPELINE _Pipeline, oGPU_PIPELINE_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oGPU_PIPELINE_DESC));

	#define oPL_RIGID(_Enum, _PS) do { \
		_pDesc->DebugName = #_Enum; \
		_pDesc->InputType = ouro::gpu::primitive_type::triangles; \
		oGfxGetVertexElements(oGFX_VE_RIGID, &_pDesc->pElements, &_pDesc->NumElements); \
		_pDesc->pVertexShader = VSRigid; \
		_pDesc->pPixelShader = _PS; \
	} while(false)

	#define oCASE_RIGID(_Enum, _PS) case _Enum: \
		oPL_RIGID(_Enum, _PS); \
		return true;

	switch (_Pipeline)
	{
		case oGFX_PIPELINE_VERTEX_NORMALS:
			_pDesc->DebugName = "oGFX_PIPELINE_VERTEX_NORMALS";
			_pDesc->InputType = ouro::gpu::primitive_type::points;
			oGfxGetVertexElements(oGFX_VE_RIGID, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSPassThrough;
			_pDesc->pGeometryShader = GSVertexNormals;
			_pDesc->pPixelShader = PSColor;
			return true;

		case oGFX_PIPELINE_VERTEX_TANGENTS:
			_pDesc->DebugName = "oGFX_PIPELINE_VERTEX_TANGENTS";
			_pDesc->InputType = ouro::gpu::primitive_type::points;
			oGfxGetVertexElements(oGFX_VE_RIGID, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSPassThrough;
			_pDesc->pGeometryShader = GSVertexTangents;
			_pDesc->pPixelShader = PSColor;
			return true;

		case oGFX_PIPELINE_LINES:
			_pDesc->DebugName = "oGFX_PIPELINE_LINES";
			_pDesc->InputType = ouro::gpu::primitive_type::lines;
			oGfxGetVertexElements(oGFX_VE_LINE, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSLines;
			_pDesc->pPixelShader = PSColor;
			return true;

		case oGFX_PIPELINE_LINE_STRIPS:
			_pDesc->DebugName = "oGFX_PIPELINE_LINE_STRIPS";
			_pDesc->InputType = ouro::gpu::primitive_type::line_strips;
			oGfxGetVertexElements(oGFX_VE_LINE, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSLines;
			_pDesc->pPixelShader = PSColor;
			return true;

		case oGFX_PIPELINE_PASS_THROUGH:
			_pDesc->DebugName = "oGFX_PIPELINE_PASS_THROUGH";
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSPositionPassThrough;
			_pDesc->pPixelShader = PSWhite;
			return true;

		case oGFX_PIPELINE_RIGID_ZPREPASS:
			_pDesc->DebugName = "oGFX_PIPELINE_RIGID_ZPREPASS";
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSPosition;
			return true;

		case oGFX_PIPELINE_RIGID_SHADOW:
			_pDesc->DebugName = "oGFX_PIPELINE_RIGID_SHADOW";
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSShadow;
			_pDesc->pPixelShader = PSShadow;
			return true;

		case oGFX_PIPELINE_RIGID_WHITE:
			_pDesc->DebugName = "oGFX_PIPELINE_RIGID_WHITE";
			_pDesc->InputType = ouro::gpu::primitive_type::triangles;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = VSPosition;
			_pDesc->pPixelShader = PSWhite;
			return true;

		oCASE_RIGID(oGFX_PIPELINE_RIGID_TEXCOORD, PSTexcoord)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_WSVNORMAL, PSWSVertexNormal)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_WSPNORMAL, PSWSPixelNormal)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_VSDEPTH, PSVSDepth)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_GRID, PSGrid)

		oCASE_RIGID(oGFX_PIPELINE_RIGID_OBJECT_ID, PSObjectID)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_TEXTURE_COLOR, PSTextureColor)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_VERTEX_PHONG, PSVertexPhong)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_MATERIAL, PSMaterial)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_HERO, PSHero)

		default: return oErrorSetLast(std::errc::not_supported);
	}
}
