// $(header)
#include <oGfx/oGfxState.h>

#define GFX_STATE_ASSTRING(_Count, _State) \
	oSTATICASSERT(_Count == oCOUNTOF(sStrings)); \
	oASSERT(_State >= 0 && _State < _Count, "Out of range"); \
	return sStrings[_State];

const char* oAsString(const oOMSTATE& _OMState)
{
	static const char* sStrings[] = 
	{
		"oOMOPAQUE",
		"oOMTEST",
		"oOMACCUMULATE",
		"oOMADDITIVE",
		"oOMTRANSLUCENT",
	};
	GFX_STATE_ASSTRING(oOMNUMSTATES, _OMState);
}

const char* oAsString(const oRSSTATE& _RSState)
{
	static const char* sStrings[] = 
	{
		"oRSFRONTFACE",
		"oRSBACKFACE",
		"oRSTWOSIDEDFACE",
		"oRSFRONTWIRE",
		"oRSBACKWIRE",
		"oRSTWOSIDEDWIRE",
		"oRSFRONTPOINTS",
		"oRSBACKPOINTS",
		"oRSTWOSIDEDPOINTS",
	};
	GFX_STATE_ASSTRING(oRSNUMSTATES, _RSState);
}

const char* oAsString(const oDSSTATE& _DSState)
{
	static const char* sStrings[] = 
	{
		"oDSNONE",
		"oDSTESTANDWRITE",
		"oDSTEST",
	};
	GFX_STATE_ASSTRING(oDSNUMSTATES, _DSState);
}

const char* oAsString(const oSASTATE& _SAState)
{
	static const char* sStrings[] = 
	{
		"oSAPOINTCLAMP",
		"oSAPOINTWRAP",
		"oSALINEARCLAMP",
		"oSALINEARWRAP",
		"oSAANISOCLAMP",
		"oSAANISOWRAP",
	};
	GFX_STATE_ASSTRING(oSANUMSTATES, _SAState);
}

const char* oAsString(const oMBSTATE& _MBState)
{
	static const char* sStrings[] = 
	{
		"oMBNONE",
		"oMBUP1",
		"oMBUP2",
		"oMBDOWN1",
		"oMBDOWN2",
	};
	GFX_STATE_ASSTRING(oMBNUMSTATES, _MBState);
}
