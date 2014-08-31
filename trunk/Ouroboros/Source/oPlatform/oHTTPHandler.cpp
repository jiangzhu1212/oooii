// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oHTTPHandler.h>
#include <oBasis/oRTTI.h>

using namespace ouro;

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oHTTPHandlerCommonQueryParams)
	oRTTI_COMPOUND_ABSTRACT(oHTTPHandlerCommonQueryParams)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oHTTPHandlerCommonQueryParams)
		oRTTI_COMPOUND_ATTR(oHTTPHandlerCommonQueryParams, Range, oRTTI_OF(int2), "range", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oHTTPHandlerCommonQueryParams, Count, oRTTI_OF(bool), "count", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oHTTPHandlerCommonQueryParams)
oRTTI_COMPOUND_END_DESCRIPTION(oHTTPHandlerCommonQueryParams)

void oHTTPHandlerBuildStaticFileResponse(const char* _FullPath, oHTTPHandler::CommonParams& _CommonParams)
{
	try
	{
		scoped_allocation alloc = filesystem::load(path(_FullPath));
		_CommonParams.AllocateResponse(alloc.size());
		memcpy(_CommonParams.pResponse->Content.pData, alloc, alloc.size());
	}
	catch (std::exception&)
	{
		return;
	}
	oMIMEFromExtension(&_CommonParams.pResponse->Content.Type, path(_FullPath).extension());
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_OK;
}

void oHTTPHandlerErrorV(oHTTPHandler::CommonParams& _CommonParams, oHTTP_STATUS_CODE _HTTPErrorCode, const char* _Format, va_list _Args)
{ 
	xxlstring buildString;
	vsnprintf(buildString, _Format, _Args);

	_CommonParams.AllocateResponse(buildString.length());
	memcpy(_CommonParams.pResponse->Content.pData, buildString.c_str(), buildString.length());
	_CommonParams.pResponse->Content.Type = oMIME_TEXT_PLAIN;
	_CommonParams.pResponse->StatusLine.StatusCode = _HTTPErrorCode;
}

void oHTTPHandlerError(oHTTPHandler::CommonParams& _CommonParams, oHTTP_STATUS_CODE _HTTPErrorCode, const char* _Format, ...)
{ 
	va_list args;
	va_start(args, _Format);
	oHTTPHandlerErrorV(_CommonParams, _HTTPErrorCode, _Format, args);
	va_end(args);
}