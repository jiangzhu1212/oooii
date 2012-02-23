// $(header)
#include "oGfxTestHLSL.h"

VSOUT main(VSININSTANCED In)
{
	VSOUT Out = (VSOUT)0;

	Out.WSPosition = oQRotate(In.Rotation, In.LSPosition) + In.Translation;
	Out.SSPosition = oMul(DeferredViewConstants.ViewProjection, float4(Out.WSPosition, 1));
	Out.WSNormal = oQRotate(In.Rotation, In.LSNormal);
	return Out;
}
