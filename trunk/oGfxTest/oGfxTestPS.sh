// $(header)
#include "oGfxTestHLSL.h"

PSOUT main(VSOUT In) : SV_Target
{
	PSOUT Out = (PSOUT)0;
	Out.Color = oRED;
	return Out;
}
