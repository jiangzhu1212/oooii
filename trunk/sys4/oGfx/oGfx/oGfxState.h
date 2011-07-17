// $(header)
#pragma once
#ifndef oGfxState_h
#define oGfxState_h

enum oOMSTATE // Output Merge (Blend) State
{
	oOMOPAQUE, // Output.rgba = Source.rgba
	oOMTEST, // Same as oOMOPAQUE, test is done in user code
	oOMACCUMULATE, // Output.rgba = Source.rgba + Destination.rgba
	oOMADDITIVE, // Output.rgb = Source.rgb * Source.a + Destination.rgb
	oOMTRANSLUCENT, // Output.rgb = Source.rgb * Source.a + Destination.rgb * (1 - Source.a)
	oOMNUMSTATES,
};

enum oRSSTATE // Rasterizer State
{
	oRSFRONTFACE,
	oRSBACKFACE,
	oRSTWOSIDEDFACE,
	oRSFRONTWIRE,
	oRSBACKWIRE,
	oRSTWOSIDEDWIRE,
	oRSFRONTPOINTS,
	oRSBACKPOINTS,
	oRSTWOSIDEDPOINTS,
	oRSNUMSTATES,
};

enum oDSSTATE // Depth-Stencil State
{
	oDSNONE,
	oDSTESTANDWRITE,
	oDSTEST,
	oDSNUMSTATES,
};

enum oSASTATE // Sampler State
{
	oSAPOINTCLAMP,
	oSAPOINTWRAP,
	oSALINEARCLAMP,
	oSALINEARWRAP,
	oSAANISOCLAMP,
	oSAANISOWRAP,
	oSANUMSTATES,
};

enum oMBSTATE // Mip-Bias State
{
	oMBNONE,
	oMBUP1,
	oMBUP2,
	oMBDOWN1,
	oMBDOWN2,
	oMBNUMSTATES,
};

#endif
