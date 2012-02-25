// $(header)
#include "oGfxTestHLSL.h"

VSOUT main(VSIN In)
{
	VSOUT Out = (VSOUT)0;
	Out.SSPosition = oGfxCalculateScreenSpacePosition(In.LSPosition);
	Out.WSPosition = oMul(oGfxDraw.World, float4(In.LSPosition,1)).xyz;
	Out.WSNormal = oMul(oGfxDraw.World, float4(In.LSNormal,1)).xyz;
	Out.Color = oYELLOW;
	return Out;
}
