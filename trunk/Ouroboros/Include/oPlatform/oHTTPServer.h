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
#ifndef oHTTPServer_h
#define oHTTPServer_h

#include <oPlatform/oHTTP.h>
#include <oPlatform/oSocket.h>

// An HTTP 1.1 server that listens on the supplied port and calls StartResponse whenever a request is sent to it.
// The user will fill in the oHTTP_RESPONSE struct, of which a pointer is supplied in the StartResponse callback.
// If the user wants to send a Content Body (MIMEData) it can set a pointer to that data through _ppMIMEData.
// This pointer must remain valid, so can not be freed, until the FinishResponse callback occurs.
// Set the methods that you are able to handle in StartResponse in SupportedMethods, all other requested methods
// won't get a StartResponse callback, but will instead get an automatic oHTTP_NOT_IMPLEMENTED response.
interface oHTTPServer : oInterface
{
	struct DESC 
	{
		DESC()
			: ConnectionTimeoutMS(1000)
			, SupportedMethods(oHTTP_GET | oHTTP_HEAD | oHTTP_POST)
		{}
		unsigned short Port;
		unsigned int ConnectionTimeoutMS;
		unsigned int SupportedMethods;

		std::function<void(const oHTTP_REQUEST& _Request, const oNetHost& _Client, oHTTP_RESPONSE* _pResponse)> StartResponse;
		std::function<void(const void* _pContentBody)> FinishResponse;
	};

	virtual void GetDesc(DESC* _pDesc) = 0;
};

oAPI bool oHTTPServerCreate(const oHTTPServer::DESC& _Desc, oHTTPServer** _ppServer);

#endif // oHTTPServer_h
