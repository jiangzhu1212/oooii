// $(header)
// Common fixed-function pipeline states used throughout oGfx.
#pragma once
#ifndef oGfxState_h
#define oGfxState_h

#include <oBasis/oSurface.h>

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

struct oIAELEMENT
{
	const char* Name;
	uint Index;
	oSURFACE_FORMAT Format;
	uint InputSlot;
};

oAPI uint oGfxCalcInterleavedVertexSize(const oIAELEMENT* _pElements, size_t _NumElements, uint _InputSlot);
template<size_t size> uint oGfxCalcInterleavedVertexSize(const oIAELEMENT (&_pElements)[size], uint _InputSlot) { return oGfxCalcInterleavedVertexSize(_pElements, size, _InputSlot); }

oAPI const char* oAsString(const oOMSTATE& _OMState);
oAPI const char* oAsString(const oRSSTATE& _RSState);
oAPI const char* oAsString(const oDSSTATE& _DSState);
oAPI const char* oAsString(const oSASTATE& _SAState);
oAPI const char* oAsString(const oMBSTATE& _MBState);

#endif
