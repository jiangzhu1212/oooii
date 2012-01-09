// $(header)
#include <oBasis/oError.h>
#include <oBasis/oFunction.h>
#include <oBasis/oMacros.h>
#include <oBasis/oStdThread.h>
#include <oBasis/oString.h>
#include <oBasis/oThread.h>

const char* oAsString(const oERROR& _Error)
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
		oNODEFAULT;
	}
}

const char* oErrorGetDefaultString(const oERROR& _Error)
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
		oNODEFAULT;
	}
}

struct ERROR_CONTEXT
{
	size_t ErrorCount;
	oERROR Error;
	char ErrorString[2048];
};

ERROR_CONTEXT* GetErrorContext()
{
	static thread_local ERROR_CONTEXT* pErrorContext = nullptr;
	if(!pErrorContext)
	{
		// {99091828-104D-4320-92C9-FD41810C352D}
		static const oGUID GUIDErrorContext = { 0x99091828, 0x104d, 0x4320, { 0x92, 0xc9, 0xfd, 0x41, 0x81, 0xc, 0x35, 0x2d } };
		pErrorContext = (ERROR_CONTEXT*)oThreadlocalMalloc(GUIDErrorContext, sizeof(ERROR_CONTEXT));
		pErrorContext->ErrorCount = 0;
		pErrorContext->Error = oERROR_NONE;
		pErrorContext->ErrorString[0] = 0;
	}

	return pErrorContext;
}

bool oErrorSetLastV(oERROR _Error, const char* _Format, va_list _Args)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	pErrorContext->ErrorCount++;
	pErrorContext->Error = _Error;
	_Format = _Format ? _Format : oErrorGetDefaultString(_Error);
	oAddTruncationElipse(pErrorContext->ErrorString, oCOUNTOF(pErrorContext->ErrorString));
	vsprintf_s(pErrorContext->ErrorString, _Format, _Args);
	return false;
}

size_t oErrorGetLastCount()
{
	return GetErrorContext()->ErrorCount;
}

oERROR oErrorGetLast()
{
	return GetErrorContext()->Error;
}

const char* oErrorGetLastString()
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	return oSTRVALID(pErrorContext->ErrorString) ? pErrorContext->ErrorString : oErrorGetDefaultString(oErrorGetLast());
}
