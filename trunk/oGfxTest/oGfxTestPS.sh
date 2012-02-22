// $(header)
#include "oGfxTestHLSL.h"

PSOUT main(VSOUT In) : SV_Target
{
	float3 L = normalize(float3(2, 2, -4));

	float3 E = oGetEyePosition(DeferredViewConstants.View);

	PSOUT Out = (PSOUT)0;
	Out.Color = oPhongShade(normalize(In.NormalWS)
									, -L
									, normalize(E - In.PositionWS)
									, 1
									, oRED
									, oBLACK
									, oBLUE
									, oWHITE
									, 128
									, float4(0,0,0,0)
									, float4(0,0,0,0)
									, oWHITE.xyz
									, 1);
	return Out;
}
