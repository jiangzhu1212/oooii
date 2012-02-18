// $(header)
#include "oGfxTestHLSL.h"

VSOUT main(VSIN In)
{
	VSOUT Out = (VSOUT)0;
	Out.ScreenSpacePosition = oGfxCalculateScreenSpacePosition(In.Position);
	return Out;
}
