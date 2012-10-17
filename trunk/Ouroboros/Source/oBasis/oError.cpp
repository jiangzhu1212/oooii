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
#include <oBasis/oError.h>
#include <oBasis/oFunction.h>
#include <oBasis/oMacros.h>
#include <oBasis/oStdThread.h>
#include <oBasis/oString.h>
#include <oBasis/oThread.h>
#include <oBasis/oFixedString.h>

const char* oAsString(oERROR _Error)
{
	switch (_Error)
	{
		case oERROR_NONE: return "oERROR_NONE";
		case oERROR_GENERIC: return "oERROR_GENERIC";
		case oERROR_NOT_FOUND: return "oERROR_NOT_FOUND";
		case oERROR_REDUNDANT: return "oERROR_REDUNDANT";
		case oERROR_CANCELED: return "oERROR_CANCELED";
		case oERROR_AT_CAPACITY: return "oERROR_AT_CAPACITY";
		case oERROR_END_OF_FILE: return "oERROR_END_OF_FILE";
		case oERROR_WRONG_THREAD: return "oERROR_WRONG_THREAD";
		case oERROR_BLOCKING: return "oERROR_BLOCKING";
		case oERROR_TIMEOUT: return "oERROR_TIMEOUT";
		case oERROR_INVALID_PARAMETER: return "oERROR_INVALID_PARAMETER";
		case oERROR_TRUNCATED: return "oERROR_TRUNCATED";
		case oERROR_IO: return "oERROR_IO";
		case oERROR_REFUSED: return "oERROR_REFUSED";
		case oERROR_PLATFORM: return "oERROR_PLATFORM";
		case oERROR_CORRUPT: return "oERROR_CORRUPT";
		case oERROR_LEAKS: return "oERROR_LEAKS";
		oNODEFAULT;
	}
}

const char* oErrorGetDefaultString(oERROR _Error)
{
	switch (_Error)
	{
		case oERROR_NONE: return "operation was successful";
		case oERROR_GENERIC: return "operation failed";
		case oERROR_NOT_FOUND: return "object not found";
		case oERROR_REDUNDANT: return "redundant operation";
		case oERROR_CANCELED: return "operation canceled";
		case oERROR_AT_CAPACITY: return "storage is at capacity";
		case oERROR_END_OF_FILE: return "end of file";
		case oERROR_WRONG_THREAD: return "operation performed on wrong thread";
		case oERROR_BLOCKING: return "operation would block";
		case oERROR_TIMEOUT: return "operation timed out";
		case oERROR_INVALID_PARAMETER: return "invalid parameter specified";
		case oERROR_TRUNCATED: return "string truncated";
		case oERROR_IO: return "IO error occurred";
		case oERROR_REFUSED: return "access refused";
		case oERROR_PLATFORM: return "platform error occurred";
		case oERROR_CORRUPT: return "data is corrupt";
		case oERROR_LEAKS: return "resource(s) are not cleaned up";
		oNODEFAULT;
	}
}

struct ERROR_CONTEXT
{
	static const size_t ERROR_STRING_BUFFER_SIZE = 2048;

	size_t ErrorCount;
	oERROR Error;
	char ErrorString[ERROR_STRING_BUFFER_SIZE];
	bool UseDefaultString;
};

ERROR_CONTEXT* GetErrorContext()
{
	static thread_local ERROR_CONTEXT* pErrorContext = nullptr;
	if(!pErrorContext)
	{
		// {99091828-104D-4320-92C9-FD41810C352D}
		static const oGUID GUIDErrorContext = { 0x99091828, 0x104d, 0x4320, { 0x92, 0xc9, 0xfd, 0x41, 0x81, 0xc, 0x35, 0x2d } };

		if (oThreadlocalMalloc(GUIDErrorContext, &pErrorContext))
		{
			pErrorContext->ErrorCount = 0;
			pErrorContext->Error = oERROR_NONE;
			pErrorContext->ErrorString[0] = 0;
			pErrorContext->UseDefaultString = false;
		}
	}

	return pErrorContext;
}

bool oErrorSetLast(oERROR _Error)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	pErrorContext->ErrorCount++;
	pErrorContext->Error = _Error;
	pErrorContext->UseDefaultString = true;
	return false;
}

bool oErrorSetLastV(oERROR _Error, const char* _Format, va_list _Args)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	pErrorContext->ErrorCount++;
	pErrorContext->Error = _Error;
	pErrorContext->UseDefaultString = false;
	_Format = _Format ? _Format : oErrorGetDefaultString(_Error);
	oAddTruncationElipse(pErrorContext->ErrorString, oCOUNTOF(pErrorContext->ErrorString));
	oVPrintf(pErrorContext->ErrorString, _Format, _Args);
	return false;
}

bool oErrorPrefixLastV(const char* _Format, va_list _Args)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	oStringXL CurrentCopy;
	oPrintf(CurrentCopy, pErrorContext->ErrorString);

	oVPrintf(pErrorContext->ErrorString, _Format, _Args);
	oStrAppendf(pErrorContext->ErrorString, "%s", CurrentCopy);
	return false;
}

size_t oErrorGetLastCount()
{
	return GetErrorContext()->ErrorCount;
}

size_t oErrorGetSizeofMessageBuffer()
{
	return ERROR_CONTEXT::ERROR_STRING_BUFFER_SIZE;
}

oERROR oErrorGetLast()
{
	return GetErrorContext()->Error;
}

const char* oErrorGetLastString()
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	return (oSTRVALID(pErrorContext->ErrorString) && !pErrorContext->UseDefaultString) ? pErrorContext->ErrorString : oErrorGetDefaultString(oErrorGetLast());
}
