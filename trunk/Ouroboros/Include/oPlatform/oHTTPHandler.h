/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#pragma once
#ifndef oWebPage_h
#define oWebPage_h
#include <oBasis/oInterface.h>
#include <oPlatform/oHTTP.h>
#include <oBase/types.h>

interface oHTTPURICapture : oInterface
{
	virtual ouro::sstring GetCaptureName() const = 0;
	virtual bool AttemptCapture(const char* _URI, const char** _Remaining) const = 0;
};

//For use with oWebServer. handles http requests for a certain path
interface oHTTPHandler : oInterface
{
	//This should return a pattern that will be used for matching a uri request to a handler.
	// should return a string that looks like "streams/?8/commands/?i
	// literal strings like "streams" above will get matched exactly against a incoming uri.
	// strings marked with ? will be captured. a single osc tag should exist per? between a set of /'s. 
	// only the osc tags i, h, f, d, c, and the 1-9 fixed strings are supported at this time.
	// If you get call you can from within your handler call GetCaptured, and give a struct that matches the 
	// osc tags to get the portion of the uri that you want. the osc tags in HandlesPath are just concatenated in order
	// to describe the layout of the struct you pass to GetCaptured.
	virtual ouro::lstring HandlesPath() const = 0; 
	
	struct CommonParams
	{
		oHTTP_RESPONSE* pResponse;
		//will assert if called more than once in an "OnX" function. 
		// sets pResponse->Content.pData to the newly allocated memory. pResponse->Content.Length will also be set
		// Buffer will be freed automatically.
		oFUNCTION<void (size_t _RequiredBufferSize)> AllocateResponse;

		oFUNCTION<bool (void* _Struct, int _SizeOfStruct)> GetCapturedImpl; //generally don't use this directly, use the below template
		//the struct passed must match the layout of the osc portions of your HandlesPath string. struct members of the optional 
		//	trailing osc tags could get unmodified by this call, but still succeed.
		template<typename StructType>
		bool GetCaptured(StructType* _arg)
		{
			return GetCapturedImpl(_arg, sizeof(StructType));
		}

		const char* Query;
	};

	virtual void OnGet(CommonParams& _CommonParams) const = 0;
	virtual void OnPost(CommonParams& _CommonParams, const oHTTP_CONTENT_BODY& _Content) const = 0;
	virtual void OnPut(CommonParams& _CommonParams, const oHTTP_CONTENT_BODY& _Content) const = 0;
	virtual void OnDelete(CommonParams& _CommonParams) const = 0;
};	

inline void oHTTPHandlerMethodNotAllowed(oHTTPHandler::CommonParams& _CommonParams)
{
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_METHOD_NOT_ALLOWED;
}

//some common querys shared by many of our rest services
struct oHTTPHandlerCommonQueryParams 
{
	oHTTPHandlerCommonQueryParams() : Range(oInvalid, oInvalid), Count(false) {}
	oHTTPHandlerCommonQueryParams(int2 _Range) : Range(_Range), Count(false) {}
	int2 Range;
	bool Count;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oHTTPHandlerCommonQueryParams)

//Helper for sending a static file. Normally use oWebServer's cache for this. but this function can be useful if you want to bypass that
//	for large files.
void oHTTPHandlerBuildStaticFileResponse(const char* _FullPath, oHTTPHandler::CommonParams& _CommonParams);

void oHTTPHandlerError(oHTTPHandler::CommonParams& _CommonParams, oHTTP_STATUS_CODE _HTTPErrorCode, const char* _Format, ...);

#endif
