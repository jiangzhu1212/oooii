// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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

bool oHTTPServerCreate(const oHTTPServer::DESC& _Desc, oHTTPServer** _ppServer);

#endif // oHTTPServer_h
