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
#include <oPlatform/oHTTPServer.h>
#include "oHTTPInternal.h"
#include "oHTTPProtocol.h"
#include <oPlatform/oTCPServer.h>

namespace
{
	struct CallbackHandler : public oSocketAsyncCallback
	{
	public:
		oDEFINE_REFCOUNT_INTERFACE(Refcount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(CallbackHandler);

		CallbackHandler(const oHTTPServer::DESC& _Desc)
			: Desc(_Desc)
			, LastTimeStamp(0)
			, bCloseSocket(false)
			, Protocol(oHTTPProtocol::oHTTP_PROTOCOL_SERVER_MODE)
		{
			Protocol.Desc.StartResponse = nullptr;
			Protocol.Desc.FinishResponse = Desc.FinishResponse;
			Protocol.Desc.SupportedMethods = Desc.SupportedMethods;
		}

		~CallbackHandler() {}
		void InitiateReceive(threadsafe oSocket* _pSocket)
		{
			LastTimeStamp = ouro::timer::nowmsi();
			_pSocket->Recv(&ReceiveBuffer[0], static_cast<int>(oCOUNTOF(ReceiveBuffer)));
		}

		bool ShouldCloseSocket(const threadsafe oSocket* _pSocket) { return bCloseSocket || ouro::timer::nowmsi() - LastTimeStamp > HTTPRequestTimeoutMS; }

	private:
		virtual void ProcessSocketReceive(void* _pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe;
		virtual void ProcessSocketSend(void* _pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe;

		oRefCount Refcount;
		oHTTPServer::DESC Desc;

		char ReceiveBuffer[HTTPRequestSize];

		unsigned int LastTimeStamp;
		bool bCloseSocket;

		oHTTPProtocol Protocol;
	};
	
	void CallbackHandler::ProcessSocketReceive(void* _pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe
	{
		auto pLockedThis = thread_cast<CallbackHandler*>(this); // Safe because we only get called again when we want it to (when doing another async request)

		if (!pLockedThis->Protocol.Desc.StartResponse)
			pLockedThis->Protocol.Desc.StartResponse = std::bind(pLockedThis->Desc.StartResponse, std::placeholders::_1, _Addr.Host, std::placeholders::_2);

		if (pLockedThis->Protocol.ProcessSocketReceive(_pData, _SizeData, _pSocket))
		{
			pLockedThis->InitiateReceive(_pSocket);
		}
		else
		{
			pLockedThis->bCloseSocket=true;
		}
	}

	void CallbackHandler::ProcessSocketSend(void* _pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe
	{
		auto pLockedThis = thread_cast<CallbackHandler*>(this); // Safe because we only get called again when we want it to (when doing another async request)

		if (!pLockedThis->Protocol.ProcessSocketSend(_pHeader, _pBody, _SizeData, _pSocket))
		{
			pLockedThis->bCloseSocket=true;
		}
	}
}

const oGUID& oGetGUID(threadsafe const CallbackHandler* threadsafe const *)
{
	// {1C6B1DFE-C1AC-4EFE-B533-3865C0549187}
	static const oGUID oIIDoCallbackHandler =  { 0x1c6b1dfe, 0xc1ac, 0x4efe, { 0xb5, 0x33, 0x38, 0x65, 0xc0, 0x54, 0x91, 0x87 } };
	return oIIDoCallbackHandler;
}

class oHTTPServer_Impl : public oHTTPServer
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oHTTPServer_Impl(const DESC& _Desc, bool *pSuccess);
	void GetDesc(DESC* _pDesc) override { Server.GetDesc(_pDesc); }
	
private:
	oRefCount RefCount;

	oTCPServer<CallbackHandler, DESC> Server;
};

oHTTPServer_Impl::oHTTPServer_Impl(const DESC& _Desc, bool* _pSuccess) : Server(_Desc, _pSuccess) {}

oAPI bool oHTTPServerCreate(const oHTTPServer::DESC& _Desc, oHTTPServer** _ppServer)
{
	bool success = false;
	oCONSTRUCT(_ppServer, oHTTPServer_Impl(_Desc, &success));
	return success;
}
