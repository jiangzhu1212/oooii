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
#include "oGfxTestPipeline.h"
#include <oPlatform/oWindows.h>
#include <oGfxTestPSByteCode.h>
#include <oGfxTestVSByteCode.h>

static const oIAELEMENT IAElements[] = 
{
	{ "POSITION", 0, oSURFACE_R32G32B32_FLOAT, 0 },
	{ "TEXCOORD", 0, oSURFACE_R32G32_FLOAT, 0 },
};

bool oD3D11GetPipelineDesc(oGFXTEST_PIPELINE _Pipeline, oGfxPipeline::DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(*_pDesc));
	switch (_Pipeline)
	{
		case oGFX_FOWARD_COLOR:
			_pDesc->pElements = IAElements;
			_pDesc->NumElements = oCOUNTOF(IAElements);
			_pDesc->pVSByteCode = oGfxTestVSByteCode;
			_pDesc->pPSByteCode = oGfxTestPSByteCode;
			return true;
		default:
			break;
	}

	return false;
}