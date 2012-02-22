// $(header)
#include "oGfxTestHLSL.h"

PSOUT main(VSLINEOUT In) : SV_Target
{
	PSOUT Out = (PSOUT)0;
	Out.Color = float4(1,1,1,1);//In.Color;
	return Out;
}
