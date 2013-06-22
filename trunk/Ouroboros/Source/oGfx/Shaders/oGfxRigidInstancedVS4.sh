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
#include "oGfxCommon.h"

oGFX_VS_OUT_LIT main(oGFX_RIGID_VERTEX_INSTANCED In)
{
	oGFX_VS_OUT_LIT Out = (oGFX_VS_OUT_LIT)0;
	Out.SSPosition = oGfxWStoSS(Out.WSPosition);
	Out.WSPosition = qmul(In.Rotation, In.LSPosition) + In.Translation;
	Out.LSPosition = In.LSPosition;
	Out.VSDepth = oGfxWStoVS(Out.WSPosition).z;
	oQRotateTangentBasisVectors(In.Rotation, In.LSNormal, In.LSTangent, Out.WSNormal, Out.WSTangent, Out.WSBitangent);
	Out.VSNormal = oGfxRotateWStoVS(qmul(In.Rotation, In.LSNormal));
	Out.Texcoord = In.Texcoord;
	return Out;
}
