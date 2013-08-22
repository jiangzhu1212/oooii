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
#include "oGfxCommon.h"
#include <oCompute/oComputePhong.h>

Texture2D DiffuseTexture : register(t0);
SamplerState Sampler : register(s0);

oGFX_GBUFFER_FRAGMENT main(oGFX_VS_OUT_LIT In)
{
	float4 Diffuse = DiffuseTexture.Sample(Sampler, In.Texcoord);
	
	float3 E = oGfxGetEyePosition();
	float3 ViewVector = normalize(E - In.WSPosition);
	oGfxLight Light = oGfxGetLight(0);
	float3 L = normalize(Light.WSPosition - In.WSPosition);

	oGFX_GBUFFER_FRAGMENT Out = (oGFX_GBUFFER_FRAGMENT)0;	
	oRGBf rgb = oPhongShade(normalize(In.WSNormal)
									, L
									, ViewVector
									, 1
									, oBLACK3
									 // Figure out where these other parameters come from in the mask texture
									, oBLACK3
									, Diffuse.rgb
									, oBLACK3
									, oZERO3
									, oZERO3
									, oWHITE3
									, 1
									, 0.1f);

	Out.Color = float4(rgb, 1);
	return Out;
}
