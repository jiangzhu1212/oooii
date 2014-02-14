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

using namespace ouro::gpu;
using namespace ouro::mesh;

pipeline_info oGfxGetPipeline(oGFX_PIPELINE _Pipeline)
{
	pipeline_info i;

	#define oPL_RIGID(_Enum, _PS) do { \
		i.debug_name = #_Enum; \
		i.primitive_type = primitive_type::triangles; \
		i.vertex_layouts[0] = layout::pos_nrm_tan_uv0; \
		i.vs = VSRigid; \
		i.ps = _PS; \
	} while(false)

	#define oCASE_RIGID(_Enum, _PS) case _Enum: \
		oPL_RIGID(_Enum, _PS); \
		break;

	switch (_Pipeline)
	{
		case oGFX_PIPELINE_VERTEX_NORMALS:
			i.debug_name = "oGFX_PIPELINE_VERTEX_NORMALS";
			i.primitive_type = primitive_type::points;
			i.vertex_layouts[0] = layout::pos_nrm_tan_uv0;
			i.vs = VSPassThrough;
			i.gs = GSVertexNormals;
			i.ps = PSColor;
			break;

		case oGFX_PIPELINE_VERTEX_TANGENTS:
			i.debug_name = "oGFX_PIPELINE_VERTEX_TANGENTS";
			i.primitive_type = primitive_type::points;
			i.vertex_layouts[0] = layout::pos_nrm_tan_uv0;
			i.vs = VSPassThrough;
			i.gs = GSVertexTangents;
			i.ps = PSColor;
			break;

		case oGFX_PIPELINE_LINES:
			i.debug_name = "oGFX_PIPELINE_LINES";
			i.primitive_type = primitive_type::lines;
			i.vertex_layouts[0] = layout::pos_color;
			i.vs = VSLines;
			i.ps = PSColor;
			break;

		case oGFX_PIPELINE_LINE_STRIPS:
			i.debug_name = "oGFX_PIPELINE_LINE_STRIPS";
			i.primitive_type = primitive_type::line_strips;
			i.vertex_layouts[0] = layout::pos_color;
			i.vs = VSLines;
			i.ps = PSColor;
			break;

		case oGFX_PIPELINE_PASS_THROUGH:
			i.debug_name = "oGFX_PIPELINE_PASS_THROUGH";
			i.primitive_type = primitive_type::triangles;
			i.vertex_layouts[0] = layout::pos;
			i.vs = VSPositionPassThrough;
			i.ps = PSWhite;
			break;

		case oGFX_PIPELINE_RIGID_ZPREPASS:
			i.debug_name = "oGFX_PIPELINE_RIGID_ZPREPASS";
			i.primitive_type = primitive_type::triangles;
			i.vertex_layouts[0] = layout::pos;
			i.vs = VSPosition;
			break;

		case oGFX_PIPELINE_RIGID_SHADOW:
			i.debug_name = "oGFX_PIPELINE_RIGID_SHADOW";
			i.primitive_type = primitive_type::triangles;
			i.vertex_layouts[0] = layout::pos;
			i.vs = VSShadow;
			i.ps = PSShadow;
			break;

		case oGFX_PIPELINE_RIGID_WHITE:
			i.debug_name = "oGFX_PIPELINE_RIGID_WHITE";
			i.primitive_type = primitive_type::triangles;
			i.vertex_layouts[0] = layout::pos;
			i.vs = VSPosition;
			i.ps = PSWhite;
			break;

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

		default: oTHROW_INVARG0();
	}

	return i;
}
