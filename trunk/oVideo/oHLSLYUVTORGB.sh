// $(header)
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