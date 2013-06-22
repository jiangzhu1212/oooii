/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#ifndef oWebFileCache_h
#define oWebFileCache_h
#include <oPlatform/oHTTPServer.h>
#include <oPlatform/oHTTPHandler.h>

//Serves static and dynamic content for an http server. optionally caches static content in memory for performance
//	only reads known mime types for static content. and will only read files relative to a base path. the relative paths are not allowed to contain ".."
//	for security reasons.
//	favicon will also be handled
//	dynamic handlers get looked at first, then if no handler is found, will look for at static content. If that is not found either, then 
//	Retrieve will fill the response with an appropriate error code.

// {1d2fddfa-2343-4d21-b909-c819882cbc81}
oDEFINE_GUID_I(oWebServer, 0x1d2fddfa, 0x2343, 0x4d21, 0xb9, 0x09, 0xc8, 0x19, 0x88, 0x2c, 0xbc, 0x81);
interface oWebServer : oInterface
{
	struct DESC
	{
		DESC() : DisableCache(false), DefaultURIReference("index.html") {}

		bool DisableCache; // If true, every request will freshly load the file, i.e. no caching.
		oStd::uri_string URIBase; //static content base location as a uri, i.e. file://DATA/webstuff/
		oStd::uri_string DefaultURIReference; //If a request comes in, and it is empty. i.e. http://localhost/ then it will get redirected to this uri. omit the http://localhost/. 
		oFUNCTION<void* (size_t _RequiredBufferSize)> AllocBufferCallback;
	};

	// note that once Retrieve has been called, any future calls to AddHTTPHandler will be ignored (and will assert in debug)
	//	these are for handling dynamic content
	virtual void AddHTTPHandler(oHTTPHandler *_Handler) threadsafe = 0;
	virtual void AddURICaptureHandler(oHTTPURICapture *_CaptureHandler) threadsafe = 0;

	//Will handle both dynamic content (pages) and static content found under URIBase. AllocBufferCallback
	//	will get called to allocate the appropriate buffer size
	virtual bool Retrieve(const oHTTP_REQUEST& _RequestType, oHTTP_RESPONSE* _pResponse) threadsafe = 0;
};	

bool oWebServerCreate(const oWebServer::DESC& _Desc, threadsafe oWebServer** _ppObject);

#endif
