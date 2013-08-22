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

struct VSOUT
{
	float4 Color : SV_Target;
};

oGFX_GBUFFER_FRAGMENT main(oGFX_VS_OUT_LIT In)
{
	oGFX_GBUFFER_FRAGMENT Out = (oGFX_GBUFFER_FRAGMENT)0;	

	float4 Diffuse = oGfxColorSample(In.Texcoord);

	float3 E = oGfxGetEyePosition();
	float3 EyeVector = normalize(E - In.WSPosition);
	oGfxLight Light = oGfxGetLight(0);
	float3 L = normalize(Light.WSPosition - In.WSPosition);
	float3 HalfVector = normalize(EyeVector + L);

  float3 WSUnnormalizedNormal = oDecodeTSNormal(In.WSTangent, In.WSBitangent, In.WSNormal
		, oGfxNormalSample(In.Texcoord).xyz
		, 1.0f); // FIXME: Figure out what mask channel this comes from
		
	
	oRGBf rgb = oPhongShade(normalize(WSUnnormalizedNormal)
									, L
									, EyeVector
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
	Out.VSNormalXY = oFullToHalfSphere(normalize(In.VSNormal));
	Out.LinearDepth = oGfxNormalizeDepth(In.VSDepth);
	Out.Mask0 = float4(0,0,0,0);
	return Out;
}
