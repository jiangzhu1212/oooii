// $(header)
#include "oGfxTestHLSL.h"

VSOUT main(VSININSTANCED In)
{
	VSOUT Out = (VSOUT)0;

	Out.WSPosition = oQRotate(In.Rotation, In.LSPosition) + In.Translation;
	Out.SSPosition = oMul(DeferredViewConstants.ViewProjection, float4(Out.WSPosition, 1));
	Out.WSNormal = oQRotate(In.Rotation, In.LSNormal);

	if (In.InstanceID == 0)
		Out.Color = oRED;
	else if (In.InstanceID == 1)
		Out.Color = oGREEN;
	else if (In.InstanceID == 2)
		Out.Color = oBLUE;
	else 
		Out.Color = oWHITE;

	return Out;
}
