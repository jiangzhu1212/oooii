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
#include <oGPU/oGPUUtil.h>

#include <oGPU_PS4_0_DEPTHByteCode.h>
#include <oGPU_PS4_0_SHADOWByteCode.h>
#include <oGPU_PS4_0_GRIDByteCode.h>
#include <oGPU_PS4_0_LINEByteCode.h>
#include <oGPU_PS4_0_OBJ_PIXEL_LITByteCode.h>
#include <oGPU_PS4_0_OBJ_UNLITByteCode.h>
#include <oGPU_PS4_0_OBJ_VERTEX_LITByteCode.h>
#include <oGPU_PS4_0_POSITIONByteCode.h>
#include <oGPU_PS4_0_TEXCOORDByteCode.h>
#include <oGPU_PS4_0_WSNORMALByteCode.h>
#include <oGPU_VS4_0_DEPTHByteCode.h>
#include <oGPU_VS4_0_DEPTH_INSTANCEDByteCode.h>
#include <oGPU_VS4_0_SHADOWByteCode.h>
#include <oGPU_VS4_0_LINEByteCode.h>
#include <oGPU_VS4_0_OBJ_COMMONByteCode.h>
#include <oGPU_VS4_0_OBJ_COMMON_INSTANCEDByteCode.h>
#include <oGPU_VS4_0_POSITIONByteCode.h>
#include <oGPU_VS4_0_POSITION_INSTANCEDByteCode.h>
#include <oGPU_PS4_0_OBJECT_IDByteCode.h>

static const oGPU_VERTEX_ELEMENT VEPositionOnly[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
};

static const oGPU_VERTEX_ELEMENT VEInstancedPositionOnly[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TX  ', oSURFACE_R32G32B32_FLOAT, 1, true },
	{ 'ROT ', oSURFACE_R32G32B32A32_FLOAT, 1, true },
};

static const oGPU_VERTEX_ELEMENT VELineList[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'CLR0', oSURFACE_B8G8R8A8_UNORM, 0, false },
};

static const oGPU_VERTEX_ELEMENT VEOBJElements[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TEX0', oSURFACE_R32G32_FLOAT, 0, false },
	{ 'NML0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TAN0', oSURFACE_R32G32B32A32_FLOAT, 0, false },
};

static const oGPU_VERTEX_ELEMENT VEInstancedOBJElements[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TEX0', oSURFACE_R32G32_FLOAT, 0, false },
	{ 'NML0', oSURFACE_R32G32B32_FLOAT, 0, false },
	{ 'TAN0', oSURFACE_R32G32B32A32_FLOAT, 0, false },
	{ 'TX  ', oSURFACE_R32G32B32_FLOAT, 1, true }, // translation
	{ 'ROT ', oSURFACE_R32G32B32A32_FLOAT, 1, true }, // quaternion
};

bool oGPUGetPipeline(oGPU_UTIL_DEFAULT_PIPELINE _Pipeline, oGPUPipeline::DESC* _pDesc)
{
	#define oPOSONLY(_Name, _PixelShader) _Name, VEPositionOnly, oCOUNTOF(VEPositionOnly), oGPU_TRIANGLES, oGPU_VS4_0_POSITIONByteCode, nullptr, nullptr, nullptr, _PixelShader
	#define oPOSONLYINST(_Name, _PixelShader) _Name, VEInstancedPositionOnly, oCOUNTOF(VEInstancedPositionOnly), oGPU_TRIANGLES, oGPU_VS4_0_POSITION_INSTANCEDByteCode, nullptr, nullptr, nullptr, _PixelShader

	#define oOBJ(_Name, _PixelShader) _Name, VEOBJElements, oCOUNTOF(VEOBJElements), oGPU_TRIANGLES, oGPU_VS4_0_OBJ_COMMONByteCode, nullptr, nullptr, nullptr, _PixelShader
	#define oOBJINST(_Name, _PixelShader) _Name, VEInstancedOBJElements, oCOUNTOF(VEInstancedOBJElements), oGPU_TRIANGLES, oGPU_VS4_0_OBJ_COMMON_INSTANCEDByteCode, nullptr, nullptr, nullptr, _PixelShader

	static const oGPU_STATIC_PIPELINE_DESC sDefaultPipelines[] = 
	{
		{ "DefaultPipeline.ShadowMap", VEPositionOnly, oCOUNTOF(VEPositionOnly), oGPU_TRIANGLES, oGPU_VS4_0_SHADOWByteCode, nullptr, nullptr, nullptr, oGPU_PS4_0_SHADOWByteCode },
		{ oPOSONLY("DefaultPipeline.Grid", oGPU_PS4_0_GRIDByteCode) },
		{ oPOSONLYINST("DefaultPipeline.GridInst", oGPU_PS4_0_GRIDByteCode) },
		{ oPOSONLY("DefaultPipeline.Pos", oGPU_PS4_0_POSITIONByteCode) },
		{ oPOSONLYINST("DefaultPipeline.PosInst", oGPU_PS4_0_POSITIONByteCode) },
		{ "DefaultPipeline.Depth", VEPositionOnly, oCOUNTOF(VEPositionOnly), oGPU_TRIANGLES, oGPU_VS4_0_DEPTHByteCode, nullptr, nullptr, nullptr, oGPU_PS4_0_DEPTHByteCode },
		{ "DefaultPipeline.DepthInst", VEPositionOnly, oCOUNTOF(VEPositionOnly), oGPU_TRIANGLES, oGPU_VS4_0_DEPTH_INSTANCEDByteCode, nullptr, nullptr, nullptr, oGPU_PS4_0_DEPTHByteCode },
		{ oPOSONLY("DefaultPipeline.ObjectID", oGPU_PS4_0_OBJECT_IDByteCode) },
		{ oPOSONLYINST("DefaultPipeline.ObjectIDInst", oGPU_PS4_0_OBJECT_IDByteCode) },
		{ "DefaultPipeline.Line", VELineList, oCOUNTOF(VELineList), oGPU_LINES, oGPU_VS4_0_LINEByteCode, nullptr, nullptr, nullptr, oGPU_PS4_0_LINEByteCode },
		{ oOBJ("DefaultPipeline.Texcoord", oGPU_PS4_0_TEXCOORDByteCode) },
		{ oOBJINST("DefaultPipeline.TexcoordInst", oGPU_PS4_0_TEXCOORDByteCode) },
		{ oOBJ("DefaultPipeline.WSNormal", oGPU_PS4_0_WSNORMALByteCode) },
		{ oOBJINST("DefaultPipeline.WSNormalInst", oGPU_PS4_0_WSNORMALByteCode) },
		{ oOBJ("DefaultPipeline.Unlit", oGPU_PS4_0_OBJ_UNLITByteCode) },
		{ oOBJINST("DefaultPipeline.UnlitInst", oGPU_PS4_0_OBJ_UNLITByteCode) },
		{ oOBJ("DefaultPipeline.VLit", oGPU_PS4_0_OBJ_VERTEX_LITByteCode) },
		{ oOBJINST("DefaultPipeline.VLitInst", oGPU_PS4_0_OBJ_VERTEX_LITByteCode) },
		{ oOBJ("DefaultPipeline.PLit", oGPU_PS4_0_OBJ_PIXEL_LITByteCode) },
		{ oOBJINST("DefaultPipeline.PLitInst", oGPU_PS4_0_OBJ_PIXEL_LITByteCode) },
	};
	static_assert(oCOUNTOF(sDefaultPipelines) == oGPU_UTIL_DEFAULT_PIPELINE_COUNT, "sDefaultPipelines array size mismatch");
	*_pDesc = sDefaultPipelines[_Pipeline];
	return true;
}
