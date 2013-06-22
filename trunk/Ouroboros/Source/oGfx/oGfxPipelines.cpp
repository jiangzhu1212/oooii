/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

#include <oGfxColorPS4ByteCode.h>
#include <oGfxGridPS4ByteCode.h>
#include <oGfxHeroPS4ByteCode.h>
#include <oGfxLinesVS4ByteCode.h>
#include <oGfxObjectIDPS4ByteCode.h>
#include <oGfxMaterialPS4ByteCode.h>
#include <oGfxPassThroughVS4ByteCode.h>
#include <oGfxPositionVS4ByteCode.h>
#include <oGfxRigidVS4ByteCode.h>
#include <oGfxShadowPS4ByteCode.h>
#include <oGfxShadowVS4ByteCode.h>
#include <oGfxTexcoordPS4ByteCode.h>
#include <oGfxTextureColorPS4ByteCode.h>
#include <oGfxVertexNormalsGS4ByteCode.h>
#include <oGfxVertexPhongPS4ByteCode.h>
#include <oGfxVertexTangentsGS4ByteCode.h>
#include <oGfxVSDepthPS4ByteCode.h>
#include <oGfxWhitePS4ByteCode.h>
#include <oGfxWSPNormalPS4ByteCode.h>
#include <oGfxWSVNormalPS4ByteCode.h>

bool oGfxGetPipeline(oGFX_PIPELINE _Pipeline, oGPU_PIPELINE_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oGPU_PIPELINE_DESC));

	#define oPL_RIGID(_Enum, _PS) do { \
		_pDesc->DebugName = #_Enum; \
		_pDesc->InputType = oGPU_TRIANGLES; \
		oGfxGetVertexElements(oGFX_VE_RIGID, &_pDesc->pElements, &_pDesc->NumElements); \
		_pDesc->pVertexShader = oGfxRigidVS4ByteCode; \
		_pDesc->pPixelShader = _PS; \
	} while(false)

	#define oCASE_RIGID(_Enum, _PS) case _Enum: \
		oPL_RIGID(_Enum, _PS); \
		return true;

	switch (_Pipeline)
	{
		case oGFX_PIPELINE_VERTEX_NORMALS:
			_pDesc->DebugName = "oGFX_PIPELINE_VERTEX_NORMALS";
			_pDesc->InputType = oGPU_POINTS;
			oGfxGetVertexElements(oGFX_VE_RIGID, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxPassThroughVS4ByteCode;
			_pDesc->pGeometryShader = oGfxVertexNormalsGS4ByteCode;
			_pDesc->pPixelShader = oGfxColorPS4ByteCode;
			return true;

		case oGFX_PIPELINE_VERTEX_TANGENTS:
			_pDesc->DebugName = "oGFX_PIPELINE_VERTEX_TANGENTS";
			_pDesc->InputType = oGPU_POINTS;
			oGfxGetVertexElements(oGFX_VE_RIGID, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxPassThroughVS4ByteCode;
			_pDesc->pGeometryShader = oGfxVertexTangentsGS4ByteCode;
			_pDesc->pPixelShader = oGfxColorPS4ByteCode;
			return true;

		case oGFX_PIPELINE_LINES:
			_pDesc->DebugName = "oGFX_PIPELINE_LINES";
			_pDesc->InputType = oGPU_LINES;
			oGfxGetVertexElements(oGFX_VE_LINE, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxLinesVS4ByteCode;
			_pDesc->pPixelShader = oGfxColorPS4ByteCode;
			return true;

		case oGFX_PIPELINE_LINE_STRIPS:
			_pDesc->DebugName = "oGFX_PIPELINE_LINE_STRIPS";
			_pDesc->InputType = oGPU_LINE_STRIPS;
			oGfxGetVertexElements(oGFX_VE_LINE, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxLinesVS4ByteCode;
			_pDesc->pPixelShader = oGfxColorPS4ByteCode;
			return true;

		case oGFX_PIPELINE_RIGID_ZPREPASS:
			_pDesc->DebugName = "oGFX_PIPELINE_RIGID_ZPREPASS";
			_pDesc->InputType = oGPU_TRIANGLES;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxPositionVS4ByteCode;
			return true;

		case oGFX_PIPELINE_RIGID_SHADOW:
			_pDesc->DebugName = "oGFX_PIPELINE_RIGID_SHADOW";
			_pDesc->InputType = oGPU_TRIANGLES;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxShadowVS4ByteCode;
			_pDesc->pPixelShader = oGfxShadowPS4ByteCode;
			return true;

		case oGFX_PIPELINE_RIGID_WHITE:
			_pDesc->DebugName = "oGFX_PIPELINE_RIGID_WHITE";
			_pDesc->InputType = oGPU_TRIANGLES;
			oGfxGetVertexElements(oGFX_VE_POSITION, &_pDesc->pElements, &_pDesc->NumElements);
			_pDesc->pVertexShader = oGfxPositionVS4ByteCode;
			_pDesc->pPixelShader = oGfxWhitePS4ByteCode;
			return true;

		oCASE_RIGID(oGFX_PIPELINE_RIGID_TEXCOORD, oGfxTexcoordPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_WSVNORMAL, oGfxWSVNormalPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_WSPNORMAL, oGfxWSPNormalPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_VSDEPTH, oGfxVSDepthPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_GRID, oGfxGridPS4ByteCode)

		oCASE_RIGID(oGFX_PIPELINE_RIGID_OBJECT_ID, oGfxObjectIDPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_TEXTURE_COLOR, oGfxTextureColorPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_VERTEX_PHONG, oGfxVertexPhongPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_MATERIAL, oGfxMaterialPS4ByteCode)
		oCASE_RIGID(oGFX_PIPELINE_RIGID_HERO, oGfxHeroPS4ByteCode)

		default: return oErrorSetLast(std::errc::not_supported);
	}
}
