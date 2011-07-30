// $(header)

struct VSIN
{
	float2 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VSOUT
{
	float4 Position : SV_Position;
	float2 Texcoord : TEXCOORD0;
};

VSOUT main(VSIN In)
{
	VSOUT Out;
	Out.Position = float4(In.Position, 0, 1);
	Out.Texcoord = In.Texcoord;
	return Out;
}