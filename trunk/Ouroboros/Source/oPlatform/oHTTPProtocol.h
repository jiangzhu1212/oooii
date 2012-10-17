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
#pragma once
#ifndef oHTTPProtocol_h
#define oHTTPProtocol_h

#include <oPlatform/oHTTP.h>
#include <oPlatform/oSocket.h>

class oHTTPProtocol
{
public:
	enum oHTTPProtocolMode
	{
		oHTTP_PROTOCOL_CLIENT_MODE,
		oHTTP_PROTOCOL_SERVER_MODE,
	};
	struct DESC 
	{
		DESC(oHTTPProtocolMode _Mode)
			: Version(oHTTP_1_1)
			, SupportedMethods(oHTTP_GET | oHTTP_HEAD | oHTTP_POST)
			, Mode(_Mode)
		{}
		oHTTP_VERSION Version;
		unsigned int SupportedMethods;
		oHTTPProtocolMode Mode;

		oFUNCTION<void(const oHTTP_REQUEST& _Request, oHTTP_RESPONSE* _pResponse)> StartResponse;
		oFUNCTION<void(const void* _pContentBody)> FinishResponse;
	};

	oHTTPProtocol(const DESC& _Desc)
		: Desc(_Desc)
		, State(oSTATE_WAIT_FOR_REQUEST)
		, bPrepareToCloseSocket(false)
		, bCallFinishResponse(false)
		, SendsInProgress(0)
		, TheBody(nullptr)
	{}


	bool ProcessSocketReceive(void* _pData, unsigned int _SizeData, interface oSocket* _pSocket);
	bool ProcessSocketSend(void* _pHeader, void* _pBody, unsigned int _SizeData, interface oSocket* _pSocket);

	DESC Desc;

private:

	enum oState
	{
		oSTATE_WAIT_FOR_REQUEST,
		oSTATE_RECEIVE_HEADER,
		oSTATE_PARSE_HEADER,
		oSTATE_PROCESS_HEADER,
		oSTATE_RECEIVE_CONTENT_BODY,
		oSTATE_START_RESPONSE,
		oSTATE_PROCESS_RESPONSE,
		oSTATE_SEND_RESPONSE,
		oSTATE_WAIT_FOR_CLOSE,
	};

	// State
	oState State;
	bool bPrepareToCloseSocket;
	bool bCallFinishResponse;
	oStd::atomic_int SendsInProgress;

	// Application and transport layer interface
	oHTTPRequestInternal TheRequest;
	oHTTPResponseInternal TheResponse;

	// HTTP layer 
	oStringXXL TheHeader;
#ifdef SUSPECTED_BUGS_IN_HTTP
	oGuardBand<64> HeaderGuard;
#endif
	size_t TheHeaderPos;
	void* TheBody;
	size_t TheBodyPos;
	oStringL DefaultResponseBody;
#ifdef SUSPECTED_BUGS_IN_HTTP
	oGuardBand<64> BodyGuard;
#endif
};

#endif // oHTTPProtocol_h
