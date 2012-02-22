// $(header)
#include "oGfxTestHLSL.h"

VSLINEOUT main(VSLINEIN In)
{
	VSLINEOUT Out = (VSLINEOUT)0;
	Out.SSPosition = oGfxCalculateScreenSpacePosition(In.LSPosition);
	Out.Color = In.Color;
	return Out;
}
