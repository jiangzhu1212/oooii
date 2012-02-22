// $(header)
#include "oGfxTestHLSL.h"

PSOUT main(VSOUT In) : SV_Target
{
	float3 E = oGetEyePosition(DeferredViewConstants.View);
	float3 L = normalize(E);

	PSOUT Out = (PSOUT)0;
	Out.Color = oPhongShade(normalize(In.WSNormal)
									, -L
									, normalize(In.WSPosition - E)
									, 1
									, oRED
									, oBLACK
									, oBLUE
									, oWHITE
									, 128
									, oZERO
									, oZERO
									, oWHITE.xyz
									, 1);
	//Out.Color = oBLUE;
	return Out;
}
