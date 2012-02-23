/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oGfx/oGfxState.h>
#include <oBasis/oAssert.h>
#include <oBasis/oMacros.h>

#define GFX_STATE_ASSTRING(_Count, _State) \
	static_assert(_Count == oCOUNTOF(sStrings), #_State " counts don't match"); \
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

uint oGfxCalcInterleavedVertexSize(const oIAELEMENT* _pElements, size_t _NumElements, uint _InputSlot)
{
	bool IsFirstRun = true;
	bool IsInstanceList = false;

	uint size = 0;
	for (size_t i = 0; i < _NumElements; i++)
	{
		if (_InputSlot == _pElements[i].InputSlot)
		{
			if (IsFirstRun)
			{
				IsInstanceList = _pElements[i].Instanced;
				IsFirstRun = false;
			}

			else
				oASSERT(IsInstanceList == _pElements[i].Instanced, "Elements in the same slot must either be all instanced or all not instanced.");

			size += oSurfaceFormatGetSize(_pElements[i].Format);
		}
	}
	return size;
}
