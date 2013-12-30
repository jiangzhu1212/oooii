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
#ifndef oHTTPClient_h
#define oHTTPClient_h

#include <oPlatform/oHTTP.h>
#include <oPlatform/oSocket.h>

// Connects to a server maintaining a connection
// in HTTP 1.1 style
interface oHTTPClient : oInterface
{
	struct DESC 
	{
		DESC()
			: TimeoutMS(ouro::infinite)
		{}
		oNetAddr ServerAddr; // TODO: Connect to clients with hostname instead, since it might be a virtual host on the server at the target IP address
		unsigned int TimeoutMS;
	};

	virtual void GetDesc(DESC* _pDesc) = 0;

	virtual bool StartRequest(oHTTP_METHOD _Method, const char* _pRelativePath, oHTTP_REQUEST** _ppRequest) = 0;
	virtual bool FinishRequest(oHTTP_RESPONSE** _ppResponse, void* _pResponseBuffer = nullptr, unsigned int _MaxResponseBufferSize = 0) = 0;

	template<int _MaxBufferSize>
	bool FinishRequest(oHTTP_RESPONSE** _ppResponse, char (&_ResponseBuffer)[_MaxBufferSize])
	{
		return FinishRequest(_ppResponse, _ResponseBuffer, _MaxBufferSize);
	}

	virtual bool Head(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse) = 0;
	virtual bool Get(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize) = 0;
	virtual bool Post(const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _SizeofData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize) = 0;
	virtual bool Put(const char* _pRelativePath, oMIME_TYPE _DataType, const void* _pData, unsigned int _SizeofData, oHTTP_RESPONSE* _pResponse, void* _pBuffer, unsigned int _MaxBufferSize) = 0;
	virtual bool Delete(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse) = 0;

	template<int _MaxBufferSize>
	bool Get(const char* _pRelativePath, oHTTP_RESPONSE* _pResponse, char (&_StrDestination)[_MaxBufferSize])
	{
		bool result = Get(_pRelativePath, _pResponse, _StrDestination, _MaxBufferSize);
		if(!result)
			return false;

		if((_pResponse->Content.Length + 4) >= _MaxBufferSize)
			return false;

		memset(oStd::byte_add(_StrDestination, _pResponse->Content.Length), 0, 4);
		return true;
	}
};

oAPI bool oHTTPClientCreate(const oHTTPClient::DESC& _Desc, oHTTPClient** _ppClient);

#endif // oHTTPClient_h
