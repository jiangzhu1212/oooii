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
#include <oPlatform/oHTTPClient.h>
#include <set>
#include "oHTTPInternal.h"
#include "oHTTPProtocol.h"


class oHTTPClient_Impl : public oHTTPClient
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oHTTPClient_Impl(DESC _Desc, bool *pSuccess);

	void GetDesc(DESC* _pDesc) override {* _pDesc = Desc; }

	bool StartRequest(oHTTP_METHOD _Method, const char* _pRelativePath, oHTTP_REQUEST** _ppRequest) override;
	bool FinishRequest(oHTTP_RESPONSE** _pResponse, void* _pResponseBuffer, unsigned int _MaxResponseBufferSize) override;

	bool Head(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse) override;
	bool Get(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize) override;
	bool Post(const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _szData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize) override;
	bool Put(const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _szData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize) override;
	bool Delete(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse) override;

private:
	bool PostPutImpl(oHTTP_METHOD _Method, const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _szData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize);

	oRefCount RefCount;
	DESC Desc;
	oRef<threadsafe oSocket> Socket;

	oHTTPProtocol Protocol;

	// Internal parsed data
	oHTTPRequestInternal TheRequest;
	oHTTPResponseInternal TheResponse;
	oStringXXL TheHeader;
};

oHTTPClient_Impl::oHTTPClient_Impl(DESC _Desc, bool *pSuccess)
	: Desc(_Desc)
	, Protocol(oHTTPProtocol::oHTTP_PROTOCOL_CLIENT_MODE)
{
	oSocket::DESC socketDesc;
	socketDesc.Addr = _Desc.ServerAddr;
	socketDesc.Protocol = oSocket::TCP;
	socketDesc.Style = oSocket::BLOCKING;
	socketDesc.ConnectionTimeoutMS = socketDesc.BlockingSettings.SendTimeout = socketDesc.BlockingSettings.RecvTimeout = _Desc.TimeoutMS;
	*pSuccess = oSocketCreate("HTTP Client", socketDesc, &Socket);
}

bool oHTTPClient_Impl::StartRequest(oHTTP_METHOD _Method, const char* _pRelativePath, oHTTP_REQUEST** _ppRequest)
{
	if (!Socket || !Socket->IsConnected())
		return oErrorSetLast(oERROR_TIMEOUT);

	oStringXXL requestPath;
	oURIPercentEncode(requestPath, _pRelativePath, " ");
	TheRequest.Reset(_Method, requestPath, oHTTP_1_1);
	if (_ppRequest)
		*_ppRequest = &TheRequest;

	// TODO: Make adding a Host: header field optional, and making it the actual hostname instead of just an IP address
	if (true)
	{
		oStringS hostname;
		oToString(hostname, Desc.ServerAddr);
		oHTTPAddHeader(TheRequest.HeaderFields, oHTTP_HEADER_HOST, hostname.c_str());
	}
	return true;
}

bool oHTTPClient_Impl::FinishRequest(oHTTP_RESPONSE** _pResponse, void* _pResponseBuffer, unsigned int _MaxResponseBufferSize)
{
	// Add content headers
	if (TheRequest.Content.Type != oMIME_UNKNOWN)
		oHTTPAddHeader(TheRequest.HeaderFields, oHTTP_HEADER_CONTENT_TYPE, oAsString(TheRequest.Content.Type));

	if (TheRequest.Content.Length)
		oHTTPAddHeader(TheRequest.HeaderFields, oHTTP_HEADER_CONTENT_LENGTH, oUInt(TheRequest.Content.Length));

	// Create request header
	oToString(TheHeader, TheRequest);

	// Send header and body
	if (TheRequest.Content.pData && TheRequest.Content.Length)
		Socket->Send(TheHeader.c_str(), (oSocket::size_t)TheHeader.length(), TheRequest.Content.pData, oUInt(TheRequest.Content.Length));
	else
		Socket->Send(TheHeader.c_str(), (oSocket::size_t)TheHeader.length());

	enum oState
	{
		oSTATE_WAIT_FOR_REQUEST,
		oSTATE_RECEIVE_HEADER,
		oSTATE_PARSE_HEADER,
		oSTATE_PROCESS_HEADER,
		oSTATE_RECEIVE_CONTENT_BODY,
		oSTATE_FINISHED,
	};
	oState State = oSTATE_WAIT_FOR_REQUEST;
	// Receive response (header and optional content body)
	char ReceiveBuffer[HTTPRequestSize];

	unsigned int SzReceived = Socket->Recv(ReceiveBuffer, HTTPRequestSize);

	size_t TheHeaderPos;
	void* TheBody = nullptr;
	size_t TheBodyPos;

	// Process all bytes until done
	size_t SzDataTaken = 0;
	while (true)
	{
		switch (State)
		{
		case oSTATE_WAIT_FOR_REQUEST:
			TheHeaderPos = 0;

			// Reset response
			TheResponse.Reset();

			State = oSTATE_RECEIVE_HEADER;
			break;

		case oSTATE_RECEIVE_HEADER:
			if (!oExtractHTTPHeader(oByteAdd((const char*)ReceiveBuffer, SzDataTaken), (SzReceived - SzDataTaken), TheHeader.c_str(), &TheHeaderPos, TheHeader.capacity(), &SzDataTaken))
			{
				// If the HTTP header is bigger than our capacity, then send an error back
				if (TheHeaderPos >= TheHeader.capacity())
				{
					TheResponse.StatusLine.StatusCode = oHTTP_REQUEST_ENTITY_TOO_LARGE;
					Socket = nullptr; // Close the socket
					State = oSTATE_FINISHED;
					break;
				}
				// oExtractHTTPHeader doesn't take any bytes if it can't extract a HTTP header (0 terminator before header end marker for example)
				if (SzReceived && !SzDataTaken)
				{
					TheResponse.StatusLine.StatusCode = oHTTP_BAD_REQUEST;
					Socket = nullptr; // Close the socket
					State = oSTATE_FINISHED;
					break;
				}

				// Request more data
				oASSERT(SzDataTaken==SzReceived, "Assuming that we took all data, before requesting more");

				SzReceived = Socket->Recv(ReceiveBuffer, HTTPRequestSize);

				// If we got called with 0 bytes, we either timed out or the connection is broken/closed/tearing down, check for that
				if (0 == SzReceived)
				{
					TheResponse.StatusLine.StatusCode = Socket->IsConnected() ? oHTTP_REQUEST_TIMEOUT : oHTTP_CLIENT_CLOSED_REQUEST;
					Socket = nullptr; // Close the socket
					State = oSTATE_FINISHED;
					break;
				}
				SzDataTaken = 0;
				break;
			}
			State = oSTATE_PARSE_HEADER;
			break;

		case oSTATE_PARSE_HEADER:
			// We received the header in full, now parse it
			if (!oFromString(&TheResponse, TheHeader.c_str()))
				return false;
			State = oSTATE_PROCESS_HEADER;
			break;

		case oSTATE_PROCESS_HEADER:
			{
				const char* pValue;
				unsigned int ValueUInt;

				// Process optional headers
				
				// Connection
				if (oHTTPFindHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONNECTION, &pValue))
				{
					char* ctx;
					char* pElement = oStrTok(pValue, ",", &ctx);
					while (pElement)
					{
						// Optimization so we can compare with lower case string matching, the actual header stays unchanged though
						oToLower(pElement);
						
						// keep-alive / close
						if (strstr(pElement, "close"))
							Socket = nullptr; // Close the socket

						pElement = oStrTok(nullptr, ",", &ctx);
					}
				}

				// Content-Type
				if (oHTTPFindHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONTENT_TYPE, &pValue))
					oFromString(&TheResponse.Content.Type, pValue);

				// Content-Length
				if (oHTTPFindHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONTENT_LENGTH, &ValueUInt))
					TheResponse.Content.Length = ValueUInt;

				if (TheRequest.RequestLine.Method != oHTTP_HEAD && TheResponse.Content.Length && _pResponseBuffer)
				{
					TheBody = _pResponseBuffer;
					TheBodyPos = 0;

					State = oSTATE_RECEIVE_CONTENT_BODY;
				}
				else
				{
					State = oSTATE_FINISHED;
				}
			}
			break;

		case oSTATE_RECEIVE_CONTENT_BODY:
			// If we got called with 0 bytes, we either timed out or the connection is broken/closed/tearing down, check for that
			if (0 == SzReceived)
			{
				TheResponse.StatusLine.StatusCode = Socket->IsConnected() ? oHTTP_REQUEST_TIMEOUT : oHTTP_CLIENT_CLOSED_REQUEST;
				Socket = nullptr; // Close the socket
				State = oSTATE_FINISHED;
				break;
			}
			if (!oExtractContent(oByteAdd(ReceiveBuffer, SzDataTaken), (SzReceived - SzDataTaken), TheBody, &TheBodyPos, oMin(_MaxResponseBufferSize, TheResponse.Content.Length), &SzDataTaken))
			{
				// Request more data
				oASSERT(SzDataTaken==SzReceived, "Assuming that we took all data, before requesting more");

				SzReceived = Socket->Recv(ReceiveBuffer, HTTPRequestSize);
				SzDataTaken = 0;
				break;
			}
			TheResponse.Content.pData = TheBody;
			if (_MaxResponseBufferSize < TheResponse.Content.Length)
			{
				TheResponse.StatusLine.StatusCode = oHTTP_REQUEST_ENTITY_TOO_LARGE;
				Socket = nullptr; // Instead of closing the Socket, we could also skip the remaining content length, but it might be a waste if it's really a lot more data to receive
			}
			State = oSTATE_FINISHED;
			break;

		case oSTATE_FINISHED:
			// FIXME: Not a great solution
			*_pResponse = &TheResponse;
			return true;
		}
	}
	return true;
}

bool oHTTPClient_Impl::Head(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse)
{
	oHTTP_RESPONSE* Response = nullptr;
	bool bResult = StartRequest(oHTTP_HEAD, _pRelativePath, nullptr) && FinishRequest(&Response, nullptr, 0);
	if (bResult) *_pResponse = *Response;
	return bResult;
}

bool oHTTPClient_Impl::Get(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize)
{
	oHTTP_RESPONSE* Response = nullptr;
	bool bResult = StartRequest(oHTTP_GET, _pRelativePath, nullptr) && FinishRequest(&Response, _pBuffer, _MaxBufferSize);
	if (bResult) *_pResponse = *Response;
	return bResult;
}

bool oHTTPClient_Impl::Post(const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _szData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize)
{
	return PostPutImpl(oHTTP_POST, _pRelativePath, _DataType, _pData, _szData, _pResponse, _pBuffer, _MaxBufferSize);
}

bool oHTTPClient_Impl::Delete(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse)
{
	oHTTP_RESPONSE* Response = nullptr;
	bool bResult = StartRequest(oHTTP_DELETE, _pRelativePath, nullptr) && FinishRequest(&Response, nullptr, 0);
	if (bResult) *_pResponse = *Response;
	return bResult;
}

bool oHTTPClient_Impl::Put(const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _szData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize)
{
	return PostPutImpl(oHTTP_PUT, _pRelativePath, _DataType, _pData, _szData, _pResponse, _pBuffer, _MaxBufferSize);
}

bool oHTTPClient_Impl::PostPutImpl(oHTTP_METHOD _Method, const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _szData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize)
{
	oHTTP_REQUEST* Request = nullptr;
	bool bResult = StartRequest(_Method, _pRelativePath, &Request);
	if (!bResult) return false;

	Request->Content.Type = _DataType;
	Request->Content.pData = (void*)_pData;
	Request->Content.Length = _szData;

	oHTTP_RESPONSE* Response = nullptr;
	bResult &= FinishRequest(&Response, _pBuffer, _MaxBufferSize);
	if (!bResult) return false;
	*_pResponse = *Response;
	return true;
}

oAPI bool oHTTPClientCreate(const oHTTPClient::DESC& _Desc, oHTTPClient** _ppClient)
{
	bool success = false;
	oCONSTRUCT(_ppClient, oHTTPClient_Impl(_Desc, &success));
	return success;
}
