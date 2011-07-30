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
#include <oooii/oHLSL.h>

SamplerState BiLinearSample : register(s0);
Texture2D YLuminance : register(t0);
Texture2D UChrominance : register(t1);
Texture2D VChrominance : register(t2);

float4 main( float4 Pos : SV_Position, float2 TexCoord : TEXCOORD ) : SV_Target0
{
	//return float4( TexCoord, 0.0f, 1.0f );
	
	float Y = YLuminance.Sample( BiLinearSample, TexCoord ).x * 255;
	float U = UChrominance.Sample( BiLinearSample, TexCoord ).x * 255;
	float V = VChrominance.Sample( BiLinearSample, TexCoord ).x * 255;
	
	int C = Y - 16;
	int	D = U - 128;
	int	E = V - 128;
	
	int R = clamp(( 298 * C           + 409 * E + 128) >> 8, 0, 255 );
	int G = clamp(( 298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255 );
	int B = clamp(( 298 * C + 516 * D           + 128) >> 8, 0, 255 );
	
	return float4( R, G, B, 255.0f) / 255.0f;
}