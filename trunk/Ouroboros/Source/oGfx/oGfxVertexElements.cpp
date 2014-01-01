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
#include <oGfx/oGfxVertexElements.h>

static const oGPU_VERTEX_ELEMENT sVEPosition[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
};

static const oGPU_VERTEX_ELEMENT sVEPositionInstanced[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TX  ', ouro::surface::r32g32b32a32_float, 1, true }, // translation
	{ 'ROT ', ouro::surface::r32g32b32a32_float, 1, true }, // quaternion
	{ 'SCAL', ouro::surface::r32_float, 1, true }, // uniform scale
};

static const oGPU_VERTEX_ELEMENT sVERigid[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TEX0', ouro::surface::r32g32_float, 0, false },
	{ 'NML0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TAN0', ouro::surface::r32g32b32a32_float, 0, false },
};

static const oGPU_VERTEX_ELEMENT sVERigidInstanced[] = 
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TEX0', ouro::surface::r32g32_float, 0, false },
	{ 'NML0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TAN0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'TX  ', ouro::surface::r32g32b32a32_float, 1, true }, // translation
	{ 'ROT ', ouro::surface::r32g32b32a32_float, 1, true }, // quaternion
	{ 'SCAL', ouro::surface::r32_float, 1, true }, // uniform scale
};

static const oGPU_VERTEX_ELEMENT sVELine[] =
{
	{ 'POS0', ouro::surface::r32g32b32a32_float, 0, false },
	{ 'CLR0', ouro::surface::b8g8r8a8_unorm, 0, false },
};

#define oGET_VE(_Name) do { *_ppVertexElements = _Name; *_pNumVertexElements = oCOUNTOF(_Name); } while (false)

void oGfxGetVertexElements(oGFX_VERTEX_ELEMENT_LIST _GfxVertexElementList, const oGPU_VERTEX_ELEMENT** _ppVertexElements, unsigned int* _pNumVertexElements)
{
	switch (_GfxVertexElementList)
	{
		case oGFX_VE_POSITION: oGET_VE(sVEPosition); break;
		case oGFX_VE_POSITION_INSTANCED: oGET_VE(sVEPositionInstanced); break;
		case oGFX_VE_RIGID: oGET_VE(sVERigid); break;
		case oGFX_VE_RIGID_INSTANCED: oGET_VE(sVERigidInstanced); break;
		case oGFX_VE_LINE: oGET_VE(sVELine); break;
		oNODEFAULT;
	}
}