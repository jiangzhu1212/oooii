// $(header)
#include "oGfxTestHLSL.h"

VSOUT main(VSIN In)
{
	VSOUT Out = (VSOUT)0;
	Out.ScreenSpacePosition = oGfxCalculateScreenSpacePosition(In.Position);
	Out.PositionWS = oMul(oGfxDraw.World, float4(In.Position,1)).xyz;
	Out.NormalWS = oMul(oGfxDraw.World, float4(In.Normal,1)).xyz;
	return Out;
}
