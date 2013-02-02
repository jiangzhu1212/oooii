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
#include <oGPUHLSLCommon.h>

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
SamplerState Sampler : register(s0);

PSOUT main(VSOUT In) : SV_Target
{
	PSOUT Out = (PSOUT)0;	

	float4 Diffuse = DiffuseTexture.Sample(Sampler, In.Texcoord);

	float3 E = oGPUGetEyePosition();
	float3 EyeVector = normalize(E - In.WSPosition);
	float3 L = oGetSpelunkerLightPosition(E);
	float3 HalfVector = normalize(EyeVector + L);

  float3 WSUnnormalizedNormal = oDecodeTSNormal(In.WSTangent, In.WSBitangent, In.WSNormal
		, NormalTexture.Sample(Sampler, In.Texcoord).xyz
		, oGPU_MATERIAL_CONSTANT_BUFFER.BumpScale);
	
	float3 rgb = oPhongShade(normalize(WSUnnormalizedNormal)
									, L
									, EyeVector
									, 1
									, oBLACK3
									, oGPU_MATERIAL_CONSTANT_BUFFER.EmissiveColor
									, oGPU_MATERIAL_CONSTANT_BUFFER.DiffuseColor * Diffuse.rgb
									, oGPU_MATERIAL_CONSTANT_BUFFER.SpecularColor
									, oGPU_MATERIAL_CONSTANT_BUFFER.Specularity
									, oZERO3
									, oZERO3
									, oWHITE3
									, 1);

	Out.Color = float4(rgb, 1);
	return Out;
}
